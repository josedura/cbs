#ifndef INITBOOKINGDATA_HPP
#define INITBOOKINGDATA_HPP

/**
 * @file initbookingdata.hpp
 * @brief Initializes the BookingData singleton.
 * 
 * Initializes booking data with around 10000 movies, 10000 theaters and 1 million combinations of movie/theater.
 * This is fake data for the purpose of testing cbs. Future developments may initialize data from a database or
 * other mechanisms.
 */


/**
 * @brief Initializes BookingData singletone wiht fake data for testing purposes.
 */
void init_booking_data();

#endif //INITBOOKINGDATA_HPP