#include "template.h"
#include "template_contents.h"

constexpr size_t byte_length(const char* buffer) {
	return *buffer ? 1 + byte_length(buffer + 1) : 0;
}

constexpr size_t length(const char* buffer) {
	return buffer ? byte_length(buffer) : 0;
}

std::string response_template(std::string const& title, std::string const& header, std::string const& content)
{
	size_t length = 0;
	for (auto const& block : tmplt_contents) {
		switch (block.kind) {
		case tmplt::text: length += ::length(block.value); break;
		case tmplt::title: length += title.length(); break;
		case tmplt::header: length += header.length(); break;
		case tmplt::content: length += content.length(); break;
		}
	}
	std::string out;
	out.reserve(length + 1);
	for (auto const& block : tmplt_contents) {
		switch (block.kind) {
		case tmplt::text: out += block.value; break;
		case tmplt::title: out += title; break;
		case tmplt::header: out += header; break;
		case tmplt::content: out += content; break;
		}
	}
	return out;
}
