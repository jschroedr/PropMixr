#include <iostream>
#include "propmixr.hpp"

// Layer 3: Deepest Nested Configuration block
struct OwnerSpecs {
    [[prop::path("/name")]]
    std::string name;
};

// Layer 2: Intermediate Object block
struct DatabaseConfig {
    [[prop::path("/host")]]
    std::string host;

    [[prop::path("/port")]]
    int port;

    [[prop::path("/meta")]] // Extends path down to "/server/connection/meta"
    OwnerSpecs owner;       // Double nested!
};

// Layer 1: Root Schema
struct AppConfig {
    [[prop::path("/server/connection")]] // Prefix base for internal components
    DatabaseConfig db;

    [[prop::path("/environment")]]
    std::string env;
};

int main() {
    std::string complex_json = R"({
        "server": {
            "connection": {
                "host": "127.0.0.1",
                "port": 5432,
                "meta": {
                    "name": "Admin-Jake"
                }
            }
        },
        "environment": "production"
    })";

    try {
        auto config = PropMixr::bake<AppConfig>(complex_json, DataFormat::Json);

        std::cout << "--- PropMixr Hierarchical Hydration Success ---\n";
        std::cout << "Env:        " << config.env << "\n";
        std::cout << "DB Host:    " << config.db.host << "\n";
        std::cout << "DB Port:    " << config.db.port << "\n";
        std::cout << "Owner Name: " << config.db.owner.name << "\n";

    } catch (const std::exception& e) {
        std::cerr << "Engine compilation runtime error: " << e.what() << "\n";
    }

    return 0;
}