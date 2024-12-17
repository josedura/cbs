# Cinema Booking Server (cbs)

The cinema booking server is an HTTP server to manage the query and booking of seats in theaters playing movies.

## Building the server

### Dependencies
cbs requires the following libraries:
- Boost.Asio
- Boost.Beast
- Boost.System
- GTest
- GMock
- Python library psutil

cbs is implemented in C++17.

cbs is platform independent, should build on any system which supports boost. It includes makefiles for Linux, which should also work under Windows with cygwin.

Documentation is generated using doxygen.

Unit tests require python 3

#### Debian/Ubuntu
```sh
sudo apt-get install build-essential libboost-all-dev doxygen libgtest-dev libgmock-dev
```

Install Python's psutil with your Python management system. In example:
```
pip install psutil
```

### Linux
Build the server with:
```sh
make -j 8
```

The server binary is named `cbs` and is located inside the `build` folder. It's built with optimization enabled.

To build a non-optimized binary with debugging symbols:
```sh
make debug -j 8
```

When switching between optimized and debug versions a make clean is needed.
```
make clean
```

## Running the server
Execute with the -h parameter to get help on the parameters supported by the server.
```sh
build/cbs -h
```

Example:
Run on localhost port 18080 using 10 threads
```sh
build/cbs 127.0.0.1 18080 10
```

## Stopping the server
The server can be stopped with SIGTERM in an orderly fashion. But a last request is needed after SIGTERM since the server otherwise would be blocking on accept.

Example:
```sh
killall -SIGTERM cbs
sleep 0.1s
curl -i http://localhost:18080/api/invalidreq
```

## Building documentation
```sh
rm -rf doc && doxygen doxygen_files/doxyfile
```

Documentation is generated inside folder `doc`. `doc/html/index.html` is the main page.

## Run unit tests
```sh
make clean && make -j 8 run_ut
```

## Integration tests
Full integration tests which verify all API endpoints with all possible responses are available.

Execute with:
```sh
make clean && make -j 8 && python3 test/integration.py
```

A small integration test is also included. It covers some scenarios which are difficult to cover with unit tests.

The small integration test starts cbs and then uses curl to perform a request for movies and another for theaters. It verifies that both answer 200 OK.

It is automatically executed when generating coverage report unless using coverage_report_ut_only.

It can be manually executed with:
```
make -j 8 && bash test/small_integration_test.sh
```

## Generate coverage report

### For unit tests only
```
make clean && make -j 8 coverage_report_ut_only
```
The report is available in coverage_report/"$PWD"/index.html

### Including partial integration tests
```
make clean && make -j 8 coverage_report
```
Some parts which are difficult to unit tests. Therefore, this way a small set of integration tests is included to get better coverage.

The report is available in coverage_report/"$PWD"/index.html

## Performance

cbs is optimized to serve requests in an efficient way.

Under these conditions:
- On a AMD Ryzen 5 1600X 2.2Ghz with 48GB RAM
- Managing 10000 movies, 10000 theaters and around 100 theaters per movie (so 20 million available seats in 1 million cinema rooms).
- Testing the 4 endpoints.
- Allways doing the same 4 requests. Requesting in parallel as many as the CPU can support.

It can serve 1000 requests per second.

Though this test is impaired by allways doing the same 4 requests which greatly favours cache locality. But this also reduces parallelism since the bookings can't be done in parallel for the same combination of movie and theater.

Furter testing is required with fully random requests to measure correctly cbs real performance. Performance in a real scenario may be higher or lower depending on whether the cache or the paralellism of bookings has more impact.
