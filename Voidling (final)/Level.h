#pragma once
#include "Scene.h"

class Level : public Scene {
private:
    int id;

    float timer = 120000;

    //bool is_lose = false;
    bool is_win = false;
    bool is_game_over = false;

    unsigned int LEVEL_DATA[112] = {0};
public:
    //Level();
    Level(unsigned int* NEW_LEVEL_DATA, int size);

    int NUM_ANIM = 2;

    ~Level();

    void initialise();
    void update(float delta_time);
    void render(ShaderProgram* program);
    void restart();

    int get_id();
    void set_id(int new_id);

    bool get_game();
    void set_game(bool new_stat);
    bool get_lose();
    bool get_win();

    float get_timer();
    void set_timer(float curr_time);
};
