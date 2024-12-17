# Client API Documentation

## API summary

cbs supports only the HTTP protocol. All requests must be performed using GET http method.

It supports 4 endpoints:

- Listing all movies.
- Listing all theaters which are playing a given movie.
- Listing seats available for a given movie and theater.
- Booking seats in a given movie and theater.

## API Endpoints

### 1. GET /api/listmovies
**Description:** This endpoint retrieves all movies available.

**HTTP Method:** `GET`

**Endpoint URL:** `/api/list`

**Response:**
- **Status Code:** `200 OK`
- **Content Type:** `text/plain`
- **Response Body:**
```text
movieid,movie_title\r\n
movieid,movie_title\r\n
...
```

The response body contains one line per movie available.
- `movieid` is a non-negative integer which identifies each movie and is unique among movies.
- `movie_title` is the title of the movie.
The first comma separates id and title. The title may contain further commas and other symbols.

**Example:**
```sh
$ curl -i http://localhost:18080/api/listmovies 
HTTP/1.1 200 OK
Content-Type: text/plain
Content-Length: 102

5,AKIRA
4,Bilal: A New Breed of Hero
3,¡Bienvenido Mr. Marshall!
2,Lucky Baskhar
1,Fist of Fury
```

### 2. GET /api/listtheaters
**Description:** This endpoint retrieves all theaters available for a given movie.

**HTTP Method:** `GET`

**Endpoint URL:** `/api/listtheaters_{movieid}`

**URL parameter:**
- **movieid**: integer identifier for a movie as retrieved from `/api/listmovies` endpoint

**Response:**
- **Status Code:** `200 OK`
- **Content Type:** `text/plain`
- **Response Body:**
```text
theaterid,theater_name\r\n
theaterid,theater_name\r\n
...
```

The response body contains one line per theater available for the given movie.
- `theaterid` is a non-negative integer which identifies each theater and is unique among theaters.
- `theater_name` is the name of the theater.
The first comma separates id and name. The name may contain further commas and other symbols.

**Example:**
```sh
$ curl -i http://localhost:18080/api/listtheaters_1 
HTTP/1.1 200 OK
Content-Type: text/plain
Content-Length: 33

1,Grand cinema
2,Cinema Royale
```

### 3. GET /api/listseats
**Description:** This endpoint retrieves all seats available for a given movie and theater.

**HTTP Method:** `GET`

**Endpoint URL:** `/api/listseats_{movieid}_{theaterid}`

**URL parameter:**
- **movieid**: integer identifier for a movie as retrieved from `/api/listmovies` endpoint
- **theaterid**: integer identifier for a theater as retrieved from `/api/listtheaters` endpoint

**Response:**
- **Status Code:** `200 OK`
- **Content Type:** `text/plain`
- **Response Body:**
```text
[seatid][,seatid...]\r\n
```

The response body contains a single with the integer ids of the seats available. Separated by commas. If no seat is available the body will contain only "\r\n"
- `seatid` is a non-negative integer which identifies each seat available for a given movie and theater.

**Example:**
```sh
$ curl -i http://localhost:18080/api/listseats_1_2
HTTP/1.1 200 OK
Content-Type: text/plain
Content-Length: 51

0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19
```

### 4. GET /api/book
**Description:** This endpoint request booking a list of seats in a given movie and threater.

**HTTP Method:** `GET`

**Endpoint URL:** `/api/book_{movieid}_{theaterid}_{seatid}[_seatid..]`

**URL parameter:**
- **movieid**: integer identifier for a movie as retrieved from `/api/listmovies` endpoint
- **theaterid**: integer identifier for a theater as retrieved from `/api/listtheaters` endpoint
- **seatid**: integer identifier for a seat as retrieved from `/api/listseats` endpoint

**Response:**
- **Status Code:** `200 OK`
- **Content Type:** `text/plain`
- **Response Body:** shall be ignored

Indicates that all requested seats have been successfully booked.
Clients must not rely on the Response Body. A 200 OK code always means successful booking.
The response body is not guaranteed to have any particular text and may change at any time.

**Response:**
- **Status Code:** `403 Forbidden`
- **Content Type:** `text/plain`
- **Response Body:** shall be ignored

Indicates that at least one of the requested seats was not available. No seats have been booked.
Clients must not rely on the Response Body. A 403 Forbidden code always means a correct request which cannot be satisfied.
The response body is not guaranteed to have any particular text and may change at any time.

**Examples:**
```sh
$ curl -i http://localhost:18080/api/book_1_2_7_9_11
HTTP/1.1 200 OK
Content-Type: text/plain
Content-Length: 12

Booking OK

$ curl -i http://localhost:18080/api/book_1_2_7
HTTP/1.1 403 Forbidden
Content-Type: text/plain
Content-Length: 21

Seats not available
```

### 5. Common Response

All endpoints except `/api/listmovies` can reply `400 Bad %Request`.

**Response:**
- **Status Code:** `400 Bad %Request`
- **Content Type:** `text/plain`
- **Response Body:** shall be ignored

Indicates a malformed request. One which does not respect the endpoint format. Which requests seats for a theater and movie when such theater is not available for such movie.  Which has invalid movie, theater or seat numbers. Which is not a valid endpoint. Or which requests the same seat more than once.
Clients must not rely on the Response Body. A 400 Bad %Request code always means an invalid request.
The response body is not guaranteed to have any particular text and may change at any time.

**Examples:**
```sh
$ curl -i http://localhost:18080/api/listtheaters_99999999 
HTTP/1.1 400 Bad %Request
Content-Type: text/plain
Content-Length: 18

Invalid movieid

$ curl -i http://localhost:18080/api/listseats_999999_9999999
HTTP/1.1 400 Bad %Request
Content-Type: text/plain
Content-Length: 48

Invalid combination of movieid and theaterid

$ curl -i http://localhost:18080/api/book_1_2_8_8
HTTP/1.1 400 Bad %Request
Content-Type: text/plain
Content-Length: 15

Invalid request

$ curl -i http://localhost:18080/api/notanendpoint
HTTP/1.1 400 Bad %Request
Content-Type: text/plain
Content-Length: 15

Invalid request
```
