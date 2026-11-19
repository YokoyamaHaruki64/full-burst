#include "collision.h"

CollisionDirection SphereVSBox(SphereCollider a, BoxCollider b)
{
	// 横方向の判定
	if (b.center.y - b.size.y * 0.5f < a.center.y &&
		b.center.y + b.size.y * 0.5f > a.center.y)
	{// この時点でy方向には重なっている
		
		if (b.center.z - b.size.z * 0.5f < a.center.z &&
			b.center.z + b.size.z * 0.5f > a.center.z)
		{
			// X軸方向の当たり判定
			if (a.center.x + a.radius > b.center.x - b.size.x * 0.5f &&
				a.center.x - a.radius < b.center.x + b.size.x * 0.5f)
			{
				if (b.center.x < a.center.x)
				{
					return CollisionDirection::LEFT;
				}
				else
				{
					return CollisionDirection::RIGHT;
				}
			}
		}
		else if (b.center.x - b.size.x * 0.5f < a.center.x &&
			b.center.x + b.size.x * 0.5f > a.center.x)
		{
			// Z軸方向の当たり判定
			if (a.center.z + a.radius > b.center.z - b.size.z * 0.5f &&
				a.center.z - a.radius < b.center.z + b.size.z * 0.5f)
			{
				if (b.center.z < a.center.z)
				{
					return CollisionDirection::BACK;
				}
				else
				{
					return CollisionDirection::FRONT;
				}
			}
		}
	}
	else // 縦方向の判定
	{
		if (a.center.z + a.radius > b.center.z - b.size.z * 0.5f &&
			a.center.z - a.radius < b.center.z + b.size.z * 0.5f)
		{
			if (a.center.x + a.radius > b.center.x - b.size.x * 0.5f &&
				a.center.x - a.radius < b.center.x + b.size.x * 0.5f)
			{
				if (a.center.y + a.radius > b.center.y - b.size.y * 0.5f &&
					a.center.y - a.radius < b.center.y + b.size.y * 0.5f)
				{
					if (b.center.y < a.center.y)
					{
						return CollisionDirection::DOWN;
					}
					else
					{
						return CollisionDirection::UP;
					}
				}
			}
		}
	}

	return CollisionDirection::NONE;
}

Vector3 SphereVSSphere(SphereCollider a, SphereCollider b)
{
	Vector3 dir = b.center - a.center;
	float distSqr = dir.SqrLength();
	float radiusSum = a.radius + b.radius;
	if (distSqr < radiusSum * radiusSum)
	{
		float dist = dir.Length();

		if (dist != 0.0f)
		{
			return dir / dist; // 正規化して返す
		}
		else
		{
			// 完全に重なっている場合、任意の方向を返す
			return Vector3::Up();
		}
	}
	return Vector3::Zero(); // 衝突していない場合はゼロベクトルを返す
}

Vector3 SphereVSSylinder(SphereCollider a, SylinderCollider b)
{
	// Y軸方向の距離を計算
	float deltaY = a.center.y - b.center.y;
	float halfHeight = b.height * 0.5f;
	// Y軸方向の衝突判定
	if (deltaY > halfHeight + a.radius || deltaY < -halfHeight - a.radius)
	{
		return Vector3::Zero(); // 衝突していない
	}

	// XZ平面での距離を計算
	Vector3 deltaXZ = Vector3(b.center.x - a.center.x, 0.0f, b.center.z - a.center.z);
	float distSqrXZ = deltaXZ.SqrLength();
	float radiusSum = a.radius + b.radius;
	// XZ平面での衝突判定
	if (distSqrXZ < radiusSum * radiusSum)
	{
		Vector3 dir = b.center - a.center;
		
		if (dir.SqrLength() != 0.0f)
		{
			return dir.Normalized(); // 正規化して返す
		}
		else
		{
			// 完全に重なっている場合、任意の方向を返す
			return Vector3::Up();
		}
	}
	return Vector3::Zero(); // 衝突していない
}

Vector3 CapsuleVSAABB_HitPos(CapsuleCollider a, BoxCollider b)
{
	Vector3 seg = a.end - a.start;
	float segLen = seg.Length();
	if (segLen < 0.0001f)
		return Vector3::Zero();

	Vector3 dir = seg / segLen;

	Vector3 boxMin = b.center - b.size * 0.5f - Vector3(a.radius, a.radius, a.radius);
	Vector3 boxMax = b.center + b.size * 0.5f + Vector3(a.radius, a.radius, a.radius);

	float tMin = 0.0f;
	float tMax = segLen;

	for (int axis = 0; axis < 3; axis++)
	{
		float start = 0.0f;
		float d = 0.0f;
		float minB = 0.0f;
		float maxB = 0.0f;

		switch (axis)
		{
		case 0: // X
			start = a.start.x;
			d = dir.x;
			minB = boxMin.x;
			maxB = boxMax.x;
			break;

		case 1: // Y
			start = a.start.y;
			d = dir.y;
			minB = boxMin.y;
			maxB = boxMax.y;
			break;

		case 2: // Z
			start = a.start.z;
			d = dir.z;
			minB = boxMin.z;
			maxB = boxMax.z;
			break;
		}

		if (fabsf(d) < 0.0001f)
		{
			if (start < minB || start > maxB)
				return Vector3::Zero();
		}
		else
		{
			float invD = 1.0f / d;
			float t1 = (minB - start) * invD;
			float t2 = (maxB - start) * invD;

			if (t1 > t2) std::swap(t1, t2);

			tMin = std::max(tMin, t1);
			tMax = std::min(tMax, t2);

			if (tMin > tMax)
				return Vector3::Zero();
		}
	}

	// 中心線が最初にAABB（膨張済み）に当たった位置
	return a.start + dir * tMin;
}

Vector3 CapsuleVSSphere(CapsuleCollider a, SphereCollider b)
{
	Vector3 dir = a.end - a.start;
	float lenSq = Vector3::Dot(dir, dir);

	if (lenSq < 0.0001f)
		return Vector3::Zero();

	float t = Vector3::Dot(b.center - a.start, dir) / lenSq;
	t = Clamp(0.0f, 1.0f,t);

	Vector3 closest = a.start + dir * t;

	float r = a.radius + b.radius;
	float distSq = (closest - b.center).SqrLength();

	if (distSq <= r * r)
	{
		if (distSq <= 0.0001f)
		{
			// 完全に重なっている場合、任意の方向を返す
			return Vector3::Up();
		}

 		return (b.center - closest).Normalized();
	}

	return Vector3::Zero();
}
