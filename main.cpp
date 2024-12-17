/**
 * @file main.cpp
 * @brief Entry point for the server application.
 *
 * This file contains the `main` function, signal handling and argument parsing.
 * It invokes server initialization and the server itself.
 */

#include <iostream>
#include <string>
#include <csignal>

#include "network.hpp"
#include "initbookingdata.hpp"


/**
 * @brief Global flag to control server shutdown.
 *
 * This atomic boolean is set to `true` when the server receives a SIGTERM signal,
 * indicating that the server should stop running.
 */
static std::atomic<bool> global_stop_server{false};

#ifdef __linux__
/**
 * @brief Signal handler for SIGTERM.
 *
 * Sets the global_stop_server flag to true and logs the signal reception.
 *
 * @param signal The signal number.
 * @note Supported only under linux.
 */
extern "C" void handle_signal(int signal) {
    if (signal == SIGTERM) {
        global_stop_server = true;
        std::cout << "SIGTERM received. Stopping server..." << std::endl;
    }
}
#endif

/**
 * @brief Displays usage information for the server application in stderr.
 */
static void help() {
    std::cerr << "Usage: cbs <address> <port> <numthreads>" << std::endl;
    std::cerr << "    address : Network address the server will bind to in order to receive requests." << std::endl;
    std::cerr << "              No address resolution is performed. So 127.0.0.1 is valid but localhost is not." << std::endl;
    std::cerr << "    port : Port where the server will listen for requests." << std::endl;
    std::cerr << "    numthreads : Max number of simultaneous requests which can be served." << std::endl;
    std::cerr << "Example:" << std::endl;
    std::cerr << "cbs 127.0.0.1 18080 8" << std::endl;
}

/**
 * @brief Parses command-line arguments.
 *
 * Validates and extracts the address, port, and number of threads from the arguments.
 *
 * @param[in] argc Number of command-line arguments.
 * @param[in] argv Array of command-line arguments.
 * @param[out] address Parsed network address.
 * @param[out] port Parsed port number.
 * @param[out] maxthreads Parsed number of threads.
 *
 * @note prints help message and finishes process with EXIT_FAILURE if arguments cannot be parsed.
 *       prints help message and finishes process with EXIT_SUCCESS if a help switch is used.
 *       Otherwise the parsed values are set in the output parameters.
 */
static void parse_arguments(int argc, char *argv[], boost::asio::ip::address &address, 
                            unsigned short &port, unsigned short &maxthreads)
{
    try {
        if (argc==2 && (std::string("-h")==argv[1] || std::string("--help")==argv[1])) {
            help();
            exit(EXIT_SUCCESS);
        }
        if (argc != 4) {
            throw std::exception();
        }
        address = boost::asio::ip::make_address(argv[1]);
        port = static_cast<unsigned short>(std::stoi(argv[2]));
        maxthreads = static_cast<unsigned short>(std::stoi(argv[3]));
        if (maxthreads<=0) {
            throw std::exception();
        }
    } catch(...) {
        std::cerr << "Invalid parameters." << std::endl;
        help();
        exit(EXIT_FAILURE);
    }
}

/**
 * @brief Macro to switch between test and production main functions.
 *
 * If the `UNIT_TESTS` macro is defined, `MAIN` is mapped to `test_main` since the actual main function will
 * be the one provided by unit tests.
 * Otherwise, it defaults to `main` for production builds.
 */
#ifdef UNIT_TESTS
#define MAIN test_main
#else
#define MAIN main
#endif

/**
 * @brief Main function for the server application.
 *
 * parses arguments, initializes the server and starts the server loop.
 *
 * @param argc Number of command-line arguments.
 * @param argv Array of command-line arguments.
 * @return EXIT_SUCCESS on success, EXIT_FAILURE on error.
 */
int MAIN(int argc, char* argv[]) {
    try {
        boost::asio::ip::address address;
        unsigned short port;
        unsigned short maxthreads;

#ifdef __linux__
        std::signal(SIGTERM, handle_signal);
#endif
        parse_arguments(argc, argv, address, port, maxthreads);
        init_booking_data();
        run_server(address, port, maxthreads, global_stop_server);
    } catch (std::exception& e) {
        std::cerr << "Server error: " << e.what() << std::endl;
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
