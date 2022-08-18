#define GL_SILENCE_DEPRECATION

#define GL_GLEXT_PROTOTYPES 1
#define FIXED_TIMESTEP 0.0166666f

#define LEVEL1_WIDTH 14
#define LEVEL1_HEIGHT 8
#define LEVEL1_LEFT_EDGE 5.0f
#define LEVEL1_RIGHT_EDGE 8.0f

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
#include <vector>

#include "Utility.h"
#include "Scene.h"
#include "Scene.h"
#include "Level.h"
#include "Menu.h"

Scene* curr_scene;

Menu* main_menu;
Level* room;

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

int prev_x = 0;
int prev_y = 0;

int x, y;
int press_x;
int press_y;

float time_left = 0.f;
int time_duration = 121;

//bool neg = false;
//bool diff_lvl = false;

//float lvl_diff = 0.f;
//int level_pos = 0;

GLuint text_texture_id, background_texture_id;

unsigned int LEVEL_1_DATA[] =
{
    4, 0, 0, 0, 0, 0, 0, 0, 5, 0, 0, 0, 0, 3,
    4, 0, 0, 0, 0, 0, 0, 0, 5, 0, 0, 0, 0, 3,
    4, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 3,
    2, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 2,
    4, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 3,
    4, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 3,
    2, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 2,
    2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
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

    room = new Level(LEVEL_1_DATA, 112);
    room->set_id(1);

    levels[0] = main_menu;
    levels[1] = room;

    switch_to_scene(levels[0]);

    background_texture_id = Utility::load_texture("background_3.png");
    text_texture_id = Utility::load_texture("pixel_font.png");

    // enable blending
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
}

void process_input()
{
    // VERY IMPORTANT: If nothing is pressed, we don't want to go anywhere

    if (curr_scene == levels[1]) 
    {
        SDL_GetMouseState(&x, &y);

        x = Utility::get_screen_to_ortho(x, x_coordinate);
        y = Utility::get_screen_to_ortho(y, y_coordinate);

        if (curr_scene->state.pet->get_position().x > LEVEL1_RIGHT_EDGE)
            curr_scene->state.player->set_movement(glm::vec3((x + 8.f) - curr_scene->state.player->get_position().x,
                (y - 3.75f) - curr_scene->state.player->get_position().y,
                0.f));
        else if (curr_scene->state.pet->get_position().x > LEVEL1_LEFT_EDGE)
            curr_scene->state.player->set_movement(glm::vec3((x + curr_scene->state.pet->get_position().x) - curr_scene->state.player->get_position().x,
                (y - 3.75f) - curr_scene->state.player->get_position().y,
                0.f));
        else
            curr_scene->state.player->set_movement(glm::vec3((x + 5.f) - curr_scene->state.player->get_position().x,
                (y - 3.75f) - curr_scene->state.player->get_position().y,
                0.f));
    }
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
            case SDLK_f:
                if (curr_scene->state.enemies[2].get_indicies() != *curr_scene->state.enemies[2].animations[curr_scene->state.enemies[2].IDL_H]
                    && curr_scene->state.enemies[2].get_lives() > 0 && curr_scene == levels[1])
                {
                    curr_scene->state.enemies[2].set_lives(curr_scene->state.enemies[2].get_lives() - 1);
                    curr_scene->state.enemies[2].set_indicies(curr_scene->state.enemies[2].animations[curr_scene->state.enemies[2].IDL_H]);
                    curr_scene->state.enemies[2].set_food(20.f);
                }
                break;
            case SDLK_l:
                if (curr_scene == levels[1]) 
                {
                    curr_scene->set_game(true);
                    curr_scene->state.pet->set_energy(0.f);
                }
            case SDLK_RETURN:
                if (curr_scene == levels[0]) 
                {
                    switch_to_scene(levels[1]);
                    press_x = curr_scene->state.pet->get_position().x;
                    press_x = curr_scene->state.pet->get_position().y;
                    time_left = (float)SDL_GetTicks() / 1000.f;
                }
            default:
                break;
            }
        case SDL_MOUSEBUTTONDOWN:
            if (curr_scene == levels[1]) 
            {
                press_x = x + 5.f;
                press_y = y - 3.75f;

                if (curr_scene->state.pet->get_position().x > LEVEL1_LEFT_EDGE)
                    press_x += fabs(x);

                switch (event.button.button)
                {
                case SDL_BUTTON_LEFT:
                    curr_scene->state.pet->set_button(true);
                    Mix_PlayChannel(-1, curr_scene->state.meow_sfx, 0);
                    if (curr_scene->state.pet->get_ai_state() != RELAXING && curr_scene->state.pet->get_ai_state() != RUNNING)
                    {
                        //if (press_y >= -3.f && curr_scene->state.pet->get_level() == 1)
                        //{
                        //    lvl_diff = 11;
                        //    level_pos = 2;
                        //    diff_lvl = true;
                        //}
                        //else if (press_y <= -4.f && curr_scene->state.pet->get_level() == 2)
                        //{
                        //    lvl_diff = -11;
                        //    level_pos = 1;
                        //    diff_lvl = true;

                        //}

                        //if (diff_lvl)
                        //{
                        //    press_x += lvl_diff;
                        //}

                        if (press_x != curr_scene->state.pet->get_position().x)
                        {
                            curr_scene->state.pet->set_ai_state(WALKING);

                            if (press_x - curr_scene->state.pet->get_position().x > 0)
                            {
                                //neg = false;
                                curr_scene->state.pet->set_dir(1);
                                curr_scene->state.pet->set_movement(glm::vec3(1.f, 0.f, 0.f));
                            }
                            else if (press_x - curr_scene->state.pet->get_position().x < 0)
                            {
                                //neg = true;
                                curr_scene->state.pet->set_dir(-1);
                                curr_scene->state.pet->set_movement(glm::vec3(-1.f, 0.f, 0.f));
                            }
                        }
                    }
                    break;
                case SDL_BUTTON_RIGHT:
                    break;
                default:
                    break;
                }
            }
        default:
            break;

        }
    }

   /* if (diff_lvl && curr_scene->state.pet->get_level() == level_pos)
    {
        press_x -= lvl_diff;
        diff_lvl = false;
    }

    if (neg) 
    {
        if (press_x >= curr_scene->state.pet->get_position().x)
        {
            curr_scene->state.pet->set_ai_state(RELAXING);
            curr_scene->state.pet->set_movement(glm::vec3(0.f, 0.f, 0.f));
        }
    }
    else {
        if (press_x <= curr_scene->state.pet->get_position().x)
        {
            curr_scene->state.pet->set_ai_state(RELAXING);
            curr_scene->state.pet->set_movement(glm::vec3(0.f, 0.f, 0.f));
        }
    }*/
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
        curr_scene->update(FIXED_TIMESTEP);
        delta_time -= FIXED_TIMESTEP;
    }

    accumulator = delta_time;

    view_matrix = glm::mat4(1.f);
    //bg_model_matrix = glm::mat4(1.f);

    if (curr_scene->get_id() > 0) 
    {
        if (curr_scene->state.pet->get_position().x > LEVEL1_RIGHT_EDGE)
        {
            view_matrix = glm::translate(view_matrix, glm::vec3(-3.f, 0.f, 0.f));
        }
        else if (curr_scene->state.pet->get_position().x > LEVEL1_LEFT_EDGE) {
            view_matrix = glm::translate(view_matrix, glm::vec3(-curr_scene->state.pet->get_position().x + 5.f, 0.f, 0.f));
            //bg_model_matrix = glm::translate(bg_model_matrix, glm::vec3(curr_scene->state.pet->get_position().x, -3.75f, 0.0f));
        }
        else {
            view_matrix = glm::translate(view_matrix, glm::vec3(0.f, 0.f, 0.f));
            //bg_model_matrix = glm::translate(bg_model_matrix, glm::vec3(5.f, -3.75f, 0));
        }
    
        view_matrix = glm::translate(view_matrix, glm::vec3(-5, 3.75, 0));
        bg_model_matrix = glm::translate(bg_model_matrix, glm::vec3(5.f, -3.75f, 0));
    }
}

void render()
{
    program.SetViewMatrix(view_matrix);
    glClear(GL_COLOR_BUFFER_BIT);

    //if (curr_scene->get_id() > 0) 
    //{
    //    //drawing background seperately because it's not like the objects
    //    program.SetModelMatrix(bg_model_matrix);

    //    float vertices[] = { -5.f, -3.75f, 5.f, -3.75f, 5.f, 3.75f, -5.f, -3.75f, 5.f, 3.75f, -5.f, 3.75f };
    //    float texCoords[] = { 0.0, 1.0, 1.0, 1.0, 1.0, 0.0, 0.0, 1.0, 1.0, 0.0, 0.0, 0.0 };

    //    glBindTexture(GL_TEXTURE_2D, background_texture_id);

    //    glVertexAttribPointer(program.positionAttribute, 2, GL_FLOAT, false, 0, vertices);
    //    glEnableVertexAttribArray(program.positionAttribute);

    //    glVertexAttribPointer(program.texCoordAttribute, 2, GL_FLOAT, false, 0, texCoords);
    //    glEnableVertexAttribArray(program.texCoordAttribute);

    //    glDrawArrays(GL_TRIANGLES, 0, 6);

    //    glDisableVertexAttribArray(program.positionAttribute);
    //    glDisableVertexAttribArray(program.texCoordAttribute);
    //}

    curr_scene->render(&program);

    if (curr_scene == levels[1])
    {
        if (curr_scene->get_timer() >= 0)
        {
            float curr_time = SDL_GetTicks() / 1000.f;
            int minutes = (time_duration - (curr_time - time_left)) / 60;
            int seconds = (time_duration - (curr_time - time_left)) - minutes * 60;

            curr_scene->set_timer(seconds + minutes * 60);

            if (curr_scene->state.pet->get_position().x > LEVEL1_RIGHT_EDGE)
            {
                Utility::draw_text(&program, text_texture_id, "E: " + std::to_string((int)curr_scene->state.pet->get_energy()),
                    .5f, .01f, glm::vec3(3.5f, -1.f, 0.f));
                Utility::draw_text(&program, text_texture_id, "C F: " + std::to_string((int)curr_scene->state.enemies[2].get_lives()),
                    .5f, .01f, glm::vec3(8.5f, -1.f, 0.f));
                Utility::draw_text(&program, text_texture_id, std::to_string(minutes) + " : " + std::to_string(seconds),
                    .5f, .01f, glm::vec3(8.5f, -7.f, 0.f));
            }
            else if (curr_scene->state.pet->get_position().x > LEVEL1_LEFT_EDGE)
            {
                Utility::draw_text(&program, text_texture_id, "E: " + std::to_string((int)curr_scene->state.pet->get_energy()),
                    .5f, .01f, glm::vec3(curr_scene->state.pet->get_position().x - 4.5f, -1.f, 0.f));
                Utility::draw_text(&program, text_texture_id, "C F: " + std::to_string((int)curr_scene->state.enemies[2].get_lives()),
                    .5f, .01f, glm::vec3(curr_scene->state.pet->get_position().x + .5f, -1.f, 0.f));
                Utility::draw_text(&program, text_texture_id, std::to_string(minutes) + " : " + std::to_string(seconds),
                    .5f, .01f, glm::vec3(curr_scene->state.pet->get_position().x + .5f, -7.f, 0.f));
            }
            else
            {
                Utility::draw_text(&program, text_texture_id, "E: " + std::to_string((int)curr_scene->state.pet->get_energy()),
                    .5f, .01f, glm::vec3(.5f, -1.f, 0.f));
                Utility::draw_text(&program, text_texture_id, "C F: " + std::to_string((int)curr_scene->state.enemies[2].get_lives()),
                    .5f, .01f, glm::vec3(5.5f, -1.f, 0.f));
                Utility::draw_text(&program, text_texture_id, std::to_string(minutes) + " : " + std::to_string(seconds),
                    .5f, .01f, glm::vec3(5.5f, -7.f, 0.f));
            }
        }
        if (curr_scene->get_game())
        {
            if (curr_scene->get_win())
            {
                if (curr_scene->state.pet->get_position().x > LEVEL1_RIGHT_EDGE)
                {
                    Utility::draw_text(&program, text_texture_id, "CONGRATS!",
                        .5f, .01f, glm::vec3(6.f, -3.5f, 0.f));
                }
                else if (curr_scene->state.pet->get_position().x > LEVEL1_LEFT_EDGE)
                {
                    Utility::draw_text(&program, text_texture_id, "CONGRATS!",
                        .5f, .01f, glm::vec3(curr_scene->state.pet->get_position().x - 2.f, -3.5f, 0.f));
                }
                else
                {
                    Utility::draw_text(&program, text_texture_id, "CONGRATS!",
                        .5f, .01f, glm::vec3(3.f, -3.5f, 0.f));
                }
            }
            else
            {
                if (curr_scene->state.pet->get_position().x > LEVEL1_RIGHT_EDGE)
                {
                    Utility::draw_text(&program, text_texture_id, "OH NO!",
                        .5f, .01f, glm::vec3(6.5f, -3.5f, 0.f));
                }
                else if (curr_scene->state.pet->get_position().x > LEVEL1_LEFT_EDGE)
                {
                    Utility::draw_text(&program, text_texture_id, "OH NO!",
                        .5f, .01f, glm::vec3(curr_scene->state.pet->get_position().x - 1.5f, -3.5f, 0.f));
                }
                else
                {
                    Utility::draw_text(&program, text_texture_id, "OH NO!",
                        .5f, .01f, glm::vec3(3.5f, -3.5f, 0.f));
                }
            }
        }
    }
    SDL_GL_SwapWindow(display_window);
}

void shutdown()
{
    SDL_Quit();

    delete room;

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