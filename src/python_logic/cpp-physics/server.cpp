#include "include/PhysicsEngine.h"
#include <httplib.h>
#include <iostream>
#include <thread>
#include <chrono>
#include <nlohmann/json.hpp>

using namespace TetrisTowers;
using json = nlohmann::json;

int main() {
    std::cout << "Starting physics server..." << std::endl;
    
    // Initialize physics engine
    PhysicsEngine engine;
    engine.initialize();
    
    // Create HTTP server
    httplib::Server svr;
    
    // Health check endpoint
    svr.Get("/health", [](const httplib::Request&, httplib::Response& res) {
        res.set_content("OK", "text/plain");
    });
    
    // Get physics state
    svr.Get("/state", [&engine](const httplib::Request&, httplib::Response& res) {
        std::string state = engine.exportStateToJson();
        res.set_content(state, "application/json");
    });
    
    // Update physics state
    svr.Post("/state", [&engine](const httplib::Request& req, httplib::Response& res) {
        bool success = engine.importStateFromJson(req.body);
        if (success) {
            res.set_content("{\"status\":\"success\"}", "application/json");
        } else {
            res.status = 400;
            res.set_content("{\"status\":\"error\",\"message\":\"Invalid state format\"}", "application/json");
        }
    });
    
    // Start server in a separate thread
    std::thread server_thread([&svr]() {
        svr.listen("0.0.0.0", 9000);
    });
    
    // Main physics update loop
    constexpr float fixedDelta = 1.0f / 60.0f; // Fixed time step of 1/60 second
    while (true) {
        engine.update(fixedDelta);
        std::this_thread::sleep_for(std::chrono::milliseconds(16)); // ~60 FPS
    }
    
    return 0;
} 