class Vector2 {
public:
    float x, y;

    Vector2() : x(0), y(0) {}
    Vector2(float x, float y) : x(x), y(y) {}

    // Basic operations
    Vector2 operator+(const Vector2& other) const { return Vector2(x + other.x, y + other.y); }
    Vector2 operator-(const Vector2& other) const { return Vector2(x - other.x, y - other.y); }
    Vector2 operator*(float scalar) const { return Vector2(x * scalar, y * scalar); }
    Vector2 operator/(float scalar) const { return Vector2(x / scalar, y / scalar); }
    
    // Assignment operators
    Vector2& operator+=(const Vector2& other) { x += other.x; y += other.y; return *this; }
    Vector2& operator-=(const Vector2& other) { x -= other.x; y -= other.y; return *this; }
    Vector2& operator*=(float scalar) { x *= scalar; y *= scalar; return *this; }
    Vector2& operator/=(float scalar) { x /= scalar; y /= scalar; return *this; }
    
    // Comparison operators
    bool operator==(const Vector2& other) const { return x == other.x && y == other.y; }
    bool operator!=(const Vector2& other) const { return !(*this == other); }
    
    // Vector operations
    float length() const { return std::sqrt(x * x + y * y); }
    float lengthSquared() const { return x * x + y * y; }
    float dot(const Vector2& other) const { return x * other.x + y * other.y; }
    float cross(const Vector2& other) const { return x * other.y - y * other.x; }
    
    // Rotation and normalization
    Vector2 rotated(float angle) const {
        float cos = std::cos(angle);
        float sin = std::sin(angle);
        return Vector2(x * cos - y * sin, x * sin + y * cos);
    }
    
    Vector2 normalized() const {
        float len = length();
        if (len > 0) {
            return Vector2(x / len, y / len);
        }
        return *this;
    }
    
    // Wrapper methods for backward compatibility
    void rotate(float angle) { *this = rotated(angle); }
    void normalize() { *this = normalized(); }
    
    // Utility methods
    void set(float x, float y) { this->x = x; this->y = y; }
    void zero() { x = 0; y = 0; }
    
    // Static methods
    static Vector2 zero() { return Vector2(0, 0); }
    static Vector2 one() { return Vector2(1, 1); }
    static Vector2 up() { return Vector2(0, 1); }
    static Vector2 down() { return Vector2(0, -1); }
    static Vector2 left() { return Vector2(-1, 0); }
    static Vector2 right() { return Vector2(1, 0); }
}; 