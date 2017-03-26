#pragma once
#include <string>
std::string response_template(std::string const& title, std::string const& header, std::string const& content);
inline std::string response_template(std::string const& title, std::string const& content) { return response_template(title, title, content); }
