/**
 * \file bookingdata.cpp
 * \brief Classes and methods responsible for handling booking data.
 *
 * The BookingData and CinemaRoom classes manage the booking of seats in cinemas, handle movie and theater information, 
 * and support multi-threaded access to the data.
 * 
 * All operations are thead-safe. Reading can be concurrent, writing is exclusive.
 * 
 * Except for writing to booked seats. Seats for each movie,theater pair are represented by an object of CinemaRoom
 * class.
 * It is possible to write concurrently to different CinemaRoom objects and also concurrent to reads of BookkingData.
 * Writing to the same CinemaRoom is exclusive.
 * 
 * Reading operations are cached.
 */

#include "bookingdata.hpp"

/**
 * \class CinemaRoom
 * \brief Keeps track of the seats available for a movie,theater pair and allows managing them.
 */
class CinemaRoom {
public:
    /**
     * \brief Constructs a CinemaRoom with all seats available.
     */
    CinemaRoom()
        : seat_cache{}
        , seat_available{}
    {
        std::fill(seat_available.begin(), seat_available.end(), true);
        rebuild_cache();
    }

    /**
     * \brief Retrieves the current available seats.
     * 
     * \return A shared pointer to a constant string representing the available seats.
     */
    std::shared_ptr<const std::string> get_seats() const  {
        std::shared_lock lock(mutex); // Shared lock for reading
        return seat_cache;
    }

    /**
     * \brief Books the specified seats if they are available.
     *
     * \param[in] seats_to_book A set of seat IDs to book.
     * 
     * \return The result of the booking attempt.
     * - INVALID if any seat is invalid or tries to book same seat twice.
     * - NOT_AVAILABLE if any seat is already taken.
     * - ACCEPTED if the booking is successful.
     */
    BookingResult book_seats(std::unordered_set<SeatId> seats_to_book)
    {
        std::unique_lock lock(mutex); // Exclusive lock for writing
        // Check if all seats are available and valid
        for(auto seat: seats_to_book) {
            if (seat >= seat_available.size()) {
                return INVALID;
            } else if (!seat_available[seat]) {
                return NOT_AVAILABLE;
            }
        }
        // Make the booking
        for(auto seat: seats_to_book) {
            seat_available[seat] = false;
        }

        rebuild_cache();
        return ACCEPTED;
    }

private:
    mutable std::shared_mutex mutex; // Mutex to ensure thread safety

    std::shared_ptr<std::string> seat_cache;
    std::array<bool, SEATS_PER_ROOM> seat_available;

    /**
     * \brief Rebuilds the seat cache based on the available seats.
     * 
     * Must be called with an already held writing lock
     */
    void rebuild_cache() {
        std::stringstream ss;
        bool first = true;
        for(size_t idx = 0; idx < seat_available.size(); ++idx)
        {
            if (seat_available[idx]) {
                if (first) {
                    first = false;
                }
                else {
                    ss << ",";
                }
                ss << idx;
            }
        }
        ss << CBS_EOL;
        seat_cache = std::make_shared<std::string>(ss.str());
    }
};

BookingData::BookingData() {
    rebuild_cache();
}

BookingData::~BookingData() = default;

BookingData& BookingData::get_instance() {
    static BookingData instance;
    return instance;
}

std::shared_ptr<const std::string> BookingData::get_movies() const {
    std::shared_lock lock(mutex); // Shared lock for reading
    return movie_list.get_id_string_list();
}

std::set<MovieId> BookingData::get_sorted_movie_ids() const {
    std::shared_lock lock(mutex); // Shared lock for reading
    return movie_list.get_sorted_keys();
}

std::shared_ptr<const std::string> BookingData::get_theaters_for_movie(MovieId movie_id) const {
    std::shared_lock lock(mutex); // Shared lock for reading
    return theaters_per_movie_cache.at(movie_id);
}

std::set<MovieId> BookingData::get_sorted_theater_ids() const {
    std::shared_lock lock(mutex); // Shared lock for reading
    return theater_list.get_sorted_keys();
}

std::shared_ptr<const std::string> BookingData::get_available_seats(MovieId movie_id, TheaterId theater_id) const {
    std::shared_lock lock(mutex); // Shared lock for reading
    const auto& rooms_for_movie = rooms.at(movie_id);
    const auto& room = rooms_for_movie.at(theater_id);
    return room->get_seats();
}

BookingResult BookingData::book_seats(MovieId movie_id, TheaterId theater_id, std::unordered_set<SeatId> seats) {
    std::shared_lock lock(mutex); // Shared lock for reading
    const auto& rooms_for_movie = rooms.at(movie_id);
    auto& room = rooms_for_movie.at(theater_id);
    return room->book_seats(std::move(seats));
}

void BookingData::add_movies(std::unordered_set<std::string> movies) {
    std::unique_lock lock(mutex); // Exclusive lock for writing
    auto inserted_ids = movie_list.add(std::move(movies));
    for (auto movie_id: inserted_ids) {
        rooms[movie_id] = {};
        rebuild_cache(movie_id);
    }
}

void BookingData::add_theaters(std::unordered_set<std::string> theaters) {
    std::unique_lock lock(mutex); // Exclusive lock for writing
    theater_list.add(std::move(theaters));
}

void BookingData::add_theaters_to_movie(MovieId movie_id, std::unordered_set<TheaterId> theaters) {
    std::unique_lock lock(mutex); // Exclusive lock for writing
    auto& rooms_for_this_movie = rooms.at(movie_id);
    // Verify that no theater is already displaying the movie
    for(auto theater_id: theaters) {
        if (rooms_for_this_movie.find(theater_id) != rooms_for_this_movie.end()) {
            throw std::runtime_error("Theater already displaying the movie");
        }
    }
    // Create rooms
    for(auto theater_id: theaters) {
        auto cinema_room = std::make_shared<CinemaRoom>();
        rooms_for_this_movie.emplace(theater_id, cinema_room);
    }

    rebuild_cache(movie_id);
}

void BookingData::rebuild_cache(MovieId movie_id) {
    std::stringstream ss;
    ss.str("");
    auto movie_title = movie_list.get_string(movie_id);
    auto rooms_for_movie = rooms.at(movie_id);
    for(const auto& [theater_id, room]: rooms_for_movie)
    {
        ss << theater_id << "," << theater_list.get_string(theater_id) << CBS_EOL;
    }
    theaters_per_movie_cache[movie_id] = std::make_shared<std::string>(ss.str());
}

void BookingData::rebuild_cache() {
    theaters_per_movie_cache.clear();
    for(const auto& [movie_id, rooms] :rooms) {
        rebuild_cache(movie_id);
    }
}

void BookingData::clear() {
    std::unique_lock lock(mutex); // Exclusive lock for writing

    movie_list.clear();
    theater_list.clear();
    rooms.clear();
    theaters_per_movie_cache.clear();

    rebuild_cache();
}
