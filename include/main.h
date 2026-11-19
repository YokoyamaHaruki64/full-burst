#pragma once

enum Scene
{
	SCENE_NONE,
	SCENE_TITLE,
	SCENE_GAME,
	SCENE_RESULT
};

void SetScene(Scene next);
const Scene GetScene();

void EndApplication();

