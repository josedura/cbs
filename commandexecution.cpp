#include "commandexecution.hpp"
#include "bookingdata.hpp"
#include "common.hpp"

CommandResult list_movies()
{
    return CommandResult{BookingData::get_instance().get_movies(), CommandStatus::OK};
}

CommandResult list_theaters(unsigned long movie_id)
{
        try {
            return CommandResult{BookingData::get_instance().get_theaters_for_movie(movie_id), CommandStatus::OK};
        }
        catch(...) {
            static const auto invalid = std::make_shared<std::string>("Invalid movieid" CBS_EOL);
            return CommandResult{invalid, CommandStatus::INVALID_REQ};
        }
}

CommandResult list_seats(unsigned long movie_id, unsigned long theater_id)
{
    try {
        return CommandResult{BookingData::get_instance().get_available_seats(movie_id, theater_id), CommandStatus::OK};
    }
    catch(...) {
        static const auto invalid = std::make_shared<std::string>("Invalid combination of movieid and theaterid" CBS_EOL);
        return CommandResult{invalid, CommandStatus::INVALID_REQ};
    }
}

CommandResult book(unsigned long movie_id, unsigned long theater_id, 
                 std::unordered_set<unsigned long> seat_numbers)
{
    try {
        auto booking_result = BookingData::get_instance().book_seats(movie_id, theater_id, std::move(seat_numbers));
        if (booking_result==BookingResult::ACCEPTED) {
            static const auto booking_ok_body = std::make_shared<std::string>("Booking OK" CBS_EOL);
            return CommandResult{booking_ok_body, CommandStatus::OK};
        }
        if (booking_result==BookingResult::NOT_AVAILABLE) {
            static const auto seats_not_available = std::make_shared<std::string>("Seats not available" CBS_EOL);
            return CommandResult{seats_not_available, CommandStatus::REJECTED};
        }
        else {
            static const auto invalid = std::make_shared<std::string>("Invalid movieid, theaterid or seatnumbers" CBS_EOL);
            return CommandResult{invalid, CommandStatus::INVALID_REQ};
        }
    }
    catch(...)
    {
            static const auto invalid = std::make_shared<std::string>("Invalid movieid, theaterid or seatnumbers" CBS_EOL);
            return CommandResult{invalid, CommandStatus::INVALID_REQ};
    }
}
