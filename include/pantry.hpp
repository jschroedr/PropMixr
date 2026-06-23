#pragma once
#include <string_view>
#include <vector>
#include <string>
#include <optional>


/**
 * Represents a single step in a path, which can be either an element or an attribute.
 */
struct PathStep {
    std::string key;
    bool is_attribute = false;
};


enum class DataFormat {
    Json,
    Xml,
    Yaml
};


/**
 * Represents the direction of a PropertyMapping
 */
// enum class Direction {
//     INPUT,
//     OUTPUT,
//     INPUT_OUTPUT,
// };


// why are we using inline?

inline std::vector<PathStep> parse_path(std::string_view path) {
    std::vector<PathStep> steps;
    
    // Immediately return if the path is empty or invalid
    if (path.empty() || path[0] != '/') {
        return steps;
    }

    size_t position;
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

        /**
         * Find the string value at the path provided.
         */
        virtual std::optional<std::string> read_string(const std::vector<PathStep>& steps) = 0;

        /**
         * Find the int value at the path provided.
         */
        virtual std::optional<int> read_int(const std::vector<PathStep>& steps) = 0;

};
