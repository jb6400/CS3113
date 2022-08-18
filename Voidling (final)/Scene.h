#pragma once
#define GL_SILENCE_DEPRECATION

#ifdef _WINDOWS
#include <GL/glew.h>
#endif

#define GL_GLEXT_PROTOTYPES 1
#include <SDL_mixer.h>
#include <SDL.h>
#include <SDL_opengl.h>
#include "glm/mat4x4.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "ShaderProgram.h"
#include "Utility.h"
#include "Object.h"
#include "Map.h"

/**
    Notice that the game's state is now part of the Scene class, not the main file.
*/
struct GameState
{
    Map* map = NULL;
    Object* player = NULL;
    Object* pet = NULL;
    Object* enemies = NULL;

    Mix_Music* bgm = NULL;
    Mix_Chunk* click_sfx = NULL;
    Mix_Chunk* meow_sfx = NULL;

    int next_scene_id;
};

class Scene {
public:
    int ENEMY_COUNT = 3;  // We can, of course, change this later
    int AMMO = 20;

    GameState state;

    virtual void initialise() = 0;
    virtual void update(float delta_time) = 0;
    virtual void render(ShaderProgram* program) = 0;

    virtual void restart() = 0;

    virtual int get_id() = 0;
    virtual void set_id(int new_id) = 0;

    virtual bool get_game() = 0;
    virtual void set_game(bool new_stat) = 0;

    //virtual bool get_lose() = 0;
    virtual bool get_win() = 0;

    virtual float get_timer() = 0;
    virtual void set_timer(float curr_time) = 0;

    GameState const get_state() const { return this->state; }
};
