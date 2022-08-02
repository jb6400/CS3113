#define GL_SILENCE_DEPRECATION
#define STB_IMAGE_IMPLEMENTATION

#define GL_GLEXT_PROTOTYPES 1
#define FIXED_TIMESTEP 0.0166666f
#define ENEMY_COUNT 3
#define LEVEL1_WIDTH 14
#define LEVEL1_HEIGHT 5
#define AMMO 20

#ifdef _WINDOWS
#include <GL/glew.h>
#endif

#include <SDL_mixer.h>
#include <SDL.h>
#include <SDL_opengl.h>
#include "glm/mat4x4.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "ShaderProgram.h"
#include "stb_image.h"
#include "cmath"
#include <ctime>
#include <vector>
#include "Object.h"

const int WINDOW_WIDTH = 640,
WINDOW_HEIGHT = 480;

const float BG_RED = 0.1922f,
BG_BLUE = 0.549f,
BG_GREEN = 0.9059f,
BG_OPACITY = 1.0f;

const int VIEWPORT_X = 0,
VIEWPORT_Y = 0,
VIEWPORT_WIDTH = WINDOW_WIDTH,
VIEWPORT_HEIGHT = WINDOW_HEIGHT;

const char V_SHADER_PATH[] = "shaders/vertex_textured.glsl",
F_SHADER_PATH[] = "shaders/fragment_textured.glsl";

const char SPRITESHEET_FILEPATH[] = "zoella_sprite_sheet_64.png";
const char PLATFORM_FILEPATH[] = "tileset_blue.png";
const char TEXT_FILEPATH[] = "pixel_font.png";
const char ENEMY_FILEPATH[] = "Ghostie_64.png";

const char BULLET_FILEPATH[] = "Zoella_Shuriken.png";
const char ENEMY_BULLET_FILEPATH[] = "Zoella_Spike_Proj.png";

const char BACKGROUND_FILEPATH[] = "background_3.png";

const int NUMBER_OF_TEXTURES = 1; // to be generated, that is
const GLint LEVEL_OF_DETAIL = 0;  // base image level; Level n is the nth mipmap reduction image
const GLint TEXTURE_BORDER = 0;   // this value MUST be zero

SDL_Window* display_window;
bool game_is_running = true;

ShaderProgram program;
glm::mat4 bg_model_matrix, view_matrix, projection_matrix;

float previous_ticks = 0.0f;
float accumulator = 0.0f;

bool is_lose = false;
bool is_win = false;
bool in_game = true;

int FONTBANK_SIZE = 16;

GLuint text_texture_id, background_texture_id;

unsigned int LEVEL_1_DATA[] =
{
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0,
    1, 1, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1,
    2, 2, 1, 1, 0, 0, 1, 1, 1, 2, 2, 2, 2, 2,
    2, 2, 2, 2, 0, 0, 2, 2, 2, 2, 2, 2, 2, 2
};

struct GameState
{
    Object* player;
    Object* enemies;

    Object* bullets;
    Object* enemy_bullets;

    Map* map;

    //Mix_Music* bgm;
    //Mix_Chunk* jump_sfx;
};

GameState state;

GLuint load_texture(const char* filepath);
void initialise();
void process_input();
void update();
void render();
void shutdown();

int main(int argc, char* argv[]) {
    initialise();

    while (game_is_running)
    {
        process_input();
        update();
        render();
    }

    shutdown();
    return 0;
}

GLuint load_texture(const char* filepath)
{
    // STEP 1: Loading the image file
    int width, height, number_of_components;
    unsigned char* image = stbi_load(filepath, &width, &height, &number_of_components, STBI_rgb_alpha);

    if (image == NULL)
    {
        //LOG("Unable to load image. Make sure the path is correct.");
        assert(false);
    }

    // STEP 2: Generating and binding a texture ID to our image
    GLuint texture_id;
    glGenTextures(NUMBER_OF_TEXTURES, &texture_id);
    glBindTexture(GL_TEXTURE_2D, texture_id);
    glTexImage2D(GL_TEXTURE_2D, LEVEL_OF_DETAIL, GL_RGBA, width, height, TEXTURE_BORDER, GL_RGBA, GL_UNSIGNED_BYTE, image);

    // STEP 3: Setting our texture filter modes
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    // STEP 4: Setting our texture wrapping modes
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT); // the last argument can change depending on what you are looking for
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

    // STEP 5: Releasing our file from memory and returning our texture id
    stbi_image_free(image);

    return texture_id;
}

void initialise() 
{
    SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO);
    display_window = SDL_CreateWindow("Rise of the AI",
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
    view_matrix = glm::mat4(1.0f);  // Defines the position (location and orientation) of the camera
    projection_matrix = glm::ortho(-5.0f, 5.0f, -3.75f, 3.75f, -1.0f, 1.0f);  // Defines the characteristics of your camera, such as clip planes, field of view, projection method etc.

    program.SetProjectionMatrix(projection_matrix);
    program.SetViewMatrix(view_matrix);

    glUseProgram(program.programID);

    glClearColor(BG_RED, BG_BLUE, BG_GREEN, BG_OPACITY);

    //TEXT AND BACKGROUND
    text_texture_id = load_texture(TEXT_FILEPATH);
    background_texture_id = load_texture(BACKGROUND_FILEPATH);

    //MAP
    GLuint map_texture_id = load_texture(PLATFORM_FILEPATH);
    state.map = new Map(LEVEL1_WIDTH, LEVEL1_HEIGHT, LEVEL_1_DATA, map_texture_id, 1.0f, 3, 1);

    //PLAYER
    //Existing
    state.player = new Object();

    state.player->set_type(PLAYER);

    state.player->set_position(glm::vec3(0.0f, 0.0f, 0.0f));
    state.player->set_movement(glm::vec3(0.0f));
    state.player->set_speed(1.5f);
    state.player->set_acceleration(glm::vec3(0.0f, -5.5f, 0.0f));

    state.player->set_texture(load_texture(SPRITESHEET_FILEPATH));

    // Animations
    state.player->animations[state.player->IDL] = new int[6]{ 0, 1, 2, 3, 4 };
    state.player->animations[state.player->LEFT] = new int[6]{ 6, 7, 8, 9, 10, 11 };
    state.player->animations[state.player->RIGHT] = new int[6]{ 6, 7, 8, 9, 10, 11 };

    state.player->set_indicies(state.player->animations[state.player->IDL]);  // start George looking left
    state.player->set_anim_frames(6);
    state.player->set_anim_cols(6);
    state.player->set_anim_rows(2);
    state.player->set_height(0.75f);
    state.player->set_width(0.75f);

    // Jumping
    state.player->set_power(5.0f);

    //ENEMIES
    //Enemies' stuff
    GLuint enemy_texture_id = load_texture(ENEMY_FILEPATH);

    state.enemies = new Object[ENEMY_COUNT];

    state.enemies[0].set_type(ENEMY);
    state.enemies[0].set_ai_type(SHOOTER);
    state.enemies[0].set_ai_state(IDLE);

    state.enemies[0].set_texture(enemy_texture_id);
    state.enemies[0].animations[state.player->IDL] = new int[7]{ 0, 1, 2, 3, 4, 5, 6 };

    state.enemies[0].set_indicies(state.player->animations[state.player->IDL]);  // start George looking left
    state.enemies[0].set_anim_frames(6);
    state.enemies[0].set_anim_cols(7);
    state.enemies[0].set_anim_rows(1);
    state.enemies[0].set_height(1.f);
    state.enemies[0].set_width(1.f);
    state.enemies[0].set_dir(-1);

    state.enemies[0].set_position(glm::vec3(4.5f, 0.0f, 0.0f));
    state.enemies[0].set_movement(glm::vec3(0.0f));
    state.enemies[0].set_speed(1.0f);
    state.enemies[0].set_acceleration(glm::vec3(0.0f, -5.5f, 0.0f));

    state.enemies[1].set_type(ENEMY);
    state.enemies[1].set_ai_type(WALKER);
    state.enemies[1].set_ai_state(IDLE);

    state.enemies[1].set_texture(enemy_texture_id);
    state.enemies[1].animations[state.player->IDL] = new int[7]{ 0, 1, 2, 3, 4, 5, 6 };

    state.enemies[1].set_indicies(state.player->animations[state.player->IDL]);  // start George looking left
    state.enemies[1].set_anim_frames(6);
    state.enemies[1].set_anim_cols(7);
    state.enemies[1].set_anim_rows(1);
    state.enemies[1].set_height(1.f);
    state.enemies[1].set_width(1.f);
    state.enemies[1].set_dir(-1);

    state.enemies[1].set_position(glm::vec3(6.5f, -2.0f, 0.0f));
    state.enemies[1].set_movement(glm::vec3(-1.0f, 0.0f, 0.0f));
    state.enemies[1].set_speed(1.0f);
    state.enemies[1].set_acceleration(glm::vec3(0.0f, -5.5f, 0.0f));

    state.enemies[2].set_type(ENEMY);
    state.enemies[2].set_ai_type(GUARD);
    state.enemies[2].set_ai_state(IDLE);

    state.enemies[2].set_texture(enemy_texture_id);
    state.enemies[2].animations[state.player->IDL] = new int[7]{ 0, 1, 2, 3, 4, 5, 6 };

    state.enemies[2].set_indicies(state.player->animations[state.player->IDL]);  // start George looking left
    state.enemies[2].set_anim_frames(6);
    state.enemies[2].set_anim_cols(7);
    state.enemies[2].set_anim_rows(1);
    state.enemies[2].set_height(1.f);
    state.enemies[2].set_width(1.f);
    state.enemies[2].set_dir(-1);

    state.enemies[2].set_position(glm::vec3(10.5f, -1.0f, 0.0f));
    state.enemies[2].set_movement(glm::vec3(0.0f));
    state.enemies[2].set_speed(1.0f);
    state.enemies[2].set_acceleration(glm::vec3(0.0f, -5.5f, 0.0f));

    state.bullets = new Object[AMMO];

    for (int i = 0; i < AMMO; i++)
    {
        state.bullets[i].set_texture(load_texture(BULLET_FILEPATH));
        state.bullets[i].set_type(BULLET);

        state.bullets[i].set_width(0.5f);
        state.bullets[i].set_height(0.5f);

        state.bullets[i].set_activity(false);
    }

    state.enemy_bullets = new Object[AMMO];

    for (int i = 0; i < AMMO; i++)
    {
        state.enemy_bullets[i].set_texture(load_texture(ENEMY_BULLET_FILEPATH));
        state.enemy_bullets[i].set_type(ENEMYBULLET);

        state.enemy_bullets[i].set_width(0.5f);
        state.enemy_bullets[i].set_height(0.5f);

        state.enemy_bullets[i].set_activity(false);
    }

    /*

     //BGM and SFX
    Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 4096);

    state.bgm = Mix_LoadMUS("assets/dooblydoo.mp3");
    Mix_PlayMusic(state.bgm, -1);
    Mix_VolumeMusic(MIX_MAX_VOLUME / 4.0f);

    state.jump_sfx = Mix_LoadWAV("assets/bounce.wav");
    */

    // enable blending
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
}

void restart() 
{
    state.player->set_position(glm::vec3(0.0f, 0.0f, 0.0f));
    state.player->set_activity(true);
    state.player->set_ammo_count(0);

    state.enemies[0].set_position(glm::vec3(4.5f, 0.0f, 0.0f));
    state.enemies[0].set_activity(true);
    state.enemies[0].set_ai_state(IDLE);
    state.enemies[0].set_ammo_count(0);

    state.enemies[1].set_position(glm::vec3(6.5f, -2.0f, 0.0f));
    state.enemies[1].set_ai_state(IDLE);
    state.enemies[1].set_activity(true);
    state.enemies[1].reset_delay();
    state.enemies[1].clear_turnaround();

    state.enemies[2].set_position(glm::vec3(10.5f, -1.0f, 0.0f));
    state.enemies[2].set_ai_state(IDLE);
    state.enemies[2].set_activity(true);
    state.enemies[2].clear_turnaround();

    for (int i = 0; i < AMMO; i++) state.bullets[i].set_activity(false);
}

void process_input()
{
    // VERY IMPORTANT: If nothing is pressed, we don't want to go anywhere
    state.player->set_movement(glm::vec3(0.0f));

    SDL_Event event;
    glm::vec3 curr_movement = state.player->get_movement();

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
                if (in_game && state.player->get_ammo_count() < AMMO)
                {
                    state.bullets[state.player->get_ammo_count()].set_position(glm::vec3(state.player->get_position().x, state.player->get_position().y, 0));
                    state.bullets[state.player->get_ammo_count()].set_activity(true);
                    state.bullets[state.player->get_ammo_count()].set_speed(5.f);
                    state.bullets[state.player->get_ammo_count()].set_movement(glm::vec3(state.player->get_dir() * 1, 0, 0));

                    state.player->set_ammo_count(state.player->get_ammo_count() + 1);
                }
                break;
            case SDLK_SPACE:
                // Jump
                if (in_game && state.player->get_collision(2))
                {
                    state.player->set_jump(true);
                    //Mix_PlayChannel(-1, state.jump_sfx, 0);
                }
                else if (!in_game) 
                {
                    in_game = true;
                    is_lose = false;
                    is_win = false;
                    restart();
                }
                break;
                
            default:
                break;
            }

        default:
            break;
        }
    }

    const Uint8* key_state = SDL_GetKeyboardState(NULL);
    
    if (key_state[SDL_SCANCODE_LEFT])
    {
        curr_movement = glm::vec3(-1.f, curr_movement.y, curr_movement.z);
        state.player->set_movement(curr_movement);
        state.player->set_dir(-1.f);
        state.player->set_indicies(state.player->animations[state.player->LEFT]);
    }
    else if (key_state[SDL_SCANCODE_RIGHT])
    {
        curr_movement = glm::vec3(1.f, curr_movement.y, curr_movement.z);
        state.player->set_movement(curr_movement);
        state.player->set_dir(1.f);
        state.player->set_indicies(state.player->animations[state.player->RIGHT]);
    }
    else 
    {
        state.player->set_indicies(state.player->animations[state.player->IDL]);
    }

    // This makes sure that the player can't move faster diagonally
    if (glm::length(state.player->get_movement()) > 1.0f)
    {
        state.player->set_movement(glm::normalize(state.player->get_movement()));
    }
}

void update()
{
    float ticks = (float)SDL_GetTicks() / 1000;
    float delta_time = ticks - previous_ticks;
    previous_ticks = ticks;

    delta_time += accumulator;

    if (delta_time < FIXED_TIMESTEP)
    {
        accumulator = delta_time;
        return;
    }

    while (delta_time >= FIXED_TIMESTEP) {
        // Update. Notice it's FIXED_TIMESTEP. Not deltaTime
        if (!state.player->get_activity())
        {
            is_lose = true;
            in_game = false;
            return;
        }
        else
        {
            //we assume it's true until we're proven it isn't
            bool enemy_dead = true;
            for (int i = 0; i < ENEMY_COUNT; i++) enemy_dead = enemy_dead && !state.enemies[i].get_activity();
            if (enemy_dead)
            {
                is_win = true;
                in_game = false;
                return;
            }
        }

        if (state.player->get_position().y < -6)
        {
            is_lose = true;
            in_game = false;
            return;
        }

        state.player->update(FIXED_TIMESTEP, state.player, state.enemies, ENEMY_COUNT, state.map, state.enemy_bullets);

        for (int i = 0; i < ENEMY_COUNT; i++) state.enemies[i].update(FIXED_TIMESTEP, state.player, state.enemies, 0, state.map, state.enemy_bullets);
        for (int i = 0; i < AMMO; i++) state.bullets[i].update(FIXED_TIMESTEP, state.player, state.enemies, ENEMY_COUNT, state.map, NULL);
        for (int i = 0; i < AMMO; i++) state.enemy_bullets[i].update(FIXED_TIMESTEP, state.player, state.player, 1, state.map, NULL);

        delta_time -= FIXED_TIMESTEP;
    }

    accumulator = delta_time;

    bg_model_matrix = glm::mat4(1.f);
    bg_model_matrix = glm::translate(bg_model_matrix, glm::vec3(state.player->get_position().x, 0.0f, 0.0f));

    view_matrix = glm::mat4(1.0f);
    view_matrix = glm::translate(view_matrix, glm::vec3(-state.player->get_position().x, 0.0f, 0.0f));
}

void DrawText(ShaderProgram* program, GLuint font_texture_id, std::string text, float screen_size, float spacing, glm::vec3 position)
{
    // Scale the size of the fontbank in the UV-plane
    // We will use this for spacing and positioning
    float width = 1.0f / FONTBANK_SIZE;
    float height = 1.0f / FONTBANK_SIZE;

    // Instead of having a single pair of arrays, we'll have a series of pairs—one for each character
    // Don't forget to include <vector>!
    std::vector<float> vertices;
    std::vector<float> texture_coordinates;

    // For every character...
    for (int i = 0; i < text.size(); i++) {
        // 1. Get their index in the spritesheet, as well as their offset (i.e. their position
        //    relative to the whole sentence)
        int spritesheet_index = (int)text[i];  // ascii value of character
        float offset = (screen_size + spacing) * i;

        // 2. Using the spritesheet index, we can calculate our U- and V-coordinates
        float u_coordinate = (float)(spritesheet_index % FONTBANK_SIZE) / FONTBANK_SIZE;
        float v_coordinate = (float)(spritesheet_index / FONTBANK_SIZE) / FONTBANK_SIZE;

        // 3. Inset the current pair in both vectors
        vertices.insert(vertices.end(), {
            offset + (-0.5f * screen_size), 0.5f * screen_size,
            offset + (-0.5f * screen_size), -0.5f * screen_size,
            offset + (0.5f * screen_size), 0.5f * screen_size,
            offset + (0.5f * screen_size), -0.5f * screen_size,
            offset + (0.5f * screen_size), 0.5f * screen_size,
            offset + (-0.5f * screen_size), -0.5f * screen_size,
            });

        texture_coordinates.insert(texture_coordinates.end(), {
            u_coordinate, v_coordinate,
            u_coordinate, v_coordinate + height,
            u_coordinate + width, v_coordinate,
            u_coordinate + width, v_coordinate + height,
            u_coordinate + width, v_coordinate,
            u_coordinate, v_coordinate + height,
            });
    }

    // 4. And render all of them using the pairs
    glm::mat4 model_matrix = glm::mat4(1.0f);
    model_matrix = glm::translate(model_matrix, position);

    program->SetModelMatrix(model_matrix);
    glUseProgram(program->programID);

    glVertexAttribPointer(program->positionAttribute, 2, GL_FLOAT, false, 0, vertices.data());
    glEnableVertexAttribArray(program->positionAttribute);
    glVertexAttribPointer(program->texCoordAttribute, 2, GL_FLOAT, false, 0, texture_coordinates.data());
    glEnableVertexAttribArray(program->texCoordAttribute);

    glBindTexture(GL_TEXTURE_2D, font_texture_id);
    glDrawArrays(GL_TRIANGLES, 0, (int)(text.size() * 6));

    glDisableVertexAttribArray(program->positionAttribute);
    glDisableVertexAttribArray(program->texCoordAttribute);
}


void render() 
{
    program.SetViewMatrix(view_matrix);
    glClear(GL_COLOR_BUFFER_BIT);

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

    DrawText(&program, text_texture_id, "AMMO:" + std::to_string(AMMO - state.player->get_ammo_count()), .5f, .01f, glm::vec3(state.player->get_position().x - 4.5f, 3.25f, 0.f));

    state.player->render(&program);

    state.map->render(&program);
    for (int i = 0; i < ENEMY_COUNT; i++) state.enemies[i].render(&program);
    for (int i = 0; i < AMMO; i++) state.bullets[i].render(&program);
    for (int i = 0; i < AMMO; i++) state.enemy_bullets[i].render(&program);

    //lose or win, draw the text
    if (is_lose)
    {
        DrawText(&program, text_texture_id, "YOU LOSE", .5f, .01f, glm::vec3(state.player->get_position().x - 1.5f, 0.f, 0.f));
    }
    else if (is_win) 
    {
        DrawText(&program, text_texture_id, "YOU WIN", .5f, .01f, glm::vec3(state.player->get_position().x - 1.5f, 0.f, 0.f));
    }

    SDL_GL_SwapWindow(display_window);
}

void shutdown() 
{ 
    SDL_Quit(); 
    delete state.player;
    delete [] state.enemies;
    delete[] state.bullets;
    delete[] state.enemy_bullets;
    delete state.map;
}
