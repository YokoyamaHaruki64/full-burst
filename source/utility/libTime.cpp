#include "libTime.h"
#include "pause.h"

std::chrono::high_resolution_clock::time_point Time::lastFrame;
std::vector<Time*>* Time::instancies = nullptr;
float Time::deltaTime;
float Time::sceneTime;
float Time::totalTime;

void Time::InitializeTime()
{
    deltaTime = 0.0f;
    sceneTime = 0.0f;
    totalTime = 0.0f;
    lastFrame = std::chrono::high_resolution_clock::now();

    instancies = new std::vector<Time*>;
}

void Time::InitializeSceneTime()
{
    sceneTime = 0.0f;
}

void Time::FinalizeTime()
{
    delete instancies;
    instancies = nullptr;
}

void Time::UpdateTime()
{
    if(IsGamePaused())
    {
        deltaTime = 0.0f;
        return;
	}

    auto now = std::chrono::high_resolution_clock::now();
    float dt = std::chrono::duration<float>(now - lastFrame).count();
    lastFrame = now;

    deltaTime = dt;
    sceneTime += dt;
    totalTime += dt;

    if (instancies)
    {
        for (Time* t : *instancies)
        {
            t->m_elapsed += dt;
            t->instanceTime += dt;
        }
    }
}

Time::Time(float interval) : m_interval(interval), m_elapsed(0.0f), instanceTime(0.0f)
{
    if (instancies)
    {
        instancies->push_back(this);
    }
}

Time::~Time()
{
    if (instancies)
    {
        auto it = std::find(instancies->begin(), instancies->end(), this);
        if (it != instancies->end())
        {
            instancies->erase(it);
        }
    }
}
