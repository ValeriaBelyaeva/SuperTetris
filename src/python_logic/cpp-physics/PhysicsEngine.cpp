#include "include/PhysicsEngine.h"
#include <algorithm>
#include <random>
#include <sstream>
#include <iomanip>
#include <chrono>
#include <nlohmann/json.hpp>

namespace TetrisTowers {

// Конструктор
PhysicsEngine::PhysicsEngine() 
    : m_gravity(0.0f, -9.8f), 
      m_timeStep(1.0f/60.0f),
      m_velocityIterations(8),
      m_positionIterations(3),
      m_isRunning(false),
      m_isPaused(false) {
}

// Деструктор
PhysicsEngine::~PhysicsEngine() {
    shutdown();
}

// Инициализация движка
bool PhysicsEngine::initialize() {
    m_bodies.clear();
    m_contacts.clear();
    m_gravity = Vector2(0.0f, -9.8f);
    m_isRunning = false;
    m_isPaused = false;
    return true;
}

// Завершение работы движка
void PhysicsEngine::shutdown() {
    stopSimulation();
    m_bodies.clear();
    m_contacts.clear();
}

// Обновление физики
void PhysicsEngine::update(float deltaTime) {
    if (m_isPaused) return;
    
    std::lock_guard<std::mutex> lock(m_mutex);
    
    // Обнаружение коллизий
    detectCollisions();
    
    // Разрешение коллизий
    resolveCollisions();
    
    // Интеграция физики
    integrate(deltaTime);
}

// Создание нового тела
std::string PhysicsEngine::createBody(const PhysicsBody& body) {
    std::string id = generateUniqueId();
    PhysicsBody newBody = body;
    newBody.id = id;
    
    std::lock_guard<std::mutex> lock(m_mutex);
    m_bodies[id] = std::make_unique<PhysicsBody>(newBody);
    return id;
}

// Получение тела по ID
PhysicsBody* PhysicsEngine::getBody(const std::string& id) {
    std::lock_guard<std::mutex> lock(m_mutex);
    auto it = m_bodies.find(id);
    if (it != m_bodies.end()) {
        return it->second.get();
    }
    return nullptr;
}

// Удаление тела
bool PhysicsEngine::removeBody(const std::string& id) {
    std::lock_guard<std::mutex> lock(m_mutex);
    auto it = m_bodies.find(id);
    if (it != m_bodies.end()) {
        m_bodies.erase(it);
        return true;
    }
    return false;
}

// Создание тетромино заданного типа
Tetromino PhysicsEngine::createTetromino(TetrominoType type, const Vector2& position, float rotation) {
    Tetromino tetromino(type);
    
    // Размер блока
    const float blockSize = 1.0f;
    
    // Определение формы тетромино в зависимости от типа
    std::vector<Vector2> blockPositions;
    
    switch (type) {
        case TetrominoType::I:
            // I-тетромино (4 блока в линию)
            blockPositions = {
                Vector2(-1.5f * blockSize, 0.0f),
                Vector2(-0.5f * blockSize, 0.0f),
                Vector2(0.5f * blockSize, 0.0f),
                Vector2(1.5f * blockSize, 0.0f)
            };
            break;
            
        case TetrominoType::J:
            // J-тетромино
            blockPositions = {
                Vector2(-1.0f * blockSize, 0.5f * blockSize),
                Vector2(-1.0f * blockSize, -0.5f * blockSize),
                Vector2(0.0f * blockSize, -0.5f * blockSize),
                Vector2(1.0f * blockSize, -0.5f * blockSize)
            };
            break;
            
        case TetrominoType::L:
            // L-тетромино
            blockPositions = {
                Vector2(-1.0f * blockSize, -0.5f * blockSize),
                Vector2(0.0f * blockSize, -0.5f * blockSize),
                Vector2(1.0f * blockSize, -0.5f * blockSize),
                Vector2(1.0f * blockSize, 0.5f * blockSize)
            };
            break;
            
        case TetrominoType::O:
            // O-тетромино (квадрат)
            blockPositions = {
                Vector2(-0.5f * blockSize, -0.5f * blockSize),
                Vector2(-0.5f * blockSize, 0.5f * blockSize),
                Vector2(0.5f * blockSize, -0.5f * blockSize),
                Vector2(0.5f * blockSize, 0.5f * blockSize)
            };
            break;
            
        case TetrominoType::S:
            // S-тетромино
            blockPositions = {
                Vector2(-1.0f * blockSize, -0.5f * blockSize),
                Vector2(0.0f * blockSize, -0.5f * blockSize),
                Vector2(0.0f * blockSize, 0.5f * blockSize),
                Vector2(1.0f * blockSize, 0.5f * blockSize)
            };
            break;
            
        case TetrominoType::T:
            // T-тетромино
            blockPositions = {
                Vector2(-1.0f * blockSize, -0.5f * blockSize),
                Vector2(0.0f * blockSize, -0.5f * blockSize),
                Vector2(1.0f * blockSize, -0.5f * blockSize),
                Vector2(0.0f * blockSize, 0.5f * blockSize)
            };
            break;
            
        case TetrominoType::Z:
            // Z-тетромино
            blockPositions = {
                Vector2(-1.0f * blockSize, 0.5f * blockSize),
                Vector2(0.0f * blockSize, 0.5f * blockSize),
                Vector2(0.0f * blockSize, -0.5f * blockSize),
                Vector2(1.0f * blockSize, -0.5f * blockSize)
            };
            break;
    }
    
    // Создание блоков тетромино
    for (const auto& blockPos : blockPositions) {
        PhysicsBody block;
        
        // Поворот позиции блока
        float cosA = std::cos(rotation);
        float sinA = std::sin(rotation);
        Vector2 rotatedPos;
        rotatedPos.x = cosA * blockPos.x - sinA * blockPos.y;
        rotatedPos.y = sinA * blockPos.x + cosA * blockPos.y;
        
        // Установка параметров блока
        block.position = position + rotatedPos;
        block.rotation = rotation;
        block.width = blockSize;
        block.height = blockSize;
        block.mass = 1.0f;
        block.restitution = 0.1f;
        block.friction = 0.8f;
        block.material = MaterialType::NORMAL;
        block.updateMassData();
        
        // Добавление блока в тетромино
        tetromino.blocks.push_back(block);
    }
    
    return tetromino;
}

// Применение силы к телу
void PhysicsEngine::applyForce(const std::string& bodyId, const Vector2& force) {
    std::lock_guard<std::mutex> lock(m_mutex);
    auto it = m_bodies.find(bodyId);
    if (it != m_bodies.end()) {
        it->second->applyForce(force);
    }
}

// Применение импульса к телу
void PhysicsEngine::applyImpulse(const std::string& bodyId, const Vector2& impulse, const Vector2& contactPoint) {
    std::lock_guard<std::mutex> lock(m_mutex);
    auto it = m_bodies.find(bodyId);
    if (it != m_bodies.end()) {
        it->second->applyImpulse(impulse, contactPoint);
    }
}

// Установка гравитации
void PhysicsEngine::setGravity(const Vector2& gravity) {
    std::lock_guard<std::mutex> lock(m_mutex);
    m_gravity = gravity;
}

// Получение текущей гравитации
Vector2 PhysicsEngine::getGravity() const {
    return m_gravity;
}

// Проверка стабильности башни
bool PhysicsEngine::checkTowerStability(const std::vector<std::string>& towerBlockIds) {
    std::lock_guard<std::mutex> lock(m_mutex);
    
    if (towerBlockIds.empty()) {
        return true;
    }
    
    std::vector<PhysicsBody*> towerBlocks;
    for (const auto& id : towerBlockIds) {
        auto it = m_bodies.find(id);
        if (it != m_bodies.end()) {
            towerBlocks.push_back(it->second.get());
        } else {
            return false;
        }
    }
    
    const float velocityThreshold = 0.1f;
    const float angularVelocityThreshold = 0.1f;
    
    for (const auto& block : towerBlocks) {
        if (block->velocity.length() > velocityThreshold || 
            std::abs(block->angularVelocity) > angularVelocityThreshold) {
            return false;
        }
    }
    
    PhysicsBody* lowestBlock = nullptr;
    float lowestY = std::numeric_limits<float>::max();
    
    for (const auto& block : towerBlocks) {
        if (block->position.y < lowestY) {
            lowestY = block->position.y;
            lowestBlock = block;
        }
    }
    
    if (!lowestBlock) {
        return false;
    }
    
    Vector2 centerOfMass(0.0f, 0.0f);
    float totalMass = 0.0f;
    
    for (const auto& block : towerBlocks) {
        centerOfMass = centerOfMass + block->position * block->mass;
        totalMass += block->mass;
    }
    
    if (totalMass > 0.0f) {
        centerOfMass = centerOfMass * (1.0f / totalMass);
    }
    
    float baseLeft = lowestBlock->position.x - lowestBlock->width / 2.0f;
    float baseRight = lowestBlock->position.x + lowestBlock->width / 2.0f;
    
    return (centerOfMass.x >= baseLeft && centerOfMass.x <= baseRight);
}

// Регистрация колбэка для обработки коллизий
void PhysicsEngine::registerCollisionCallback(std::function<void(const Contact&)> callback) {
    std::lock_guard<std::mutex> lock(m_mutex);
    m_collisionCallback = callback;
}

// Запуск симуляции
void PhysicsEngine::startSimulation() {
    m_isRunning = true;
}

// Пауза симуляции
void PhysicsEngine::pauseSimulation() {
    m_isPaused = true;
}

// Остановка симуляции
void PhysicsEngine::stopSimulation() {
    m_isRunning = false;
}

// Получение всех тел
std::vector<PhysicsBody*> PhysicsEngine::getAllBodies() {
    std::lock_guard<std::mutex> lock(m_mutex);
    std::vector<PhysicsBody*> bodies;
    bodies.reserve(m_bodies.size());
    
    for (auto& pair : m_bodies) {
        bodies.push_back(pair.second.get());
    }
    
    return bodies;
}

// Проверка пересечения двух тел
bool PhysicsEngine::checkCollision(const PhysicsBody& bodyA, const PhysicsBody& bodyB, Contact* contact) {
    // Сначала проверяем AABB для быстрого отсечения
    if (!checkAABBCollision(bodyA, bodyB)) {
        return false;
    }
    
    // Затем проверяем OBB для точного определения коллизии
    return checkOBBCollision(bodyA, bodyB, contact);
}

// Проверка, находится ли точка внутри тела
bool PhysicsEngine::isPointInBody(const std::string& bodyId, const Vector2& point) {
    PhysicsBody* body = getBody(bodyId);
    if (!body) return false;
    
    // Преобразование точки в локальные координаты тела
    float cosA = std::cos(-body->rotation);
    float sinA = std::sin(-body->rotation);
    
    Vector2 localPoint;
    localPoint.x = cosA * (point.x - body->position.x) - sinA * (point.y - body->position.y);
    localPoint.y = sinA * (point.x - body->position.x) + cosA * (point.y - body->position.y);
    
    return body->containsPoint(localPoint);
}

// Получение тел в заданной области
std::vector<PhysicsBody*> PhysicsEngine::getBodiesInArea(const Vector2& min, const Vector2& max) {
    std::lock_guard<std::mutex> lock(m_mutex);
    std::vector<PhysicsBody*> result;
    
    for (auto& pair : m_bodies) {
        PhysicsBody* body = pair.second.get();
        
        float halfWidth = body->width / 2.0f;
        float halfHeight = body->height / 2.0f;
        
        float bodyMinX = body->position.x - halfWidth;
        float bodyMaxX = body->position.x + halfWidth;
        float bodyMinY = body->position.y - halfHeight;
        float bodyMaxY = body->position.y + halfHeight;
        
        if (bodyMaxX >= min.x && bodyMinX <= max.x &&
            bodyMaxY >= min.y && bodyMinY <= max.y) {
            result.push_back(body);
        }
    }
    
    return result;
}

// Применение заклинания (изменение физических свойств блоков)
void PhysicsEngine::applySpell(const std::string& spellType, const std::vector<std::string>& targetBlockIds) {
    for (const auto& blockId : targetBlockIds) {
        auto it = m_bodies.find(blockId);
        if (it != m_bodies.end()) {
            PhysicsBody* body = it->second.get();
            // Применяем заклинание к телу
            if (spellType == "heavy") {
                body->mass *= 2.0f;
            } else if (spellType == "light") {
                body->mass *= 0.5f;
            } else if (spellType == "slippery") {
                body->friction *= 0.5f;
            } else if (spellType == "sticky") {
                body->friction *= 2.0f;
            } else if (spellType == "bouncy") {
                body->restitution *= 2.0f;
            }
            body->updateMassData();
        }
    }
}

// Экспорт состояния физики в JSON
std::string PhysicsEngine::exportStateToJson() const {
    std::lock_guard<std::mutex> lock(m_mutex);
    
    nlohmann::json j;
    
    // Сериализация тел
    nlohmann::json bodiesJson = nlohmann::json::array();
    for (const auto& [id, body] : m_bodies) {
        nlohmann::json bodyJson;
        bodyJson["id"] = body->id;
        bodyJson["position"] = {{"x", body->position.x}, {"y", body->position.y}};
        bodyJson["velocity"] = {{"x", body->velocity.x}, {"y", body->velocity.y}};
        bodyJson["force"] = {{"x", body->force.x}, {"y", body->force.y}};
        bodyJson["rotation"] = body->rotation;
        bodyJson["angularVelocity"] = body->angularVelocity;
        bodyJson["torque"] = body->torque;
        bodyJson["mass"] = body->mass;
        bodyJson["inverseMass"] = body->inverseMass;
        bodyJson["inertia"] = body->inertia;
        bodyJson["inverseInertia"] = body->inverseInertia;
        bodyJson["restitution"] = body->restitution;
        bodyJson["friction"] = body->friction;
        bodyJson["isStatic"] = body->isStatic;
        bodyJson["isActive"] = body->isActive;
        bodyJson["material"] = static_cast<int>(body->material);
        bodyJson["width"] = body->width;
        bodyJson["height"] = body->height;
        
        bodiesJson.push_back(bodyJson);
    }
    j["bodies"] = bodiesJson;
    
    // Сериализация гравитации
    j["gravity"] = {{"x", m_gravity.x}, {"y", m_gravity.y}};
    
    // Сериализация настроек
    j["timeStep"] = m_timeStep;
    j["velocityIterations"] = m_velocityIterations;
    j["positionIterations"] = m_positionIterations;
    j["isRunning"] = m_isRunning;
    j["isPaused"] = m_isPaused;
    
    return j.dump();
}

// Импорт состояния физики из JSON
bool PhysicsEngine::importStateFromJson(const std::string& json) {
    // Примечание: для полноценной реализации требуется библиотека для работы с JSON
    // В данном примере мы просто возвращаем false, чтобы показать, что функция не реализована
    return false;
}

// Внутренние методы

// Обнаружение коллизий
void PhysicsEngine::detectCollisions() {
    m_contacts.clear();
    
    // Проверяем все пары тел на коллизии
    std::vector<PhysicsBody*> bodies = getAllBodies();
    
    for (size_t i = 0; i < bodies.size(); ++i) {
        for (size_t j = i + 1; j < bodies.size(); ++j) {
            Contact contact;
            if (checkCollision(*bodies[i], *bodies[j], &contact)) {
                m_contacts.push_back(contact);
                
                // Вызываем колбэк, если он зарегистрирован
                if (m_collisionCallback) {
                    m_collisionCallback(contact);
                }
            }
        }
    }
}

// Разрешение коллизий
void PhysicsEngine::resolveCollisions() {
    for (auto& contact : m_contacts) {
        PhysicsBody* bodyA = contact.bodyA;
        PhysicsBody* bodyB = contact.bodyB;
        
        // Если оба тела статические, пропускаем
        if (bodyA->isStatic && bodyB->isStatic) {
            continue;
        }
        
        // Относительная скорость в точке контакта
        Vector2 relativeVelocity = bodyB->getVelocityAtPoint(contact.point) - 
                                  bodyA->getVelocityAtPoint(contact.point);
        
        // Скорость вдоль нормали
        float velocityAlongNormal = relativeVelocity.dot(contact.normal);
        
        // Если тела разлетаются, пропускаем
        if (velocityAlongNormal > 0) {
            continue;
        }
        
        // Вычисление коэффициента восстановления (среднее значение)
        float restitution = std::min(bodyA->restitution, bodyB->restitution);
        
        // Вычисление импульса
        float j = -(1.0f + restitution) * velocityAlongNormal;
        j /= bodyA->inverseMass + bodyB->inverseMass;
        
        // Применение импульса
        Vector2 impulse = contact.normal * j;
        
        if (!bodyA->isStatic) {
            bodyA->applyImpulse(impulse * -1.0f, contact.point);
        }
        
        if (!bodyB->isStatic) {
            bodyB->applyImpulse(impulse, contact.point);
        }
        
        // Трение
        // Вычисление тангенциальной составляющей
        Vector2 tangent = relativeVelocity - contact.normal * velocityAlongNormal;
        float tangentLength = tangent.length();
        
        if (tangentLength > 0.0001f) {
            tangent = tangent * (1.0f / tangentLength);
            
            // Вычисление силы трения
            float jt = -relativeVelocity.dot(tangent);
            jt /= bodyA->inverseMass + bodyB->inverseMass;
            
            // Коэффициент трения (среднее значение)
            float friction = (bodyA->friction + bodyB->friction) * 0.5f;
            
            // Ограничение силы трения
            float maxJt = j * friction;
            jt = std::max(-maxJt, std::min(jt, maxJt));
            
            // Применение силы трения
            Vector2 frictionImpulse = tangent * jt;
            
            if (!bodyA->isStatic) {
                bodyA->applyImpulse(frictionImpulse * -1.0f, contact.point);
            }
            
            if (!bodyB->isStatic) {
                bodyB->applyImpulse(frictionImpulse, contact.point);
            }
        }
        
        // Коррекция позиций для предотвращения проникновения
        const float percent = 0.2f; // Обычно 20-80%
        const float slop = 0.01f; // Небольшое значение для предотвращения дрожания
        
        Vector2 correction = contact.normal * (std::max(contact.penetration - slop, 0.0f) * percent);
        correction = correction * (bodyA->inverseMass + bodyB->inverseMass);
        
        if (!bodyA->isStatic) {
            bodyA->position = bodyA->position - correction * bodyA->inverseMass;
        }
        
        if (!bodyB->isStatic) {
            bodyB->position = bodyB->position + correction * bodyB->inverseMass;
        }
    }
}

// Интеграция физики
void PhysicsEngine::integrate(float deltaTime) {
    for (auto& pair : m_bodies) {
        PhysicsBody* body = pair.second.get();
        if (!body->isActive || body->isStatic) {
            continue;
        }
        
        // Интегрируем скорость
        body->velocity = body->velocity + (body->force * body->inverseMass + m_gravity) * deltaTime;
        body->angularVelocity += body->torque * body->inverseInertia * deltaTime;
        
        // Интегрируем позицию
        body->position = body->position + body->velocity * deltaTime;
        body->rotation += body->angularVelocity * deltaTime;
        
        // Сбрасываем силы
        body->force = Vector2();
        body->torque = 0.0f;
    }
}

// Генерация уникального ID
std::string PhysicsEngine::generateUniqueId() {
    static std::random_device rd;
    static std::mt19937 gen(rd());
    static std::uniform_int_distribution<> dis(0, 15);
    static const char* digits = "0123456789abcdef";
    
    std::stringstream ss;
    ss << "body_";
    
    for (int i = 0; i < 8; ++i) {
        ss << digits[dis(gen)];
    }
    
    return ss.str();
}

// Проверка пересечения двух прямоугольников (AABB)
bool PhysicsEngine::checkAABBCollision(const PhysicsBody& bodyA, const PhysicsBody& bodyB) {
    // Вычисляем AABB для каждого тела
    float halfWidthA = bodyA.width / 2.0f;
    float halfHeightA = bodyA.height / 2.0f;
    float halfWidthB = bodyB.width / 2.0f;
    float halfHeightB = bodyB.height / 2.0f;
    
    // Вычисляем границы AABB
    float minAx = bodyA.position.x - halfWidthA;
    float maxAx = bodyA.position.x + halfWidthA;
    float minAy = bodyA.position.y - halfHeightA;
    float maxAy = bodyA.position.y + halfHeightA;
    
    float minBx = bodyB.position.x - halfWidthB;
    float maxBx = bodyB.position.x + halfWidthB;
    float minBy = bodyB.position.y - halfHeightB;
    float maxBy = bodyB.position.y + halfHeightB;
    
    // Проверяем пересечение AABB
    return (maxAx >= minBx && minAx <= maxBx && maxAy >= minBy && minAy <= maxBy);
}

// Проверка пересечения двух ориентированных прямоугольников (OBB)
bool PhysicsEngine::checkOBBCollision(const PhysicsBody& bodyA, const PhysicsBody& bodyB, Contact* contact) {
    // Получаем вершины прямоугольников
    std::vector<Vector2> verticesA = bodyA.getVertices();
    std::vector<Vector2> verticesB = bodyB.getVertices();
    
    // Оси для проверки (нормали к сторонам прямоугольников)
    std::vector<Vector2> axes;
    
    // Добавляем оси для первого прямоугольника
    for (size_t i = 0; i < verticesA.size(); ++i) {
        Vector2 edge = verticesA[(i + 1) % verticesA.size()] - verticesA[i];
        Vector2 normal(-edge.y, edge.x); // Перпендикуляр к ребру
        axes.push_back(normal.normalized());
    }
    
    // Добавляем оси для второго прямоугольника
    for (size_t i = 0; i < verticesB.size(); ++i) {
        Vector2 edge = verticesB[(i + 1) % verticesB.size()] - verticesB[i];
        Vector2 normal(-edge.y, edge.x); // Перпендикуляр к ребру
        axes.push_back(normal.normalized());
    }
    
    // Проверяем разделение по каждой оси
    float minOverlap = std::numeric_limits<float>::max();
    Vector2 minAxis;
    
    for (const auto& axis : axes) {
        float overlap = 0.0f;
        Vector2 normal;
        
        if (!checkSeparatingAxis(verticesA, verticesB, axis, overlap, normal)) {
            // Если найдена разделяющая ось, тела не пересекаются
            return false;
        }
        
        // Запоминаем минимальное перекрытие
        if (overlap < minOverlap) {
            minOverlap = overlap;
            minAxis = normal;
        }
    }
    
    // Если мы дошли до этой точки, значит тела пересекаются
    if (contact) {
        contact->bodyA = const_cast<PhysicsBody*>(&bodyA);
        contact->bodyB = const_cast<PhysicsBody*>(&bodyB);
        contact->normal = minAxis;
        contact->penetration = minOverlap;
        
        // Вычисление точки контакта (упрощенно - средняя точка пересечения)
        Vector2 centerA = bodyA.position;
        Vector2 centerB = bodyB.position;
        
        // Направление от A к B
        Vector2 direction = centerB - centerA;
        
        // Если направление совпадает с нормалью, используем его
        if (direction.dot(minAxis) < 0) {
            contact->normal = minAxis * -1.0f;
        }
        
        // Вычисляем точку контакта (упрощенно)
        contact->point = centerA + direction * 0.5f;
    }
    
    return true;
}

// Разделяющая ось для OBB
bool PhysicsEngine::checkSeparatingAxis(const std::vector<Vector2>& verticesA, 
                                       const std::vector<Vector2>& verticesB, 
                                       const Vector2& axis, 
                                       float& minOverlap,
                                       Vector2& normal) {
    // Проекции вершин на ось
    float minA = std::numeric_limits<float>::max();
    float maxA = -std::numeric_limits<float>::max();
    float minB = std::numeric_limits<float>::max();
    float maxB = -std::numeric_limits<float>::max();
    
    // Проецируем вершины первого тела
    for (const auto& vertex : verticesA) {
        float projection = vertex.dot(axis);
        minA = std::min(minA, projection);
        maxA = std::max(maxA, projection);
    }
    
    // Проецируем вершины второго тела
    for (const auto& vertex : verticesB) {
        float projection = vertex.dot(axis);
        minB = std::min(minB, projection);
        maxB = std::max(maxB, projection);
    }
    
    // Проверяем перекрытие проекций
    float overlap = std::min(maxA, maxB) - std::max(minA, minB);
    
    if (overlap < 0) {
        // Разделяющая ось найдена
        return false;
    }
    
    // Запоминаем перекрытие и нормаль
    minOverlap = overlap;
    normal = axis;
    
    // Определяем направление нормали
    if (minA < minB) {
        normal = normal * -1.0f;
    }
    
    return true;
}

// Поток симуляции
void PhysicsEngine::simulationThread() {
    auto lastTime = std::chrono::high_resolution_clock::now();
    
    while (m_isRunning) {
        auto currentTime = std::chrono::high_resolution_clock::now();
        float deltaTime = std::chrono::duration<float>(currentTime - lastTime).count();
        lastTime = currentTime;
        
        // Ограничиваем deltaTime для стабильности симуляции
        if (deltaTime > 0.05f) {
            deltaTime = 0.05f;
        }
        
        // Обновление физики
        update(deltaTime);
        
        // Пауза для снижения нагрузки на CPU
        std::this_thread::sleep_for(std::chrono::milliseconds(16)); // ~60 FPS
    }
}

int PhysicsEngine::findClosestBody(const Vector2& point, float maxDistance) {
    float minDist = maxDistance;
    int closestId = -1;
    
    for (const auto& [id, body] : m_bodies) {
        if (!body->isActive) continue;
        
        float dist = (body->position - point).length();
        if (dist < minDist) {
            minDist = dist;
            closestId = std::stoi(id);
        }
    }
    
    return closestId;
}

void PhysicsEngine::applyExplosion(const Vector2& center, float radius, float force) {
    std::lock_guard<std::mutex> lock(m_mutex);
    
    for (auto& [id, body] : m_bodies) {
        Vector2 direction = body->position - center;
        float distance = direction.length();
        
        if (distance <= radius) {
            float strength = force * (1.0f - distance / radius);
            direction = direction.normalized();
            body->applyForce(direction * strength);
        }
    }
}

void PhysicsEngine::applyWind(const Vector2& direction, float strength) {
    std::lock_guard<std::mutex> lock(m_mutex);
    
    Vector2 normalizedDir = direction.normalized();
    
    for (auto& [id, body] : m_bodies) {
        body->applyForce(normalizedDir * strength);
    }
}

std::string PhysicsEngine::serializeToJson() const {
    std::lock_guard<std::mutex> lock(m_mutex);
    
    nlohmann::json j;
    
    // Сериализация тел
    nlohmann::json bodiesJson = nlohmann::json::array();
    for (const auto& [id, body] : m_bodies) {
        nlohmann::json bodyJson;
        bodyJson["id"] = body->id;
        bodyJson["position"] = {{"x", body->position.x}, {"y", body->position.y}};
        bodyJson["velocity"] = {{"x", body->velocity.x}, {"y", body->velocity.y}};
        bodyJson["force"] = {{"x", body->force.x}, {"y", body->force.y}};
        bodyJson["rotation"] = body->rotation;
        bodyJson["angularVelocity"] = body->angularVelocity;
        bodyJson["torque"] = body->torque;
        bodyJson["mass"] = body->mass;
        bodyJson["inverseMass"] = body->inverseMass;
        bodyJson["inertia"] = body->inertia;
        bodyJson["inverseInertia"] = body->inverseInertia;
        bodyJson["restitution"] = body->restitution;
        bodyJson["friction"] = body->friction;
        bodyJson["isStatic"] = body->isStatic;
        bodyJson["isActive"] = body->isActive;
        bodyJson["material"] = static_cast<int>(body->material);
        bodyJson["width"] = body->width;
        bodyJson["height"] = body->height;
        
        bodiesJson.push_back(bodyJson);
    }
    j["bodies"] = bodiesJson;
    
    // Сериализация гравитации
    j["gravity"] = {{"x", m_gravity.x}, {"y", m_gravity.y}};
    
    // Сериализация настроек
    j["timeStep"] = m_timeStep;
    j["velocityIterations"] = m_velocityIterations;
    j["positionIterations"] = m_positionIterations;
    j["isRunning"] = m_isRunning;
    j["isPaused"] = m_isPaused;
    
    return j.dump();
}

bool PhysicsEngine::deserializeFromJson(const std::string& jsonStr) {
    try {
        std::lock_guard<std::mutex> lock(m_mutex);
        
        auto j = nlohmann::json::parse(jsonStr);
        
        // Очистка текущего состояния
        m_bodies.clear();
        m_contacts.clear();
        
        // Десериализация тел
        for (const auto& bodyJson : j["bodies"]) {
            PhysicsBody body;
            body.id = bodyJson["id"];
            body.position.x = bodyJson["position"]["x"];
            body.position.y = bodyJson["position"]["y"];
            body.velocity.x = bodyJson["velocity"]["x"];
            body.velocity.y = bodyJson["velocity"]["y"];
            body.force.x = bodyJson["force"]["x"];
            body.force.y = bodyJson["force"]["y"];
            body.rotation = bodyJson["rotation"];
            body.angularVelocity = bodyJson["angularVelocity"];
            body.torque = bodyJson["torque"];
            body.mass = bodyJson["mass"];
            body.inverseMass = bodyJson["inverseMass"];
            body.inertia = bodyJson["inertia"];
            body.inverseInertia = bodyJson["inverseInertia"];
            body.restitution = bodyJson["restitution"];
            body.friction = bodyJson["friction"];
            body.isStatic = bodyJson["isStatic"];
            body.isActive = bodyJson["isActive"];
            body.material = static_cast<MaterialType>(bodyJson["material"].get<int>());
            body.width = bodyJson["width"];
            body.height = bodyJson["height"];
            
            m_bodies[body.id] = std::make_unique<PhysicsBody>(body);
        }
        
        // Десериализация гравитации
        m_gravity.x = j["gravity"]["x"];
        m_gravity.y = j["gravity"]["y"];
        
        // Десериализация настроек
        m_timeStep = j["timeStep"];
        m_velocityIterations = j["velocityIterations"];
        m_positionIterations = j["positionIterations"];
        m_isRunning = j["isRunning"];
        m_isPaused = j["isPaused"];
        
        return true;
    } catch (const std::exception& e) {
        std::cerr << "Error deserializing state: " << e.what() << std::endl;
        return false;
    }
}

} // namespace TetrisTowers
