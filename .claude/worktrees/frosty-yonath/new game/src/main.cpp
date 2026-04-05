// main.cpp - new game
#include "reginegine.h"
int main() {
    FilePath::SetGameRoot("new game");
    GLApp::init(1920, 1080, "New Game", true);
    CoreEngine& engine = CoreEngine::GetInstance();
    engine.Initialize();
    engine.Run();
    engine.Shutdown();
    return 0;
}
