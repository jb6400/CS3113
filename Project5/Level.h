#pragma once
#include "Scene.h"

class Level : public Scene {
private:

    glm::vec3 player_init_pos;
    glm::vec3 player_retry_pos;

    glm::vec3 enem1_init_pos;
    glm::vec3 enem2_init_pos;
    glm::vec3 enem3_init_pos;

    int id;

    unsigned int LEVEL_DATA[112] = {0};
public:
    int ENEMY_COUNT = 3;
    int AMMO = 20;

    //Level();
    //Level(unsigned int* NEW_LEVEL_DATA, int size);
    Level(unsigned int* NEW_LEVEL_DATA, int size_l,
          glm::vec3* NEW_OBJ_POS, int size_o);

    ~Level();

    void initialise();
    void update(float delta_time);
    void render(ShaderProgram* program);
    void restart();

    int get_id();
    void set_id(int new_id);
};
