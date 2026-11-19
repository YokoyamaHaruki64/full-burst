#include "frameTime.h"
#include "game.h"
#include "pause.h"

std::vector<FrameTime*>* FrameTime::instancies = nullptr;
int FrameTime::sceneFrame;
int FrameTime::totalFrame;

void FrameTime::InitializeFrameTime()
{
    sceneFrame = 0;
    totalFrame = 0;

    instancies = new std::vector<FrameTime*>;
}

void FrameTime::InitializeSceneFrameTime()
{
    sceneFrame = 0;
}

void FrameTime::FinalizeFrameTime()
{
    delete instancies;
    instancies = nullptr;
}

void FrameTime::UpdateFrameTime()
{
    if(IsHitStop())
    {
        return;
	}
    if(IsGamePaused())
    {
        return;
	}

    sceneFrame++;
    totalFrame++;

    if (instancies)
    {
        for (FrameTime* t : *instancies)
        {
            t->instanceFrame++;
            t->m_elapsed++;
        }
    }
}

FrameTime::FrameTime(int interval) : m_interval(interval), m_elapsed(0),instanceFrame(0)
{
    if (instancies)
    {
        instancies->push_back(this);
    }
}

FrameTime::~FrameTime()
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
