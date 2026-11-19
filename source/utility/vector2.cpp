#include "vector2.h"
#include "vector3.h"
#include "libMath.h"

// Vector3 ‚Æ‚Ì‰‰ŽZ
Vector2 Vector2::operator+(const Vector3& rhs) const { return { x + rhs.x, y + rhs.y }; }
Vector2 Vector2::operator-(const Vector3& rhs) const { return { x - rhs.x, y - rhs.y }; }
Vector2 Vector2::operator*(const Vector3& rhs) const { return { x * rhs.x, y * rhs.y }; }
Vector2 Vector2::operator/(const Vector3& rhs) const { return { x / rhs.x, y / rhs.y }; }

Vector2& Vector2::operator+=(const Vector3& rhs) { *this = *this + rhs; return *this; }
Vector2& Vector2::operator-=(const Vector3& rhs) { *this = *this - rhs; return *this; }
Vector2& Vector2::operator*=(const Vector3& rhs) { *this = *this * rhs; return *this; }
Vector2& Vector2::operator/=(const Vector3& rhs) { *this = *this / rhs; return *this; }

// Vector3 ‚©‚ç‚Ì‘ã“ü
Vector2& Vector2::operator=(const Vector3& rhs)
{
    x = rhs.x;
    y = rhs.y;
    return *this;
}

Vector2::Vector2(const Vector3& vec3) : x(vec3.x), y(vec3.y) {}

void Vector2::Normalize(float length)
{
    if (length != 0.0f) { x /= length; y /= length; }
}

void Vector2::Normalize()
{
    float length = Length();
    if (length != 0.0f) { x /= length; y /= length; }
}

Vector2 Vector2::Normalized() const
{
    float length = Length();
    if (length != 0.0f) return { x / length, y / length };
    return { 0,0 };
}

Vector2 Vector2::Normalized(float length) const
{
    if (length != 0.0f) return { x / length, y / length };
    return { 0,0 };
}

float Vector2::Distance(const Vector2& a, const Vector2& b)
{
    float dx = a.x - b.x;
    float dy = a.y - b.y;
    return Sqrt(dx * dx + dy * dy);
}

float Vector2::SqrDistance(const Vector2& a, const Vector2& b)
{
    float dx = a.x - b.x;
    float dy = a.y - b.y;
    return dx * dx + dy * dy;
}


void Vector2::Rotate(float degrees)
{
	float rad = DegreeToRadian(degrees);
	float cosA = cosf(rad);
	float sinA = sinf(rad);
	float newX = x * cosA - y * sinA;
	float newY = x * sinA + y * cosA;
	x = newX;
	y = newY;
}

Vector2 Vector2::Rotated(float degrees) const
{
    float rad = DegreeToRadian(degrees);
    float cosA = cosf(rad);
    float sinA = sinf(rad);
    return { x * cosA - y * sinA, x * sinA + y * cosA };
}