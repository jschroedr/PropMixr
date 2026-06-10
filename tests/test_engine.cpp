#include <gtest/gtest.h>
#include "propmixr.hpp"

struct TestProfile {
    [[prop::path("/name")]]  std::string name;
    [[prop::path("/age")]]   int age;
    
    // Field with a default value attribute logic
    [[prop::path("/role")]] 
    [[prop::default_value("guest")]] 
    std::string role;

    // Field with size validation constraints
    [[prop::path("/code")]] 
    [[prop::max_size(4)]] 
    std::string tracking_code;
};

TEST(PropMixrEngineTest, SuccessfulBakeWithAllFields) {
    std::string json = R"({
        "name": "Alice",
        "age": 30,
        "role": "admin",
        "code": "ABCD"
    })";

    auto result = PropMixr::bake<TestProfile>(json, DataFormat::Json);

    EXPECT_EQ(result.name, "Alice");
    EXPECT_EQ(result.age, 30);
    EXPECT_EQ(result.role, "admin");
    EXPECT_EQ(result.tracking_code, "ABCD");
}

TEST(PropMixrEngineTest, ActivatesDefaultValueWhenKeyIsMissing) {
    std::string json = R"({
        "name": "Bob",
        "age": 25,
        "code": "XYZ"
    })"; // "role" is omitted entirely

    auto result = PropMixr::bake<TestProfile>(json, DataFormat::Json);

    EXPECT_EQ(result.name, "Bob");
    EXPECT_EQ(result.role, "guest"); // Successfully caught default fallback
}

TEST(PropMixrEngineTest, ThrowsExceptionOnSizeConstraintViolation) {
    std::string json = R"({
        "name": "Charlie",
        "age": 40,
        "code": "LONG_CODE_ERR" // Exceeds our max_size(4) constraint
    })";

    // Verify that the engine accurately propagates a validation error
    EXPECT_THROW({
        PropMixr::bake<TestProfile>(json, DataFormat::Json);
    }, std::runtime_error);
}