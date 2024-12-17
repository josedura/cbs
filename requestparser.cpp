/**
 * @file requestparser.cpp
 * @brief Parses API requests.
 */

#include <string>
#include <memory>
#include <regex>
#include <sstream>

#include "requestparser.hpp"

/**
 * @brief Regular expression for parsing book requests.
 */
const static std::regex REGEX_BOOK(R"(^\/api\/book_(\d+)_(\d+)_(\d+(_\d+){0,19})$)");

/**
 * @brief Parses an "/api/book" request and generates a corresponding Request object.
 * 
 * @param[in] match A regular expression match object containing parsed components of the request.
 * @return A shared pointer to a Request object representing the parsed request.
 * Returns RequestInvalid if the input contains duplicate seat numbers or is otherwise invalid.
 * Returns RequestBook if the input is valid.
 */
static std::shared_ptr<Request> parse_request_book(const std::smatch &match)
{
    auto movie_id = std::stoul(match[1]);
    auto theater_id = std::stoul(match[2]);
    std::istringstream stream(match[3]);
    std::unordered_set<unsigned long> seat_numbers;
    std::string seat_str;

    // C++ regexes do not support repeated captures, hence we need to parse the last group manually.
    while(std::getline(stream, seat_str, '_')) {
        auto result = seat_numbers.insert(std::stoul(seat_str));
        if (!result.second) { // Do not accept duplicates
            return std::make_shared<RequestInvalid>();
        }
    }

    return std::make_shared<RequestBook>(movie_id, theater_id, std::move(seat_numbers));
}

/**
 * @brief Parses an API request string and generates a corresponding Request object.
 * @param[in] input The API request string to parse.
 * @return A shared pointer to a Request object representing the parsed request. 
 *         Returns RequestInvalid if the input does not match any recognized pattern or contains errors.
 * @see \ref doxygen_files/client_api.md "Client API Documentation".
 */
std::shared_ptr<Request> parse_request(const std::string& input) {
    std::smatch match;
    const static std::regex regex_theaters(R"(^\/api\/listtheaters_(\d+)$)");
    const static std::regex regex_seats(R"(^\/api\/listseats_(\d+)_(\d+)$)");

    try {
        if (input == "/api/listmovies") {
            return std::make_shared<RequestListMovies>();
        }
        else if (std::regex_match(input, match, regex_theaters)) {
            return std::make_shared<RequestListTheaters>(std::stoul(match[1].str()));
        }
        else if (std::regex_match(input, match, regex_seats)) {
            return std::make_shared<RequestListSeats>(std::stoul(match[1].str()),
                                                      std::stoul(match[2].str()));
        }
        else if (std::regex_match(input, match, REGEX_BOOK)) {
            return parse_request_book(match);
        }
        else {
            return std::make_shared<RequestInvalid>();
        }
    } catch (...) {
        return std::make_shared<RequestInvalid>();
    }
}
