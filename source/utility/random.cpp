#include "random.h"
#include <random>

// グローバル乱数エンジン
static std::mt19937 g_engine;

void InitRandom()
{
    std::random_device rd;
    g_engine.seed(rd());
}

float RandomRange(float min, float max)
{
    std::uniform_real_distribution<float> dist(min, max);
    return dist(g_engine);
}

int RandomRange(int min, int max)
{
    std::uniform_int_distribution<int> dist(min, max);
    return dist(g_engine);
}

Vector3 RandomVector3(const Vector3& min, const Vector3& max)
{
	Vector3 result;
	result.x = RandomRange(min.x, max.x);
	result.y = RandomRange(min.y, max.y);
	result.z = RandomRange(min.z, max.z);
	return result;
}
