#pragma once
#include "utility.h"

void FieldDecalInitialize();
void FieldDecalFinalize();
void FieldDecalUpdate();
void FieldDecalDraw();

int CreateFieldDecal(const Vector3& pos);
void AddFieldDecalPosition(int index, const Vector3& pos);
void SetFieldDecalWidth(float width);