/**
 * @file stringidmap.hpp
 * @brief Holds a map class with ulong as key and string as value which allows fast look ups of values.
 */

#ifndef STRINGIDMAP_HPP
#define STRINGIDMAP_HPP

#include <string>
#include <sstream>
#include <unordered_map>
#include <unordered_set>
#include <set>
#include <vector>
#include <memory>
#include <mutex>
#include <shared_mutex>

#include "common.hpp"

/**
 * @class StringIdMap
 * @brief Manages a collection of strings with unique IDs.
 *
 * This class provides functionality to add strings and associate them with
 * unique IDs, retrieve strings by their IDs, and maintain a list of all
 * strings and IDs. It uses efficient data structures to ensure fast lookups.
 */
class StringIdMap {
public:
    /**
     * @brief Constructor.
     */
    StringIdMap();

    /**
     * @brief Add a set of new strings and automatically assign unique IDs.
     * @param[in] strings A set of strings to be added to the map.
     * @return A vector of unique IDs assigned to the added strings. There is no guarantee about the order in which
     * the IDs are assigned to the input strings.
     * @throws std::invalid_argment when strings_in contains a string already present in the StringIdMap, in this
     * case the StringIdMap is guaranteed to reamin unmodified.
     */
    std::vector<unsigned long> add(const std::unordered_set<std::string> strings_in);

    /**
     * @brief Get the list of all IDs and corresponding strings as a shared pointer to a string.
     * @return A shared pointer to a constant string containing all IDs and strings. Each string is in a single line.
     * Each line contains the string ID, a comma and the string. Lines are ended with CBS_EOL
     * @note The list is cached and rebuilt when needed.
     */
    std::shared_ptr<const std::string> get_id_string_list() const;

    /**
     * @brief Check if a string ID exists in the map.
     * @param[in] string_id The ID to check for existence.
     * @return True if the ID exists in the map, false otherwise.
     */
    bool has_id(unsigned long string_id) const;

    /**
     * @brief Retrieve a string associated with a specific string ID.
     * @param[in] string_id The ID of the string to retrieve.
     * @return The string associated with the given ID.
     * @throws std::out_of_range If the ID does not exist.
     */
    std::string get_string(unsigned long string_id) const;

    /**
     * @brief Get all the keys (IDs).
     * @return A sorted set of IDs.
     */
    std::set<unsigned long> get_sorted_keys() const;

    /**
     * @brief Clear all stored strings and IDs.
     * @note This method resets the map and cache, effectively removing all data.
     */
    void clear();

private:
    /**
     * @brief Counter to assign unique IDs.
     */
    unsigned long next_id;

    /**
     * @brief Map that stores the mapping of ID to string.
     */
    std::unordered_map<unsigned long, const char*> id_strings;

    /**
     * @brief Set of strings currently stored in the map.
     * @note Strings are held in a separate set since look up of values in a map is slow.
     */
    std::unordered_set<std::string> strings;

    /**
     * @brief Cached list of IDs and strings.
     */
    mutable std::shared_ptr<std::string> cached_list;

    /**
     * @brief Rebuilds the cached list of strings and IDs.
     * @note This method is called internally to refresh the cache when needed.
     */
    void rebuild_cache();
};

#endif //STRINGIDMAP_HPP
