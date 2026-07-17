#pragma once
#include "pantry.hpp"
#include <ryml.hpp>
#include <ryml_std.hpp>   // std::string/std::string_view interop (to_csubstr, from_chars)

/**
 * Shared Base for Ryml-driven Pantry implementations.
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

    private:

        ryml::Tree root;

        std::optional<std::string> read_string(const std::vector<PathStep>& steps) override {
            return navigate<std::string>(steps);
        }

        std::optional<int> read_int(const std::vector<PathStep>& steps) override {
            return navigate<int>(steps);
        }

        /**
         * Crawl the YAML tree to the path provided.
         */
        template <typename T>
        std::optional<T> navigate(const std::vector<PathStep> &steps) {
            ryml::NodeRef cursor = root.rootref();
            for (const auto& step : steps) {
                cursor = cursor[ryml::to_csubstr(step.key)];
                // invalid() alone misses "seed" nodes: NodeRef::operator[]
                // returns a seed placeholder (not invalid, but not readable)
                // when the key doesn't exist yet. readable() catches both.
                if (!cursor.readable()) {
                    return std::nullopt;
                }
            }
            if (!cursor.has_val()) {
                return std::nullopt;
            }
            // deserialize() reports failure via ReadResult instead of the
            // error callback, keeping type-mismatch on the nullopt path.
            T value;
            if (!cursor.deserialize(&value)) {
                return std::nullopt;
            }
            return value;
        }
};