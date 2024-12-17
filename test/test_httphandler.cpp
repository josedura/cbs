#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>
#include <boost/asio.hpp>

// Forward declare mock specializations of http::read and http::write 
namespace boost::beast::http {
    template<>
    std::size_t read(beast::tcp_stream& stream, beast::flat_buffer& buffers,
                     http::request<http::string_body>& req);
    
    template<>
    std::size_t write(beast::tcp_stream& stream, const http::response<http::string_body>& msg);
}

#include "httphandler.cpp"

namespace beast = boost::beast;
namespace http = beast::http;
namespace asio = boost::asio;

class MockHttpHandler : public HttpHandler {
public:
    MOCK_METHOD(void, read, (beast::tcp_stream & stream, beast::flat_buffer & buffer, 
                             http::request<http::string_body> & req), (const, override));

    MOCK_METHOD(void, write, (beast::tcp_stream & stream, const http::response<http::string_body> & res),
                             (const, override));
};

class HttpHandlerTest : public ::testing::Test {
protected:

    void SetUp() override {
        mock_http_handler = std::make_shared<MockHttpHandler>();
        global_mock_http_handler = mock_http_handler;
    }

    void TearDown() override {
        mock_http_handler.reset();
        global_mock_http_handler.reset();
    }

    asio::io_context ioc;
    beast::tcp_stream stream{ioc};
    beast::flat_buffer buffer;
    http::request<http::string_body> req;
    http::response<http::string_body> res;

    std::shared_ptr<MockHttpHandler> mock_http_handler;

public:
    static std::shared_ptr<MockHttpHandler> global_mock_http_handler;
};
std::shared_ptr<MockHttpHandler> HttpHandlerTest::global_mock_http_handler;

namespace boost::beast::http {
    template<>
    std::size_t read(beast::tcp_stream& stream, beast::flat_buffer& buffers,
                                    http::request<http::string_body>& req) {
        HttpHandlerTest::global_mock_http_handler->read(stream, buffers, req);
        return 0;
    }

    template<>
    std::size_t write(beast::tcp_stream& stream, const http::response<http::string_body>& msg) {
        HttpHandlerTest::global_mock_http_handler->write(stream, msg);
        return 0;
    }
}

// Define a parameterized matcher to compare stream pointers
MATCHER_P(IsSameAddress, expected_ptr, "Checks if parameter has same address as expected.") {
    return &arg == expected_ptr;
}


TEST_F(HttpHandlerTest, ReadInvokesBeastRead) {
    HttpHandler http_handler;
    EXPECT_CALL(*mock_http_handler, read(IsSameAddress(&stream), IsSameAddress(&buffer), IsSameAddress(&req)))
        .Times(1);

    http_handler.read(stream, buffer, req);
}

TEST_F(HttpHandlerTest, WriteInvokesBeastWrite) {
    HttpHandler http_handler;
    EXPECT_CALL(*mock_http_handler, write(IsSameAddress(&stream), IsSameAddress(&res)))
        .Times(1);

    http_handler.write(stream, res);
}
