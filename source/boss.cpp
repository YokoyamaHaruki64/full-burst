#include "boss.h"
#include "directx.h"
#include "model.h"
#include "shader.h"
#include "camera.h"
#include "utility.h"
#include "light.h"
#include "input.h"
#include "sound.h"
#include "game.h"
#include "fade.h"
#include "result.h"
#include "gameTimer.h"
#include "sky.h"

#include "B_verticalTarget.h"
#include "B_radialLaser.h"
#include "player.h"
#include "effect.h"
#include "shooter.h"
#include "field.h"

#define INT(x) static_cast<int>(x)


struct Boss
{
	Vector3 position[3]{ Vector3::Zero() };
	Vector3 scale	[3]{ Vector3::One()  };
	Vector3 rotation[3]{ Vector3::Zero() };

	BossState state = BossState::PHASE1;
	SphereCollider collider;

	float HP = BOSS_MAX_HP;

	FrameTime* targetInterval;
	FrameTime* radialInterval;
	FrameTime* randomLaserIntervel;

};

enum BossSound
{
	BOSS_HIT,
	BOSS_ROCKON,
	BOSS_CHANGE_PHASE,
	BOSS_ALERT,
	BOSS_RADIAL_LASER,
	BOSS_RADIAL_LASER_2,
	BOSS_EXPLOSION,

	BOSS_SOUND_MAX,
};

static constexpr float BOSS_SCALE = 5.0f;
static constexpr float BOSS_CORE_SCALE = 2.0f;
static constexpr float MIN_OFFSET_Y = 4.0f;
static constexpr float BASE_OFFSET_Y = 5.0f;
static constexpr int TARGET_INTERVAL[INT(BossState::PHASE_MAX)]
= { 120, 90, 60 };

static constexpr int RADIAL_INTERVAL[INT(BossState::PHASE_MAX)]
= { 99999999, 300, 150 };

static constexpr int RANDOM_LASER_INTERVAL[INT(BossState::PHASE_MAX)]
= { 90, 120, 80 };

static constexpr int RANDOM_LASER_NUM[INT(BossState::PHASE_MAX)]
= { 30,30,50};

static constexpr float ROTATE_SPEED[INT(BossState::PHASE_MAX)]
 = { 2.0f, 5.0f, 10.0f };

static constexpr float MOVE_SPEED[INT(BossState::PHASE_MAX)]
= { 0.05f, 0.1f, 0.3f };

static constexpr float AMPLITUDE_Y[INT(BossState::PHASE_MAX)]
= { 1.0f, 1.3f, 2.0f };

static constexpr Color CLEAR_COLOR[INT(BossState::PHASE_MAX)]
= {
	Color(0.0f,  0.0f,  0.2f, 1.0f),
	Color(0.15f, 0.0f, 0.2f, 1.0f),
	Color(0.25f, 0.0f, 0.15f, 1.0f),
	Color(0.4f, 0.0f, 0.1f, 1.0f),
};

static constexpr float RADIALLASER_RANGE = 30.0f;
static constexpr int RADIALLASER_NUM = 13;
static constexpr int DEFEAT_DELAY = 360;
static constexpr int PHASE4_DURATION = DEFEAT_DELAY - 30;
static constexpr int EXPLOSION_INTERVAL = 75;

void Phase1Behavior();
void Phase2Behavior();
void Phase3Behavior();
void Phase4Behavior();
void ChangePhase(BossState nextState);
void RadialLaser();
void RandomVerticalTarget();
bool IsAttackablePlayer();

static Boss g_boss;
static MODEL* g_bossModel[3];
static FrameTime* g_defeatDelay;
static FrameTime* g_radialLaserSoundFrag;
static Vector3 g_lastBasePos[3]{ Vector3::Zero() };
static int g_bossSounds[BOSS_SOUND_MAX];

void BossInitialize()
{
	g_boss.position[0] = FIELD_CENTER + Vector3(0.0f, 5.0f, 0.0f);
	g_boss.scale[0] = Vector3(BOSS_CORE_SCALE, BOSS_CORE_SCALE, BOSS_CORE_SCALE);
	g_boss.rotation[0] = Vector3(0.0f, 0.0f, 0.0f);

	for(int i = 1,j = -1; i < 3; i++,j+=2)
	{
		g_boss.position[i] = g_boss.position[0] + Vector3(0.0f, j * MIN_OFFSET_Y,0.0f );
		g_boss.scale[i] = Vector3(BOSS_SCALE, BOSS_SCALE, BOSS_SCALE);
		g_boss.rotation[i] = Vector3::Zero();
	}
	g_boss.HP = BOSS_MAX_HP;
	g_boss.collider.center = g_boss.position[0];
	g_boss.collider.radius = BOSS_CORE_SCALE;
	g_bossModel[0] = ModelLoad("asset\\model\\bossBall.fbx");
	g_bossModel[1] = ModelLoad("asset\\model\\boss01.fbx");
	g_bossModel[2] = ModelLoad("asset\\model\\boss01.fbx");

	g_boss.targetInterval = new FrameTime(9999999);
	g_boss.radialInterval = new FrameTime(9999999);
	g_boss.randomLaserIntervel = new FrameTime(9999999);
	g_defeatDelay = new FrameTime(DEFEAT_DELAY);
	g_radialLaserSoundFrag = new FrameTime(99999999);
	g_boss.state = BossState::TUTORIAL;
	ChangeSky(SKY_MODEL_01);

	g_bossSounds[BOSS_HIT] = LoadSound("asset\\sound\\attackHit.wav");
	g_bossSounds[BOSS_ROCKON] = LoadSound("asset\\sound\\rockon.wav");
	g_bossSounds[BOSS_CHANGE_PHASE] = LoadSound("asset\\sound\\toPhase2.wav");
	g_bossSounds[BOSS_ALERT] = LoadSound("asset\\sound\\bossAlert.wav");
	g_bossSounds[BOSS_RADIAL_LASER] = LoadSound("asset\\sound\\radialLaser.wav");
	g_bossSounds[BOSS_RADIAL_LASER_2] = LoadSound("asset\\sound\\radialLaser2.wav");
	g_bossSounds[BOSS_EXPLOSION] = LoadSound("asset\\sound\\bossExplosion.wav");
	SetVolume(g_bossSounds[BOSS_ROCKON], 0.7f);

	SetRadialLaserDamage(0.8f);
}

void BossFinalize()
{
	for (int i = 0; i < 3; i++)
	{
		ModelRelease(g_bossModel[i]);
	}
	delete g_boss.targetInterval;
	delete g_boss.radialInterval;
	delete g_boss.randomLaserIntervel;

	delete g_defeatDelay;
	delete g_radialLaserSoundFrag;
}

void BossUpdate()
{
	if (g_radialLaserSoundFrag->GetInterval() < 1000)
	{
		if (g_radialLaserSoundFrag->IsFinished())
		{
			StopSound(g_bossSounds[BOSS_RADIAL_LASER]);
			StopSound(g_bossSounds[BOSS_RADIAL_LASER_2]);
			g_radialLaserSoundFrag->SetInterval(1000);
		}
	}

	switch (g_boss.state)
	{
	case BossState::TUTORIAL:
		return; // チュートリアル中は何もしない
	case BossState::PHASE1:
		Phase1Behavior();
		break;

	case BossState::PHASE2:
		Phase2Behavior();
		break;

	case BossState::PHASE3:
		Phase3Behavior();
		break;

	case BossState::PHASE4:
		Phase4Behavior();
		return; // ラストは動かない
		break;

	case BossState::DEFEAT:

		if (g_defeatDelay->GetElapsed() == 10)
		{
			CreateEffect(g_boss.position[0], 25.0f, EffectType::EXPLOSION_BIG);
			PlaySound(g_bossSounds[BOSS_EXPLOSION], 0);
		}

		if (g_defeatDelay->GetElapsed() > EXPLOSION_INTERVAL)
		{
			for (int i = 0; i < 3; i++)
			{
				g_boss.position[i].y -= 0.05f;
				g_boss.rotation[i].z = 10.0f;
			}

			if (g_boss.position[0].y < -1.0f)
			{
				SceneChange_Fade(SCENE_RESULT, 240, 0);
				g_boss.state = BossState::NONE;
			}
		}

		return;// 早期リターンであとの処理を無視
		break;

	case BossState::NONE:
		for (int i = 0; i < 3; i++)
		{
			g_boss.position[i].y -= 0.05f;
			g_boss.rotation[i].z = 10.0f;

		}
		return;// 早期リターンであとの処理を無視
		break;

	default:
		break;
	}

	for (int i = 1, j = -1; i < 3; i++, j += 2)
	{
		g_boss.rotation[i].y += ROTATE_SPEED[INT(g_boss.state)] * j;
		float offsetY = sinf(FrameTime::totalFrame * MOVE_SPEED[INT(g_boss.state)]) * 1.0f * j;
		g_boss.position[i].y = g_boss.position[0].y + j * (MIN_OFFSET_Y + AMPLITUDE_Y[INT(g_boss.state)]) + offsetY;
	}


	if(g_boss.targetInterval->IsFinished())
	{
		if (IsAttackablePlayer())
		{
			PlaySound(g_bossSounds[BOSS_ROCKON], 0);

			Vector3 targetPos = GetPlayerPosision();
			targetPos.y += 10.0f;
			CreateVerticalTarget(targetPos, 30, 0.5f);
		}
	}

	if(g_boss.radialInterval->IsFinished())
	{
		if (IsAttackablePlayer())
		{
			RadialLaser();
		}
	}

	if(g_boss.randomLaserIntervel->IsFinished())
	{
		if (IsAttackablePlayer())
		{
			for (int i = 0; i < RANDOM_LASER_NUM[INT(g_boss.state)]; i++)
			{
				RandomVerticalTarget();
			}
		}
	}

}

void BossDraw()
{

	Light light = GetLight();
	light.ambientIntensity = 0.5f;
	Shader_SetLight(light);

	IsDepthEnable(true);
	Shader_Begin_3D();

	Matrix mtx;
	Rimlight rim;
	rim.cameraPos = GetCameraPos().ToXMFLOAT3();
	rim.color = XMFLOAT3(1.0f, 0.5f, 0.5f);

	switch (g_boss.state)
	{
	case BossState::TUTORIAL:
		rim.color = XMFLOAT3(0.0f,0.0f,0.0f);
		break;
	case BossState::PHASE1:
		rim.color = XMFLOAT3(0.0f,0.0f,0.0f);
		break;
	case BossState::PHASE2:
		rim.rimPower = 2.0f;
		break;
	case BossState::PHASE3:
		rim.rimPower = 1.0f;
		break;
	case BossState::DEFEAT:
	case BossState::NONE:
		rim.color = XMFLOAT3(0.0f, 0.0f, 0.0f);
		break;
	default:
		break;
	}
	Shader_SetRimlight(rim);

	for(int i = 0 ; i < 3; i++)
	{
		// 本体描画
		mtx.world = XMMatrixIdentity();
		mtx.world *= XMMatrixRotationX(XMConvertToRadians(90.0f * (i % 2 * 2 - 1)));
		mtx.world *= XMMatrixScaling(g_boss.scale[i].x, g_boss.scale[i].y, g_boss.scale[i].z);
		mtx.world *= XMMatrixRotationRollPitchYaw(XMConvertToRadians(g_boss.rotation[i].x),
			XMConvertToRadians(g_boss.rotation[i].y),
			XMConvertToRadians(g_boss.rotation[i].z));
		mtx.world *= XMMatrixTranslation(g_boss.position[i].x, g_boss.position[i].y, g_boss.position[i].z);
		mtx.matrix = mtx.world;
		mtx.matrix *= GetCameraViewMatrix();
		mtx.matrix *= GetProjectionMatrix();
		Shader_SetMatrix_3D(mtx);
		ModelDraw(g_bossModel[i]);
	}
	IsDepthEnable(false);

	Shader_SetLight(GetLight());

}

void Phase1Behavior()
{
	// フェーズ移行
	if (g_boss.HP <= BOSS_MAX_HP / 2)
	{
		ChangePhase(BossState::PHASE2);
		g_boss.HP = BOSS_MAX_HP / 2;


		HitStop(20);
	}

}

void Phase2Behavior()
{
	// フェーズ移行
	if (g_boss.HP <= BOSS_MAX_HP / 10)
	{
		ChangePhase(BossState::PHASE3);
		g_boss.HP = BOSS_MAX_HP / 10;

		HitStop(20);

	}
}

void Phase3Behavior()
{
	// フェーズ移行
	if (g_boss.HP <= 0.0f)
	{
		// ラストフェーズへ（特殊仕様）
		g_boss.state = BossState::PHASE4;
		g_boss.targetInterval->SetInterval(999999);
		g_boss.radialInterval->SetInterval(300);
		g_boss.randomLaserIntervel->SetInterval(999999);
		g_boss.targetInterval->Reset();
		g_boss.radialInterval->Reset();
		g_boss.randomLaserIntervel->Reset();
		ChangeSky(SKY_MODEL_04);
		SetAllShooterActive(false);
		CaptureGameSnapshot();

		PlaySound(g_bossSounds[BOSS_ALERT], 0);

		// 基準位置保存
		for(int i = 0; i < 3; i++)
		{
			g_lastBasePos[i] = g_boss.position[i];
		}

		// 防壁生成
		const Vector3 OFFSETS[4] = {
			Vector3(0.0f,0.0f,2.0f),
			Vector3(-2.0f,0.0f,0.0f),
			Vector3(0.0f,0.0f,-2.0f),
			Vector3(2.0f,0.0f,0.0f),
		};

		for(int i = 0; i < 4; i++)
		{
			CreateWall(RETURN_POSITIONS[i] + OFFSETS[i]);
		}

		HitStop(20);

	}
}

void Phase4Behavior()
{
	// ラスト
	if(g_boss.radialInterval->IsFinished())
	{
		SetRadialLaserDamage(15.0f);	// 大ダメージに変更
		// 最後のあがきとして放射状レーザーを放つ
		float toPlayerAngle = atan2f(
			GetPlayerPosision().z - g_boss.position[0].z,
			GetPlayerPosision().x - g_boss.position[0].x);
		toPlayerAngle = XMConvertToDegrees(toPlayerAngle);

		const int finalLaserNum = 50;
		for (int i = 0; i < finalLaserNum; i++)
		{
			float angle = toPlayerAngle +
				Lerp(-180.0f, 180.0f, i / static_cast<float>(finalLaserNum - 1));
			angle = XMConvertToRadians(angle);

			float dirX = cosf(angle);
			float dirZ = sinf(angle);
			Vector3 offset = Vector3(dirX, 0.0f, dirZ) * BOSS_CORE_SCALE;
			CreateRadialLaser(g_lastBasePos[0] + offset, Vector3(dirX, -0.5f, dirZ), 1.5f, PHASE4_DURATION);
		}

		PlaySound(g_bossSounds[BOSS_RADIAL_LASER_2], 0);
		g_radialLaserSoundFrag->SetInterval(PHASE4_DURATION);
		g_radialLaserSoundFrag->Reset();

		g_boss.radialInterval->SetInterval(10000); // もう撃たない

		g_defeatDelay->Reset();
	}

	if( g_boss.radialInterval->GetInterval() > 9999 && 
		GetPlayerState() != PlayerState::DEFEAT)
	{
		// ボス消滅待機
		if(g_defeatDelay->IsFinished())
		{
			g_boss.state = BossState::DEFEAT;
			ChangeSky(SKY_MODEL_01);	// 空を元に戻す
			g_defeatDelay->Reset();
			SetResultClearTime(GetGameTime());
			SetBattleSuccess(true);
		}
	}

	for (int i = 0; i < 3; i++)
	{
		// 震える
		Vector3 offset;
		const float shakeAmount = 0.2f;
		offset.x = RandomRange(-shakeAmount, shakeAmount);
		offset.y = RandomRange(-shakeAmount, shakeAmount);
		offset.z = RandomRange(-shakeAmount, shakeAmount);
		g_boss.position[i] = g_lastBasePos[i] + offset;
	}

}

void ChangePhase(BossState nextState)
{
	g_boss.state = nextState;
	g_boss.targetInterval->SetInterval(TARGET_INTERVAL[INT(nextState)]);
	g_boss.radialInterval->SetInterval(RADIAL_INTERVAL[INT(nextState)]);
	g_boss.randomLaserIntervel->SetInterval(RANDOM_LASER_INTERVAL[INT(nextState)]);
	g_boss.targetInterval->Reset();
	g_boss.radialInterval->Reset();
	g_boss.randomLaserIntervel->Reset();
	PlaySound(g_bossSounds[BOSS_CHANGE_PHASE], 0);
	ChangeSky(static_cast<SkyModel>(INT(nextState)));

}

void RadialLaser()
{
	float toPlayerAngle = atan2f(
		GetPlayerPosision().z - g_boss.position[0].z,
		GetPlayerPosision().x - g_boss.position[0].x);
	toPlayerAngle = XMConvertToDegrees(toPlayerAngle);


	for (int i = 0; i < RADIALLASER_NUM; i++)
	{
		float angle = toPlayerAngle + 
			Lerp(-RADIALLASER_RANGE, RADIALLASER_RANGE, i / static_cast<float>(RADIALLASER_NUM - 1));
		angle = XMConvertToRadians(angle);

		float dirX = cosf(angle);
		float dirZ = sinf(angle);
		Vector3 offset = Vector3(dirX, 0.0f, dirZ) * BOSS_CORE_SCALE;
		CreateRadialLaser(g_boss.position[0] + offset, Vector3(dirX, -0.5f, dirZ));
	}
	PlaySound(g_bossSounds[BOSS_RADIAL_LASER], 0);
	// 音再生用タイマーリセット
	g_radialLaserSoundFrag->SetInterval(120);
	g_radialLaserSoundFrag->Reset();
}

void RandomVerticalTarget()
{
	Vector3 targetPos;
	int tryCount = 0;

	// ホール内に出現しないようにする
	do
	{
		targetPos.x = RandomRange(FIELD_MIN.x, FIELD_MAX.x);
		targetPos.z = RandomRange(FIELD_MIN.z, FIELD_MAX.z);

		tryCount++;

		if(tryCount > 100)
		{
			// 無限ループ防止
			break;
		}

	} while (	(targetPos.x >= HALL_MIN.x && targetPos.x <= HALL_MAX.x) && 
				(targetPos.z >= HALL_MIN.z && targetPos.z <= HALL_MAX.z));

	targetPos.y = GetPlayerPosision().y;
	CreateVerticalTarget(targetPos, 50, 0.25f);
}

bool IsAttackablePlayer()
{
	PlayerState state = GetPlayerState();
	return (state != PlayerState::CHARGE &&
		state != PlayerState::SHOT &&
		state != PlayerState::RETURN);
}

SphereCollider GetBossCollider()
{
	return g_boss.collider;
}

float GetBossCurrentHP()
{
	return g_boss.HP;
}

void DamageBoss(float damage)
{
	PlaySound(g_bossSounds[BOSS_HIT], 0);
	if(g_boss.state == BossState::TUTORIAL)
	{
		// 一発与えたらPHASE1へ移行
		ChangePhase(BossState::PHASE1);
		PlayGameBGM();
		return;
	}

	g_boss.HP -= damage;
}

BossState GetBossState()
{
	return g_boss.state;
}