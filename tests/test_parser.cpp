#include <gtest/gtest.h>
#include "pantry.hpp"

TEST(PathParserTest, HandlesStandardJsonPointers) {
    auto steps = parse_path("/user/settings/theme");
    
    ASSERT_EQ(steps.size(), 3);
    EXPECT_EQ(steps[0].key, "user");      EXPECT_FALSE(steps[0].is_attribute);
    EXPECT_EQ(steps[1].key, "settings");  EXPECT_FALSE(steps[1].is_attribute);
    EXPECT_EQ(steps[2].key, "theme");     EXPECT_FALSE(steps[2].is_attribute);
}

TEST(PathParserTest, HandlesBadgerfishAttributes) {
    auto steps = parse_path("/server/meta/@id");
    
    ASSERT_EQ(steps.size(), 3);
    EXPECT_EQ(steps[2].key, "id");
    EXPECT_TRUE(steps[2].is_attribute); // Successfully caught the XML attribute rule!
}

TEST(PathParserTest, ReturnsEmptyOnInvalidPointer) {
    // Missing leading slash
    auto steps = parse_path("invalid/path");
    EXPECT_TRUE(steps.empty());
}

TEST(PathParserTest, ReturnsEmptyOnRootOnlyPointer) {
    auto steps = parse_path("/");
    EXPECT_TRUE(steps.empty());
}

TEST(PathParserTest, ReturnsEmptyOnEmptyString) {
    auto steps = parse_path("");
    EXPECT_TRUE(steps.empty());
}