#pragma once
#include "Scene.h"

class Menu : public Scene {
private:
    int id;
public:
    int ENEMY_COUNT = 3;  // We can, of course, change this later
    int AMMO = 20;

    void initialise();
    void update(float delta_time) {};
    void render(ShaderProgram* program);

    void restart() {};

    int get_id() { return 0; }
    void set_id(int new_id) {};
};
