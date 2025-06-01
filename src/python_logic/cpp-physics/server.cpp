#include "include/PhysicsEngine.h"
#include <httplib.h>
#include <memory>
#include <thread>
#include <iostream>
#include <chrono>

int main() {
    using namespace TetrisTowers;
    
    auto engine = std::make_unique<PhysicsEngine>();
    
    // Создаем HTTP сервер
    httplib::Server svr;
    
    // Эндпоинт для проверки здоровья
    svr.Get("/health", [](const httplib::Request&, httplib::Response& res) {
        res.set_content("OK", "text/plain");
    });
    
    // Эндпоинт для получения состояния
    svr.Get("/state", [&engine](const httplib::Request&, httplib::Response& res) {
        res.set_content(engine->exportStateToJson(), "application/json");
    });
    
    // Эндпоинт для установки состояния
    svr.Post("/state", [&engine](const httplib::Request& req, httplib::Response& res) {
        if (engine->importStateFromJson(req.body)) {
            res.set_content("OK", "text/plain");
        } else {
            res.status = 400;
            res.set_content("Invalid state", "text/plain");
        }
    });
    
    // Запускаем сервер в отдельном потоке
    std::thread server_thread([&svr]() {
        if (!svr.listen("0.0.0.0", 9000)) {
            std::cerr << "Failed to start server" << std::endl;
        }
    });
    
    // Основной цикл обновления физики
    while (true) {
        engine->update(1.0f / 60.0f);
        std::this_thread::sleep_for(std::chrono::milliseconds(16));
    }
    
    return 0;
} 