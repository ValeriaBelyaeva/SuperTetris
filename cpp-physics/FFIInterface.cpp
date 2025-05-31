#include "include/PhysicsEngine.h"
#include "include/FFIInterface.h"
#include <cstring>
#include <string>
#include <vector>
#include <unordered_map>
#include <mutex>

using namespace TetrisTowers;

// Глобальное хранилище обратных вызовов
static CollisionCallback_FFI g_collisionCallback = nullptr;
static std::mutex g_callbackMutex;

// Обертка для обратного вызова коллизий
void collisionCallbackWrapper(const Contact& contact) {
    std::lock_guard<std::mutex> lock(g_callbackMutex);
    if (g_collisionCallback) {
        ContactInfo_FFI contactFFI;
        contactFFI.blockIdA = std::stoi(contact.bodyA->id);
        contactFFI.blockIdB = std::stoi(contact.bodyB->id);
        contactFFI.point.x = contact.point.x;
        contactFFI.point.y = contact.point.y;
        contactFFI.normal.x = contact.normal.x;
        contactFFI.normal.y = contact.normal.y;
        contactFFI.penetration = contact.penetration;
        
        g_collisionCallback(contactFFI);
    }
}

// Реализация функций FFI

void* physics_engine_create(Vector2_FFI gravity, int iterations) {
    Vector2 gravityVec(gravity.x, gravity.y);
    auto engine = new PhysicsEngine();
    engine->setGravity(gravityVec);
    engine->setIterations(iterations);
    return engine;
}

void physics_engine_destroy(void* engine) {
    if (engine) {
        delete static_cast<PhysicsEngine*>(engine);
    }
}

void physics_engine_set_gravity(void* engine, Vector2_FFI gravity) {
    if (engine) {
        Vector2 gravityVec(gravity.x, gravity.y);
        static_cast<PhysicsEngine*>(engine)->setGravity(gravityVec);
    }
}

Vector2_FFI physics_engine_get_gravity(void* engine) {
    Vector2_FFI result = {0, 0};
    if (engine) {
        Vector2 gravity = static_cast<PhysicsEngine*>(engine)->getGravity();
        result.x = gravity.x;
        result.y = gravity.y;
    }
    return result;
}

void physics_engine_set_iterations(void* engine, int iterations) {
    if (engine) {
        static_cast<PhysicsEngine*>(engine)->setIterations(iterations);
    }
}

int physics_engine_get_iterations(void* engine) {
    if (engine) {
        return static_cast<PhysicsEngine*>(engine)->getIterations();
    }
    return 0;
}

void physics_engine_set_collision_callback(void* engine, CollisionCallback_FFI callback) {
    if (engine) {
        std::lock_guard<std::mutex> lock(g_callbackMutex);
        g_collisionCallback = callback;
        
        if (callback) {
            static_cast<PhysicsEngine*>(engine)->registerCollisionCallback(collisionCallbackWrapper);
        } else {
            static_cast<PhysicsEngine*>(engine)->registerCollisionCallback(nullptr);
        }
    }
}

void physics_engine_update(void* engine, float deltaTime) {
    if (engine) {
        static_cast<PhysicsEngine*>(engine)->update(deltaTime);
    }
}

void physics_engine_start_simulation(void* engine, float fixedTimeStep) {
    if (engine) {
        static_cast<PhysicsEngine*>(engine)->startSimulation();
    }
}

void physics_engine_stop_simulation(void* engine) {
    if (engine) {
        static_cast<PhysicsEngine*>(engine)->stopSimulation();
    }
}

int physics_engine_is_simulation_running(void* engine) {
    if (engine) {
        return static_cast<PhysicsEngine*>(engine)->isSimulationRunning() ? 1 : 0;
    }
    return 0;
}

int physics_engine_create_block(void* engine, Vector2_FFI position, Vector2_FFI size, float angle, PhysicsMaterial_FFI material, int isStatic) {
    if (engine) {
        PhysicsBody body;
        body.position = Vector2(position.x, position.y);
        body.width = size.x;
        body.height = size.y;
        body.rotation = angle;
        body.mass = material.density;
        body.restitution = material.restitution;
        body.friction = material.friction;
        body.isStatic = isStatic != 0;
        body.material = static_cast<MaterialType>(material.isSensor);
        body.updateMassData();
        
        std::string id = static_cast<PhysicsEngine*>(engine)->createBody(body);
        return std::stoi(id);
    }
    return -1;
}

int* physics_engine_create_tetris_block(void* engine, int type, Vector2_FFI position, float blockSize, float angle, PhysicsMaterial_FFI material, int* count) {
    if (engine && count) {
        Vector2 posVec(position.x, position.y);
        Tetromino tetromino = static_cast<PhysicsEngine*>(engine)->createTetromino(
            static_cast<TetrominoType>(type), posVec, angle);
        
        *count = static_cast<int>(tetromino.blocks.size());
        if (*count > 0) {
            int* result = new int[*count];
            for (int i = 0; i < *count; ++i) {
                PhysicsBody& block = tetromino.blocks[i];
                block.mass = material.density;
                block.restitution = material.restitution;
                block.friction = material.friction;
                block.material = static_cast<MaterialType>(material.isSensor);
                block.updateMassData();
                
                std::string id = static_cast<PhysicsEngine*>(engine)->createBody(block);
                result[i] = std::stoi(id);
            }
            return result;
        }
    }
    
    if (count) *count = 0;
    return nullptr;
}

int physics_engine_remove_block(void* engine, int blockId) {
    if (engine) {
        return static_cast<PhysicsEngine*>(engine)->removeBody(std::to_string(blockId)) ? 1 : 0;
    }
    return 0;
}

int physics_engine_check_collision(void* engine, int blockIdA, int blockIdB) {
    if (engine) {
        PhysicsBody* bodyA = static_cast<PhysicsEngine*>(engine)->getBody(std::to_string(blockIdA));
        PhysicsBody* bodyB = static_cast<PhysicsEngine*>(engine)->getBody(std::to_string(blockIdB));
        if (bodyA && bodyB) {
            return static_cast<PhysicsEngine*>(engine)->checkCollision(*bodyA, *bodyB) ? 1 : 0;
        }
    }
    return 0;
}

int physics_engine_is_point_in_block(void* engine, int blockId, Vector2_FFI point) {
    if (engine) {
        Vector2 pointVec(point.x, point.y);
        return static_cast<PhysicsEngine*>(engine)->isPointInBody(std::to_string(blockId), pointVec) ? 1 : 0;
    }
    return 0;
}

int* physics_engine_query_aabb(void* engine, Vector2_FFI lowerBound, Vector2_FFI upperBound, int* count) {
    if (engine && count) {
        Vector2 lbVec(lowerBound.x, lowerBound.y);
        Vector2 ubVec(upperBound.x, upperBound.y);
        
        std::vector<PhysicsBody*> bodies = static_cast<PhysicsEngine*>(engine)->getBodiesInArea(lbVec, ubVec);
        
        *count = static_cast<int>(bodies.size());
        if (*count > 0) {
            int* result = new int[*count];
            for (int i = 0; i < *count; ++i) {
                result[i] = std::stoi(bodies[i]->id);
            }
            return result;
        }
    }
    
    if (count) *count = 0;
    return nullptr;
}

int physics_engine_find_closest_block(void* engine, Vector2_FFI point, float maxDistance) {
    if (engine) {
        Vector2 pointVec(point.x, point.y);
        return static_cast<PhysicsEngine*>(engine)->findClosestBody(pointVec, maxDistance);
    }
    return -1;
}

void physics_engine_apply_explosion(void* engine, Vector2_FFI center, float radius, float force) {
    if (engine) {
        Vector2 centerVec(center.x, center.y);
        static_cast<PhysicsEngine*>(engine)->applyExplosion(centerVec, radius, force);
    }
}

void physics_engine_apply_wind(void* engine, Vector2_FFI direction, float strength) {
    if (engine) {
        Vector2 dirVec(direction.x, direction.y);
        static_cast<PhysicsEngine*>(engine)->applyWind(dirVec, strength);
    }
}

Vector2_FFI physics_block_get_position(void* engine, int blockId) {
    Vector2_FFI result = {0, 0};
    if (engine) {
        PhysicsBody* body = static_cast<PhysicsEngine*>(engine)->getBody(std::to_string(blockId));
        if (body) {
            result.x = body->position.x;
            result.y = body->position.y;
        }
    }
    return result;
}

void physics_block_set_position(void* engine, int blockId, Vector2_FFI position) {
    if (engine) {
        PhysicsBody* body = static_cast<PhysicsEngine*>(engine)->getBody(std::to_string(blockId));
        if (body) {
            body->position = Vector2(position.x, position.y);
        }
    }
}

float physics_block_get_angle(void* engine, int blockId) {
    if (engine) {
        PhysicsBody* body = static_cast<PhysicsEngine*>(engine)->getBody(std::to_string(blockId));
        if (body) {
            return body->rotation;
        }
    }
    return 0.0f;
}

void physics_block_set_angle(void* engine, int blockId, float angle) {
    if (engine) {
        PhysicsBody* body = static_cast<PhysicsEngine*>(engine)->getBody(std::to_string(blockId));
        if (body) {
            body->rotation = angle;
        }
    }
}

Vector2_FFI physics_block_get_linear_velocity(void* engine, int blockId) {
    Vector2_FFI result = {0, 0};
    if (engine) {
        PhysicsBody* body = static_cast<PhysicsEngine*>(engine)->getBody(std::to_string(blockId));
        if (body) {
            result.x = body->velocity.x;
            result.y = body->velocity.y;
        }
    }
    return result;
}

void physics_block_set_linear_velocity(void* engine, int blockId, Vector2_FFI velocity) {
    if (engine) {
        PhysicsBody* body = static_cast<PhysicsEngine*>(engine)->getBody(std::to_string(blockId));
        if (body) {
            body->velocity = Vector2(velocity.x, velocity.y);
        }
    }
}

float physics_block_get_angular_velocity(void* engine, int blockId) {
    if (engine) {
        PhysicsBody* body = static_cast<PhysicsEngine*>(engine)->getBody(std::to_string(blockId));
        if (body) {
            return body->angularVelocity;
        }
    }
    return 0.0f;
}

void physics_block_set_angular_velocity(void* engine, int blockId, float velocity) {
    if (engine) {
        PhysicsBody* body = static_cast<PhysicsEngine*>(engine)->getBody(std::to_string(blockId));
        if (body) {
            body->angularVelocity = velocity;
        }
    }
}

void physics_block_apply_force(void* engine, int blockId, Vector2_FFI force, Vector2_FFI point) {
    if (engine) {
        PhysicsBody* body = static_cast<PhysicsEngine*>(engine)->getBody(std::to_string(blockId));
        if (body) {
            Vector2 forceVec(force.x, force.y);
            Vector2 pointVec(point.x, point.y);
            body->applyForce(forceVec);
        }
    }
}

void physics_block_apply_impulse(void* engine, int blockId, Vector2_FFI impulse, Vector2_FFI point) {
    if (engine) {
        PhysicsBody* body = static_cast<PhysicsEngine*>(engine)->getBody(std::to_string(blockId));
        if (body) {
            Vector2 impulseVec(impulse.x, impulse.y);
            Vector2 pointVec(point.x, point.y);
            body->applyImpulse(impulseVec, pointVec);
        }
    }
}

void physics_block_apply_torque(void* engine, int blockId, float torque) {
    if (engine) {
        PhysicsBody* body = static_cast<PhysicsEngine*>(engine)->getBody(std::to_string(blockId));
        if (body) {
            body->torque = torque;
        }
    }
}

Vector2_FFI physics_block_get_size(void* engine, int blockId) {
    Vector2_FFI result = {0, 0};
    if (engine) {
        PhysicsBody* body = static_cast<PhysicsEngine*>(engine)->getBody(std::to_string(blockId));
        if (body) {
            result.x = body->width;
            result.y = body->height;
        }
    }
    return result;
}

float physics_block_get_mass(void* engine, int blockId) {
    if (engine) {
        PhysicsBody* body = static_cast<PhysicsEngine*>(engine)->getBody(std::to_string(blockId));
        if (body) {
            return body->mass;
        }
    }
    return 0.0f;
}

float physics_block_get_inertia(void* engine, int blockId) {
    if (engine) {
        PhysicsBody* body = static_cast<PhysicsEngine*>(engine)->getBody(std::to_string(blockId));
        if (body) {
            return body->inertia;
        }
    }
    return 0.0f;
}

int physics_block_is_static(void* engine, int blockId) {
    if (engine) {
        PhysicsBody* body = static_cast<PhysicsEngine*>(engine)->getBody(std::to_string(blockId));
        if (body) {
            return body->isStatic ? 1 : 0;
        }
    }
    return 0;
}

void physics_block_set_static(void* engine, int blockId, int isStatic) {
    if (engine) {
        PhysicsBody* body = static_cast<PhysicsEngine*>(engine)->getBody(std::to_string(blockId));
        if (body) {
            body->isStatic = isStatic != 0;
            body->updateMassData();
        }
    }
}

PhysicsMaterial_FFI physics_block_get_material(void* engine, int blockId) {
    PhysicsMaterial_FFI result = {0, 0, 0, 0};
    if (engine) {
        PhysicsBody* body = static_cast<PhysicsEngine*>(engine)->getBody(std::to_string(blockId));
        if (body) {
            result.density = body->mass;
            result.restitution = body->restitution;
            result.friction = body->friction;
            result.isSensor = static_cast<int>(body->material);
        }
    }
    return result;
}

void physics_block_set_material(void* engine, int blockId, PhysicsMaterial_FFI material) {
    if (engine) {
        PhysicsBody* body = static_cast<PhysicsEngine*>(engine)->getBody(std::to_string(blockId));
        if (body) {
            body->mass = material.density;
            body->restitution = material.restitution;
            body->friction = material.friction;
            body->material = static_cast<MaterialType>(material.isSensor);
            body->updateMassData();
        }
    }
}

int physics_block_is_active(void* engine, int blockId) {
    if (engine) {
        PhysicsBody* body = static_cast<PhysicsEngine*>(engine)->getBody(std::to_string(blockId));
        if (body) {
            return body->isActive ? 1 : 0;
        }
    }
    return 0;
}

void physics_block_set_active(void* engine, int blockId, int isActive) {
    if (engine) {
        PhysicsBody* body = static_cast<PhysicsEngine*>(engine)->getBody(std::to_string(blockId));
        if (body) {
            body->isActive = isActive != 0;
        }
    }
}

// Глобальное хранилище для строк JSON
static std::string g_lastJsonString;
static std::mutex g_jsonMutex;

const char* physics_engine_serialize_to_json(void* engine) {
    if (engine) {
        std::lock_guard<std::mutex> lock(g_jsonMutex);
        g_lastJsonString = static_cast<PhysicsEngine*>(engine)->serializeToJson();
        char* result = new char[g_lastJsonString.length() + 1];
        std::strcpy(result, g_lastJsonString.c_str());
        return result;
    }
    return nullptr;
}

int physics_engine_deserialize_from_json(void* engine, const char* json) {
    if (engine && json) {
        return static_cast<PhysicsEngine*>(engine)->deserializeFromJson(json) ? 1 : 0;
    }
    return 0;
}

void physics_free_string(const char* str) {
    if (str) {
        delete[] str;
    }
}

void physics_free_int_array(int* array) {
    if (array) {
        delete[] array;
    }
}
