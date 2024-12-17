/**
 * \file request.cpp
 * \brief Implementation of subclasses of class Request.
 */

#include "request.hpp"

#include <unordered_set>

#include "common.hpp"
#include "commandexecution.hpp"

Request::~Request() = default;

RequestListMovies::RequestListMovies()
{}

Request::Type RequestListMovies::get_type() const { 
    return LIST_MOVIES;
}

CommandResult RequestListMovies::execute() const {
    return list_movies();
}

RequestListTheaters::RequestListTheaters(unsigned long movie_id)
    :movie_id(movie_id)
{}

Request::Type RequestListTheaters::get_type() const {
    return LIST_THEATERS;
}
CommandResult RequestListTheaters::execute() const {
    return list_theaters(movie_id);
}

RequestListSeats::RequestListSeats(unsigned long movie_id, unsigned long theater_id)
    :movie_id(movie_id)
    ,theater_id(theater_id)
{}

Request::Type RequestListSeats::get_type() const {
    return LIST_SEATS;
}

CommandResult RequestListSeats::execute() const {
    return list_seats(movie_id, theater_id);
}

RequestBook::RequestBook(unsigned long movie_id, unsigned long theater_id,
                         std::unordered_set<unsigned long> seats_nums)
        :movie_id(movie_id)
        ,theater_id(theater_id)
        ,seat_nums(std::move(seats_nums))
{}
    
Request::Type RequestBook::get_type() const {
    return BOOK;
}

CommandResult RequestBook::execute() const {
    return book(movie_id, theater_id, seat_nums);
}

RequestInvalid::RequestInvalid()
{}

Request::Type RequestInvalid::get_type() const {
    return INVALID;
}

CommandResult RequestInvalid::execute() const {
    static const auto invalid = std::make_shared<std::string>("Invalid request" CBS_EOL);
    return CommandResult{invalid, CommandStatus::INVALID_REQ};
}
