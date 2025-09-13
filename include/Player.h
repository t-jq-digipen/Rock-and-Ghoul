#ifndef PLAYER_H
#define PLAYER_H

class Player {
public:
    float x, y;   // position
    float size;     // square size
    float speed;  // units per second

    Player(float startX = 0.0f, float startY = 0.0f);

    void Update(float dt);  // update position using Input
    void Draw();            // for now, debug print
};

#endif // PLAYER_H
