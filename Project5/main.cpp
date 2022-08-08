#define GL_SILENCE_DEPRECATION

#define GL_GLEXT_PROTOTYPES 1
#define FIXED_TIMESTEP 0.0166666f

#define LEVEL1_WIDTH 14
#define LEVEL1_HEIGHT 8
#define LEVEL1_LEFT_EDGE 5.0f

#ifdef _WINDOWS
#include <GL/glew.h>
#endif

#include <SDL_mixer.h>
#include <SDL.h>
#include <SDL_opengl.h>
#include "glm/mat4x4.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "ShaderProgram.h"
#include "cmath"
#include <ctime>
#include <vector>

#include "Utility.h"
#include "Scene.h"
#include "Scene.h"
#include "Level.h"
#include "Menu.h"

Scene* curr_scene;

Menu* main_menu;
Level* level_1;
Level* level_2;
Level* level_3;

Scene* levels[3];

const int WINDOW_WIDTH = 640,
          WINDOW_HEIGHT = 480;

const float BG_RED = 0.1769f,
            BG_BLUE = 0.2f,
            BG_GREEN = 0.6231f,
            BG_OPACITY = 1.0f;

const int VIEWPORT_X = 0,
          VIEWPORT_Y = 0,
          VIEWPORT_WIDTH = WINDOW_WIDTH,
          VIEWPORT_HEIGHT = WINDOW_HEIGHT;

const char V_SHADER_PATH[] = "shaders/vertex_textured.glsl",
           F_SHADER_PATH[] = "shaders/fragment_textured.glsl";

const char TEXT_FILEPATH[] = "pixel_font.png";
const char BACKGROUND_FILEPATH[] = "background_3.png";

SDL_Window* display_window;
bool game_is_running = true;

ShaderProgram program;
glm::mat4 bg_model_matrix, view_matrix, projection_matrix;

float previous_ticks = 0.0f;
float accumulator = 0.0f;

bool is_lose = false;
bool is_win = false;
bool in_game = true;

int level_num = 0;

GLuint text_texture_id, background_texture_id;

glm::vec3 OBJECT_POS[3][5]
{
    {
        glm::vec3(2.0f, 0.0f, 0.0f),
        glm::vec3(2.f, -4.f, 0.f),
        glm::vec3(4.5f, -3.0f, 0.0f),
        glm::vec3(10.f, -4.0f, 0.0f),
        glm::vec3(10.5f, -1.0f, 0.0f),
    },
    {
        glm::vec3(2.0f, -4.0f, 0.0f),
        glm::vec3(2.f, -4.f, 0.f),
        glm::vec3(4.5f, -3.0f, 0.0f),
        glm::vec3(10.f, -1.0f, 0.0f),
        glm::vec3(10.5f, -1.0f, 0.0f),
    },
    {
        glm::vec3(2.0f, -4.0f, 0.0f),
        glm::vec3(2.f, -4.f, 0.f),
        glm::vec3(9.5f, -1.0f, 0.0f),
        glm::vec3(4.5f, -3.0f, 0.0f),
        glm::vec3(7.f, -3.0f, 0.0f)
    }
};

unsigned int LEVEL_1_DATA[] =
{
    3, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    3, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    3, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    3, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    3, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0,
    3, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1,
    3, 1, 1, 1, 0, 0, 1, 1, 1, 2, 2, 2, 2, 2,
    3, 2, 2, 2, 0, 0, 2, 2, 2, 2, 2, 2, 2, 2
};

unsigned int LEVEL_2_DATA[] =
{
    3, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    3, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1,
    3, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 2, 2,
    3, 0, 0, 0, 0, 0, 0, 0, 1, 2, 2, 2, 2, 2,
    3, 0, 0, 0, 1, 1, 0, 1, 2, 2, 2, 2, 2, 2,
    3, 0, 0, 0, 0, 0, 0, 2, 2, 2, 2, 2, 2, 2,
    3, 1, 1, 1, 1, 0, 0, 2, 2, 2, 2, 2, 2, 2,
    3, 2, 2, 2, 2, 0, 0, 2, 2, 2, 2, 2, 2, 2
};

unsigned int LEVEL_3_DATA[] =
{
    3, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    3, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1,
    3, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 2, 2,
    3, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    3, 0, 0, 0, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0,
    3, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    3, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    3, 2, 2, 2, 2, 0, 0, 0, 0, 0, 0, 0, 0, 0
};

void switch_to_scene(Scene* scene)
{
    curr_scene = scene;
    curr_scene->initialise();
}

void initialise()
{
    SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO);
    display_window = SDL_CreateWindow("Yuurei: The Platformer",
        SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
        WINDOW_WIDTH, WINDOW_HEIGHT,
        SDL_WINDOW_OPENGL);

    SDL_GLContext context = SDL_GL_CreateContext(display_window);
    SDL_GL_MakeCurrent(display_window, context);

#ifdef _WINDOWS
    glewInit();
#endif

    glViewport(VIEWPORT_X, VIEWPORT_Y, VIEWPORT_WIDTH, VIEWPORT_HEIGHT);

    program.Load(V_SHADER_PATH, F_SHADER_PATH);

    bg_model_matrix = glm::mat4(1.f);
    view_matrix = glm::mat4(1.0f);
    projection_matrix = glm::ortho(-5.0f, 5.0f, -3.75f, 3.75f, -1.0f, 1.0f);

    program.SetProjectionMatrix(projection_matrix);
    program.SetViewMatrix(view_matrix);

    glUseProgram(program.programID);

    glClearColor(BG_RED, BG_BLUE, BG_GREEN, BG_OPACITY);

    main_menu = new Menu();

    level_1 = new Level(LEVEL_1_DATA, 112,
                        OBJECT_POS[0], 5);
    level_1->set_id(1);

    level_2 = new Level(LEVEL_2_DATA, 112,
                        OBJECT_POS[1], 5);
    level_2->set_id(2);

    level_3 = new Level(LEVEL_3_DATA, 112,
                        OBJECT_POS[2], 5);
    level_3->set_id(3);

    levels[0] = level_1;
    levels[1] = level_2;
    levels[2] = level_3;

    switch_to_scene(main_menu);

    background_texture_id = Utility::load_texture("background_3.png");
    text_texture_id = Utility::load_texture("pixel_font.png");

    // enable blending
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
}

void process_input()
{
    // VERY IMPORTANT: If nothing is pressed, we don't want to go anywhere

    if(curr_scene->get_id() > 0)
        curr_scene->state.player->set_movement(glm::vec3(0.0f));

    SDL_Event event;

    while (SDL_PollEvent(&event))
    {
        switch (event.type) {
        // End game
        case SDL_QUIT:
        case SDL_WINDOWEVENT_CLOSE:
            game_is_running = false;
            break;

        case SDL_KEYDOWN:
            switch (event.key.keysym.sym) {
            case SDLK_q:
                // Quit the game with a keystroke
                game_is_running = false;
                break;
            case SDLK_z:
                if (curr_scene->get_id() > 0 && in_game && 
                    curr_scene->state.player->get_ammo_count() < curr_scene->AMMO)
                {
                    Mix_PlayChannel(-1, curr_scene->state.enemy_die_sfx, 0);
                    curr_scene->state.bullets[curr_scene->state.player->get_ammo_count()].set_position(glm::vec3(curr_scene->state.player->get_position().x, curr_scene->state.player->get_position().y, 0));
                    curr_scene->state.bullets[curr_scene->state.player->get_ammo_count()].set_activity(true);
                    curr_scene->state.bullets[curr_scene->state.player->get_ammo_count()].set_speed(5.f);
                    curr_scene->state.bullets[curr_scene->state.player->get_ammo_count()].set_movement(glm::vec3(curr_scene->state.player->get_dir() * 1, 0, 0));

                    curr_scene->state.player->set_ammo_count(curr_scene->state.player->get_ammo_count() + 1);
                }
                break;
            case SDLK_SPACE:
                // Jump
                if (curr_scene->get_id() > 0 && in_game &&
                    curr_scene->state.player->get_collision(2))
                {
                    curr_scene->state.player->set_jump(true);
                    Mix_PlayChannel(-1, curr_scene->state.jump_sfx, 0);
                }
                break;
            if (curr_scene->get_id() == 0) 
            {
                case SDLK_RETURN:
                    switch_to_scene(levels[level_num]);
            }
            default:
                break;
            }

        default:
            break;
        }
    }

    if(curr_scene->get_id() > 0){
        const Uint8* key_state = SDL_GetKeyboardState(NULL);
    
        glm::vec3 curr_movement = curr_scene->state.player->get_movement();

        if (key_state[SDL_SCANCODE_LEFT])
        {
            curr_movement = glm::vec3(-1.f, curr_movement.y, curr_movement.z);
            curr_scene->state.player->set_movement(curr_movement);
            curr_scene->state.player->set_dir(-1.f);
            curr_scene->state.player->set_indicies(curr_scene->state.player->animations[curr_scene->state.player->LEFT]);
        }
        else if (key_state[SDL_SCANCODE_RIGHT])
        {
            curr_movement = glm::vec3(1.f, curr_movement.y, curr_movement.z);
            curr_scene->state.player->set_movement(curr_movement);
            curr_scene->state.player->set_dir(1.f);
            curr_scene->state.player->set_indicies(curr_scene->state.player->animations[curr_scene->state.player->RIGHT]);
        }
        else 
        {
            curr_scene->state.player->set_indicies(curr_scene->state.player->animations[curr_scene->state.player->IDL]);
        }

        // This makes sure that the player can't move faster diagonally
        if (glm::length(curr_scene->state.player->get_movement()) > 1.0f)
        {
            curr_scene->state.player->set_movement(glm::normalize(curr_scene->state.player->get_movement()));
        }
    }
}

void update()
{
    float ticks = (float)SDL_GetTicks() / 1000.f;
    float delta_time = ticks - previous_ticks;
    previous_ticks = ticks;

    delta_time += accumulator;

    if (delta_time < FIXED_TIMESTEP)
    {
        accumulator = delta_time;
        return;
    }

    while (delta_time >= FIXED_TIMESTEP) {
        if (curr_scene->get_id() > 0)
        {
            if (!curr_scene->state.player->get_activity())
            {
                int curr_lives = curr_scene->state.player->get_lives();

                if (curr_lives - 1 > 0)
                {
                    curr_scene->state.player->set_lives(curr_lives - 1);
                    curr_scene->restart();
                    return;
                }
                else
                    curr_scene->state.player->set_lives(0);

                is_lose = true;
                in_game = false;
                return;
            }
            else
            {
                //we assume it's true until we're proven it isn't
                bool enemy_dead = true;
                for (int i = 0; i < curr_scene->ENEMY_COUNT; i++) enemy_dead = enemy_dead && !curr_scene->state.enemies[i].get_activity();
                if (enemy_dead)
                {
                    if (curr_scene->get_id() < 3)
                    {
                        switch_to_scene(levels[++level_num]);
                        return;
                    }
                    is_win = true;
                    in_game = false;
                    return;
                }
            }

            if (curr_scene->state.player->get_position().y < -9)
            {
                curr_scene->state.player->set_activity(false);
                return;
            }
        }
        curr_scene->update(FIXED_TIMESTEP);

        delta_time -= FIXED_TIMESTEP;
    }

    accumulator = delta_time;

    view_matrix = glm::mat4(1.0f);
    bg_model_matrix = glm::mat4(1.f);

    if (curr_scene->get_id() > 0) 
    {
        if (curr_scene->state.player->get_position().x > LEVEL1_LEFT_EDGE) {
            view_matrix = glm::translate(view_matrix, glm::vec3(-curr_scene->state.player->get_position().x, 3.75, 0));
            bg_model_matrix = glm::translate(bg_model_matrix, glm::vec3(curr_scene->state.player->get_position().x, -3.75f, 0.0f));
        }
        else {
            view_matrix = glm::translate(view_matrix, glm::vec3(-5, 3.75, 0));
            bg_model_matrix = glm::translate(bg_model_matrix, glm::vec3(5.f, -3.75f, 0));
        }
    }
}

void render()
{
    program.SetViewMatrix(view_matrix);
    glClear(GL_COLOR_BUFFER_BIT);

    if (curr_scene->get_id() > 0) 
    {
        //drawing background seperately because it's not like the objects
        program.SetModelMatrix(bg_model_matrix);

        float vertices[] = { -5.f, -3.75f, 5.f, -3.75f, 5.f, 3.75f, -5.f, -3.75f, 5.f, 3.75f, -5.f, 3.75f };
        float texCoords[] = { 0.0, 1.0, 1.0, 1.0, 1.0, 0.0, 0.0, 1.0, 1.0, 0.0, 0.0, 0.0 };

        glBindTexture(GL_TEXTURE_2D, background_texture_id);

        glVertexAttribPointer(program.positionAttribute, 2, GL_FLOAT, false, 0, vertices);
        glEnableVertexAttribArray(program.positionAttribute);

        glVertexAttribPointer(program.texCoordAttribute, 2, GL_FLOAT, false, 0, texCoords);
        glEnableVertexAttribArray(program.texCoordAttribute);

        glDrawArrays(GL_TRIANGLES, 0, 6);

        glDisableVertexAttribArray(program.positionAttribute);
        glDisableVertexAttribArray(program.texCoordAttribute);
    }
    
    curr_scene->render(&program);

    if (curr_scene->get_id() > 0) 
    {
        if (curr_scene->state.player->get_position().x > LEVEL1_LEFT_EDGE) {
            Utility::draw_text(&program, text_texture_id,
                "AMMO:" + std::to_string(curr_scene->AMMO - curr_scene->state.player->get_ammo_count()),
                .5f, .01f, glm::vec3(curr_scene->state.player->get_position().x - 4.5f, -1.f, 0.f));
            Utility::draw_text(&program, text_texture_id,
                "LIVES:" + std::to_string(curr_scene->state.player->get_lives()),
                .5f, .01f, glm::vec3(curr_scene->state.player->get_position().x - 4.5f, -6.75f, 0.f));
        }
        else
        {
            Utility::draw_text(&program, text_texture_id,
                "AMMO:" + std::to_string(curr_scene->AMMO - curr_scene->state.player->get_ammo_count()),
                .5f, .01f, glm::vec3(.5f, -1.f, 0.f));
            Utility::draw_text(&program, text_texture_id,
                "LIVES:" + std::to_string(curr_scene->state.player->get_lives()),
                .5f, .01f, glm::vec3(.5f, -6.75f, 0.f));
        }
        //lose or win, draw the text
        if (is_lose)
        {
            if (curr_scene->state.player->get_position().x > LEVEL1_LEFT_EDGE)
            {
                Utility::draw_text(&program, text_texture_id,
                    "YOU LOSE", .5f, .01f,
                    glm::vec3(curr_scene->state.player->get_position().x - 1.5f,
                        -3.25f, 0.f));
            }
            else
                Utility::draw_text(&program, text_texture_id,
                    "YOU LOSE", .5f, .01f,
                    glm::vec3(3.25f,
                        -3.25f, 0.f));
        }
        else if (is_win)
        {
            if (curr_scene->state.player->get_position().x > LEVEL1_LEFT_EDGE)
            {
                Utility::draw_text(&program, text_texture_id,
                    "YOU WIN", .5f, .01f,
                    glm::vec3(curr_scene->state.player->get_position().x - 1.5f,
                        -3.25f, 0.f));
            }
            else
                Utility::draw_text(&program, text_texture_id,
                    "YOU WIN", .5f, .01f,
                    glm::vec3(3.25,
                        -3.25f, 0.f));
        }
    }

    SDL_GL_SwapWindow(display_window);
}

void shutdown()
{
    SDL_Quit();

    delete level_1;
    delete level_2;
    delete level_3;

    delete main_menu;
}

int main(int argc, char* argv[]) {
    initialise();

    while (game_is_running)
    {
        process_input();
        update();

        if (curr_scene->state.next_scene_id >= 0) 
            switch_to_scene(levels[curr_scene->state.next_scene_id]);

        render();
    }

    shutdown();
    return 0;
}