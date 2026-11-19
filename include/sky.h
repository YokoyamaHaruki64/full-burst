#pragma once

enum SkyModel
{
	SKY_MODEL_01,
	SKY_MODEL_02,
	SKY_MODEL_03,
	SKY_MODEL_04,
	SKY_MODEL_NUM
};

void SkyInitialize();
void SkyFinalize();
void SkyUpdate();
void SkyDraw();

void ChangeSky(SkyModel skyNum);
