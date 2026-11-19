#pragma once
#include "vector3.h"

enum class CollisionDirection
{
	UP,
	DOWN,
	LEFT,
	RIGHT,
	FRONT,
	BACK,

	NONE,
};

struct BoxCollider
{
	Vector3 center;
	Vector3 size;
}; 

struct SphereCollider
{
	Vector3 center;
	float radius;
};

struct SylinderCollider
{
	Vector3 center;
	float radius;
	float height;
};

struct CapsuleCollider
{
	Vector3 start;
	Vector3 end;
	float radius;
};

CollisionDirection SphereVSBox(SphereCollider a, BoxCollider b);

Vector3 SphereVSSphere(SphereCollider a, SphereCollider b);

/// @brief 球と円柱の衝突判定
/// @param a 球コライダー
/// @param b 円柱コライダー
/// @return 衝突している場合、aからbへの単位ベクトル。衝突していない場合はゼロベクトル。
Vector3 SphereVSSylinder(SphereCollider a, SylinderCollider b);


Vector3 CapsuleVSAABB_HitPos(CapsuleCollider a, BoxCollider b);

/// @brief カプセルと球の衝突判定
/// @param a カプセルコライダー
/// @param b 球コライダー
/// @return 衝突している場合、aからbへ押し出す単位ベクトル。衝突していない場合はゼロベクトル。
Vector3 CapsuleVSSphere(CapsuleCollider a, SphereCollider b);
