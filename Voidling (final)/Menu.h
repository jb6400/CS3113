#pragma once
#include "Scene.h"

class Menu : public Scene {
private:
    int id;
    glm::mat4 model_matrix = glm::mat4(1.f);

public:
    int ENEMY_COUNT = 3;  // We can, of course, change this later
    int AMMO = 20;

    void initialise();
    void update(float delta_time) {};
    void render(ShaderProgram* program);

    void restart() {};

    int get_id() { return 0; }
    void set_id(int new_id) {};

    bool get_game() { return false; }
    void set_game(bool new_stat) {};
    //bool get_lose() { return false; }

    bool get_win() {return false;}

    float get_timer() { return 0.f; }
    void set_timer(float curr_time) {};
};
