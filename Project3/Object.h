#pragma once

#define GL_SILENCE_DEPRECATION
#define STB_IMAGE_IMPLEMENTATION

#ifdef _WINDOWS
#include <GL/glew.h>
#endif

#define GL_GLEXT_PROTOTYPES 1
#include <SDL.h>
#include <SDL_opengl.h>
#include "glm/mat4x4.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "ShaderProgram.h"
#include <vector>
enum ObjectType {WIN, LOSE, PLAYER};

class Object {
private:
    int* animation_right = NULL; // move to the right
    int* animation_left = NULL; // move to the left
    int* animation_idle = NULL;

    glm::vec3 position;
    glm::vec3 movement;
    float speed;

    glm::vec3 velocity;
    glm::vec3 acceleration;

    GLuint textureID;

    glm::mat4 model_matrix;

    float width = 1;
    float height = 1;

    ObjectType type;

    int* animation_indices = NULL;

    int animation_frames = 0;
    int animation_index = 0;

    float animation_time = 0.0f;

    int animation_cols = 0;
    int animation_rows = 0;

    float dir = 1;

    bool is_win = false;
    bool in_game = true;

public:
    int** animations = new int* [3]
    {
        animation_left, animation_right, animation_idle
    };

    static const int SECONDS_PER_FRAME = 4;

    static const int LEFT = 0,
                     RIGHT = 1,
                     IDLE = 2;

    Object();
    ~Object();

    void draw_sprite_from_texture_atlas(ShaderProgram* program, GLuint textureID, int index);
    void update(float delta_time, Object* colliders, int num_collider);
    void render(ShaderProgram* program);

    bool const check_collision(Object* other);

    //getters and setters
    glm::vec3 get_position();
    void set_position(glm::vec3 new_pos);

    glm::vec3 get_movement();
    void set_movement(glm::vec3 new_mov);

    float get_speed();
    void set_speed(float new_speed);

    GLuint get_texture();
    void set_texture(GLuint new_tex);

    glm::vec3 get_acceleration();
    void set_acceleration(glm::vec3 new_acc);

    void set_velocity(glm::vec3 new_vel);

    float get_width();
    void set_width(float new_width);

    float get_height();
    void set_height(float new_height);

    ObjectType get_type();
    void set_type(ObjectType new_type);

    void set_anim_cols(int num);
    void set_anim_rows(int num);

    void set_indicies(int* new_ind);
    void set_anim_frames(int num);

    void set_dir(float new_dir);

    bool get_state();
    bool get_status();
};