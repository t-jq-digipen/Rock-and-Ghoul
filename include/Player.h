#pragma once
#ifndef PLAYER_H
#define PLAYER_H

class Player {
public:
    Player(float startX, float startY);

    // Copy constructor (for cloning)
    Player(const Player& other);

    void Update(float dt);
    void Draw();

private:
    float x, y;
    float size;
    float speed;
};

#endif // PLAYER_H
