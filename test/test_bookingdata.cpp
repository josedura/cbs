#include <gtest/gtest.h>
#include <unordered_set>
#include <regex>

#include "bookingdata.cpp"

class BookingDataTest : public ::testing::Test {
protected:
    BookingData& booking_data = BookingData::get_instance();

    void SetUp() override {
        // Reset BookingData instance state before each test.
        booking_data.clear();
    }
};

TEST_F(BookingDataTest, AddMoviesSuccessfully) {
    std::unordered_set<std::string> movies = {"Movie A", "Movie B", "Movie C"};
    ASSERT_NO_THROW(booking_data.add_movies(movies));

    auto movie_names = booking_data.get_movies();
    auto movie_ids = booking_data.get_sorted_movie_ids();

    std::regex pattern(R"(^\d+,Movie \w+\r\n\d+,Movie \w+\r\n\d+,Movie \w+\r\n$)");
    ASSERT_TRUE(static_cast<bool>(std::regex_match(*movie_names, pattern)));

    ASSERT_EQ(movie_ids.size(), 3);
}

TEST_F(BookingDataTest, AddDuplicateMoviesThrows) {
    std::unordered_set<std::string> movies = {"Movie A"};
    booking_data.add_movies(movies);

    ASSERT_THROW(booking_data.add_movies(movies), std::exception);
}

TEST_F(BookingDataTest, AddTheatersSuccessfully) {
    std::unordered_set<std::string> theaters = {"Theater 1", "Theater 2"};
    ASSERT_NO_THROW(booking_data.add_theaters(theaters));

    auto theater_ids = booking_data.get_sorted_theater_ids();
    ASSERT_EQ(theater_ids.size(), 2);
}

TEST_F(BookingDataTest, AddTheatersToMovieSuccessfully) {
    std::unordered_set<std::string> movies = {"Movie X"};
    booking_data.add_movies(movies);
    auto movie_ids = booking_data.get_sorted_movie_ids();

    std::unordered_set<std::string> theaters = {"Theater Y"};
    booking_data.add_theaters(theaters);

    auto theater_ids = booking_data.get_sorted_theater_ids();

    ASSERT_NO_THROW(booking_data.add_theaters_to_movie(*movie_ids.begin(), {*theater_ids.begin()}));
}

TEST_F(BookingDataTest, GetAvailableSeatsAfterAssociation) {
    std::unordered_set<std::string> movies = {"Movie X"};
    booking_data.add_movies(movies);
    auto movie_ids = booking_data.get_sorted_movie_ids();

    std::unordered_set<std::string> theaters = {"Theater Y"};
    booking_data.add_theaters(theaters);

    auto theater_ids = booking_data.get_sorted_theater_ids();
    booking_data.add_theaters_to_movie(*movie_ids.begin(), {*theater_ids.begin()});

    auto available_seats = booking_data.get_available_seats(*movie_ids.begin(), *theater_ids.begin());
    ASSERT_NE(available_seats, nullptr);
    ASSERT_EQ(*available_seats, "0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19\r\n");
}

TEST_F(BookingDataTest, BookSeatsSuccessfully) {
    std::unordered_set<std::string> movies = {"Movie X"};
    booking_data.add_movies(movies);
    auto movie_ids = booking_data.get_sorted_movie_ids();

    std::unordered_set<std::string> theaters = {"Theater Y"};
    booking_data.add_theaters(theaters);

    auto theater_ids = booking_data.get_sorted_theater_ids();
    booking_data.add_theaters_to_movie(*movie_ids.begin(), {*theater_ids.begin()});

    std::unordered_set<SeatId> seats_to_book = {0, 1, 2};
    BookingResult result = booking_data.book_seats(*movie_ids.begin(), *theater_ids.begin(), seats_to_book);

    ASSERT_EQ(result, ACCEPTED);

    auto available_seats = booking_data.get_available_seats(*movie_ids.begin(), *theater_ids.begin());
    ASSERT_EQ(*available_seats, "3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19\r\n");
}

TEST_F(BookingDataTest, BookUnavailableSeats) {
    std::unordered_set<std::string> movies = {"Movie X"};
    booking_data.add_movies(movies);
    auto movie_ids = booking_data.get_sorted_movie_ids();

    std::unordered_set<std::string> theaters = {"Theater Y"};
    booking_data.add_theaters(theaters);
    auto theater_ids = booking_data.get_sorted_theater_ids();

    booking_data.add_theaters_to_movie(*movie_ids.begin(), {*theater_ids.begin()});

    std::unordered_set<SeatId> seats_to_book = {0, 1, 2, 3, 4};
    booking_data.book_seats(*movie_ids.begin(), *theater_ids.begin(), seats_to_book);

    std::unordered_set<SeatId> unavailable_seats = {3, 4};
    BookingResult result = booking_data.book_seats(*movie_ids.begin(), *theater_ids.begin(), unavailable_seats);
    ASSERT_EQ(result, NOT_AVAILABLE);
}

TEST_F(BookingDataTest, BookInvalidSeats) {
    std::unordered_set<std::string> movies = {"Movie X"};
    booking_data.add_movies(movies);
    auto movie_ids = booking_data.get_sorted_movie_ids();

    std::unordered_set<std::string> theaters = {"Theater Y"};
    booking_data.add_theaters(theaters);
    auto theater_ids = booking_data.get_sorted_theater_ids();

    booking_data.add_theaters_to_movie(*movie_ids.begin(), {*theater_ids.begin()});

    std::unordered_set<SeatId> seats_to_book = {25, 26};
    BookingResult result = booking_data.book_seats(*movie_ids.begin(), *theater_ids.begin(), seats_to_book);
    ASSERT_EQ(result, INVALID);
}
