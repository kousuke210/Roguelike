#pragma once

// ÉVÅ[ÉìÇÃéÌóﬁ
enum E_SCENE_TYPE {
    SCENE_TITLE,
    SCENE_PLAY,
    SCENE_GAMEOVER,
    SCENE_GAMECLEAR
};

class SceneManager {
public:
    SceneManager() : currentScene(SCENE_TITLE) {}

    void SetScene(E_SCENE_TYPE nextScene) { currentScene = nextScene; }
    E_SCENE_TYPE GetScene() const { return currentScene; }

private:
    E_SCENE_TYPE currentScene;
};