#pragma once
#include "ryml_pantry.hpp"

/**
 * JSON Implementation of Pantry.
 */
class JsonPantry : public RymlPantry {

    public:
        explicit JsonPantry(std::string_view raw) : RymlPantry(raw, RymlPantry::Format::JSON) {}
};
