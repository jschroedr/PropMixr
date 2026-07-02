#pragma once
#include <string_view>
#include <vector>
#include <string>
#include <optional>
#include <variant>


/**
 * Represents a single step in a path, which can be either an element or an attribute.
 */
struct PathStep {
    std::string key;
    bool is_attribute = false;
};


/** 
 * Supported data formats
 */
enum class DataFormat {
    Json,
    Xml,
    Yaml
};

/** 
 * The allowed types for PropMixer to serialize
 */
using MixerTypes = std::variant<std::string, int>;

// Metadata helper to check if the type live inside MixerTypes
template <typename T, typename Variant>
struct is_variant_member;

template <typename T, typename ... Args>
struct is_variant_member<T, std::variant<Args...>> : std::disjunction<std::is_same<T, Args>...> {};

// Restriction concept - ensure types used are only what we support
template <typename T>
concept IsAllowedMixerType = is_variant_member<T, MixerTypes>::value;

/**
 * Using inline here to bypass One Definition Rule
 * (.cpp files that include this header file would redefine this function)
 */
inline std::vector<PathStep> parse_path(std::string_view path) {
    std::vector<PathStep> steps;
    
    // Immediately return if the path is empty or invalid
    if (path.empty() || path[0] != '/') {
        return steps;
    }

    size_t position = 0;
    while (position < path.size()) {
        // find the next slash in the path
        size_t next_slash = path.find('/', position);

        // slice the string token out of the path
        std::string_view token = path.substr(position, next_slash - position);

        if (!token.empty()) {
            // check if the token is an attribute
            if (token[0] == '@') {
                steps.push_back(
                    { 
                        std::string(token.substr(1)), 
                        true 
                    }
                );
            } else {
                // otherwise simply pass the token as a path
                steps.push_back(
                    { 
                        std::string(token), 
                        false 
                    }
                );
            }
        }

        if (next_slash == std::string_view::npos) { 
            break;
        }
        
        position = next_slash + 1;
    }


    return steps;
}


/**
 * Base class for serialization and deserialization
 */
class Pantry {
    public:
        // Use the default destructor (defer to compiler)
        // and ensure concrete classes are cleaned up before the base class 
        virtual ~Pantry () = default;

        template <IsAllowedMixerType T>
        std::optional<T> read(const std::vector<PathStep>& steps) {
            if constexpr (std::is_same_v<T, int>) {
                return read_int(steps);
            } else if constexpr (std::is_same_v<T, std::string>) {
                return read_string(steps);
            }
        }

    private:
        // Support for all MixerTypes enforced here
        virtual std::optional<int> read_int(const std::vector<PathStep>& steps) = 0;
        virtual std::optional<std::string> read_string(const std::vector<PathStep>& steps) = 0;

};
