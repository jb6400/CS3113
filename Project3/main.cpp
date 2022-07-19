
#define GL_SILENCE_DEPRECATION
#define STB_IMAGE_IMPLEMENTATION
#define FIXED_TIMESTEP 0.0166666f
#define PLATFORM_COUNT 11
#define L_PLATFORM_COUNT 3
#define W_PLATFORM_COUNT 3

#ifdef _WINDOWS
#include <GL/glew.h>
#endif

#define GL_GLEXT_PROTOTYPES 1
#include <SDL.h>
#include <SDL_opengl.h>
#include "glm/mat4x4.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "ShaderProgram.h"
#include "stb_image.h"
#include "SDL_mixer.h"
#include "Object.h"

#include <vector>

const int WINDOW_WIDTH = 640;
const int WINDOW_HEIGHT = 480;

const int VIEWPORT_X = 0,
          VIEWPORT_Y = 0,
          VIEWPORT_WIDTH = WINDOW_WIDTH,
          VIEWPORT_HEIGHT = WINDOW_HEIGHT;

const char V_SHADER_PATH[] = "shaders/vertex_textured.glsl",
           F_SHADER_PATH[] = "shaders/fragment_textured.glsl";


SDL_Window* display_window;

ShaderProgram program;

bool game_is_running = true;

glm::mat4 view_matrix, bg_model_matrix, projection_matrix;

float previous_ticks= 0;

float accumulator = 0.0f;

bool slow_down = false;

struct GameState
{
    Object* player;
    Object* platform;
};

GameState state;

const int NUMBER_OF_TEXTURES = 1; // to be generated, that is
const GLint LEVEL_OF_DETAIL = 0;  // base image level; Level n is the nth mipmap reduction image
const GLint TEXTURE_BORDER = 0;   // this value MUST be zero

const char PLAYER_SPRITE[] = "player_spritesheet.png";
const char DIRT_SPRITE[] = "dirt_block.png";
const char LOSE_SPRITE[] = "grass_lose_block.png";
const char WIN_SPRITE[] = "grass_win_block.png";
const char FONT_SPRITE[] = "pixel_font.png";
const char BG_SPRITE[] = "background_ll_big.png";

GLuint player_texture_id, dirt_texture_id, lose_texture_id, win_texture_id,
       font_texture_id, bg_texture_id;

const int FONTBANK_SIZE = 16;

int timer = 0;

//prototypes
GLuint load_texture(const char* filepath);
void initialise();
void process_input();
void update();
void render();
void shutdown();

//functions
int main(int argc, char* argv[])
{
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
    int width, height, number_of_components;
    unsigned char* image = stbi_load(filepath, &width, &height, &number_of_components, STBI_rgb_alpha);

    if (image == NULL) {
        //as of right now, can't print
        assert(false);
    }

    GLuint textureID;
    glGenTextures(NUMBER_OF_TEXTURES, &textureID);
    glBindTexture(GL_TEXTURE_2D, textureID);
    glTexImage2D(GL_TEXTURE_2D, LEVEL_OF_DETAIL, GL_RGBA, width, height, TEXTURE_BORDER, GL_RGBA, GL_UNSIGNED_BYTE, image);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    stbi_image_free(image);

    return textureID;
}

void initialise() 
{
    SDL_Init(SDL_INIT_VIDEO);  // Initialising

    display_window = SDL_CreateWindow("Lunar Lander", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, WINDOW_WIDTH, WINDOW_HEIGHT, SDL_WINDOW_OPENGL);

    SDL_GLContext context = SDL_GL_CreateContext(display_window);

    SDL_GL_MakeCurrent(display_window, context);

#ifdef _WINDOWS
    glewInit();
#endif

    glViewport(VIEWPORT_X, VIEWPORT_Y, VIEWPORT_WIDTH, VIEWPORT_HEIGHT); // Init camera
    program.Load(V_SHADER_PATH, F_SHADER_PATH); // Loads up shaders

    //matrix definition
    bg_model_matrix = glm::mat4(1.f);
    view_matrix = glm::mat4(1.0f);  // Defines the position (location and orientation) of the camera
    projection_matrix = glm::ortho(-5.0f, 5.0f, -3.75f, 3.75f, -1.0f, 1.0f);  // Defines the characteristics of your camera, such as clip planes, field of view, projection method etc.

    //enable blending
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    player_texture_id = load_texture(PLAYER_SPRITE);
    dirt_texture_id = load_texture(DIRT_SPRITE);
    lose_texture_id = load_texture(LOSE_SPRITE);
    win_texture_id = load_texture(WIN_SPRITE);
    font_texture_id = load_texture(FONT_SPRITE);
    bg_texture_id = load_texture(BG_SPRITE);

    state.player = new Object();
    state.player->set_speed(1.0f);
    state.player->set_position(glm::vec3(-4.5f, 3.0f, 0.0f));
    state.player->set_type(PLAYER);

    state.player->set_acceleration(glm::vec3(0.f, -.25f, 0.f));
    state.player->set_texture(player_texture_id);
    state.player->set_anim_rows(2);
    state.player->set_anim_cols(4);

    state.player->animations[state.player->LEFT]  = new int[4] { 4, 5, 6, 7 };
    state.player->animations[state.player->RIGHT] = new int[4] { 4, 5, 6, 7 };
    state.player->animations[state.player->IDLE] = new int[4]{ 0, 1, 2, 3 };

    state.player->set_indicies(state.player->animations[state.player->IDLE]);
    state.player->set_anim_frames(4);

    state.platform = new Object[PLATFORM_COUNT];

    for (int i = 0; i < PLATFORM_COUNT; i++)
    {
        if (i <= 5) 
        {
            state.platform[i].set_texture(lose_texture_id);
            state.platform[i].set_position(glm::vec3(i - 5.0f, -3.0f, 0.0f));
            state.platform[i].set_type(LOSE);
            state.platform[i].update(0.f, NULL, 0);
        }
        if (i > 5)
        {
            state.platform[i].set_texture(win_texture_id);
            state.platform[i].set_position(glm::vec3(i - 5.0f, -3.0f, 0.0f));
            state.platform[i].set_type(WIN);
            state.platform[i].update(0.f, NULL, 0);
        }
    }

    program.SetProjectionMatrix(projection_matrix);
    program.SetViewMatrix(view_matrix);

    glUseProgram(program.programID);
}

void process_input() 
{
	SDL_Event event;
    glm::vec3 curr_movement = state.player->get_movement();
    glm::vec3 curr_acc = state.player->get_acceleration();

	while (SDL_PollEvent(&event)) {
		switch (event.type) {
		case SDL_QUIT:
		case SDL_WINDOWEVENT_CLOSE:
			game_is_running = false;
			break;
        case SDL_KEYDOWN:
            switch (event.key.keysym.sym) {
                if (!(state.player->get_status())) {
                    case SDLK_SPACE:
                        state.player->set_state(false);
                        state.player->set_status(true);
                        state.player->set_fuel(5.f);
                        state.player->set_position(glm::vec3(-4.5f, 3.0f, 0.0f));
                        break;
                }

            default:
                break;
            }
        default:
			break;
		}
	}

	const Uint8* key_state = SDL_GetKeyboardState(NULL);

    if (state.player->get_status() && state.player->get_fuel() >= 0.f) 
    {
        if (key_state[SDL_SCANCODE_D])
        {
            curr_acc.x = 70.f;

            if(timer == 0)
            {
                state.player->sub_fuel();
                timer = 5;
            }

            state.player->set_acceleration(curr_acc);
            state.player->set_dir(1);
            state.player->set_indicies(state.player->animations[state.player->RIGHT]);
        }
        else if (key_state[SDL_SCANCODE_A])
        {
            curr_acc.x = -70.f;

            if (timer == 0)
            {
                state.player->sub_fuel();
                timer = 5;
            }

            state.player->set_acceleration(curr_acc);
            state.player->set_dir(-1);
            state.player->set_indicies(state.player->animations[state.player->LEFT]);
        }
        else
        {
            state.player->set_indicies(state.player->animations[state.player->IDLE]);
            slow_down = true;
        }
    }
}

void update() 
{
    float ticks = (float)SDL_GetTicks() / 1000;
    float delta_time = ticks - previous_ticks;
    previous_ticks = ticks;

    // STEP 1: Keep track of how much time has passed since the last step    
    delta_time += accumulator;

    // STEP 2: Accumulate the amount of time passed while we're under our fixed timestep
    if (delta_time < FIXED_TIMESTEP)
    {
        accumulator = delta_time;
        return;
    }

    // STEP 3: Once we exceed that time apply that elapsed time into the objects' update
    while (delta_time >= FIXED_TIMESTEP) {
        // Update. Notice it's FIXED_TIMESTEP. Not delta_time
        if (slow_down) 
        {
            glm::vec3 curr_acc = state.player->get_acceleration();
            if (curr_acc.x == 0) 
            {
                slow_down = false;
            }
            else if (curr_acc.x > 0) 
            {
                curr_acc.x -= .5f;
                state.player->set_acceleration(curr_acc);
            }
            else if (curr_acc.x < 0)
            {
                curr_acc.x += .5f;
                state.player->set_acceleration(curr_acc);
            }
        }

        if (timer > 0) 
        {
            timer -= 1;
        }

        state.player->update(FIXED_TIMESTEP, state.platform, PLATFORM_COUNT);
        delta_time -= FIXED_TIMESTEP;
    }

    accumulator = delta_time;
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
    glClear(GL_COLOR_BUFFER_BIT);

    //drawing background seperately because it's not like the objects
    program.SetModelMatrix(bg_model_matrix);

    float vertices[] = { -5.f, -3.75f, 5.f, -3.75f, 5.f, 3.75f, -5.f, -3.75f, 5.f, 3.75f, -5.f, 3.75f };
    float texCoords[] = { 0.0, 1.0, 1.0, 1.0, 1.0, 0.0, 0.0, 1.0, 1.0, 0.0, 0.0, 0.0 };

    glBindTexture(GL_TEXTURE_2D, bg_texture_id);

    glVertexAttribPointer(program.positionAttribute, 2, GL_FLOAT, false, 0, vertices);
    glEnableVertexAttribArray(program.positionAttribute);

    glVertexAttribPointer(program.texCoordAttribute, 2, GL_FLOAT, false, 0, texCoords);
    glEnableVertexAttribArray(program.texCoordAttribute);

    glDrawArrays(GL_TRIANGLES, 0, 6);

    glDisableVertexAttribArray(program.positionAttribute);
    glDisableVertexAttribArray(program.texCoordAttribute);


    //rendering our objects
    state.player->render(&program);

    for (int i = 0; i < PLATFORM_COUNT; i++) { state.platform[i].render(&program); }

    DrawText(&program, font_texture_id, "Fuel:" + std::to_string(state.player->get_fuel()), .5f, 0.01f, glm::vec3(-4.5f, 3.25f, 0.f));

    if (!state.player->get_status()) 
    {
        if (state.player->get_state())
        {
            DrawText(&program, font_texture_id, "Mission Successful!", .5f, 0.01f, glm::vec3(-4.5f, 1.5f, 0.f));
        }
        else
        {
            DrawText(&program, font_texture_id, "Mission Failed", .5f, 0.01f, glm::vec3(-3.25f, 1.5f, 0.f));
        }

        DrawText(&program, font_texture_id, "Press Space to play", .5f, 0.01f, glm::vec3(-4.5f, 0.f, 0.f));
        DrawText(&program, font_texture_id, "again", .5f, 0.01f, glm::vec3(-1.f, -1.f, 0.f));
    }

    SDL_GL_SwapWindow(display_window);
}

void shutdown() 
{ 
    delete state.player;
    delete [] state.platform;

    SDL_Quit(); 
}
