/**
 * @file stringidmap.cpp
 * @brief Implementation of the StringIdMap class that manages string-to-ID mapping.
 */

#include "stringidmap.hpp"

StringIdMap::StringIdMap()
    :next_id(1)
    ,cached_list(new std::string(""))
{}

std::vector<unsigned long> StringIdMap::add(const std::unordered_set<std::string> strings_in) {
    // Check if any string already exists
    for (auto& name : strings_in)
    {
        if (strings.find(name) != strings.cend()) {
            throw std::invalid_argument("Name already exists.");
        }
    }
    // Insert the strings with a unique ID
    std::vector<unsigned long> inserted_ids;
    for (auto& name : strings_in)
    {
        auto result = strings.insert(std::move(name));
        id_strings[next_id] = result.first->c_str();
        inserted_ids.push_back(next_id);
        ++next_id;
    }

    rebuild_cache();
    return inserted_ids;
}

std::shared_ptr<const std::string> StringIdMap::get_id_string_list() const {
    return cached_list;
}

bool StringIdMap::has_id(unsigned long string_id) const {
    return id_strings.find(string_id) != id_strings.end();
}

std::string StringIdMap::get_string(unsigned long string_id) const {
    return id_strings.at(string_id);
}

std::set<unsigned long> StringIdMap::get_sorted_keys() const {
    std::set<unsigned long> keys;

    for (const auto& [id, str] : id_strings) {
        keys.insert(id);
    }
    return keys;
}

void StringIdMap::rebuild_cache()
{
    std::stringstream ss;
    for(const auto& [id, str]: id_strings)
    {
        ss << id << "," << str << CBS_EOL;
    }
    cached_list = std::make_shared<std::string>(ss.str());
}

void StringIdMap::clear() {
    id_strings.clear();
    strings.clear();
    
    rebuild_cache();
}
