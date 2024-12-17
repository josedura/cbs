#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include <memory>

#include "requestparser.cpp"

class MockRequest : public Request {
public:
    MOCK_METHOD(Type, get_type, (), (const, override));
    MOCK_METHOD(CommandResult, execute, (), (const, override));
};

class MockRequestListMovies : public RequestListMovies {
public:
    MOCK_METHOD(Type, get_type, (), (const, override));
    MOCK_METHOD(CommandResult, execute, (), (const, override));
};

class MockRequestListTheaters : public RequestListTheaters {
public:
    MockRequestListTheaters() : RequestListTheaters(0) {}
    MockRequestListTheaters(unsigned long movie_id) : RequestListTheaters(movie_id) {}

    MOCK_METHOD(Type, get_type, (), (const, override));
    MOCK_METHOD(CommandResult, execute, (), (const, override));
};

class MockRequestListSeats : public RequestListSeats {
public:
    MockRequestListSeats()
        : RequestListSeats(0, 0) {}

    MOCK_METHOD(Type, get_type, (), (const, override));
    MOCK_METHOD(CommandResult, execute, (), (const, override));
};

class MockRequestBook : public RequestBook {
public:
    MockRequestBook()
        : RequestBook(0, 0, {}) {}

    MOCK_METHOD(Type, get_type, (), (const, override));
    MOCK_METHOD(CommandResult, execute, (), (const, override));
};

class MockRequestInvalid : public RequestInvalid {
public:
    MOCK_METHOD(Type, get_type, (), (const, override));
    MOCK_METHOD(CommandResult, execute, (), (const, override));
};

// Empty Request Implementation
Request::~Request() = default;

RequestListMovies::RequestListMovies()
{}

Request::Type RequestListMovies::get_type() const { 
    return LIST_MOVIES;
}

CommandResult RequestListMovies::execute() const {
    return CommandResult{std::make_shared<std::string>(""), CommandStatus::INVALID_REQ};
}

RequestListTheaters::RequestListTheaters(unsigned long movie_id)
    :movie_id(movie_id)
{}

Request::Type RequestListTheaters::get_type() const {
    return LIST_THEATERS;
}
CommandResult RequestListTheaters::execute() const {
    return CommandResult{std::make_shared<std::string>(""), CommandStatus::INVALID_REQ};
}

RequestListSeats::RequestListSeats(unsigned long movie_id, unsigned long theater_id)
    :movie_id(movie_id)
    ,theater_id(theater_id)
{}

Request::Type RequestListSeats::get_type() const {
    return LIST_SEATS;
}

CommandResult RequestListSeats::execute() const {
    return CommandResult{std::make_shared<std::string>(""), CommandStatus::INVALID_REQ};
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
    return CommandResult{std::make_shared<std::string>(""), CommandStatus::INVALID_REQ};
}

RequestInvalid::RequestInvalid()
{}

Request::Type RequestInvalid::get_type() const {
    return INVALID;
}

CommandResult RequestInvalid::execute() const {
    return CommandResult{std::make_shared<std::string>(""), CommandStatus::INVALID_REQ};
}


class RequestParserTest : public ::testing::Test {
protected:
    // You can add common setup or teardown code here
    void SetUp() override {
    }
    void TearDown() override {
    }

    MockRequestListMovies mock_list_movies;
    MockRequestListTheaters mock_list_theaters;
    MockRequestListSeats mock_list_seats;
    MockRequestBook mock_request_book;
    MockRequestInvalid mock_request_invalid;
};

TEST_F(RequestParserTest, ParseListMovies) {
    std::string input = "/api/listmovies";

    auto result = parse_request(input);

    ASSERT_NE(result, nullptr); // Ensure the result is not null
    ASSERT_TRUE(std::dynamic_pointer_cast<RequestListMovies>(result) != nullptr);
}

TEST_F(RequestParserTest, ParseListTheaters) {
    std::string input = "/api/listtheaters_123";

    auto result = parse_request(input);

    ASSERT_NE(result, nullptr);
    auto theater_request = std::dynamic_pointer_cast<RequestListTheaters>(result);
    ASSERT_NE(theater_request, nullptr);
    ASSERT_EQ(theater_request->movie_id, 123);
}

TEST_F(RequestParserTest, ParseListSeats) {
    std::string input = "/api/listseats_123_456";

    auto result = parse_request(input);

    ASSERT_NE(result, nullptr);
    auto seats_request = std::dynamic_pointer_cast<RequestListSeats>(result);
    ASSERT_NE(seats_request, nullptr);
    ASSERT_EQ(seats_request->movie_id, 123);
    ASSERT_EQ(seats_request->theater_id, 456);
}

TEST_F(RequestParserTest, ParseBookValid) {
    std::string input = "/api/book_123_456_1_2_3";
    auto result = parse_request(input);
    ASSERT_NE(result, nullptr);

    auto book_request = std::dynamic_pointer_cast<RequestBook>(result);

    ASSERT_NE(book_request, nullptr);
    ASSERT_EQ(book_request->movie_id, 123);
    ASSERT_EQ(book_request->theater_id, 456);
    ASSERT_EQ(book_request->seat_nums.count(1), 1);
    ASSERT_EQ(book_request->seat_nums.count(2), 1);
    ASSERT_EQ(book_request->seat_nums.count(3), 1);
}

TEST_F(RequestParserTest, ParseBookInvalidDuplicateSeats) {
    std::string input = "/api/book_123_456_1_2_2";  // Seat 2 is duplicated

    auto result = parse_request(input);

    ASSERT_NE(result, nullptr);
    ASSERT_TRUE(std::dynamic_pointer_cast<RequestInvalid>(result) != nullptr);
}

TEST_F(RequestParserTest, ParseInalid) {
    std::string input = "/api/invalid";

    auto result = parse_request(input);

    ASSERT_NE(result, nullptr);
    auto invalid_request = std::dynamic_pointer_cast<RequestInvalid>(result);
    ASSERT_NE(invalid_request, nullptr);
}

TEST_F(RequestParserTest, ParseRequestInvalidNumbers) {
    // Invalid movie ID that passes regex but cannot be converted to an unsigned long
    // Unless our architecture uses ridiculously large unsigned longs.
    std::string input = "/api/listtheaters_99999999999999999999999999999999999999999999999999999999999999"
                        "99999999999999999999999999999999999999999999999999999999999999999999999999999999";

    auto result = parse_request(input);

    ASSERT_NE(result, nullptr);
    auto invalid_request = std::dynamic_pointer_cast<RequestInvalid>(result);
    ASSERT_NE(invalid_request, nullptr);
}