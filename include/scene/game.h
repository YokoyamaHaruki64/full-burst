#pragma once

void GameInitialize();
void GameFinalize();
void GameUpdate();
void GameDraw();

void HitStop(int frame);

bool IsHitStop();
void PlayGameBGM();

void CaptureGameSnapshot();
