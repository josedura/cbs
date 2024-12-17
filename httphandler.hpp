/**
 * @file httphandler.hpp
 * @brief Defines the HttpHandler class for managing HTTP connections using Boost.Beast.
 *
 * This class provides an interface for reading from and writing to HTTP connections.
 * It is designed to facilitate unit testing by allowing the injection of a mocked handler,
 * since mocking Boost.Beast components can be complex.
 */

#ifndef HTTPHANDLER_HPP
#define HTTPHANDLER_HPP

#include <boost/beast.hpp>

namespace beast = boost::beast;
namespace http = beast::http;

/**
 * @class HttpHandler
 * @brief Class for handling HTTP read and write operations.
 *
 * The HttpHandler class defines virtual methods for reading HTTP requests
 * and writing HTTP responses over a TCP stream.
 */
class HttpHandler {
public:
    /**
     * @brief Reads an HTTP request from the specified TCP stream.
     *
     * @param[inout] stream The TCP stream to read from.
     * @param[inout] buffer A buffer used for temporary storage during reading.
     * @param[out] req The HTTP request object to populate with the read data.
     */
    virtual void read(beast::tcp_stream& stream, beast::flat_buffer& buffer,
                      http::request<http::string_body>& req) const;

    /**
     * @brief Writes an HTTP response to the specified TCP stream.
     *
     * @param[inout] stream The TCP stream to write to.
     * @param[in] res The HTTP response to send.
     */
    virtual void write(beast::tcp_stream& stream, const http::response<http::string_body>& res) const;

    /**
     * @brief Virtual destructor to ensure proper cleanup of derived classes.
     */
    virtual ~HttpHandler();
};

#endif // HTTPHANDLER_HPP
