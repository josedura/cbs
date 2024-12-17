#include "httphandler.hpp"

void HttpHandler::read(beast::tcp_stream& stream, beast::flat_buffer& buffer, 
                       http::request<http::string_body>& req) const {
    http::read(stream, buffer, req);
}

void HttpHandler::write(beast::tcp_stream& stream, const http::response<http::string_body>& res) const {
    http::write(stream, res);
}

HttpHandler::~HttpHandler() = default;
