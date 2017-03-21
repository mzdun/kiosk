#include "kiosk.h"

#include <sys/stat.h>
#include <cstdio>

#include "include/cef_file_util.h"
#include "include/cef_browser.h"
#include "include/cef_callback.h"
#include "include/cef_frame.h"
#include "include/cef_parser.h"
#include "include/cef_resource_handler.h"
#include "include/cef_response.h"
#include "include/cef_request.h"
#include "include/cef_scheme.h"
#include "include/wrapper/cef_helpers.h"
#include "ptr.h"

#if !defined(S_ISREG) || !defined(S_ISDIR)
// Copied from linux libc sys/stat.h:
#define S_ISREG(m) (((m) & S_IFMT) == S_IFREG)
#define S_ISDIR(m) (((m) & S_IFMT) == S_IFDIR)
#endif


#ifdef WIN32
static constexpr wchar_t DIRSEP = '\\';
#else
static constexpr wchar_t DIRSEP = '/';
#endif

static std::string join_path(const std::string& dir) { return dir; }

template <typename String, typename ... Strings>
static std::string join_path(const std::string& dir, String&& subdir, Strings&& ... subdirs) {
	auto out = dir;
	out.push_back(DIRSEP);
	out.append(subdir);
	return join_path(out, std::forward<Strings>(subdirs)...);
}

static inline auto str(cef_string_t const& s)
{
	return CefString(&s).ToString();
}


struct file {
	struct closer {
		void operator()(FILE* f) const noexcept { std::fclose(f); }
	};

	using ptr = std::unique_ptr<FILE, closer>;
	static ptr open(const char* path, const char* mode = "rb") {
		return ptr{ std::fopen(path, mode) };
	}
};

struct always_continue {
	CefRefPtr<CefCallback> cb;
	~always_continue() { if (cb) cb->Continue(); }
};


class KioskSchemeHandler : public CefResourceHandler {
public:
	KioskSchemeHandler(std::shared_ptr<KioskConf> conf)
		: conf_ { conf }
	{}

	bool ProcessRequest(CefRefPtr<CefRequest> request, CefRefPtr<CefCallback> callback) override
	{
		CEF_REQUIRE_IO_THREAD();
		always_continue cont { callback };

		std::string method = request->GetMethod();
		std::string url = request->GetURL();

		DLOG(INFO) << method << " " << url;

		if (method != "GET" && method != "HEAD") {
			return error("text/html", 405, "Method not allowed",
			             "<p>Method <tt>" + method + "</tt> not allowed for <tt>" + url + "</tt>.");
		}

		CefURLParts parts;
		std::string app, path;
		if (!CefParseURL(request->GetURL(), parts)) {
			return error("text/html", 500, "Internal problem",
				"<p>Kiosk scheme handler issue when processing <tt>" + url + "</tt>.");
		}

		url = str(parts.spec);
		app = str(parts.host);
		path = str(parts.path);

		auto it = conf_->apps.find(app);
		if (it == conf_->apps.end()) {
			return error404(url);
		}

		path = path.substr(1);
#ifdef WIN32
		for (auto& c : path)
			if (c == '/') c = '\\';
#endif

		auto resource = join_path(conf_->root, it->second, path);
		DLOG(INFO) << app << " :: " << str(parts.path) << " -> " << resource;

		struct stat st;
		if (stat(resource.c_str(), &st)) {
			return error404(url);
		}


		if (S_ISDIR(st.st_mode)) {
			if (!path.empty() && path.back() != '/') {
				path.push_back('/');
				CefString(&parts.spec).clear();
				CefString(&parts.path).FromString(path);

				CefString new_url;
				if (!CefCreateURL(parts, new_url)) {
					return error("text/html", 500, "Internal problem",
						"<p>Kiosk scheme handler issue when processing directory from <tt>" + url + "</tt>.");
				}
				return redir(new_url.ToString());
			}

			resource.append("index.html");

			if (stat(resource.c_str(), &st) || S_ISDIR(st.st_mode)) {
				return error403(url);
			}
		}

		if (!S_ISREG(st.st_mode))
			return error403(url);

		auto handle = file::open(resource.c_str());
		if (!handle)
			return error403(url);

		handler_ = std::make_unique<file_handler>(
		            std::move(handle), st.st_size,
		            guess_mime(resource));
		return true;
	}

	void GetResponseHeaders(CefRefPtr<CefResponse> response,
	                        int64& response_length,
	                        CefString& redirectUrl) override
	{
		CEF_REQUIRE_IO_THREAD();

		handler_->resp(response, response_length, redirectUrl);
	}

	void Cancel() override { CEF_REQUIRE_IO_THREAD(); }

	bool ReadResponse(void* data_out,
	                  int bytes_to_read,
	                  int& bytes_read,
	                  CefRefPtr<CefCallback> callback) override
	{
		CEF_REQUIRE_IO_THREAD();

		bytes_read = 0;
		return handler_->read(
		            data_out,
		            bytes_to_read,
		            bytes_read,
		            callback);
	}
private:
	struct handler {
		virtual ~handler() noexcept = default;
		virtual void resp(CefRefPtr<CefResponse> response,
		                  int64& response_length,
		                  CefString& redirectUrl) = 0;
		virtual bool read(void* data_out,
		                  int bytes_to_read,
		                  int& bytes_read,
		                  CefRefPtr<CefCallback> callback) = 0;
	};

	struct error_handler : handler {
		int status_;
		std::string::size_type ptr_ = 0;
		std::string mime_;
		std::string status_text_;
		std::string content_;
		std::string redir_;

		error_handler(int status, const char* mime, const char* status_text, const std::string& msg, const std::string& redir)
		: status_{status}
		, mime_{mime}
		, status_text_{status_text}
		, content_{}
		, redir_{redir}
		{
			auto stat = std::to_string(status);
			content_ =
			        "<title>" + stat + " " + status_text + "</title>"
			        "<h1>" + stat + " " + status_text + "</h1>" + msg;
		}

		void resp(CefRefPtr<CefResponse> response, int64& response_length,
		          CefString& redirectUrl) override
		{
			response->SetMimeType(mime_);
			response->SetStatus(status_);
			response->SetStatusText(status_text_);
			if (status_ == 405) {
				CefResponse::HeaderMap headers;
				response->GetHeaderMap(headers);
				headers.insert({ "Allow", "GET, HEAD" });
				response->SetHeaderMap(headers);
			}

			response_length = content_.length();

			if (!redir_.empty())
				redirectUrl.FromString(redir_);
		}

		bool read(void* data_out, int bytes_to_read, int& bytes_read,
		          CefRefPtr<CefCallback> callback) override
		{

			if (content_.length() <= ptr_)
				return false;

			auto bytes_left = content_.length() - ptr_;
			using size_type = decltype(bytes_left);

			if (bytes_to_read < 0)
				bytes_to_read = 0;

			if ((size_type)bytes_to_read > bytes_left)
				bytes_to_read = (int)bytes_left;

			// actual read:
			memcpy(data_out, content_.c_str() + ptr_, bytes_to_read);
			ptr_ += bytes_to_read;
			bytes_read = bytes_to_read;

			callback->Continue();
			return true;
		}
	};

	struct file_handler : handler {
		file::ptr handle_;
		size_t size_;
		std::string mime_;

		file_handler(file::ptr handle, size_t size, const std::string& mime)
		: handle_{std::move(handle)}
		, size_{size}
		, mime_{mime}
		{
		}

		void resp(CefRefPtr<CefResponse> response, int64& response_length,
		          CefString& redirectUrl) override
		{
			response->SetMimeType(mime_);
			response->SetStatus(200);
			response->SetStatusText("OK");
			response_length = size_;
		}
		bool read(void* data_out, int bytes_to_read, int& bytes_read,
		          CefRefPtr<CefCallback> callback) override
		{
			bytes_read = std::fread(data_out, 1, bytes_to_read, handle_.get());
			return !!bytes_read;
		}
	};

	bool error404(const std::string& url)
	{
		return error("text/html", 404, "Not Found",
		      "<p>Address <tt>" + url + "</tt> was not found.");
	}

	bool error403(const std::string& url)
	{
		return error("text/html", 403, "Forbidden",
		      "<p>Address <tt>" + url + "</tt> was not allowed.");
	}

	bool error(const char* mime, int status, const char* status_text, const std::string& msg)
	{
		handler_ = std::make_unique<error_handler>(status, mime, status_text, msg, std::string{});
		return true;
	}

	bool redir(const std::string& redir)
	{
		handler_ = std::make_unique<error_handler>(302, "text/html", "Found", "", redir);
		return true;
	}

	static std::string guess_mime(const std::string& path);

	std::shared_ptr<KioskConf> conf_;
	std::unique_ptr<handler> handler_;

	IMPLEMENT_REFCOUNTING(KioskSchemeHandler);
};

struct mime_mapping {
	const char* ext;
	const char* mime;
};

static constexpr mime_mapping mime_map[] = {
    {".css",  "text/css"},
    {".gif",  "image/gif"},
    {".html", "text/html"},
    {".jpeg", "image/jpeg"},
    {".jpg",  "image/jpeg"},
    {".js",   "application/javascript"},
    {".json", "application/json"},
    {".png",  "image/png"},
    {".svg",  "image/svg+xml"},
    {".xml",  "application/xml"},
};

std::string KioskSchemeHandler::guess_mime(const std::string& path)
{
	auto pos = path.find_last_of(DIRSEP);
	if (pos == std::string::npos)
		pos = 0;

	auto dot = path.find_last_of('.');
	if (dot == std::string::npos)
		dot = 0;

	auto ext = dot > pos ? path.substr(dot) : std::string{};
	const char* mime = nullptr;

	for (auto const& map : mime_map) {
		if (ext == map.ext) {
			// DLOG(INFO) << path << " -> " << ext << " -> " << map.mime;
			mime = map.mime;
			break;
		}
	}
	
	if (mime)
		return mime;

	LOG(INFO) << "Extension mapping missing -> " << ext;
	return "text/plain";
}

class KioskSchemeHandlerFactory : public CefSchemeHandlerFactory {
public:
	KioskSchemeHandlerFactory(std::shared_ptr<KioskConf> conf) : m_conf { conf }
	{}

	// Return a new scheme handler instance to handle the request.
	CefRefPtr<CefResourceHandler> Create(CefRefPtr<CefBrowser> browser,
	                                     CefRefPtr<CefFrame> frame,
	                                     const CefString& scheme_name,
	                                     CefRefPtr<CefRequest> request) override
	{
		CEF_REQUIRE_IO_THREAD();
		return CefPtr<KioskSchemeHandler>(m_conf);
	}

private:
	std::shared_ptr<KioskConf> m_conf;
	IMPLEMENT_REFCOUNTING(KioskSchemeHandlerFactory);
};

void RegisterKioskScheme(CefRawPtr<CefSchemeRegistrar> registrar)
{
	registrar->AddCustomScheme("kiosk",
		true,  // is_standard
		false, // is_local -> XHR rules are stricter for local schemes
		true,  // is_display_isolated
		false, // is_secure
		true); // is_cors_enabled -> "This value should be true in most cases where |is_standard| is true."
}

bool RegisterKioskApps(KioskConf conf)
{
	for (auto const& pair : conf.apps) {
		auto const& name = std::get<0>(pair);
		auto const& dir = std::get<1>(pair);
		auto app_root = join_path(conf.root, dir);
		LOG(INFO) << "kiosk://" << name << "/ -> " << app_root;
	}

	auto pconf = std::make_shared<KioskConf>(std::move(conf));
	auto factory = CefPtr<KioskSchemeHandlerFactory>(pconf);
	return CefRegisterSchemeHandlerFactory("kiosk", {}, factory);
}

