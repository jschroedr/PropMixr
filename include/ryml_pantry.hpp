#pragma once
#include "pantry.hpp"
#include <ryml.hpp>

/**
 * YAML Implementation of Pantry.
 */
class RymlPantry : public Pantry
{

    public:
        enum class Format {
            YAML,
            JSON
        };

        explicit RymlPantry(std::string_view raw, Format format) {
            if (format == Format::JSON) {
                root = ryml::parse_json_in_arena(ryml::to_csubstr(raw));
            } else {
                root = ryml::parse_in_arena(ryml::to_csubstr(raw));
            }
        }

        std::optional<std::string> read_string(const std::vector<PathStep>& steps) override {
            return navigate<std::string>(steps);
        }

        std::optional<int> read_int(const std::vector<PathStep>& steps) override {
            return navigate<int>(steps);
        }

    private:

        ryml::Tree root;

        /**
         * Crawl the YAML tree to the path provided.
         */
        template <typename T>
        std::optional<T> navigate(const std::vector<PathStep> &steps) {
            ryml::NodeRef cursor = root.rootref();
            for (const auto& step : steps) {
                cursor = cursor[ryml::to_csubstr(step.key)];
                if (!cursor.valid()) {
                    return std::nullopt;
                }
            }
            if (!cursor.has_val()) {
                return std::nullopt;
            }
            // ryml requires the stream extraction operator
            T value;
            cursor >> value;
            return value;
        }
        
};