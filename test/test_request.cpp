#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include <unordered_set>
#include "request.cpp"

using testing::Return;

class MockCommandExecution {
public:
    MOCK_METHOD(CommandResult, list_movies, ());
    MOCK_METHOD(CommandResult, list_theaters, (unsigned long));
    MOCK_METHOD(CommandResult, list_seats, (unsigned long movie_id, unsigned long theater_id));
    MOCK_METHOD(CommandResult, book, (unsigned long movie_id, unsigned long theater_id,
                                      std::unordered_set<unsigned long> seat_numbers));
};

class RequestTest : public ::testing::Test {
protected:
    // You can add common setup or teardown code here
    void SetUp() override {
        mock_command_execution = std::make_shared<MockCommandExecution>();
        global_mock_command_execution = mock_command_execution;
    }
    void TearDown() override {
        mock_command_execution.reset();
        global_mock_command_execution.reset();
    }

    std::shared_ptr<MockCommandExecution> mock_command_execution;

public:
    static std::shared_ptr<MockCommandExecution> global_mock_command_execution;
};
std::shared_ptr<MockCommandExecution> RequestTest::global_mock_command_execution;

// Mocks for functions in commandexecution
CommandResult list_movies() {
    return RequestTest::global_mock_command_execution->list_movies();
}

CommandResult list_theaters(unsigned long movie_id) {
    return RequestTest::global_mock_command_execution->list_theaters(movie_id);
}

CommandResult list_seats(unsigned long movie_id, unsigned long theater_id) {
    return RequestTest::global_mock_command_execution->list_seats(movie_id, theater_id);
}

CommandResult book(unsigned long movie_id, unsigned long theater_id,
                   std::unordered_set<unsigned long> seat_numbers) {
    return RequestTest::global_mock_command_execution->book(movie_id, theater_id, seat_numbers);
}

// Test for RequestListMovies
TEST_F(RequestTest, RequestListMovies_GetType) {
    RequestListMovies request;

    EXPECT_EQ(request.get_type(), Request::LIST_MOVIES);
}

TEST_F(RequestTest, RequestListMovies_Execute) {
    RequestListMovies request;
    EXPECT_CALL(*mock_command_execution, list_movies)
        .WillOnce(Return(CommandResult{std::make_shared<std::string>("body"), CommandStatus::OK}));

    auto [_, status] = request.execute();
    
    EXPECT_EQ(status, CommandStatus::OK);
}

// Test for RequestListTheaters
TEST_F(RequestTest, RequestListTheaters_GetType) {
    RequestListTheaters request(1);
    EXPECT_EQ(request.get_type(), Request::LIST_THEATERS);
}

TEST_F(RequestTest, RequestListTheaters_Execute) {
    RequestListTheaters request(1);
    EXPECT_CALL(*mock_command_execution, list_theaters)
        .WillOnce(Return(CommandResult{std::make_shared<std::string>("body"), CommandStatus::OK}));

    auto [_, status] = request.execute();

    EXPECT_EQ(status, CommandStatus::OK); // Adjust based on your implementation
}

// Test for RequestListSeats
TEST_F(RequestTest, RequestListSeats_GetType) {
    RequestListSeats request(1, 1);
    EXPECT_EQ(request.get_type(), Request::LIST_SEATS);
}

TEST_F(RequestTest, RequestListSeats_Execute) {
    RequestListSeats request(1, 1);
    EXPECT_CALL(*mock_command_execution, list_seats)
        .WillOnce(Return(CommandResult{std::make_shared<std::string>("body"), CommandStatus::OK}));

    auto [_, status] = request.execute();

    EXPECT_EQ(status, CommandStatus::OK); // Adjust based on your implementation
}

// Test for RequestBook
TEST_F(RequestTest, RequestBook_GetType) {
    std::unordered_set<unsigned long> seat_nums = {1, 2, 3};
    RequestBook request(1, 1, std::move(seat_nums));

    EXPECT_EQ(request.get_type(), Request::BOOK);
}

TEST_F(RequestTest, RequestBook_Execute) {
    std::unordered_set<unsigned long> seat_nums = {1, 2, 3};
    RequestBook request(1, 1, std::move(seat_nums));
    EXPECT_CALL(*mock_command_execution, book)
        .WillOnce(Return(CommandResult{std::make_shared<std::string>("body"), CommandStatus::OK}));

    auto [_, status] = request.execute();

    EXPECT_EQ(status, CommandStatus::OK);
}

// Test for RequestInvalid
TEST_F(RequestTest, RequestInvalid_GetType) {
    RequestInvalid request;
    EXPECT_EQ(request.get_type(), Request::INVALID);
}

TEST_F(RequestTest, RequestInvalid_Execute) {
    RequestInvalid request;
    auto [_, status] = request.execute();
    EXPECT_EQ(status, CommandStatus::INVALID_REQ);
}
