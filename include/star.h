#pragma once
#include "utility.h"
#include "directx.h"

static constexpr int STAR_MAX{1024};

class Star
{
    Vector3 pos;    // 現在の位置
    Vector3 base;   // 基準位置（ゆらぎの中心）
    Vector3 separatedBase;   // スクリーンを格子状に分割した際の位置
    Vector3 size; 
    Vector3 baseSpeed;      // ゆらぎの速度
    Vector3 stuckSpeed;     // ゆらぎの速度の管理
    Vector3 amplitude;      // ゆらぎの大きさ
    Color color;         // ゆらぎの大きさ
    float scaler;           // 大きさの変化速度
    float scale = 1.0f;     // 大きさ
    int timer = 0;
    int fadeTime;
    int maxTime;

    int index;
public:
    Star(Vector3 pos,Vector3 separatedBase,int index);
    ~Star();

    void Update();
    void Draw();
    bool isDestroy(){return timer >= maxTime; }
    Vector3 GetSeparatedBase(){return separatedBase; }
};

void StarInitialize();
void StarFinalize();
void StarUpdate();
void StarDraw();

void CreateStar(Vector3 pos);


