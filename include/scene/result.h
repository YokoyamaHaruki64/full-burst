#pragma once
#ifndef RESULT_H
#define RESULT_H
#include <DirectXMath.h>
using namespace DirectX;

void ResultInitialize();
void ResultFinalize();
void ResultUpdate();
void ResultDraw();

void SetBattleSuccess(bool isSuccess);
void SetResultClearTime(float clearTime);

#endif // !RESULT_H


