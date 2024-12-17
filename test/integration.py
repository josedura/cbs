""" Full integration tests for cbs """

import subprocess
import os
import signal
import psutil
import time
import random
import re
import socket
import sys

ADDRESS = "127.0.0.1"
PORT = 18080

class InvalidFormatError(Exception):
    """Custom exception raised when the input string does not meet the expected format."""
    pass

def get_random_item(my_list):
    """
    Returns a random item from the given list.
    """
    return random.choice(my_list)

def random_items(numbers):
    """
    Randomly selects a number of items from the input list. 
    From 1 item to the the whole list. Input list must not be empty.
    
    Returns:
        list of int: A list containing randomly selected integers from the input list.
    """
    num_to_select = random.randint(1, len(numbers))
    selected_numbers = random.sample(numbers, num_to_select)
    return selected_numbers

def verify_id_string_list(input_string):
    """
    Verifies that a string is made up of lines where each line follows the 
    format: <integer>,<string>\r\n
    
    Args:
        input_string (str): The input string to be validated, which may contain multiple lines.
    
    Raises:
        InvalidFormatError: If any line in the input string does not match the expected format.
    """
    pattern = r'^\d+,[^,]+$' # regex for a single line
    
    lines = input_string.split('\r\n')
    if lines[-1] != "":
        raise InvalidFormatError(f"Invalid format: last line is not empty")
    for line in lines[:-1]:
        if not re.match(pattern, line):
            raise InvalidFormatError(f"Invalid format: {line}")


def parse_id_string_list(input_string):
    """
    Parses lines with an id and a string.

    Args:
        input_string (str): A string with lines separated by "\r\n". Each line contains an
                             integer followed by a string, separated by a comma.

    Returns:
        list of tuple: A list where each tuple contains an integer id and the associated string.
    """
    lines = input_string.split("\r\n")
    result = []
    
    for line in lines:
        if line:
            parts = line.split(",", 1) # Split the line by the first comma
            id_value = int(parts[0])
            string_value = parts[1]
            result.append((id_value, string_value))
    
    return result

def parse_seats(input_string):
    """
    Parses a string of non-negative numbers separated by commas and ending with "\r\n".
    
    Returns:
        List[int]: A list of non-negative integers parsed from the input string.
    
    Raises:
        InvalidFormatError: If the input string does not end with "\r\n", or if any of the numbers
                             cannot be parsed as non-negative integers.
    """
    if not input_string.endswith("\r\n"):
        raise InvalidFormatError("Input string must end with '\\r\\n'.")
    
    string_stripped = input_string.strip("\r\n") # Strip the trailing "\r\n" from the string

    if (string_stripped == ""):
        return []
    
    numbers_str = string_stripped.split(",")
    numbers = []
    for num in numbers_str:
        if num.isdigit():
            numbers.append(int(num))
        else:
            raise InvalidFormatError(f"Invalid number found: {num}. All entries must be non-negative integers.")
    
    return numbers

def verify_valid_seats(numbers):
    """
    Verifies that the list contains exactly 20 unique numbers in the range from 0 to 19.
    
    Raises:
        InvalidFormatError: If the list does not contain exactly 20 unique numbers from 0 to 19.
    """
    if len(set(numbers)) != len(numbers):
        raise InvalidFormatError("The numbers must be unique.")
    
    if not all(0 <= num <= 19 for num in numbers):
        raise InvalidFormatError("All numbers must be in the range from 0 to 19.")
    
    return True  # If all checks pass

def verify_updated_seats(available_seats, chosen_seats, updated_seats):
    """
    Verifies that the updated_seats is equal to available_seats minus chosen_seats.
    
    Args:
        available_seats (list of int): A list of available seats.
        chosen_seats (list of int): A list of chosen seats.
        updated_seats (list of int): A list of seats after choosing some from available_seats.
    
    Raises:
        InvalidFormatError: If updated_seats is not equal to available_seats minus chosen_seats.
    """
    available_seats_set = set(available_seats)
    chosen_seats_set = set(chosen_seats)
    updated_seats_set = set(updated_seats)
    
    if updated_seats_set != available_seats_set - chosen_seats_set:
        raise InvalidFormatError("Updated seats do not match available seats minus chosen seats.")
    
def do_http_request(endpoint, address, port):
    """
    Makes an HTTP GET request to a server specified by an IP address and port,
    and returns the HTTP status code and body of the response.
    
    Args:
        endpoint (str): The path of the resource to request (e.g., "/api/listmovies").
        address (str): The IP address of the server to send the request to.
        port (int): The port on which the server is listening for HTTP requests.
    
    Returns:
        tuple: A tuple containing the HTTP status code (int) and the response body (str).
    """
    with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as sock:
        sock.connect((address, port))
        request = f"GET {endpoint} HTTP/1.1\r\nHost: {address}\r\nConnection: close\r\n\r\n"
        sock.sendall(request.encode())

        # Receive the response data
        response = b""
        while True:
            chunk = sock.recv(4096)
            if not chunk:
                break
            response += chunk

    response = response.decode("utf-8")
    headers, body = response.split("\r\n\r\n", 1)
    
    status_line = headers.split("\r\n")[0]
    status_code = int(status_line.split(" ")[1])

    return status_code, body

def build_book_endpoint(movie_id, theater_id, chosen_seats):
    """
    Builds a book endpoint

    Args:
        movie_id : integer id of the movie
        theater_id : integer id of the theater
        chose_seats : list of integer ids of the chosen seats
    """
    result =  "/api/book_" + str(movie_id) + "_" + str(theater_id)
    for seat in chosen_seats:
        result += "_" + str(seat)
    return result

def get_any_movie_theater_with_seats():
    """
    Gets random movie and theater with available seats

    Returns tuple with:
        movie_id : integer id of the movie
        theater_id : integer id of the theater
        seats : non empty list of integer ids of the seats available
    """
    _, body_movies = do_http_request("/api/listmovies", ADDRESS, PORT)
    movie_list = parse_id_string_list(body_movies)

    while True:
        movie_id, _ = get_random_item(movie_list)
        _, body_theater = do_http_request(f"/api/listtheaters_{movie_id}", ADDRESS, PORT)

        theater_list = parse_id_string_list(body_theater)
        if not theater_list:
            continue # Some movies may not be displayed in a theater

        theater_id, _ = get_random_item(theater_list)
        _, body_seats = do_http_request(f"/api/listseats_{movie_id}_{theater_id}", ADDRESS, PORT)
        seats = parse_seats(body_seats)
        if not seats:
            continue # After previous bookings, some theaters might be fully booked

        return movie_id, theater_id, seats

def get_any_movie_theater():
    """
    Gets random movie and theater

    Returns tuple with:
        movie_id : integer id of the movie
        theater_id : integer id of the theater
        seats : possibly empty list of integer ids of the seats available
    """
    _, body_movies = do_http_request("/api/listmovies", ADDRESS, PORT)
    movie_list = parse_id_string_list(body_movies)

    while True:
        movie_id, _ = get_random_item(movie_list)
        _, body_theater = do_http_request(f"/api/listtheaters_{movie_id}", ADDRESS, PORT)

        theater_list = parse_id_string_list(body_theater)
        if not theater_list:
            continue # Some movies may not be displayed in a theater

        theater_id, _ = get_random_item(theater_list)
        _, body_seats = do_http_request(f"/api/listseats_{movie_id}_{theater_id}", ADDRESS, PORT)
        seats = parse_seats(body_seats)

        return movie_id, theater_id, seats

def choose_random_booked_seats(available_seats):
    """
    Chooses a random number of seats are NOT present in the input available_seats.
    
    Args:
    available_seats (list of int): A list containing integers between 0 and 19, which represent the available seats.
                                   Must have a len of less than 20
    
    Returns:
    list of int: A list containing randomly chosen integers between 0 and 19 that are NOT in available_seats
    """
    all_seats = set(range(20))
    booked_seats = list(all_seats - set(available_seats))
    num_choices = random.randint(1, len(booked_seats))
    chosen_seats = random.sample(booked_seats, num_choices)
    
    return chosen_seats
    
def start_cbs():
    """Runs the cbs server in background"""
    print("Start server")
    cbs_process = subprocess.Popen(['build/cbs', ADDRESS, str(PORT), "10"],
                                   stdout=subprocess.PIPE,
                                   stderr=subprocess.PIPE)
    print("Wait for server initialization")
    time.sleep(10)
    return cbs_process.pid

def stop_cbs(pid):
    """Stops the cbs server"""
    os.kill(pid, signal.SIGTERM)
    time.sleep(0.1)
    try:
        do_http_request("/api/invalid", ADDRESS, PORT)
    except:
        pass # It is not relevant if the request fails
    print("Server stopped")

def verify_cbs_not_running():
    """Verifies that no cbs server is running. Exits immediately if any is running"""
    running = False
    for proc in psutil.process_iter(['pid', 'name']):
        try:
            running = proc.name() == 'cbs'
        except:
            pass # Ignore exceptions like Zombie Processes.
        if running:
            print("Integration tests cannot run while cbs is already running. Please stop any instance of cbs.")
            sys.exit(1)

def test_listmovies():
    """
    Good weather test for the '/api/listmovies' endpoint.

    Performs the following checks on the movie list retrieved from the server:
    1. Ensures the list contains at least 10,000 movies.
    2. Verifies that all movie IDs are unique.
    3. Confirms the presence of the following movie titles:
        - "The Godfather"
        - "A night at the opera"
        - "Pulp Fiction"
        - "Seven Samurai"
        - "Terminator 2: Judgment Day"
        - "AKIRA"
        - "Bilal: A New Breed of Hero"
        - "¡Bienvenido Mr. Marshall!"
        - "Lucky Baskhar"
        - "Fist of Fury"
    4. Verifies that response is 200 OK
    5. The body of the reply has the expected format
    
    Raises an exception if any of the checks fail.
    """
    print("Good weather test /api/listmovies endpoint")

    status, body = do_http_request("/api/listmovies", ADDRESS, PORT)

    verify_id_string_list(body)

    if status != 200:
        raise Exception("Reply is not 200 OK")

    movie_list = parse_id_string_list(body)

    if len(movie_list) < 10000:
        raise Exception(f"List size is less than 10000, it is {len(movie_list)}")

    expected_titles = [
        "The Godfather",
        "A night at the opera",
        "Pulp Fiction",
        "Seven Samurai",
        "Terminator 2: Judgment Day",
        "AKIRA",
        "Bilal: A New Breed of Hero",
        "¡Bienvenido Mr. Marshall!",
        "Lucky Baskhar",
        "Fist of Fury"
    ]

    movie_ids = set()
    found_titles = set()

    # Loop through the movie list and check the conditions
    for movie_id, movie_title in movie_list:
        # Verify that all ids are different
        if movie_id in movie_ids:
            raise Exception(f"Duplicate movie ID found: {movie_id}")
        movie_ids.add(movie_id)

        # Check if the movie title is in the expected list
        if movie_title in expected_titles:
            found_titles.add(movie_title)

    # Verify that all expected titles were found in the list
    missing_titles = [title for title in expected_titles if title not in found_titles]
    if missing_titles:
        raise Exception(f"Missing expected titles: {', '.join(missing_titles)}")

    print("\033[32mOK\033[0m")

def test_listtheater():
    """
    Good weather test for the '/api/listtheater' endpoint.

    It will do a request for each of the movies available.

    Performs the following checks on each request.
    1. Verifies that all theater IDs are unique.
    2. Verifies that response is 200 OK
    3. The body of the reply has the expected format
    
    Raises an exception if any of the checks fail.
    """
    print("Good weather test /api/listmovies endpoint")

    _, body_movies = do_http_request("/api/listmovies", ADDRESS, PORT)
    movie_list = parse_id_string_list(body_movies)

    for movie_id, movie_name in movie_list:
        status, body = do_http_request(f"/api/listtheaters_{movie_id}", ADDRESS, PORT)
        
        if status != 200:
            raise Exception("Reply is not 200 OK")

        verify_id_string_list(body)
        theater_list = parse_id_string_list(body)
   
        theater_ids = set()
        # Loop through the theater list and check the conditions
        for theater_id, theater_name in theater_list:
            # Verify that all ids are different
            if theater_id in theater_ids:
                raise Exception(f"Duplicate theater ID found: {theater_id} for movie ID: {movie_id}")
            theater_ids.add(theater_id)

    print("\033[32mOK\033[0m")

def test_listseats():
    """
    Good weather test for the '/api/listseats' endpoint.

    Do 1000 requests for random valid combinations of movie and theater.

    Performs the following checks on each request
    1. Verifies that response is 200 OK
    2. The body of the reply has the expected format
    3. The response has 20 seats numbered from 0 to 19, in any order.
    4. More than half of the queried movies are displayed in a theater.
    
    Raises an exception if any of the checks fail.
    """
    print("Good weather test /api/listseats endpoint")

    _, body_movies = do_http_request("/api/listmovies", ADDRESS, PORT)
    movie_list = parse_id_string_list(body_movies)
    movies_displayed = 0

    for _ in range(1000):
        movie_id, _ = get_random_item(movie_list)

        _, body_theater = do_http_request(f"/api/listtheaters_{movie_id}", ADDRESS, PORT)

        theater_list = parse_id_string_list(body_theater)
        if not theater_list:
            continue # Some movies may not be displayed in a theater
        movies_displayed += 1

        theater_id, _ = get_random_item(theater_list)

        status, body = do_http_request(f"/api/listseats_{movie_id}_{theater_id}", ADDRESS, PORT)
   
        if status != 200:
            raise Exception("Reply is not 200 OK")

        seats = parse_seats(body)
        verify_valid_seats(seats)
        if len(seats) != 20:
            raise InvalidFormatError("Got a list of seats which is not of size 20")

    if movies_displayed < 500:
        raise InvalidFormatError("Too many movies are reported as not displayed in any theater")

    print("\033[32mOK\033[0m")

def test_bookseats():
    """
    Good weather test for the '/api/listbook' endpoint.

    Do 1000 requests for random valid combinations of movie, theater and booked seats.

    Performs the following checks on each request
    1. Verifies that response is 200 OK
    2. A listseats request after the book requests shows properly updated available seats.
    
    Raises an exception if any of the checks fail.
    """
    print("Good weather test /api/book endpoint")

    _, body_movies = do_http_request("/api/listmovies", ADDRESS, PORT)
    movie_list = parse_id_string_list(body_movies)

    for _ in range(1000):
        movie_id, _ = get_random_item(movie_list)

        _, body_theater = do_http_request(f"/api/listtheaters_{movie_id}", ADDRESS, PORT)

        theater_list = parse_id_string_list(body_theater)
        if not theater_list:
            continue # Some movies may not be displayed in a theater

        theater_id, _ = get_random_item(theater_list)

        _, body_seats = do_http_request(f"/api/listseats_{movie_id}_{theater_id}", ADDRESS, PORT)
        seats = parse_seats(body_seats)
        verify_valid_seats(seats)
        
        if not seats:
            continue # After previous bookings, some theaters might be fully booked

        chosen_seats = random_items(seats)
        endpoint = build_book_endpoint(movie_id, theater_id, chosen_seats)
        status_book, _ = do_http_request(endpoint, ADDRESS, PORT)
        if status_book != 200:
            raise Exception("Reply is not 200 OK")

        # Verify that a 2nd listseats request has the proper seats
        status, body_updated_seats = do_http_request(f"/api/listseats_{movie_id}_{theater_id}", ADDRESS, PORT)
        if status_book != 200:
            raise Exception("Reply is not 200 OK")
        updated_seats = parse_seats(body_updated_seats)
        verify_valid_seats(updated_seats)
        verify_updated_seats(seats, chosen_seats, updated_seats)

    print("\033[32mOK\033[0m")

def test_invalid_requests():
    """
    Test requests which do not comply with the API requirements.
        1. Invalid endpoint
        2. listtheaters with invalid theater id
        3. listseats with invaild movie id and theater id
        4. book with repeated seat number
        5. book with invalid seat number
    
    Raises an exception if any of the requests does not return 400 Bad Request
    """
    print("Test invalid requests")

    status, _ = do_http_request("/api/invalid", ADDRESS, PORT)
    if status != 400:
        raise Exception("Reply is not 400 Bad Request")

    status, _ = do_http_request("/api/listtheaters_99999999", ADDRESS, PORT)
    if status != 400:
        raise Exception("Reply is not 400 Bad Request")

    status, _ = do_http_request("/api/listseats_999999_9999999", ADDRESS, PORT)
    if status != 400:
        raise Exception("Reply is not 400 Bad Request")

    movie_id, theater_id, seats = get_any_movie_theater_with_seats()
    endpoint = f"/api/book_{movie_id}_{theater_id}_{seats[0]}_{seats[0]}"
    status, _ = do_http_request(endpoint, ADDRESS, PORT)
    if status != 400:
        raise Exception("Reply is not 400 Bad Request")

    endpoint = f"/api/book_{movie_id}_{theater_id}_25"
    status, _ = do_http_request(endpoint, ADDRESS, PORT)
    if status != 400:
        raise Exception("Reply is not 400 Bad Request")

    print("\033[32mOK\033[0m")

def test_forbidden_booking():
    """
    Test 1000 times trying to book some non-available seats
    
    Raises an exception if any of the requests does not return 403 Forbidden
    """
    print("Test forbidden bookings.")

    for i in range(1000):
        movie_id, theater_id, seats = get_any_movie_theater()
        if len(seats)==20:
            # Ensure that some seats are not available
            chosen_seats = random_items(seats)
            book_endpoint = build_book_endpoint(movie_id, theater_id, chosen_seats)
            do_http_request(book_endpoint, ADDRESS, PORT)
            _, body_seats = do_http_request(f"/api/listseats_{movie_id}_{theater_id}", ADDRESS, PORT)
            seats = parse_seats(body_seats)
        chosen_seats = choose_random_booked_seats(seats)
        endpoint = build_book_endpoint(movie_id, theater_id, chosen_seats)
        status, _ = do_http_request(endpoint, ADDRESS, PORT)

        if status != 403:
            raise Exception("Reply is not 403 Forbidden")

    print("\033[32mOK\033[0m")


def main():
    pid = None
    try:
        verify_cbs_not_running()
        pid = start_cbs()
        test_listmovies()
        test_listtheater()
        test_listseats()
        test_bookseats()
        test_invalid_requests()
        test_forbidden_booking()
    finally:
        if pid:
            stop_cbs(pid)

if __name__ == "__main__":
    main()
