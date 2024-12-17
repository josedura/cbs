#include "gtest/gtest.h"
#include "gmock/gmock.h"
#include <unordered_set>
#include <tuple>
#include <memory>
#include <string>
#include <commandexecution.cpp>

// Mocking BookingData for testing purposes using Google Mock
class MockBookingData : public BookingData {
public:
    MOCK_METHOD(std::shared_ptr<const std::string>, get_movies, (), (const, override));
    MOCK_METHOD(std::shared_ptr<const std::string>, get_theaters_for_movie, (unsigned long movie_id), (const, override));
    MOCK_METHOD(std::shared_ptr<const std::string>, get_available_seats, (unsigned long movie_id, unsigned long theater_id), (const, override));
    MOCK_METHOD(BookingResult, book_seats, (unsigned long movie_id, unsigned long theater_id, std::unordered_set<unsigned long> seat_numbers), (override));
    MOCK_METHOD(void, clear, (), (override));
    MOCK_METHOD(std::set<MovieId>, get_sorted_movie_ids, (), (const, override));
    MOCK_METHOD(std::set<MovieId>, get_sorted_theater_ids, (), (const, override));
    MOCK_METHOD(void, add_movies, (std::unordered_set<std::string> movies), (override));
    MOCK_METHOD(void, add_theaters, (std::unordered_set<std::string> theaters), (override));
    MOCK_METHOD(void, add_theaters_to_movie, (MovieId movie_id, std::unordered_set<TheaterId> theaters), (override));

    static std::shared_ptr<MockBookingData> instance;
};
std::shared_ptr<MockBookingData> MockBookingData::instance;

// Empty implementation of BookingData. Except get_instance which returns the mocked one

BookingData::BookingData() = default;

BookingData::~BookingData() = default;

BookingData& BookingData::get_instance() {
    return *MockBookingData::instance;
}

std::shared_ptr<const std::string> BookingData::get_movies() const {
    return {};
}

std::set<MovieId> BookingData::get_sorted_movie_ids() const {
    return {};
}

std::shared_ptr<const std::string> BookingData::get_theaters_for_movie(MovieId movie_id) const {
    (void) movie_id;
    return {};
}

std::set<MovieId> BookingData::get_sorted_theater_ids() const {
    return {};
}

std::shared_ptr<const std::string> BookingData::get_available_seats(MovieId movie_id, TheaterId theater_id) const {
    (void) movie_id;
    (void) theater_id;
    return {};
}

BookingResult BookingData::book_seats(MovieId movie_id, TheaterId theater_id, std::unordered_set<SeatId> seats) {
    (void) movie_id;
    (void) theater_id;
    (void) seats;
    return {};
}

void BookingData::add_movies(std::unordered_set<std::string> movies) {
    (void) movies;
}

void BookingData::add_theaters(std::unordered_set<std::string> theaters) {
    (void) theaters;
}

void BookingData::add_theaters_to_movie(MovieId movie_id, std::unordered_set<TheaterId> theaters) {
    (void) movie_id;
    (void) theaters;
}

void BookingData::rebuild_cache(MovieId movie_id) {
    (void) movie_id;
}

void BookingData::rebuild_cache() {
}

void BookingData::clear() {
}

// Setting up tests
class CommandExecutionTest : public ::testing::Test {
protected:
    std::shared_ptr<MockBookingData> mock_booking_data;

    void SetUp() override {
        mock_booking_data = std::make_shared<MockBookingData>();
        MockBookingData::instance = mock_booking_data;
    }

    void TearDown() override {
        MockBookingData::instance.reset();
    }
};

TEST_F(CommandExecutionTest, ListMovies_Success) {
    EXPECT_CALL(*mock_booking_data, get_movies())
        .WillOnce(testing::Return(std::make_shared<std::string>("Movie List")));

    auto [response, status] = list_movies();

    EXPECT_EQ(*response, "Movie List");
    EXPECT_EQ(status, CommandStatus::OK);
}

TEST_F(CommandExecutionTest, ListTheaters_Success) {
    EXPECT_CALL(*mock_booking_data, get_theaters_for_movie(1))
        .WillOnce(testing::Return(std::make_shared<std::string>("Theater List")));

    auto [response, status] = list_theaters(1);

    EXPECT_EQ(*response, "Theater List");
    EXPECT_EQ(status, CommandStatus::OK);
}

TEST_F(CommandExecutionTest, ListTheaters_InvalidMovieId) {
    EXPECT_CALL(*mock_booking_data, get_theaters_for_movie(0))
        .WillOnce(testing::Throw(std::invalid_argument("Invalid movie_id")));

    auto [response, status] = list_theaters(0);

    EXPECT_EQ(*response, "Invalid movieid\r\n");
    EXPECT_EQ(status, CommandStatus::INVALID_REQ);
}

TEST_F(CommandExecutionTest, ListSeats_Success) {
    EXPECT_CALL(*mock_booking_data, get_available_seats(1, 1))
        .WillOnce(testing::Return(std::make_shared<std::string>("Seat List")));

    auto [response, status] = list_seats(1, 1);

    EXPECT_EQ(*response, "Seat List");
    EXPECT_EQ(status, CommandStatus::OK);
}

TEST_F(CommandExecutionTest, ListSeats_InvalidCombination) {
    EXPECT_CALL(*mock_booking_data, get_available_seats(0, 1))
        .WillOnce(testing::Throw(std::invalid_argument("Invalid combination of movieid and theaterid")));

    auto [response, status] = list_seats(0, 1);

    EXPECT_EQ(*response, "Invalid combination of movieid and theaterid\r\n");
    EXPECT_EQ(status, CommandStatus::INVALID_REQ);
}

TEST_F(CommandExecutionTest, BookSeats_Success) {
    EXPECT_CALL(*mock_booking_data, book_seats(1, 1, testing::_))
        .WillOnce(testing::Return(BookingResult::ACCEPTED));

    auto [response, status] = book(1, 1, {1, 2, 3});

    EXPECT_EQ(*response, "Booking OK\r\n");
    EXPECT_EQ(status, CommandStatus::OK);
}

TEST_F(CommandExecutionTest, BookSeats_InvalidInput) {
    EXPECT_CALL(*mock_booking_data, book_seats(0, 1, testing::_))
        .WillOnce(testing::Return(BookingResult::INVALID));

    auto [response, status] = book(0, 1, {1, 2});

    EXPECT_EQ(*response, "Invalid movieid, theaterid or seatnumbers\r\n");
    EXPECT_EQ(status, CommandStatus::INVALID_REQ);
}

TEST_F(CommandExecutionTest, BookSeats_NotAvailable) {
    EXPECT_CALL(*mock_booking_data, book_seats(0, 1, testing::_))
        .WillOnce(testing::Return(BookingResult::NOT_AVAILABLE));

    auto [response, status] = book(0, 1, {1, 2});

    EXPECT_EQ(*response, "Seats not available\r\n");
    EXPECT_EQ(status, CommandStatus::REJECTED);
}

TEST_F(CommandExecutionTest, BookSeats_ExceptionThrown) {
    EXPECT_CALL(*mock_booking_data, book_seats(0, 1, testing::_))
        .WillOnce(testing::Throw(std::runtime_error("Exception thrown in book_seats")));

    auto [response, status] = book(0, 1, {1, 2});

    EXPECT_EQ(*response, "Invalid movieid, theaterid or seatnumbers\r\n");
    EXPECT_EQ(status, CommandStatus::INVALID_REQ);
}
