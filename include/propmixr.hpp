#pragma once
#include "pantry.hpp"
#include "json_pantry.hpp"
#include "yaml_pantry.hpp"
#include "xml_pantry.hpp"
#include <meta>            // Standard C++26 Reflection header
#include <memory>
#include <type_traits>
#include <stdexcept>
#include <charconv>

namespace prop {

    // A fixed-capacity, inline (no heap, no pointer) string, used for
    // annotation values. Annotations must be structural types, which rules
    // out std::string/std::string_view/std::optional (private members, and
    // std::string additionally can't smuggle heap storage out of a single
    // constant evaluation). It also rules out a bare const char*: this
    // compiler's std::meta::extract can't reconstruct a pointer into
    // string-literal storage once it's wrapped in a class template. A plain
    // inline char array sidesteps all of that.
    template <std::size_t N>
    struct fixed_string {
        char data[N] = {};
        std::size_t size = 0;

        constexpr fixed_string() = default;

        // Truncates rather than failing to compile if the literal (minus
        // its trailing null) doesn't fit in this capacity.
        template <std::size_t M>
        constexpr fixed_string(const char (&s)[M]) {
            size = (M - 1 <= N) ? (M - 1) : N;
            for (std::size_t i = 0; i < size; ++i) data[i] = s[i];
        }

        constexpr operator std::string_view() const { return std::string_view(data, size); }
        constexpr bool empty() const { return size == 0; }
    };

    // A path string is capped at 255 characters - the classic VARCHAR(255)
    // middle ground.
    inline constexpr std::size_t path_capacity = 255;

    template <typename T>
    struct path {
        // Input configuration
        fixed_string<path_capacity> input_path;
        bool (*deserialize)(std::string_view raw, T* out) = nullptr;
        // Output configuration
        fixed_string<path_capacity> output_path;
        bool (*serialize)(T* in, std::string& out) = nullptr;
        // Default value
        fixed_string<path_capacity> default_value;
    };
}

namespace PropMixr {
    namespace detail {

        /**
         * Inspect the data member of the class and see if the prop::path attribute is set.
         */
        template <auto MemberToken>
        constexpr auto get_path_attribute() -> std::optional<prop::path<typename [: std::meta::type_of(MemberToken) :]>> {
            // The field's type, recovered from the member itself.
            using FieldType = typename [: std::meta::type_of(MemberToken) :];
            constexpr auto wanted = std::meta::substitute(^^prop::path, {std::meta::type_of(MemberToken)});
            template for (constexpr auto annotation : auto(std::define_static_array(std::meta::annotations_of(MemberToken)))) {
                // Annotation values are const, so type_of() reflects
                // `const prop::path<T>`; strip the qualifier before
                // comparing against `wanted`, which has none.
                constexpr auto attribute = std::meta::remove_const(std::meta::type_of(annotation));
                if constexpr (std::meta::has_template_arguments(attribute) && std::meta::template_of(attribute) == ^^prop::path) {
                    static_assert(attribute == wanted, "prop::path<T> annotation's T must match the field type");
                    return std::optional<prop::path<FieldType>>{std::meta::extract<prop::path<FieldType>>(annotation)};
                }
            }
            return std::optional<prop::path<FieldType>>{std::nullopt};
        }

        template <typename T>
        T bake_internal(Pantry& pantry, std::string_view path_prefix) {
            T obj;
            constexpr auto type_meta = ^^T; 

            // FIX: Use std::define_static_array to safely capture the transient 
            // heap vector and instantly freeze it into static compiler data.
            // Wrap the target loop collection in auto() to yield a pure literal sequence.
            template for (constexpr auto member : auto(std::define_static_array(
                std::meta::members_of(type_meta, std::meta::access_context::unchecked())
            ))) {
                // is_variable() is for declarations with independent storage
                // duration (static/thread/automatic) - a non-static data
                // member has no storage of its own, so it doesn't qualify.
                // is_nonstatic_data_member() is the correct predicate here.
                if constexpr (std::meta::is_nonstatic_data_member(member)) {
                    constexpr auto attr_path = get_path_attribute<member>();
                    std::string local_path = "";
                    
                    if (attr_path != std::nullopt) {
                        local_path = std::string(attr_path->input_path);
                    } else {
                        local_path = "/" + std::string(std::meta::identifier_of(member));
                    }

                    std::string full_path = std::string(path_prefix) + local_path;
                    auto& field_ref = obj.[:member:];
                    using FieldType = std::decay_t<decltype(field_ref)>;

                    // --- Hydration Layer Execution ---
                    if constexpr (attr_path.has_value() && attr_path->deserialize != nullptr) {
                        // A custom deserialize callback handles conversion for
                        // types the built-in scalar/class handling below
                        // doesn't know about (enums, domain types, etc.), and
                        // takes priority whenever one is supplied.
                        auto steps = parse_path(full_path);
                        auto raw = pantry.read<std::string>(steps);
                        if (raw) {
                            attr_path->deserialize(*raw, &field_ref);
                        } else if (!attr_path->default_value.empty()) {
                            // Route the default through the same codec, so a
                            // complex type's default is parsed the same way
                            // a real value would be.
                            attr_path->deserialize(std::string_view(attr_path->default_value), &field_ref);
                        }
                    }
                    else if constexpr (std::is_same_v<FieldType, std::string>) {
                        auto steps = parse_path(full_path);
                        auto val = pantry.read<std::string>(steps);
                        if (val) {
                            field_ref = std::move(*val);
                        } else if (attr_path.has_value() && !attr_path->default_value.empty()) {
                            field_ref = std::string(attr_path->default_value);
                        }
                    }
                    else if constexpr (std::is_same_v<FieldType, int>) {
                        auto steps = parse_path(full_path);
                        auto val = pantry.read<int>(steps);
                        if (val) {
                            field_ref = *val;
                        } else if (attr_path.has_value() && !attr_path->default_value.empty()) {
                            std::string_view dv = attr_path->default_value;
                            int parsed = 0;
                            auto res = std::from_chars(dv.data(), dv.data() + dv.size(), parsed);
                            if (res.ec == std::errc{}) field_ref = parsed;
                        }
                    }
                    else if constexpr (std::is_class_v<FieldType>) {
                        field_ref = bake_internal<FieldType>(pantry, full_path);
                    }
                }
            }
            return obj;
        }
    } // namespace detail

    template <typename T>
    T bake(std::string_view raw_data, DataFormat format, std::string_view path_prefix = "") {
        std::unique_ptr<Pantry> pantry;
        
        if (format == DataFormat::Json) {
            pantry = std::make_unique<JsonPantry>(raw_data);
        } else if (format == DataFormat::Yaml) {
            pantry = std::make_unique<YamlPantry>(raw_data);
        } else if (format == DataFormat::Xml) {
            pantry = std::make_unique<XmlPantry>(raw_data);
        } else {
            throw std::runtime_error("Unsupported format engine backend.");
        }
        return detail::bake_internal<T>(*pantry, path_prefix);
    };
}