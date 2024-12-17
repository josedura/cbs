#include <gtest/gtest.h>
#include <unordered_set>
#include <stdexcept>

#include "stringidmap.cpp"

class StringIdMapTest : public ::testing::Test {
protected:
    StringIdMap id_map;

    void SetUp() override {
        // This will run before each test case
    }

    void TearDown() override {
        // This will run after each test case
    }
};

// Test adding strings and assigning unique IDs
TEST_F(StringIdMapTest, AddUniqueStrings) {
    std::unordered_set<std::string> strings = {"Terminator", "The Matrix", "The Flintstones"};

    auto inserted_ids = id_map.add(strings);

    ASSERT_EQ(inserted_ids.size(), 3);
    ASSERT_TRUE(id_map.has_id(inserted_ids[0]));
    ASSERT_TRUE(id_map.has_id(inserted_ids[1]));
    ASSERT_TRUE(id_map.has_id(inserted_ids[2]));

    // Test if the strings can be retrieved correctly by ID
    std::unordered_set<std::string> retrieved_strings{};
    retrieved_strings.insert(id_map.get_string(inserted_ids[0]));
    retrieved_strings.insert(id_map.get_string(inserted_ids[1]));
    retrieved_strings.insert(id_map.get_string(inserted_ids[2]));
    ASSERT_EQ(strings, retrieved_strings);
}

// Test that adding a duplicate string throws an exception
TEST_F(StringIdMapTest, AddDuplicateStringThrowsException) {
    std::unordered_set<std::string> strings = {"Terminator", "The Matrix", "The Flintstones"};
    id_map.add(strings);

    // Try adding a duplicate string "Terminator"
    std::unordered_set<std::string> duplicate_strings = {"Terminator"};
    
    EXPECT_THROW(id_map.add(duplicate_strings), std::invalid_argument);
}

// Test retrieving the sorted keys
TEST_F(StringIdMapTest, GetSortedKeys) {
    std::unordered_set<std::string> strings = {"Terminator", "The Matrix", "The Flintstones"};
    auto inserted_ids = id_map.add(strings);

    std::set<unsigned long> sorted_keys = id_map.get_sorted_keys();

    ASSERT_EQ(sorted_keys.size(), 3);
    auto it = sorted_keys.begin();
    EXPECT_EQ(*it, inserted_ids[0]);
    ++it;
    EXPECT_EQ(*it, inserted_ids[1]);
    ++it;
    EXPECT_EQ(*it, inserted_ids[2]);
}

// Test retrieving the cached ID-string list
TEST_F(StringIdMapTest, GetIdStringList) {
    std::unordered_set<std::string> strings = {"Terminator", "The Matrix", "The Flintstones"};
    id_map.add(strings);

    auto cached_list = id_map.get_id_string_list();

    ASSERT_NE(cached_list, nullptr);
    EXPECT_TRUE(cached_list->find("Terminator") != std::string::npos);
    EXPECT_TRUE(cached_list->find("The Matrix") != std::string::npos);
    EXPECT_TRUE(cached_list->find("The Flintstones") != std::string::npos);
}

// Test clearing the ID map
TEST_F(StringIdMapTest, ClearIdMap) {
    std::unordered_set<std::string> strings = {"Terminator", "The Matrix", "The Flintstones"};
    id_map.add(strings);

    // Clear the map
    id_map.clear();

    // Check if the map is empty
    EXPECT_EQ(id_map.get_sorted_keys().size(), 0);
    EXPECT_EQ(id_map.get_id_string_list()->size(), 0);
}

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
