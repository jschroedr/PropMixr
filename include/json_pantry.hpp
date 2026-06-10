#pragma once
#include "pantry.hpp"
#include <nlohmann/json.hpp>

class JsonPantry : public PantryReader {
private:
    nlohmann::json root;

    // Helper to safely navigate through the JSON tree using our parsed steps
    const nlohmann::json* navigate(const std::vector<PathStep>& steps) {
        const nlohmann::json* current = &root;
        for (const auto& step : steps) {
            if (!current->contains(step.key)) return nullptr;
            current = &((*current)[step.key]);
        }
        return current;
    }

public:
    explicit JsonPantry(std::string_view raw_json) {
        root = nlohmann::json::parse(raw_json);
    }

    std::optional<std::string> read_string(const std::vector<PathStep>& steps) override {
        auto* node = navigate(steps);
        if (node && node->is_string()) return node->get<std::string>();
        return std::nullopt;
    }

    std::optional<int> read_int(const std::vector<PathStep>& steps) override {
        auto* node = navigate(steps);
        if (node && node->is_number_integer()) return node->get<int>();
        return std::nullopt;
    }
};