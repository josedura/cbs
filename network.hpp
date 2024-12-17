/**
 * @file network.hpp
 * @brief Manages network connections.
 * 
 * The network module manages accepting and reading/writing HTTP connections. It uses other modules to parse requests
 * and execute them. After doing so, the results are written back to the HTTP connection that initiated the requests.
 * 
 * Connections are managed through a pool of threads of configurable size.
 * 
 * An atomic boolean flag must be provided to this module to know when the server must finish.
 * 
 * The server blocks on accepting connections (but still manages ongoing connections in parallel threads). Therefore,
 * when an administrator wants to stop the server, a final request must be performed after the stop command to ensure 
 * that the server does not remain blocked while accepting connections.
 */

#ifndef NETWORK_HPP
#define NETWORK_HPP

#include <boost/asio.hpp>

/**
 * @brief Starts the server and manages HTTP connections.
 * 
 * This function initializes the server to listen on the specified address and port. It manages connections 
 * through a thread pool of configurable size. The server continues running until the `stop_server` flag is set 
 * to true, which signals the server to terminate.
 * 
 * @param[in] address The address to bind the server to.
 * @param[in] port The port on which the server listens for incoming connections.
 * @param[in] maxthreads The maximum number of threads to use for handling connections.
 * @param[inout] stop_server A flag to signal when the server should stop. The caller sets this flag to `true` 
 *                           to request server shutdown. The server reads this flag to check for termination.
 */
void run_server(const boost::asio::ip::address &address, unsigned short port, unsigned short maxthreads, 
                std::atomic<bool>& stop_server);

#endif // NETWORK_HPP
