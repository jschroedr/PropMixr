#pragma once
#include "ryml_pantry.hpp"

/**
 * YAML Implementation of Pantry.
 */
class YamlPantry : public RymlPantry {

    public:
        explicit YamlPantry(std::string_view raw) : RymlPantry(raw, RymlPantry::Format::YAML) {}
};
