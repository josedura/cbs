# Compiler and flags
CXX = g++
CXXFLAGS = -std=c++17 -Wall -Wextra -MMD -MP

# Coverage flags
COVERAGE_FLAGS = -fprofile-arcs -ftest-coverage

# Source files and object files
SRCS = bookingdata.cpp \
       commandexecution.cpp \
       initbookingdata.cpp \
       main.cpp \
       network.cpp \
       requestparser.cpp \
       stringidmap.cpp \
       request.cpp \
       httphandler.cpp
BUILD_DIR = build
OBJS = $(patsubst %.cpp,$(BUILD_DIR)/%.o,$(SRCS))
DEPS = $(OBJS:.o=.d)

# Unit test files
TEST_DIR = test
TEST_SRCS = $(wildcard $(TEST_DIR)/*.cpp) # Ensure it picks up all test .cpp files
TEST_BUILD_DIR = test_build
TEST_OBJS = $(patsubst $(TEST_DIR)/%.cpp,$(TEST_BUILD_DIR)/%.o,$(TEST_SRCS))
TEST_TARGET = $(TEST_BUILD_DIR)/run_tests

# Targets
TARGET = $(BUILD_DIR)/cbs

default: optimized

# Debug build
debug: CXXFLAGS += -g
debug: $(TARGET)

# Optimized build
optimized: CXXFLAGS += -O2
optimized: $(TARGET)

# Build with coverage
coverage: CXXFLAGS += $(COVERAGE_FLAGS)
coverage: LDFLAGS += $(COVERAGE_FLAGS)
coverage: $(TARGET)

# Linking the executable
$(TARGET): $(OBJS)
	@mkdir -p $(BUILD_DIR)
	$(CXX) $(CXXFLAGS) $(LDFLAGS) -o $@ $^

# Compiling object files
$(BUILD_DIR)/%.o: %.cpp
	@mkdir -p $(BUILD_DIR)
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Unit test target
unit_tests: CXXFLAGS += $(COVERAGE_FLAGS)
unit_tests: LDFLAGS += $(COVERAGE_FLAGS)
unit_tests: $(TEST_BUILD_DIR)/test_main \
            $(TEST_BUILD_DIR)/test_network \
            $(TEST_BUILD_DIR)/test_bookingdata \
            $(TEST_BUILD_DIR)/test_commandexecution \
            $(TEST_BUILD_DIR)/test_request \
            $(TEST_BUILD_DIR)/test_requestparser \
            $(TEST_BUILD_DIR)/test_stringmap \
            $(TEST_BUILD_DIR)/test_httphandler
	@echo "start unit_tests"

# StringIdMap is used directly in some Unit tests instead of mocked.
$(TEST_BUILD_DIR)/stringidmap.o : stringidmap.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(TEST_BUILD_DIR)/test_main: $(TEST_BUILD_DIR)/test_main.o
	@mkdir -p $(TEST_BUILD_DIR)
	$(CXX) $(CXXFLAGS) -o $@ $^ -L/usr/lib -lgmock -lgtest -lgtest_main -pthread

$(TEST_BUILD_DIR)/test_network: $(TEST_BUILD_DIR)/test_network.o
	@mkdir -p $(TEST_BUILD_DIR)
	$(CXX) $(CXXFLAGS) -o $@ $^ -L/usr/lib -lgmock -lgtest -lgtest_main -pthread

$(TEST_BUILD_DIR)/test_bookingdata: $(TEST_BUILD_DIR)/test_bookingdata.o $(TEST_BUILD_DIR)/stringidmap.o
	@mkdir -p $(TEST_BUILD_DIR)
	$(CXX) $(CXXFLAGS) -o $@ $^ -L/usr/lib -lgmock -lgtest -lgtest_main -pthread

$(TEST_BUILD_DIR)/test_commandexecution: $(TEST_BUILD_DIR)/test_commandexecution.o $(TEST_BUILD_DIR)/stringidmap.o
	@mkdir -p $(TEST_BUILD_DIR)
	$(CXX) $(CXXFLAGS) -o $@ $^ -L/usr/lib -lgmock -lgtest -lgtest_main -pthread

$(TEST_BUILD_DIR)/test_request: $(TEST_BUILD_DIR)/test_request.o
	@mkdir -p $(TEST_BUILD_DIR)
	$(CXX) $(CXXFLAGS) -o $@ $^ -L/usr/lib -lgmock -lgtest -lgtest_main -pthread

$(TEST_BUILD_DIR)/test_requestparser: $(TEST_BUILD_DIR)/test_requestparser.o
	@mkdir -p $(TEST_BUILD_DIR)
	$(CXX) $(CXXFLAGS) -o $@ $^ -L/usr/lib -lgmock -lgtest -lgtest_main -pthread

$(TEST_BUILD_DIR)/test_stringmap: $(TEST_BUILD_DIR)/test_stringmap.o
	@mkdir -p $(TEST_BUILD_DIR)
	$(CXX) $(CXXFLAGS) -o $@ $^ -L/usr/lib -lgmock -lgtest -lgtest_main -pthread

$(TEST_BUILD_DIR)/test_httphandler: $(TEST_BUILD_DIR)/test_httphandler.o
	@mkdir -p $(TEST_BUILD_DIR)
	$(CXX) $(CXXFLAGS) -o $@ $^ -L/usr/lib -lgmock -lgtest -lgtest_main -pthread


# Compiling unit test object files
$(TEST_BUILD_DIR)/%.o: $(TEST_DIR)/%.cpp
	@mkdir -p $(TEST_BUILD_DIR)
	$(CXX) $(CXXFLAGS) -D UNIT_TESTS -I . -c $< -o $@

# Run unit tests
.PHONY: run_ut
run_ut: unit_tests
	@echo "start run_ut"
	$(TEST_BUILD_DIR)/test_main 
	$(TEST_BUILD_DIR)/test_network
	$(TEST_BUILD_DIR)/test_bookingdata
	$(TEST_BUILD_DIR)/test_commandexecution
	$(TEST_BUILD_DIR)/test_request
	$(TEST_BUILD_DIR)/test_requestparser
	$(TEST_BUILD_DIR)/test_stringmap
	$(TEST_BUILD_DIR)/test_httphandler

# Generating coverage report
.PHONY: coverage_report
coverage_report: run_ut unit_tests coverage
	bash test/small_integration_test.sh
	gcov $(SRCS)
	lcov --capture --directory . --output-file coverage.info
	genhtml coverage.info --output-directory coverage_report
	@echo "Coverage report available in coverage_report/$(shell pwd)/index.html"

# Generating coverage report for unit tests only
.PHONY: coverage_report_ut_only
coverage_report_ut_only: run_ut unit_tests coverage
	gcov $(SRCS)
	lcov --capture --directory . --output-file coverage.info
	genhtml coverage.info --output-directory coverage_report
	@echo "Coverage report available in coverage_report/$(shell pwd)/index.html"

# Include dependency files
-include $(DEPS)

# Clean up build folder
.PHONY: clean
clean:
	rm -rf $(BUILD_DIR) $(TEST_BUILD_DIR) *.gcda *.gcno coverage.info coverage_report doc
