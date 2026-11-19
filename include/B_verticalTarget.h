#pragma once
#include "vector3.h"

void VerticalTargetInitialize();
void VerticalTargetFinalize();
void VerticalTargetUpdate();
void VerticalTargetDraw();

bool CreateVerticalTarget(const Vector3& pos, const int lockonFrame, const float radius);


