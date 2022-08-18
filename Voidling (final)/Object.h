#pragma once

#define GL_SILENCE_DEPRECATION

#ifdef _WINDOWS
#include <GL/glew.h>
#endif

#define GL_GLEXT_PROTOTYPES 1
#include <SDL.h>
#include <SDL_opengl.h>
#include "glm/mat4x4.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "ShaderProgram.h"
#include "Map.h"
#include <vector>

enum ObjectType { PLAYER, ANIMAL, DISH };
enum AIType { CAT, DOG, MOUSE };
enum AIState { IDLE, WALKING, ATTACKING, 
               CHASING, RUNNING, BARKING,
               RELAXING, EATING,

               FULL, EMPTY};

class Object {
private:
    int* animation_right_walk = NULL; // move to the right
    int* animation_left_walk = NULL; // move to the left

    int* animation_right_run = NULL; // move to the right
    int* animation_left_run = NULL; // move to the left

    int* animation_idle = NULL;
    int* animation_idle_happy = NULL;

    int* animation_sleep = NULL;

    glm::vec3 position;
    glm::vec3 movement;
    float speed;
    float distance = 0;

    glm::vec3 velocity;
    glm::vec3 acceleration;

    GLuint textureID;

    glm::mat4 model_matrix;

    float width = 1;
    float height = 1;
    float shift_down = height / 2;
    ObjectType type;
    AIType ai_type;
    AIState ai_state;

    int* animation_indices = NULL;

    int animation_frames = 0;
    int animation_index = 0;

    float animation_time = 0.0f;

    int animation_cols = 0;
    int animation_rows = 0;

    float dir = 1;

    bool is_win = false;
    bool in_game = true;

    bool is_active = true;

    // Colliding
    bool collided_top = false;
    bool collided_bottom = false;
    bool collided_left = false;
    bool collided_right = false;

    //bool is_about_to_fall_left = false;
    //bool is_about_to_fall_right = false;

    bool is_touching_bottom_center,
         is_touching_bottom_left,
         is_touching_bottom_right;

    bool turn_around = false;

    // Jumping
    bool is_jumping = false;
    float jumping_power = 0;
    int count_delay = 11;

    float food = 0;

    int lives = 10;

    float current_distance_m = 0;
    float current_distance_d = 0;

    int level_pos = 1;

    int relax_time = 0;

    float energy = 100.f;
    bool hunger = false;

    float spawn_time;
    float sleeping_time;

    float walking_time = 300; 
    bool button_down = false;

    int laps = 0;

public:
    int** animations = new int* [7]
    {
        animation_idle, animation_idle_happy, animation_left_walk, 
        animation_right_walk, animation_left_run, animation_right_run,
        animation_sleep
    };

    static const int SECONDS_PER_FRAME = 4;

    static const int IDL     = 0,
                     IDL_H   = 1,
                     LEFT_W  = 2,
                     RIGHT_W = 3,
                     LEFT_R  = 4,
                     RIGHT_R = 5,
                     SLEEP   = 6;
                     

    Object();
    ~Object();

    void draw_sprite_from_texture_atlas(ShaderProgram* program, GLuint textureID, int index);

    void activate_ai(Object* player, Object* objects, int object_count);
    //void ai_walker();
    //void ai_guard(Object* player);

    void ai_cat(Object* player, Object* objects, int object_count);
    void ai_mouse(Object* player);
    void ai_dog(Object* player);

    void activate_dish(Object* player);
    /*void ai_shooter(Object* player, Object* bullets);*/

    void update(float delta_time, Object* player, Object* objects, int object_count, Map* map);

    void const check_collision_y(Object* collidable_entities, int collidable_entity_count);
    void const check_collision_x(Object* collidable_entities, int collidable_entity_count);

    void const check_collision_y(Map* map);
    void const check_collision_x(Map* map);

    //void const check_pit(Map* map);

    bool const check_collision(Object* other) const;
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
    int& get_indicies();
    void set_anim_frames(int num);

    float get_dir();
    void set_dir(float new_dir);

    bool get_state();
    void set_state(bool new_state);

    bool get_status();
    void set_status(bool new_status);

    bool get_collision(int type);

    bool get_jump();
    void set_jump(bool stat);

    void set_power(float new_power);

    void set_ai_type(AIType new_type);
    void set_ai_state(AIState new_state);
    AIState get_ai_state();

    bool get_activity();
    void set_activity(bool act);

    void reset_delay();

    void clear_turnaround();

    int get_lives();
    void set_lives(int new_life);

    float get_energy();
    void set_energy(float new_energ);

    int get_level();

    bool get_button();
    void set_button(bool button);

    void set_food(float new_food);
};