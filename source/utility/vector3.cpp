#include "vector3.h"
#include "vector2.h"

Vector3::Vector3(const Vector2& vec2) : x(vec2.x), y(vec2.y), z(0.0f) {}

// Vector2‚Æ‚Ì‰‰ŽZ
Vector3 Vector3::operator+(const Vector2& rhs) const { return Vector3{ x + rhs.x, y + rhs.y, z }; }
Vector3 Vector3::operator-(const Vector2& rhs) const { return Vector3{ x - rhs.x, y - rhs.y, z }; }
Vector3 Vector3::operator*(const Vector2& rhs) const { return Vector3{ x * rhs.x, y * rhs.y, z }; }
Vector3 Vector3::operator/(const Vector2& rhs) const { return Vector3{ x / rhs.x, y / rhs.y, z }; }

Vector3& Vector3::operator+=(const Vector2& rhs) { return *this = *this + rhs; }
Vector3& Vector3::operator-=(const Vector2& rhs) { return *this = *this - rhs; }
Vector3& Vector3::operator*=(const Vector2& rhs) { return *this = *this * rhs; }
Vector3& Vector3::operator/=(const Vector2& rhs) { return *this = *this / rhs; }

// Vector3‚É‘ã“ü
Vector3& Vector3::operator=(const Vector2& rhs)
{
    x = rhs.x;
    y = rhs.y;
    z = 0.0f;
    return *this;
}


void Vector3::Normalize()
{
    float l = Length();
    if (l != 0.0f) 
    {
        x /= l;
        y /= l;
        z /= l;
    }
}

Vector3 Vector3::Normalized() const
{
    float l = Length();
    if (l == 0.0f) 
    {
        return Vector3::Zero();
    }
    return Vector3(x / l, y / l, z / l);
}

float Vector3::Distance(const Vector3& a, const Vector3& b)
{
    return (a - b).Length();
}

Vector3 Vector3::MoveTowards(const Vector3& current, const Vector3& target, float maxDistanceDelta)
{
    Vector3 delta = target - current;
    float dist = delta.Length();
    if (dist <= maxDistanceDelta || dist == 0.0f)
        return target;
    return current + delta / dist * maxDistanceDelta;
}

float Vector3::Angle(const Vector3& from, const Vector3& to)
{
    float denom = from.Length() * to.Length();
    if (denom == 0) return 0.0f;
    float cosTheta = Dot(from, to) / denom;
    cosTheta = std::max(-1.0f, std::min(1.0f, cosTheta));
    return Acos(cosTheta) * (180.0f / 3.14159265359f);
}
