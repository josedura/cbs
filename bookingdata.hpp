/**
 * @file bookingdata.hpp
 * @brief Defines the singleton BookingData class for managing cinema booking commands. And related types.
 */

#ifndef BOOKINGDATA_HPP
#define BOOKINGDATA_HPP

#include <mutex>
#include <shared_mutex>
#include <memory>
#include <sstream>
#include <set>

#include "stringidmap.hpp"
#include "common.hpp"

/**
 * @brief Constant defining the number of seats per room.
 */
constexpr size_t SEATS_PER_ROOM = 20;

/**
 * @typedef SeatId
 * @brief Represents the ID of a seat.
 */
using SeatId = size_t;

/**
 * @typedef MovieId
 * @brief Represents the ID of a movie.
 */
using MovieId = unsigned long;

/**
 * @typedef TheaterId
 * @brief Represents the ID of a theater.
 */
using TheaterId = unsigned long;

/**
 * @enum BookingResult
 * @brief Represents the possible outcomes of a booking command.
 */
enum BookingResult {
    ACCEPTED,       /**< The seats have been booked correctly. */
    NOT_AVAILABLE,   /**< At least one of the requested seats is already booked. */
    INVALID         /**< At least one of the requested seats is not valid (equal or greater than SEATS_PER_ROOM). */
};

class CinemaRoom;

/**
 * @class BookingData
 * @brief Singleton class that manages booking data for movies and theaters.
 * 
 * All methods are thread-safe.
 * 
 * Reading operations can happen concurrently and do not block each other.
 * 
 * Write operations will happen atomically and block and are blocked by other read or write operations.
 * 
 * The book_seats method is a write operation but it is special. It will execute in a thread-safe way concurrently
 * with other read operations and with other book_seats invocations for different combinations of movie and theater.
 * But it will not execute concurrently with other write operations neither with invocations of book_seats for the
 * same movie and theater.
 */
class BookingData {
protected:
    /**
     * @brief Protected constructor to enforce singleton pattern but allow unit testing.
     */
    BookingData();

    /**
     * @brief Deleted copy constructor.
     */
    BookingData(const BookingData&) = delete;

    /**
     * @brief Deleted copy assignment operator.
     */
    BookingData& operator=(const BookingData&) = delete;

    virtual ~BookingData();

public:
    /**
     * @brief Gets the singleton instance of the BookingData class.
     * @return Reference to the singleton BookingData instance.
     */
    static BookingData& get_instance();

    /**
     * @brief Empties the contents of class
     */
    virtual void clear();

    /**
     * @brief Gets the list of movies
     * 
     * The returned string contains one line per movie, using "\r\n" as End Of Line.
     * Each line has this format:
     * 
     * movie_id,title
     * 
     * movie_id is a non-negative integer which is unique per movie
     * 
     * title is the title of the movie. May contain extra commas and symbols, cannot contain "\r" or "\n".
     * 
     * @return Shared pointer to a string of movies.
     * @note This method is cached, so it is fast to execute.
     */
    virtual std::shared_ptr<const std::string> get_movies() const;

    /**
     * @brief Gets a sorted set of all movie IDs.
     * @return Sorted set of movie IDs.
     * @note This method is not cached and is therefore expensive to execute.
     */
    virtual std::set<MovieId> get_sorted_movie_ids() const;

    /**
     * @brief Gets a string of all theaters for a specific movie.
     * 
     * The returned string contains one line per movie, using "\r\n" as End Of Line.
     * Each line has this format:
     * 
     * theater_id,theater_name
     * 
     * theater_id is a non-negative integer which is unique per theater.
     * 
     * theater_name is the name of the theater. May contain extra commas and symbol, cannot contain "\r" or "\n"
     * 
     * @param[in] movie_id The ID of the movie.
     * @return Shared pointer to a string of theaters for the given movie.
     * @note This method is cached, so it is fast to execute.
     */
    virtual std::shared_ptr<const std::string> get_theaters_for_movie(MovieId movie_id) const;

    /**
     * @brief Gets a sorted set of all theater IDs.
     * @return Sorted set of theater IDs.
     * @note This method is not cached and is therefore expensive to execute.
     */
    virtual std::set<MovieId> get_sorted_theater_ids() const;

    /**
     * @brief Gets a string with the list of available seats for a specific movie and theater.
     * 
     * The returned string contains a single line which ends with "\r\n" as End Of Line (EOL).
     * 
     * The line contains a comma separated list of non-negative integers. Each integer represents an available seat.
     * If no seat is available for that combination of theater and movie, the line will contain only the EOL
     * 
     * @param[in] movie_id The ID of the movie.
     * @param[in] theater_id The ID of the theater.
     * @return Shared pointer to a string of available seats.
     * @note This method is cached, so it is fast to execute.
     */
    virtual std::shared_ptr<const std::string> get_available_seats(MovieId movie_id, TheaterId theater_id) const;

    /**
     * @brief Books seats for a specific movie and theater.
     * 
     * The seats are booked if and only if this method returns \ref ACCEPTED. In that case all requested
     * seats have been succesfully booked.
     * Otherwise no seat is booked.
     * 
     * @param[in] movie_id The ID of the movie.
     * @param[in] theater_id The ID of the theater.
     * @param[in] seats The set of seat IDs to book.
     * @return The result of the booking operation.
     */
    virtual BookingResult book_seats(MovieId movie_id, TheaterId theater_id, std::unordered_set<SeatId> seats);

    /**
     * @brief Adds movies.
     * @param[in] movies A set of movie titles to add.
     * @throw std::exception if movies contains a movie title which is already present in BookingData.
     * @note Guarantees strong exception safety.
     */
    virtual void add_movies(std::unordered_set<std::string> movies);

    /**
     * @brief Adds theaters.
     * @param[in] theaters A set of theater names to add.
     * @throw std::exception if theaters contains a theater name which is already present in BookingData.
     * @note Guarantees strong exception safety.
     */
    virtual void add_theaters(std::unordered_set<std::string> theaters);

    /**
     * @brief Associates a set of theaters with a specific movie. 
     * 
     * Creates 20 available seats in each given theater for the given movie.
     * 
     * @param[in] movie_id The ID of the movie.
     * @param[in] theaters A set of theater IDs to associate with the movie.
     * @throw std::exception if theaters contains a theater id which is already associated with the given movie.
     * @note Guarantees strong exception safety.
     */
    virtual void add_theaters_to_movie(MovieId movie_id, std::unordered_set<TheaterId> theaters);

private:
    /**
     * @brief Mutex for ensuring thread safety.
     */
    mutable std::shared_mutex mutex;

    /**
     * @brief List of movies managed by the system.
     */
    StringIdMap movie_list;

    /**
     * @brief List of theaters managed by the system.
     */
    StringIdMap theater_list;

    /**
     * @brief Maps movies and theaters to their respective CinemaRoom instances.
     */
    std::unordered_map<MovieId, std::unordered_map<TheaterId, std::shared_ptr<CinemaRoom>>> rooms;

    /**
     * @brief Cache for theaters associated with each movie.
     */
    std::unordered_map<MovieId, std::shared_ptr<std::string>> theaters_per_movie_cache;

    /**
     * @brief Rebuilds the theaters_per_movie_cache for a specific movie.
     * @param[in] movie_id The ID of the movie.
     */
    void rebuild_cache(MovieId movie_id);

    /**
     * @brief Rebuilds the entire theaters_per_movie_cache for all movies.
     */
    void rebuild_cache();
};

#endif // BOOKINGDATA_HPP
