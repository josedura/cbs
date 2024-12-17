/**
 *   \file initbookingdata.cpp
 *   \brief Initialize Movies and Theaters with some semi-random data.
 * 
 *   This is not part of cbs proper, it exists just to enable integration testing.
 *   A real implementation of cbs would read such data from a configuration file or would
 * provide interfaces to modifiy the movies of theaters available, but that is out of the
 * current scope.
 *   
 *   Which is why this file is not unit tested. We get coverage from integration tests, but
 * that is a side-effect, no effort is made on testing this code since it is functionlly
 * test code itself; it is included inside production code since cbs does not yet contain
 * an administrator mode which would allow changing theaters, movies, etc.
 */

#include <sstream>
#include <iostream>

#include "initbookingdata.hpp"
#include "bookingdata.hpp"

static void init_movie_list(void) {
    std::unordered_set<std::string> movie_titles;

    // 10 real movies
    movie_titles.insert("The Godfather");
    movie_titles.insert("A night at the opera");
    movie_titles.insert("Pulp Fiction");
    movie_titles.insert("Seven Samurai");
    movie_titles.insert("Terminator 2: Judgment Day");
    movie_titles.insert("AKIRA");
    movie_titles.insert("Bilal: A New Breed of Hero");
    movie_titles.insert("¡Bienvenido Mr. Marshall!");
    movie_titles.insert("Lucky Baskhar");
    movie_titles.insert("Fist of Fury");
    // 10000 fake movies
    std::stringstream ss;
    for (int idx=0; idx<10000; ++idx) {
        ss.str(std::string());
        ss << "Movie " << std::to_string(idx);
        movie_titles.insert(ss.str());
    }

    BookingData::get_instance().add_movies(std::move(movie_titles));
}

static void init_theater_list() {
    std::unordered_set<std::string> theater_names;
    // 10000 fake theaters
    std::stringstream ss;
    for (int idx=0; idx<10000; ++idx) {
        ss.str(std::string());
        ss << "theater " << std::to_string(idx);
        theater_names.insert(ss.str());
    }

    BookingData::get_instance().add_theaters(std::move(theater_names));
}

static void add_fixed_theaters_to_movie(MovieId movie_id, const std::set<TheaterId>& theaters) {
    static std::unordered_set<TheaterId> fixed_theaters;
    
    if (fixed_theaters.empty()) {
        size_t count = 0;

        for(auto theater_id: theaters) {
            if (count>=10) {
                break;
            }
            ++ count;
            fixed_theaters.insert(theater_id);
        }
    }

    BookingData::get_instance().add_theaters_to_movie(movie_id, fixed_theaters);
}

unsigned fast_random() {
    // The C++ mersene twister is a fast quality pseudoRNG generator.
    // But we do not care about randomness quality here. Using this very simple RNG
    // we get more than a 50% reduction of cbs initialization time for this
    // fake test data.
    static unsigned state = 0x12345678U;
    state ^= (state << 13);
    state ^= (state >> 17);
    state ^= (state << 5);
    return state & 0x7F;
}

static void add_random_theaters_to_movie(MovieId movie_id, const std::set<TheaterId>& theaters) {
    std::unordered_set<TheaterId> random_theaters;

    for(auto theater_id: theaters) {
        if(fast_random()==0) { // 1/128 chance
            random_theaters.insert(theater_id);
        }
    }
    BookingData::get_instance().add_theaters_to_movie(movie_id, std::move(random_theaters));
}

static void init_theaters_with_movies() {
    auto movies = BookingData::get_instance().get_sorted_movie_ids();
    auto theaters = BookingData::get_instance().get_sorted_theater_ids();

    size_t count = 0;
    for (auto movie: movies) {
        if (count<10) {
            add_fixed_theaters_to_movie(movie, theaters);
        }
        else {
            add_random_theaters_to_movie(movie, theaters);
        }
        ++count;
    }
}

void init_booking_data() {
    std::cout << "Please wait while data is initialized." << std::endl;
    init_movie_list();
    init_theater_list();
    init_theaters_with_movies();
    std::cout << "Data has been initialized." << std::endl;
}