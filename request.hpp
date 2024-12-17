/**
 * @file request.hpp
 * @brief Defines the Request base class and its derived classes for handling different types of requests.
 */

#ifndef REQUEST_HPP
#define REQUEST_HPP

#include <unordered_set>

#include "common.hpp"
#include "commandexecution.hpp"

/**
 * @class Request
 * @brief Abstract base class representing a generic request.
 */
class Request {
public:
    /**
     * @enum Type
     * @brief Enum representing the type of request.
     */
    enum Type
    {
        LIST_MOVIES,     /**< List movies request. */
        LIST_THEATERS,   /**< List theaters request. */
        LIST_SEATS,      /**< List seats request. */
        BOOK,            /**< Book seats request. */
        INVALID          /**< Invalid request. */
    };

    /**
     * @brief Get the type of the request.
     * @return The type of the request.
     */
    virtual Type get_type() const = 0;

    /**
     * @brief Execute the request.
     * @return The result of the command execution.
     */
    virtual CommandResult execute() const = 0;

    /**
     * @brief Virtual destructor.
     */
    virtual ~Request();
};

/**
 * @class RequestListMovies
 * @brief Handles requests to list all movies.
 */
class RequestListMovies : public Request
{
public:
    /**
     * @brief Constructor for RequestListMovies.
     */
    RequestListMovies();

    /**
     * @copydoc Request::get_type
     */
    Type get_type() const override;

    /**
     * @copydoc Request::execute
     */
    CommandResult execute() const override;
};

/**
 * @class RequestListTheaters
 * @brief Handles requests to list theaters for a specific movie.
 */
class RequestListTheaters : public Request
{
public:
    /**
     * @brief Constructor for RequestListTheaters.
     * @param[in] movie_id The ID of the movie.
     */
    RequestListTheaters(unsigned long movie_id);

    /**
     * @copydoc Request::get_type
     */
    Type get_type() const override;

    /**
     * @copydoc Request::execute
     */
    CommandResult execute() const override;

    const unsigned long movie_id; /**< The ID of the movie. */
};

/**
 * @class RequestListSeats
 * @brief Handles requests to list available seats for a specific movie in a theater.
 */
class RequestListSeats : public Request
{
public:
    /**
     * @brief Constructor for RequestListSeats.
     * @param[in] movie_id The ID of the movie.
     * @param[in] theater_id The ID of the theater.
     */
    RequestListSeats(unsigned long movie_id, unsigned long theater_id);

    /**
     * @copydoc Request::get_type
     */
    Type get_type() const override;

    /**
     * @copydoc Request::execute
     */
    CommandResult execute() const override;

    const unsigned long movie_id; /**< The ID of the movie. */
    const unsigned long theater_id; /**< The ID of the theater. */
};

/**
 * @class RequestBook
 * @brief Handles requests to book seats for a specific movie in a theater.
 */
class RequestBook : public Request
{
public:
    /**
     * @brief Constructor for RequestBook.
     * @param[in] movie_id The ID of the movie.
     * @param[in] theater_id The ID of the theater.
     * @param[in] seats_nums The set of seat numbers to book.
     */
    RequestBook(unsigned long movie_id, unsigned long theater_id,
                std::unordered_set<unsigned long> seats_nums);

    /**
     * @copydoc Request::get_type
     */
    Type get_type() const override;

    /**
     * @copydoc Request::execute
     */
    CommandResult execute() const override;

    const unsigned long movie_id; /**< The ID of the movie. */
    const unsigned long theater_id; /**< The ID of the theater. */
    const std::unordered_set<unsigned long> seat_nums; /**< The set of seat numbers. */
};

/**
 * @class RequestInvalid
 * @brief Handles invalid requests.
 */
class RequestInvalid : public Request
{
public:
    /**
     * @brief Constructor for RequestInvalid.
     */
    RequestInvalid();

    /**
     * @copydoc Request::get_type
     */
    Type get_type() const override;

    /**
     * @copydoc Request::execute
     */
    CommandResult execute() const override;
};

#endif //REQUEST_HPP
