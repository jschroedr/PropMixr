#pragma once
#include "pantry.hpp"
#include "json_pantry.hpp"
#include <meta>            // Standard C++26 Reflection header
#include <memory>
#include <type_traits>
#include <stdexcept>

// Define our standard attribute namespace
namespace prop {
    struct path { std::string_view value; };
}

namespace PropMixr {
    namespace detail {

        // Standard C++26 attribute value reader
        template <typename T, auto MemberToken>
        constexpr std::string_view extract_path_attribute() {
            constexpr auto annotations = std::meta::annotations_of(MemberToken);
            template for (constexpr auto attr : annotations) {
                if constexpr (std::meta::type_of(attr) == ^^prop::path) {
                    constexpr prop::path p = std::meta::extract<prop::path>(attr);
                    return p.value;
                }
            }
            return ""; 
        }

        template <typename T>
        T bake_internal(PantryReader& pantry, std::string_view path_prefix) {
            T obj;
            constexpr auto type_meta = ^^T; 

            // FIX: Use std::define_static_array to safely capture the transient 
            // heap vector and instantly freeze it into static compiler data.
            // Wrap the target loop collection in auto() to yield a pure literal sequence.
            template for (constexpr auto member : auto(std::define_static_array(
                std::meta::members_of(type_meta, std::meta::access_context::unchecked())
            ))) {
                
                if constexpr (std::meta::is_variable(member)) {
                    constexpr std::string_view attr_path = extract_path_attribute<T, member>();
                    std::string local_path = "";
                    
                    if (!attr_path.empty()) {
                        local_path = std::string(attr_path);
                    } else {
                        local_path = "/" + std::string(std::meta::identifier_of(member));
                    }

                    std::string full_path = std::string(path_prefix) + local_path;
                    auto& field_ref = obj.[:member:];
                    using FieldType = std::decay_t<decltype(field_ref)>;

                    // --- Hydration Layer Execution ---
                    if constexpr (std::is_same_v<FieldType, std::string>) {
                        auto steps = parse_path(full_path);
                        auto val = pantry.read_string(steps);
                        if (val) field_ref = std::move(*val);
                    } 
                    else if constexpr (std::is_same_v<FieldType, int>) {
                        auto steps = parse_path(full_path);
                        auto val = pantry.read_int(steps);
                        if (val) field_ref = *val;
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
    T bake(std::string_view raw_data, DataFormat format) {
        std::unique_ptr<PantryReader> pantry;
        
        if (format == DataFormat::Json) {
            pantry = std::make_unique<JsonPantry>(raw_data);
        } else {
            throw std::runtime_error("Unsupported format engine backend.");
        }

        return detail::bake_internal<T>(*pantry, "");
    };
}