/**
 * @file commandexecution.hpp
 * @brief Contains command functions and related types.
 * 
 * Commands supported are only of request type, but future developments may add administrative or other kind of
 * commands.
 */

#ifndef COMMANDEXECUTION_HPP
#define COMMANDEXECUTION_HPP

#include<string>
#include<memory>
#include<unordered_set>

/**
 * @enum CommandStatus
 * @brief Represents the status of executing a command.
 */
enum CommandStatus {
    OK,           ///< Command was carried out successfully.
    INVALID_REQ,  ///< The command was invalid. This indicates an error in client code.
    REJECTED,     ///< Command was valid but rejected. For example, trying to book an already existing seat.
};

/**
 * @typedef CommandResult
 * @brief Represents the result of executing a command.
 *
 * This type is defined as a `std::tuple` containing two elements:
 * - `std::shared_ptr<const std::string>`: A string to be used as the body of the HTTP response.
 * - `CommandStatus`: A flag indicating the resulting status of executing the command.
 */
using CommandResult = std::tuple<std::shared_ptr<const std::string>, CommandStatus>;

/**
 * @brief Retrieves a list of movies.
 *
 * @return A CommandResult containing the HTTP response body with the list of movies.
 */
CommandResult list_movies();

/**
 * @brief Retrieves a list of theaters showing a specific movie.
 *
 * @param[in] movie_id The ID of the movie.
 * @return A CommandResult which, if the command is succesful, contains the HTTP response body with the list of
 * theaters.
 */
CommandResult list_theaters(unsigned long movie_id);

/**
 * @brief Retrieves a list of seats available in a specific theater for a specific movie.
 *
 * @param[in] movie_id The ID of the movie.
 * @param[in] theater_id The ID of the theater.
 * @return A CommandResult which, if the command is succesful, containins the HTTP response body with the list of
 * available seats.
 */
CommandResult list_seats(unsigned long movie_id, unsigned long theater_id);

/**
 * @brief Attempts to book one or more seats for a specific movie in a specific theater.
 *
 * @param[in] movie_id The ID of the movie.
 * @param[in] theater_id The ID of the theater.
 * @param[in] seat_numbers A set of seat numbers to book.
 * @return A CommandResult with the outcome of the command.
 */
CommandResult book(unsigned long movie_id, unsigned long theater_id,
                   std::unordered_set<unsigned long> seat_numbers);

#endif //COMMANDEXECUTION_HPP
