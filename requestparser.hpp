/**
 * @file requestparser.hpp
 * @brief Module for parsing HTTP requests.
 */

#ifndef REQUESTPARSER_HPP
#define REQUESTPARSER_HPP

#include<string>
#include<memory>

#include "request.hpp"

    /**
     * @brief Parses an HTTP requests.
     * @param[in] input The full HTTP endpoint to parse.
     * @see \ref doxygen_files/client_api.md "Client API Documentation".
     * @return A shared pointer to a class deriving from Request which holds the parsed requests.
     */
std::shared_ptr<Request> parse_request(const std::string& input);

#endif //REQUESTPARSER_HPP