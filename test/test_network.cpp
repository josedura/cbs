#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include <boost/asio/io_context.hpp>
#include <boost/asio/thread_pool.hpp>
#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <memory>
#include "network.cpp"

// Mock classes for dependencies

class MockHttpHandler : public HttpHandler {
public:
    MOCK_METHOD(void, read, (beast::tcp_stream&, beast::flat_buffer&, http::request<http::string_body>&),
                (const, override));
    MOCK_METHOD(void, write, (beast::tcp_stream&, const http::response<http::string_body>&), (const, override));
};

void HttpHandler::read(beast::tcp_stream& stream, beast::flat_buffer& buffer, 
                       http::request<http::string_body>& req) const {
    (void) stream;
    (void) buffer;
    (void) req;
}

void HttpHandler::write(beast::tcp_stream& stream, const http::response<http::string_body>& res) const {
    (void) stream;
    (void) res;
}

HttpHandler::~HttpHandler() = default;


class MockParser {
public:
    MOCK_METHOD(std::shared_ptr<Request>, parse_request, (const std::string& request), (const));
};

class MockRequest : public Request{
public:
    MOCK_METHOD(Type, get_type, (), (const, override));
    MOCK_METHOD(CommandResult, execute, (), (const, override));
};

// Unit Test Cases
class HandleCommandTest : public ::testing::Test {
protected:
    void SetUp() override {
        mock_request = std::make_shared<MockRequest>();
        global_mock_request = mock_request;
        mock_parser = std::make_shared<MockParser>();
        global_mock_parser = mock_parser;
    }

    void TearDown() override {
        mock_request.reset();
        global_mock_request.reset();
        mock_parser.reset();
        global_mock_parser.reset();
    }

    boost::asio::io_context io_context;
    std::shared_ptr<MockParser> mock_parser;
    std::shared_ptr<MockRequest> mock_request;
    MockHttpHandler mock_http_handler;

public:
    static std::shared_ptr<MockRequest> global_mock_request;
    static std::shared_ptr<MockParser> global_mock_parser;
};
std::shared_ptr<MockRequest> HandleCommandTest::global_mock_request;
std::shared_ptr<MockParser>  HandleCommandTest::global_mock_parser;

std::shared_ptr<Request> parse_request(const std::string& input) {
    HandleCommandTest::global_mock_parser->parse_request(input);
    return HandleCommandTest::global_mock_request;
}

Request::~Request() = default;

MATCHER_P2(ResponseHas, expected_body, expected_status, "") {
    return arg.body() == expected_body &&
           arg.result() == expected_status;
}

TEST_F(HandleCommandTest, HandlesValidEndpoint) {
    boost::asio::io_context ioc;
    beast::tcp_stream stream(ioc);
    // Prepare a mock request and response
    http::request<http::string_body> request;
    const char* endpoint = "/api/valid_endpoint";
    const char* body = "valid_body";
    request.target(endpoint);

    EXPECT_CALL(mock_http_handler, read(testing::_, testing::_, testing::_))
        .WillOnce(testing::SetArgReferee<2>(request));

    EXPECT_CALL(*mock_parser, parse_request(endpoint))
        .WillOnce(testing::Return(std::make_shared<MockRequest>()));

    EXPECT_CALL(*mock_request, execute())
        .WillOnce(testing::Return(CommandResult{std::make_shared<std::string>(body), CommandStatus::OK}));

    EXPECT_CALL(mock_http_handler, write(testing::_, ResponseHas(body, http::status::ok)));

    // Call handle_command and verify expected behavior
    handle_command(stream, mock_http_handler);
}

TEST_F(HandleCommandTest, HandlesForbiddenEndpoint) {
    boost::asio::io_context ioc;
    beast::tcp_stream stream(ioc);
    // Prepare a mock request and response
    http::request<http::string_body> request;
    const char* endpoint = "/api/forbidden_endpoint";
    const char* body = "forbidden_body";
    request.target(endpoint);

    EXPECT_CALL(mock_http_handler, read(testing::_, testing::_, testing::_))
        .WillOnce(testing::SetArgReferee<2>(request));

    EXPECT_CALL(*mock_parser, parse_request(endpoint))
        .WillOnce(testing::Return(std::make_shared<MockRequest>()));

    EXPECT_CALL(*mock_request, execute())
        .WillOnce(testing::Return(CommandResult{std::make_shared<std::string>(body), CommandStatus::REJECTED}));

    EXPECT_CALL(mock_http_handler, write(testing::_, ResponseHas(body, http::status::forbidden)));

    // Call handle_command and verify expected behavior
    handle_command(stream, mock_http_handler);
}

TEST_F(HandleCommandTest, HandlesInvalidEndpoint) {
    boost::asio::io_context ioc;
    beast::tcp_stream stream(ioc);
    // Prepare a mock request and response
    http::request<http::string_body> request;
    const char* endpoint = "/api/invalid_endpoint";
    const char* body = "invalid_body";
    request.target(endpoint);

    EXPECT_CALL(mock_http_handler, read(testing::_, testing::_, testing::_))
        .WillOnce(testing::SetArgReferee<2>(request));

    EXPECT_CALL(*mock_parser, parse_request(endpoint))
        .WillOnce(testing::Return(std::make_shared<MockRequest>()));

    EXPECT_CALL(*mock_request, execute())
        .WillOnce(testing::Return(CommandResult{std::make_shared<std::string>(body), CommandStatus::INVALID_REQ}));

    EXPECT_CALL(mock_http_handler, write(testing::_, ResponseHas(body, http::status::bad_request)));

    // Call handle_command and verify expected behavior
    handle_command(stream, mock_http_handler);
}


TEST_F(HandleCommandTest, HandlesParsingException) {
    boost::asio::io_context ioc;
    beast::tcp_stream stream(ioc);
    // Prepare a mock request

    EXPECT_CALL(mock_http_handler, read(testing::_, testing::_, testing::_))
        .Times(1);

    // Simulate parse_request throwing an exception
    EXPECT_CALL(*mock_parser, parse_request(testing::_))
        .WillOnce(testing::Throw(std::runtime_error("Parse request failed")));

    ASSERT_NO_THROW(handle_command(stream, mock_http_handler));
}


