#include "player.h"
#include "main.h"
#include "game.h"
#include "directx.h"
#include "model.h"
#include "shader.h"
#include "camera.h"
#include "input.h"
#include "fade.h"
#include "field.h"
#include "vector3.h"
#include "collision.h"
#include "effect.h"
#include "boss.h"
#include "B_verticalLaser.h"
#include "starTrail.h"
#include "sprite.h"
#include "texture.h"
#include "sound.h"
#include "Shooter.h"	

#include "imgui.h"
struct Player
{
	Vector3 position;
	Vector3 velocity;
	Vector3 rotation;
	Vector3 scale;
	PlayerState state;

	bool isGround = false;
	bool isJump = false;
	int onShooter = -1;
	SphereCollider collider;

	float shooterAngle = 0.0f;
	float HP = MAX_PLAYER_HP;

	FrameTime* damageTimer = nullptr;
};

struct DefeatEffect
{
	bool isActive = false;
	Vector3 position;
	Vector3 velocity;
	Color color;
	float scale;
	int life;
	int maxLife;
};

enum PlayerSound
{
	PLAYER_CHARGE,
	PLAYER_SHOT,
	PLAYER_HIT,


	PLAYER_SOUND_MAX,
};


void PlayerMove();
void PlayerCharge();
void PlayerShot();
void PlayerReturn();
void PlayerDefeat();
void DefeatEffectDraw();
void CreateDefeatEffect();	
void PlayerDie();


Vector3 CalculateBallisticVelocity(Vector3 start, Vector3 end, float time);
void CheckCollisionWithField();
bool CheckCollisionWithBoss();
bool CheckCollisionWithLaser();

static constexpr Vector3 DAMPING = Vector3(3.0f, 0.0f, 3.0f);
static constexpr Vector3 SHOT_DAMPING = Vector3(0.0f, 0.0f, 0.0f);

static constexpr float SHOT_SPEED = 30.0f;
static constexpr float GRAVITY = 9.8f;
static constexpr float SCALE = 1.5f;
static constexpr int INVINCIBLE_TIME = 60;
static constexpr int CHARGE_TIME = 98;
static constexpr float JUMP_POWER = 8.0f;
static constexpr float MOVE_SPEED = 20.0f;
static constexpr float ADD_SHOTPOWER = 2.0f;
static constexpr int MAX_DEFEAT_EFFECT = 64;

static MODEL* g_model = nullptr;
static Player g_player;
static DefeatEffect g_defeatEffects[MAX_DEFEAT_EFFECT];
static FrameTime* g_ChargeTime;
static int g_playerSounds[PLAYER_SOUND_MAX];
static int g_defeatEffectTex = -1;

static float g_shotPower;

static float dt = 0.016f;
static bool isHitStop = false;
static float g_effectScale = 0.0f;
static int g_dieAfterCounter = 0;

void PlayerInitialize()
{
	g_model = ModelLoad("asset\\model\\ball03.fbx");
	g_defeatEffectTex = TextureLoad(L"asset\\texture\\orb.png");

	g_player.position = RETURN_POSITIONS[0];
	g_player.rotation = Vector3(0.0f, 0.0f, 0.0f);
	g_player.scale = Vector3(SCALE, SCALE, SCALE);
	g_player.velocity = Vector3(0.0f, 0.0f, 0.0f);
	g_player.state = PlayerState::MOVE;
	g_player.HP = MAX_PLAYER_HP;
	g_player.collider.center = g_player.position;
	g_player.collider.radius = SCALE * 0.2f;

	for(DefeatEffect& effect : g_defeatEffects)
	{
		effect.isActive = false;
		effect.position = Vector3::Zero();
		effect.velocity = Vector3::Zero();
		effect.scale = 0.0f;
		effect.life = 0.0f;
		effect.maxLife = 0.0f;
	}

	g_ChargeTime = new FrameTime(CHARGE_TIME);
	g_player.damageTimer = new FrameTime(30);
	isHitStop = false;
	g_effectScale = 0.0f;
	g_dieAfterCounter = 0;

	g_playerSounds[PLAYER_CHARGE] = LoadSound("asset\\sound\\charge.wav");
	g_playerSounds[PLAYER_SHOT] = LoadSound("asset\\sound\\shot.wav");
	g_playerSounds[PLAYER_HIT] = LoadSound("asset\\sound\\playerHit.wav");

	SetVolume(g_playerSounds[PLAYER_CHARGE], 0.5f);
}

void PlayerFinalize()
{
	ModelRelease(g_model);
	delete g_ChargeTime;
	delete g_player.damageTimer;
}

void PlayerUpdate()
{
	if(g_player.state == PlayerState::NONE)
	{
		return;
	}
	if(isHitStop)
	{
		CreateEffect(g_player.position, g_effectScale, EffectType::EXPLOSION);
		isHitStop = false;
	}

	switch (g_player.state)
	{
	case PlayerState::MOVE:
		PlayerMove();
		break;
	case PlayerState::CHARGE:
		PlayerCharge();
		break;
	case PlayerState::SHOT:
		PlayerShot();
		break;
	case PlayerState::RETURN:
		PlayerReturn();
		break;
	case PlayerState::KNOCKBACK:
		if (g_player.damageTimer->IsFinished())
		{
			g_player.state = PlayerState::INVINCIBLE;
			g_player.damageTimer->SetInterval(INVINCIBLE_TIME);
			g_player.damageTimer->Reset();
		}
		g_player.velocity -= g_player.velocity * DAMPING * dt;
		g_player.position += g_player.velocity * dt;
		break;
	case PlayerState::INVINCIBLE:
		if (g_player.damageTimer->IsFinished())
		{
			g_player.state = PlayerState::MOVE;
		}
		PlayerMove();
		break;
	case PlayerState::DEFEAT:
		PlayerDefeat();
		break;
	default:
		break;
	}

	g_player.collider.center = g_player.position;

	CheckCollisionWithField();
	CheckCollisionWithLaser();

	// 落下処理
	if (g_player.position.y < -5.0f)
	{
		g_player.HP -= 2.0f;

		int index = 0;
		float minDist = FLT_MAX;
		for(int i = 0;i < 4;i++)
		{
			// 一番近いリターン位置を探す
			float dist = (g_player.position - RETURN_POSITIONS[i]).SqrLength();
			if(dist < minDist)
			{
				minDist = dist;
				index = i;
			}
		}

		// 最も近いリターン位置へ
		g_player.position = RETURN_POSITIONS[index];
		g_player.isGround = true;
		g_player.isJump = false;

		g_player.velocity = Vector3::Zero();
		g_player.state = PlayerState::RETURN;
		PlaySound(g_playerSounds[PLAYER_HIT], 0);

		g_player.state = PlayerState::INVINCIBLE;
		g_player.damageTimer->SetInterval(INVINCIBLE_TIME);
		g_player.damageTimer->Reset();

		if (g_player.HP <= 0)
		{
			PlayerDie();
		}


	}


#ifdef _DEBUG
	ImGui::Begin("Player");
	ImGui::Text("Position:%.2f, %.2f, %.2f", g_player.position.x, g_player.position.y, g_player.position.z);
	ImGui::Text("Velocity:%.2f, %.2f, %.2f", g_player.velocity.x, g_player.velocity.y, g_player.velocity.z);
	ImGui::Text("HP:%d", (int)g_player.HP);
	ImGui::Text("State:%d", (int)g_player.state);
	ImGui::End();
#endif

}

void PlayerShadowDraw()
{
	if(g_player.state == PlayerState::DEFEAT)
	{
		return;
	}

	Matrix mtx;
	mtx.world = XMMatrixIdentity();
	mtx.world *= XMMatrixScaling(g_player.scale.x, g_player.scale.y, g_player.scale.z);
	mtx.world *= XMMatrixRotationRollPitchYaw(XMConvertToRadians(g_player.rotation.x),
		XMConvertToRadians(g_player.rotation.y),
		XMConvertToRadians(g_player.rotation.z));
	mtx.world *= XMMatrixTranslation(g_player.position.x, g_player.position.y, g_player.position.z);
	mtx.matrix = GetLightViewProj();
	Shader_SetMatrix_Shadow(mtx);
	ModelDrawShadow(g_model);
}

void PlayerDraw()
{
	if(g_player.state == PlayerState::NONE)
	{
		return;
	}
	if(g_player.state == PlayerState::INVINCIBLE)
	{
		if( (g_player.damageTimer->GetElapsed() / 5) % 2 ==0)
		{
			return;
		}
	}

	if(g_player.state == PlayerState::DEFEAT)
	{
		DefeatEffectDraw();
		return;
	}

	IsDepthEnable(true);

	
	Shader_Begin_3D();
	Shader_SetTime(Time::totalTime);
	Matrix mtx;
	mtx.world = XMMatrixIdentity();
	mtx.world *= XMMatrixScaling(g_player.scale.x, g_player.scale.y, g_player.scale.z);
	mtx.world *= XMMatrixRotationRollPitchYaw(XMConvertToRadians(g_player.rotation.x),
		XMConvertToRadians(g_player.rotation.y),
		XMConvertToRadians(g_player.rotation.z));

	mtx.world *= XMMatrixTranslation(g_player.position.x, g_player.position.y, g_player.position.z);

	mtx.matrix = mtx.world;

	mtx.matrix *= GetCameraViewMatrix();
	mtx.matrix *= GetProjectionMatrix();

	Shader_SetMatrix_3D(mtx);
	float t = g_shotPower / MAX_SHOT_POWER;
	t = std::max(t, 0.05f);
	t = EaseOutQuad(t);
	Rimlight rim;
	rim.cameraPos = GetCameraPos().ToXMFLOAT3();
	rim.rimPower = 1.0f;
	rim.color = {0.5f * t,0.5f * t,1.0f * t};
	Shader_SetRimlight(rim);

	ModelDraw(g_model);
	Shader_SetTime(0.0f);
	IsDepthEnable(false);
}

void PlayerMove()
{
	Vector3 camForward = GetCameraForward();
	camForward.y = 0.0f;
	camForward.Normalize();

	Vector3 addVelocity = Vector3::Zero();

	if(Input::Key_Down(KK_W))
	{
		addVelocity += camForward * MOVE_SPEED * dt;
	}
	if(Input::Key_Down(KK_S))
	{
		addVelocity -= camForward * MOVE_SPEED * dt;
	}
	if(Input::Key_Down(KK_A))
	{
		addVelocity.x -= camForward.z * MOVE_SPEED * dt;
		addVelocity.z += camForward.x * MOVE_SPEED * dt;
	}
	if(Input::Key_Down(KK_D))
	{
		addVelocity.x += camForward.z * MOVE_SPEED * dt;
		addVelocity.z -= camForward.x * MOVE_SPEED * dt;
	}

	addVelocity.Normalize();
	g_player.velocity += addVelocity * MOVE_SPEED * dt;

	if(Input::Key_Trigger(KK_SPACE) &&
		g_player.isGround &&
		g_player.isJump == false)
	{
		g_player.velocity.y = JUMP_POWER;
		g_player.isJump = true;
	}

	g_player.velocity.y -= GRAVITY * dt;

	g_player.velocity -= g_player.velocity * DAMPING * dt;

	g_player.position += g_player.velocity * dt;


}

void PlayerCharge()
{
	if( Input::Key_Trigger(KK_W)||
		Input::Key_Trigger(KK_A)||
		Input::Key_Trigger(KK_S)||
		Input::Key_Trigger(KK_D))
	{
		g_shotPower += ADD_SHOTPOWER;
		g_shotPower = std::min(g_shotPower, MAX_SHOT_POWER);
	}

	// シュート
	if (g_ChargeTime->IsFinished())
	{
		g_player.velocity.x = cosf(g_player.shooterAngle) * SHOT_SPEED;
		g_player.velocity.y = 5.0f;
		g_player.velocity.z = sinf(g_player.shooterAngle) * SHOT_SPEED;
		g_player.state = PlayerState::SHOT;
		PlaySound(g_playerSounds[PLAYER_SHOT],0);
	}
}

void PlayerShot()
{
	CreateStarTrail(g_player.position);
	CreateStarTrail(g_player.position);
	CreateStarTrail(g_player.position);
	CreateStarTrail(g_player.position);
	CreateStarTrail(g_player.position);

	g_player.velocity -= g_player.velocity * SHOT_DAMPING * dt;
	g_player.position += g_player.velocity * dt;

	Vector2  horizontalVelocity = Vector2(g_player.velocity.x, g_player.velocity.z);
	if (CheckCollisionWithBoss())
	{
		int index = g_player.onShooter - RandomRange(0, 1);
		index < 0 ? index = 3 : index;

		DamageBoss(g_shotPower);
		CameraShake(3.0f, 45);
		HitStop(10);
		g_effectScale = Lerp(2.0f, 8.0f, g_shotPower / MAX_SHOT_POWER);
		CreateEffect(g_player.position, g_effectScale, EffectType::EXPLOSION);
		isHitStop = true;
		g_player.velocity = CalculateBallisticVelocity(
			g_player.position, RETURN_POSITIONS[index], 3.0f);
		g_player.state = PlayerState::RETURN;
		g_shotPower = 0.0f;

		// 一部シューターを無効化
		SetAllShooterActive(true);

		
		if (RandomRange(1, 100) <= 30)
		{
			// 30%の確率で隣二つを無効化
			SetShooterActive(index, false);
			SetShooterActive((index + 1) % 4, false);
		}
		else
		{
			// 50%の確率ランダムで2つ無効化
			int shooterIndex = RandomRange(0, 3);
			SetShooterActive(shooterIndex, false);
			int secondIndex = shooterIndex;
			int cnt = 0;
			do
			{
				secondIndex = RandomRange(0, 3);
				cnt++;
				if (cnt > 50)
				{
					break;
				}
			} while (secondIndex == shooterIndex);
			SetShooterActive(secondIndex, false);
		}

	}

}

void PlayerReturn()
{
	if(RandomRange(0.00f, 1.00f) < 0.7f)
	{
		CreateStarTrail(g_player.position);
	}

	g_player.velocity.y -= GRAVITY * dt;
	g_player.position += g_player.velocity * dt;


	if(g_player.isGround)
	{
		g_player.velocity = Vector3(0.0f, g_player.velocity.y * 0.3f, 0.0f);
		g_player.state = PlayerState::MOVE;
	}

}

void PlayerDefeat()
{
	g_dieAfterCounter++;

	if(g_dieAfterCounter == 10)
	{
		CaptureGameSnapshot();
	}
	for (DefeatEffect& effect : g_defeatEffects)
	{
		if (effect.isActive == false)
		{
			continue;
		}
		effect.life++;
		if (effect.life >= effect.maxLife)
		{
			effect.isActive = false;
		}

		effect.velocity.y += GRAVITY * dt;
		effect.position += effect.velocity * dt;

	}
}

void DefeatEffectDraw()
{
	IsDepthEnable(true);
	IsDepthWriteEnable(false);
	SetBlendState(ADD);

	for (DefeatEffect& effect : g_defeatEffects)
	{
		if (effect.isActive == false)
		{
			continue;
		}

		float t = (float)effect.life / effect.maxLife;

		float lerp_t = EaseOutQuad(t);

		float scale = Lerp(effect.scale, effect.scale * 2.0f, lerp_t);

		effect.color.a = lerp_t;

		BillboardDraw(effect.position,
						Vector3(scale, scale, scale),
						0.0f,
						effect.color,
						g_defeatEffectTex);


	}
	SetBlendState(ALPHABLEND);
	IsDepthEnable(false);
	IsDepthWriteEnable(true);

}

void CheckCollisionWithField()
{
	std::vector<Block> block = GetBlocks();

	// ボールのscale1.0fは正確ではないので補正する
	float playerScaleCorrection = 0.4f;
	XMFLOAT3 colScale = XMFLOAT3(	g_player.scale.x * playerScaleCorrection,
									g_player.scale.y * playerScaleCorrection,
									g_player.scale.z * playerScaleCorrection);

	float bounciness = 0.5f;
	float hitSpeed = 0.0f;
	g_player.isGround = false;

	for(int i = 0; i < (int)block.size(); i++)
	{
		BoxCollider blockCol = block[i].collider;

		switch (SphereVSBox(g_player.collider,blockCol))
		{
		case CollisionDirection::UP:
			// y軸方向の速度を反転させる
			g_player.velocity.y *= -bounciness;
			// ボールの位置をブロックの下に修正する
			g_player.position.y =
				blockCol.center.y - (blockCol.size.y * 0.5f + g_player.collider.radius);

			hitSpeed = g_player.velocity.y;
			break;

		case CollisionDirection::DOWN:
			// y軸方向の速度を反転させる
			g_player.velocity.y *= -bounciness;
			// ボールの位置をブロックの上に修正する
			g_player.position.y = 
				blockCol.center.y + (blockCol.size.y * 0.5f + g_player.collider.radius);

			hitSpeed = g_player.velocity.y;

			g_player.isGround = true;
			g_player.isJump = false;
			break;

		case CollisionDirection::LEFT:
			// x軸方向の速度を反転させる
			g_player.velocity.x *= -bounciness;
			// ボールの位置をブロックの右に修正する
			g_player.position.x =
				blockCol.center.x + (blockCol.size.x * 0.5f + g_player.collider.radius);

			hitSpeed = g_player.velocity.x;
			break;

		case CollisionDirection::RIGHT:
			// x軸方向の速度を反転させる
			g_player.velocity.x *= -bounciness;
			// ボールの位置をブロックの左に修正する
			g_player.position.x =
				blockCol.center.x - (blockCol.size.x * 0.5f + g_player.collider.radius);

			hitSpeed = g_player.velocity.x;
			break;

		case CollisionDirection::FRONT:
			// Z軸方向の速度を反転させる
			g_player.velocity.z *= -bounciness;
			// ボールの位置をブロックの奥に修正する
			g_player.position.z =
				blockCol.center.z - (blockCol.size.z * 0.5f + g_player.collider.radius);

			hitSpeed = g_player.velocity.z;
			break;

		case CollisionDirection::BACK:
			// Z軸方向の速度を反転させる
			g_player.velocity.z *= -bounciness;
			// ボールの位置をブロックの手前に修正する
			g_player.position.z =
				blockCol.center.z + (blockCol.size.z * 0.5f + g_player.collider.radius);

			hitSpeed = g_player.velocity.z;
			break;

		default:
			break;
		}

		g_player.collider.center = g_player.position;

	}
}

bool CheckCollisionWithBoss()
{
	SphereCollider bossCol = GetBossCollider();
	Vector3 push = SphereVSSphere(g_player.collider, bossCol);
	if (push != Vector3::Zero())
	{
		push *= (g_player.collider.radius + bossCol.radius);
		push *= -1.0f;
		g_player.position = bossCol.center + push;
		g_player.collider.center = g_player.position;

		return true;
	}
	return false;
}

bool CheckCollisionWithLaser()
{
	VerticalLaser* lasers = GetVerticalLasers();

	for(int i = 0; i < MAX_VERTICAL_LASER; i++)
	{
		if(lasers[i].isActive == false)
		{
			continue;
		}

		SylinderCollider laserCol = lasers[i].collider;

		Vector3 dir = SphereVSSylinder(g_player.collider, laserCol);
		if(dir != Vector3::Zero())
		{
			if(dir == Vector3::Up())
			{
				dir = Vector3(RandomRange(-1.0f, 1.0f), 0.0f, RandomRange(-1.0f, 1.0f));
			}
			else
			{
				dir.y = 0.0f;
				dir *= -1.0f;
			}

			PlayerDamage(1, dir.Normalized(), 30);
			return true;
		}
	}
	return false;
}


void CreateDefeatEffect()
{

	for(DefeatEffect& effect : g_defeatEffects)
	{
		if(effect.isActive == false)
		{
			effect.isActive = true;
			effect.position = g_player.position;
			effect.velocity = Vector3(	RandomRange(-1.0f, 1.0f),
										RandomRange(-2.0f, -1.0f),
										RandomRange(-1.0f, 1.0f));
			effect.scale = RandomRange(0.3f,0.5f);
			effect.life = 0;
			effect.maxLife = RandomRange(60, 90);
			effect.color = Color::HSVtoRGB(RandomRange(0, 360), 100, 50,1.0f);

			return;
		}
	}
}

Vector3 CalculateBallisticVelocity(Vector3 start,Vector3 end,float time)
{
	Vector3 g = Vector3(0.0f, -GRAVITY, 0.0f);

	return (end - start - g * 0.5f * time * time) / time;
}

Vector3 GetPlayerPosision()
{
	return g_player.position;
}

void SetPlayerPosision(const Vector3& pos)
{
	g_player.position = pos;
}

PlayerState GetPlayerState()
{
	return g_player.state;
}

void SetPlayerState(PlayerState state)
{
	g_player.state = state;

	if(state == PlayerState::CHARGE)
	{
		g_ChargeTime->Reset();
		g_shotPower = 0.0f;
	}
}

void SetShooter(float angle,int index)
{
	g_player.shooterAngle = angle;
	g_player.onShooter = index;
	PlaySound(g_playerSounds[PLAYER_CHARGE], 0);
}

int GetOnShooterIndex()
{
	return g_player.onShooter;
}

bool IsPlayerGround()
{
	return g_player.isGround;
}

float GetShotPowerLength()
{
	return g_shotPower;
}

SphereCollider GetPlayerCollider()
{
	return g_player.collider;
}

void PlayerDamage(int damage, const Vector3& directionXZ,int knockBackFrame)
{
	if(g_player.state != PlayerState::MOVE)
	{
		return;
	}

	g_player.HP -= damage;
	if (g_player.HP <= 0)
	{
		PlayerDie();

	}
	else
	{
		HitStop(5);
		PlaySound(g_playerSounds[PLAYER_HIT], 0);

		// ノックバック処理
		g_player.velocity = directionXZ * 10.0f;
		g_player.damageTimer->SetInterval(knockBackFrame);
		g_player.damageTimer->Reset();
		g_player.state = PlayerState::KNOCKBACK;
	}
	CameraShake(1.0f, 20);

}

void PlayerDie()
{
	g_player.state = PlayerState::DEFEAT;
	for (int i = 0; i < MAX_DEFEAT_EFFECT; i++)
	{
		CreateDefeatEffect();
	}
	HitStop(15);
	SceneChange_Fade(SCENE_RESULT, 120, 120);
	PlaySound(g_playerSounds[PLAYER_HIT], 0);
}

float GetPlayerCurrentHP()
{
	return g_player.HP;
}

