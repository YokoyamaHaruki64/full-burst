#include "star.h"
#include "sprite.h"
#include "texture.h"
#include "utility.h"

static constexpr XMFLOAT4 STAR_COLOR{0.1f,0.3f,0.8f,0.0f};

static int g_starTex;

Star* g_star[STAR_MAX];

Star::Star(Vector3 pos, Vector3 separatedBase,int index):pos(pos),base(pos),separatedBase(separatedBase),index(index)
{
	float s = RandomRange(2.0f,6.0f);
	size = {s,s};
	baseSpeed = { RandomRange(0.01f,0.02f),RandomRange(0.01f,0.02f) };
	stuckSpeed = { RandomRange(0.00f,XM_2PI),RandomRange(0.00f,XM_2PI) };
	amplitude = { RandomRange(3.0f,5.0f),RandomRange(3.0f,5.0f) };
	scaler = RandomRange(0.01f, 0.02f);
	fadeTime = RandomRange(60,120);
	maxTime = fadeTime * 2 + RandomRange(60,240);
	color = { 0.3f + RandomRange(0.0f, 0.5f),0.3f + RandomRange(0.0f, 0.5f),0.3f + RandomRange(0.0f, 0.5f),1.0f };
}

Star::~Star()
{
	g_star[index] = nullptr;
}

void Star::Update()
{
	stuckSpeed += baseSpeed;
	pos.x = base.x + sinf(stuckSpeed.x) * amplitude.x;
	pos.y = base.y + sinf(stuckSpeed.y) * amplitude.y;
	scale = (sinf(scaler * timer) + 2.0f) * 0.5f;

	if (timer <= fadeTime)
	{
		color.a = (float)timer / fadeTime;
	}
	else if (timer >= maxTime - fadeTime)
	{
		float correctTimer = timer - (maxTime - fadeTime);
		color.a = 1.0f - correctTimer / fadeTime;
	}
	timer++;
}

void Star::Draw()
{
	SpriteDraw(pos, size, { scale,scale }, 0,color,g_starTex);
	SpriteDraw(pos, size, { scale,scale }, 0,color,g_starTex);
}

void StarInitialize()
{
	for (int i = 0; i < STAR_MAX; i++)
	{
		g_star[i] = nullptr;
	}
	g_starTex = TextureLoad(L"asset\\texture\\star.png");

	for (int x = 0; x < 28; x++)
	{
		for (int y = 0; y < 12; y++)
		{
			Vector3 pos{(float)WINDOW_WIDTH / 28 * (x + 1),(float)WINDOW_HEIGHT / 12 * (y + 1) };
			CreateStar(pos);
		}
	}

}

void StarFinalize()
{
	for (int i = 0; i < STAR_MAX; i++)
	{
		delete g_star[i];
		g_star[i] = nullptr;
	}
}

void StarUpdate()
{
	auto start = std::chrono::high_resolution_clock::now();
	for (int i = 0; i < STAR_MAX; i++)
	{
		if (g_star[i])
		{
			g_star[i]->Update();
			if (g_star[i]->isDestroy())
			{
				CreateStar(g_star[i]->GetSeparatedBase());
				delete g_star[i];
			}
		}
	}
}

void StarDraw()
{
	SetBlendState(ADD);
	IsDepthEnable(false);
	for (int i = 0; i < STAR_MAX; i++)
	{
		if (g_star[i])
		{
			g_star[i]->Draw();
		}
	}
	IsDepthEnable(true);
	SetBlendState(ALPHABLEND);

}



void CreateStar(Vector3 pos)
{
	for (int i = 0; i < STAR_MAX; i++)
	{
		if (!g_star[i])
		{
			Vector3 offset{RandomRange(-150.0f,150.0f),RandomRange(-150.0f,150.0f)};
			g_star[i] = new Star(pos + offset,pos,i);
			break;
		}
	}
}

