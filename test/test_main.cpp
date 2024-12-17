#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "main.cpp"

class MockRunServer {
public:
    MOCK_METHOD(void, run_server, 
                (const boost::asio::ip::address &address, unsigned short port, unsigned short maxthreads,
                 std::atomic<bool>& stop_server));
};

class MainTest : public ::testing::Test {
protected:

    void SetUp() override {
        mock_run_server = std::make_shared<MockRunServer>();
        global_mock_run_server = mock_run_server;
    }

    void TearDown() override {
        mock_run_server.reset();
        global_mock_run_server.reset();
    }

    std::shared_ptr<MockRunServer> mock_run_server;

public:
    static std::shared_ptr<MockRunServer> global_mock_run_server;
};
std::shared_ptr<MockRunServer> MainTest::global_mock_run_server;

void init_booking_data() {
}

void run_server(const boost::asio::ip::address &address, unsigned short port, unsigned short maxthreads,
                std::atomic<bool>& stop_server) {
    MainTest::global_mock_run_server->run_server(address, port, maxthreads, stop_server);
}

TEST(ParseArgumentsTest, ValidInputs) {
    boost::asio::ip::address address;
    unsigned short port;
    unsigned short maxthreads;
    const char* argv[] = {"cbs", "127.0.0.1", "8080", "4"};

    parse_arguments(4, const_cast<char**>(argv), address, port, maxthreads);

    EXPECT_EQ(address, boost::asio::ip::make_address("127.0.0.1"));
    EXPECT_EQ(port, 8080);
    EXPECT_EQ(maxthreads, 4);
}

TEST(ParseArgumentsTest, InvalidArgCount) {
    boost::asio::ip::address address;
    unsigned short port;
    unsigned short maxthreads;
    const char* argv[] = {"cbs", "127.0.0.1", "8080"};

    EXPECT_EXIT(parse_arguments(3, const_cast<char**>(argv), address, port, maxthreads), 
                ::testing::ExitedWithCode(EXIT_FAILURE), "Invalid parameters.*");
}

TEST(ParseArgumentsTest, InvalidAddress) {
    boost::asio::ip::address address;
    unsigned short port;
    unsigned short maxthreads;
    const char* argv[] = {"cbs", "invalid_address", "8080", "4"};

    EXPECT_EXIT(parse_arguments(4, const_cast<char**>(argv), address, port, maxthreads),
                ::testing::ExitedWithCode(EXIT_FAILURE), "Invalid parameters.*");
}

TEST(ParseArgumentsTest, InvalidPort) {
    boost::asio::ip::address address;
    unsigned short port;
    unsigned short maxthreads;
    const char* argv[] = {"cbs", "127.0.0.1", "invalid_port", "4"};

    EXPECT_EXIT(parse_arguments(4, const_cast<char**>(argv), address, port, maxthreads),
                ::testing::ExitedWithCode(EXIT_FAILURE), "Invalid parameters.*");
}

TEST(ParseArgumentsTest, InvalidMaxThreads) {
    boost::asio::ip::address address;
    unsigned short port;
    unsigned short maxthreads;
    const char* argv[] = {"cbs", "127.0.0.1", "8080", "0"};

    EXPECT_EXIT(parse_arguments(4, const_cast<char**>(argv), address, port, maxthreads),
                ::testing::ExitedWithCode(EXIT_FAILURE), "Invalid parameters.*");
}

TEST(HelpMessageTest, DisplayHelp) {
    const char* argv[] = {"cbs", "-h"};
    boost::asio::ip::address address;
    unsigned short port;
    unsigned short maxthreads;

    EXPECT_EXIT(parse_arguments(2, const_cast<char**>(argv), address, port, maxthreads),
                ::testing::ExitedWithCode(EXIT_SUCCESS), "Usage: cbs.*");
}

TEST_F(MainTest, GoodWeather) {
    const char* argv[] = {"cbs", "127.0.0.1", "8080", "4"};
    EXPECT_CALL(*mock_run_server, run_server(::testing::_, 8080U, 4U, ::testing::_));

    auto exit_code = test_main(4, const_cast<char**>(argv));

    ASSERT_EQ(exit_code, EXIT_SUCCESS);
}

TEST_F(MainTest, ExceptionFromRunServer) {
    const char* argv[] = {"cbs", "127.0.0.1", "8080", "4"};

    EXPECT_CALL(*mock_run_server, run_server(::testing::_, 8080U, 4U, ::testing::_))
        .WillOnce(::testing::Throw(std::runtime_error("run_server exception")));

    auto exit_code = test_main(4, const_cast<char**>(argv));

    ASSERT_EQ(exit_code, EXIT_FAILURE);
}
