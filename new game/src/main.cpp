// main.cpp - new game
#include "reginegine.h"
int main() {
    FilePath::SetGameRoot("new game");
    CoreEngine& engine = CoreEngine::GetInstance();
    engine.Initialize();
    engine.Run();
    engine.Shutdown();
    return 0;
}
