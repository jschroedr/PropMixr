#include <gtest/gtest.h>
#include "json_pantry.hpp"

TEST(JsonPantryTest, ExtractsValidPrimitives) {
    std::string raw_json = R"({"app": {"version": 2, "env": "prod"}})";
    JsonPantry pantry(raw_json);

    // Manually build path steps for "/app/env"
    std::vector<PathStep> string_path = {{"app", false}, {"env", false}};
    auto env_val = pantry.read_string(string_path);
    ASSERT_TRUE(env_val.has_value());
    EXPECT_EQ(*env_val, "prod");

    // Manually build path steps for "/app/version"
    std::vector<PathStep> int_path = {{"app", false}, {"version", false}};
    auto version_val = pantry.read_int(int_path);
    ASSERT_TRUE(version_val.has_value());
    EXPECT_EQ(*version_val, 2);
}

TEST(JsonPantryTest, HandlesTypeMismatchesSafely) {
    std::string raw_json = R"({"age": "thirty"})"; // String instead of Int
    JsonPantry pantry(raw_json);

    std::vector<PathStep> path = {{"age", false}};
    auto val = pantry.read_int(path);
    
    // Should return nullopt instead of throwing a hard crash
    EXPECT_FALSE(val.has_value()); 
}