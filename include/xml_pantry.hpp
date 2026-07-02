#pragma once
#include "pantry.hpp"
#include "pugixml.hpp"
#include <charconv>


class XmlPantry : public Pantry
{

    public:

        explicit XmlPantry(std::string_view raw) {
            pugi::xml_parse_result result = root.load_string(raw.data(), raw.size());
            if (result.status != pugi::xml_parse_status::status_ok) {
                throw new std::invalid_argument("Invalid XML");
            }
        }

    private:
        
        pugi::xml_document root;

        std::optional<std::string> read_string(const std::vector<PathStep>& steps) override {
            pugi::xml_node node = navigate(steps);
            return node.text().as_string();
        }

        std::optional<int> read_int(const std::vector<PathStep>& steps) override {
            pugi::xml_node node = navigate(steps);
            return node.text().as_int();
        }

        pugi::xml_node navigate(const std::vector<PathStep> &steps) {
            pugi::xml_node current = root;
            for (const auto& step : steps) {
                current = current.child(step.key.c_str());
                if (!current) {
                    break;
                }
            }
            return current;
        }

};