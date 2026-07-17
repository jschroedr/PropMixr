#include <gtest/gtest.h>
#include "propmixr.hpp"
#include <algorithm>
#include <cctype>

namespace {

    // No annotations at all -> every field falls back to its identifier as the path.
    struct SimpleProfile {
        std::string name;
        int age;
    };

    // Explicit input_path override via the prop::path<T> annotation.
    struct CustomPathProfile {
        [[=prop::path<std::string>{.input_path = "/user_name"}]]
        std::string name;
    };

    struct Address {
        std::string city;
    };

    // Nested struct field, also exercising an annotation on a class-typed member.
    struct NestedProfile {
        std::string name;

        [[=prop::path<Address>{.input_path = "/address"}]]
        Address address;
    };

    // --- Custom deserialize callback ---
    //
    // enum class isn't std::string, isn't int, and isn't is_class_v, so the
    // built-in hydration branches in bake_internal can't touch it at all -
    // this is exactly the kind of type the deserialize callback exists for.
    enum class LogLevel { Debug, Info, Warn, Error };

    bool parse_log_level(std::string_view raw, LogLevel* out) {
        if (raw == "debug") { *out = LogLevel::Debug; return true; }
        if (raw == "info")  { *out = LogLevel::Info;  return true; }
        if (raw == "warn")  { *out = LogLevel::Warn;  return true; }
        if (raw == "error") { *out = LogLevel::Error; return true; }
        return false;
    }

    struct LoggingConfig {
        [[=prop::path<LogLevel>{.input_path = "/level", .deserialize = &parse_log_level}]]
        LogLevel level = LogLevel::Debug;
    };

    // Custom callback should take priority even when the field type is one
    // the built-in handling *would* otherwise know how to hydrate.
    bool shout(std::string_view raw, std::string* out) {
        out->resize(raw.size());
        std::transform(raw.begin(), raw.end(), out->begin(),
                        [](unsigned char c) { return std::toupper(c); });
        return true;
    }

    struct ShoutingProfile {
        [[=prop::path<std::string>{.input_path = "/name", .deserialize = &shout}]]
        std::string name;
    };

    // --- default_value fallback ---
    struct DefaultedProfile {
        [[=prop::path<std::string>{.input_path = "/name", .default_value = "Anonymous"}]]
        std::string name;

        [[=prop::path<int>{.input_path = "/age", .default_value = "42"}]]
        int age;
    };

    // default_value routed through a custom deserialize callback, so a
    // complex type's default gets parsed the same way a real value would.
    struct LoggingConfigWithDefault {
        [[=prop::path<LogLevel>{
            .input_path = "/level",
            .deserialize = &parse_log_level,
            .default_value = "warn"
        }]]
        LogLevel level = LogLevel::Debug;
    };

} // namespace

TEST(PropMixrEngineTest, HydratesFieldsByIdentifierWhenUnannotated) {
    std::string json = R"({"name": "Alice", "age": 30})";
    auto result = PropMixr::bake<SimpleProfile>(json, DataFormat::Json);

    EXPECT_EQ(result.name, "Alice");
    EXPECT_EQ(result.age, 30);
}

TEST(PropMixrEngineTest, HonorsExplicitInputPathAnnotation) {
    std::string json = R"({"user_name": "Bob"})";
    auto result = PropMixr::bake<CustomPathProfile>(json, DataFormat::Json);

    EXPECT_EQ(result.name, "Bob");
}

TEST(PropMixrEngineTest, LeavesFieldDefaultConstructedWhenKeyMissing) {
    std::string json = R"({})";
    auto result = PropMixr::bake<SimpleProfile>(json, DataFormat::Json);

    // SimpleProfile's fields carry no annotation at all, so there's no
    // default_value to fall back to - just the default-constructed value.
    EXPECT_EQ(result.name, "");
    EXPECT_EQ(result.age, 0);
}

TEST(PropMixrEngineTest, RecursesIntoNestedStructs) {
    std::string json = R"({"name": "Carol", "address": {"city": "Berlin"}})";
    auto result = PropMixr::bake<NestedProfile>(json, DataFormat::Json);

    EXPECT_EQ(result.name, "Carol");
    EXPECT_EQ(result.address.city, "Berlin");
}

TEST(DeserializationTest, InvokesCustomCallbackForTypeBuiltinHandlingCannotTouch) {
    std::string json = R"({"level": "warn"})";
    auto result = PropMixr::bake<LoggingConfig>(json, DataFormat::Json);

    EXPECT_EQ(result.level, LogLevel::Warn);
}

TEST(DeserializationTest, LeavesFieldAtDefaultWhenCallbackRejectsValue) {
    std::string json = R"({"level": "bogus"})";
    auto result = PropMixr::bake<LoggingConfig>(json, DataFormat::Json);

    // parse_log_level returns false for anything it doesn't recognize, so
    // the field should be left at its own default member initializer.
    EXPECT_EQ(result.level, LogLevel::Debug);
}

TEST(DeserializationTest, LeavesFieldAtDefaultWhenKeyMissing) {
    std::string json = R"({})";
    auto result = PropMixr::bake<LoggingConfig>(json, DataFormat::Json);

    // No "level" key at all -> deserialize is never called.
    EXPECT_EQ(result.level, LogLevel::Debug);
}

TEST(DeserializationTest, CustomCallbackTakesPriorityOverBuiltinStringHandling) {
    std::string json = R"({"name": "alice"})";
    auto result = PropMixr::bake<ShoutingProfile>(json, DataFormat::Json);

    // Without the callback this would just be "alice" verbatim - its
    // presence must override the built-in std::string hydration branch.
    EXPECT_EQ(result.name, "ALICE");
}

TEST(DefaultValueTest, FallsBackToDefaultStringWhenKeyMissing) {
    std::string json = R"({})";
    auto result = PropMixr::bake<DefaultedProfile>(json, DataFormat::Json);

    EXPECT_EQ(result.name, "Anonymous");
    EXPECT_EQ(result.age, 42);
}

TEST(DefaultValueTest, PresentValueTakesPriorityOverDefault) {
    std::string json = R"({"name": "Dave", "age": 19})";
    auto result = PropMixr::bake<DefaultedProfile>(json, DataFormat::Json);

    EXPECT_EQ(result.name, "Dave");
    EXPECT_EQ(result.age, 19);
}

TEST(DefaultValueTest, PartiallyMissingFieldsEachFallBackIndependently) {
    std::string json = R"({"name": "Erin"})"; // "age" omitted
    auto result = PropMixr::bake<DefaultedProfile>(json, DataFormat::Json);

    EXPECT_EQ(result.name, "Erin");
    EXPECT_EQ(result.age, 42);
}

TEST(DefaultValueTest, RoutesDefaultThroughCustomDeserializeCallback) {
    std::string json = R"({})";
    auto result = PropMixr::bake<LoggingConfigWithDefault>(json, DataFormat::Json);

    // Distinct from LogLevel's own default member initializer (Debug) -
    // this proves the annotation's default_value was actually parsed via
    // parse_log_level, not just left untouched.
    EXPECT_EQ(result.level, LogLevel::Warn);
}
