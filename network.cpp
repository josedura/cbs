/**
 * @file network.cpp
 * @brief Implements server functionality for handling connections with HTTP requests.
 */

#include <iostream>
#include <boost/asio/thread_pool.hpp>
#include <boost/asio/post.hpp>
#include <boost/beast.hpp>

#include "network.hpp"
#include "httphandler.hpp"
#include "requestparser.hpp"

namespace beast = boost::beast;
namespace http = beast::http;
using tcp = boost::asio::ip::tcp;

/**
 * @brief Sends an HTTP reply to the client.
 *
 * @param[in] req The HTTP request received from the client.
 * @param[in] status The HTTP status code to include in the response.
 * @param[in] body Shared pointer to the response body string.
 * @param[in,out] stream TCP stream for communication with the client.
 * @param[in] http_handler HTTP handler used for writing the response.
 */
static void reply(const http::request<http::string_body> req, http::status status,
                  std::shared_ptr<const std::string> body, beast::tcp_stream& stream,
                  const HttpHandler& http_handler)
{
    // Create a response
    http::response<http::string_body> res{status, req.version()};
    res.set(http::field::content_type, "text/plain");
    res.body() = *body;
    res.prepare_payload();

    http_handler.write(stream, res); // Write the response
}

/**
 * @brief Translates application-specific command statuses to HTTP status codes.
 *
 * @param[in] command_status The command status to translate.
 * @return The corresponding HTTP status code.
 */
static http::status translate_command_status(CommandStatus command_status) {
    switch(command_status) {
        case CommandStatus::OK:          return http::status::ok;
        case CommandStatus::REJECTED:    return http::status::forbidden;
        case CommandStatus::INVALID_REQ: return http::status::bad_request;
        default:                         return http::status::bad_request;
    }
}

/**
 * @brief Handles an incoming command by reading the request, executing it, and sending a reply.
 *
 * @param[in,out] stream TCP stream for communication with the client.
 * @param[in] http_handler HTTP handler used for reading and writing data.
 */
static void handle_command(beast::tcp_stream& stream, const HttpHandler& http_handler) {
    try {
        // Read the request
        beast::flat_buffer buffer;
        http::request<http::string_body> req;
        http_handler.read(stream, buffer, req);

        auto parsed_req = parse_request(req.target().to_string()); // Parse request
        auto [body, command_status] = parsed_req->execute(); // Execute request
        
        // Reply
        auto status = translate_command_status(command_status);
        reply(req, status, body, stream, http_handler);
    } catch (std::exception& e) {
        std::cerr << "Error handling command: " << e.what() << std::endl;
    }
}

/**
 * @brief Handles a single client connection by delegating work to a thread pool.
 *
 * @param[in,out] socket The TCP socket connected to the client. This function takes ownership of the socket.
 * @param[in,out] pool Thread pool for executing tasks.
 */
static void handle_connection(tcp::socket&& socket, boost::asio::thread_pool& pool) {
    static HttpHandler http_handler;
    beast::tcp_stream stream(std::move(socket));

    // Post the command handling to the thread pool
    boost::asio::post(pool, [stream = std::move(stream)]() mutable {
        handle_command(stream, http_handler);
    });
}

/**
 * @brief Runs the server to listen for incoming connections and handle requests.
 *
 * @param[in] address The IP address to bind the server to.
 * @param[in] port The port to bind the server to.
 * @param[in] maxthreads The maximum number of threads in the thread pool.
 * @param[in,out] stop_server Atomic boolean to signal the server to stop.
 * @note stop_server is checked after every accepted connection. This function blocks while accepting connections.
 * Therefore it is necesary to send a last request (even if invalid) after the flag to stop is set in order to
 * ensure that the server stops
 */
void run_server(const boost::asio::ip::address &address, unsigned short port, unsigned short maxthreads,
                std::atomic<bool>& stop_server) {
    try {
        // Create a thread pool
        boost::asio::thread_pool pool(maxthreads);

        // Create an I/O context and acceptor
        boost::asio::io_context io_context;
        tcp::acceptor acceptor(io_context, tcp::endpoint(address, port));

        std::cout << "Server is running on port " << port << "..." << std::endl;

        // Accept connections which checking for stop_server flag
        while (!stop_server) {
            tcp::socket socket(io_context);
            acceptor.accept(socket);
            handle_connection(std::move(socket), pool); // Handle each connection in the thread pool
        }

        pool.join(); // Wait for all pending requests to finish
    } catch (std::exception& e) {
        std::cerr << "Error handling connection: " << e.what() << std::endl;
    }
}
