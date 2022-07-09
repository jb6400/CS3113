
#define GL_SILENCE_DEPRECATION
#define STB_IMAGE_IMPLEMENTATION
#define FIXED_TIMESTEP 0.0166666f
#define PLATFORM_COUNT 3

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
#include "Object.h"

const int WINDOW_WIDTH = 640;
const int WINDOW_HEIGHT = 480;
const float BG_RED = 0.1922f, BG_BLUE = 0.549f, BG_GREEN = 0.9059f;
const float BG_OPACITY = 1.0f;

const int VIEWPORT_X = 0,
          VIEWPORT_Y = 0,
          VIEWPORT_WIDTH = WINDOW_WIDTH,
          VIEWPORT_HEIGHT = WINDOW_HEIGHT;

const char V_SHADER_PATH[] = "shaders/vertex.glsl",
           F_SHADER_PATH[] = "shaders/fragment.glsl";


SDL_Window* display_window;

ShaderProgram program;

bool game_is_running = true;

glm::mat4 view_matrix, model_matrix, projection_matrix;

float previous_ticks= 0;

float accumulator = 0.0f;

bool slow_down = false;

struct GameState
{
    Object* player;
    Object* platform;
};

GameState state;

//GLuint load_texture(const char* filepath);
void initialise();
void process_input();
//bool check_collision(glm::vec3& position_1, glm::vec3& position_2, const float& collsion_dist);
void update();
//void draw_object(glm::mat4& object_model_matrix, GLuint& object_texture_id);
void render();
void shutdown();

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
    view_matrix = glm::mat4(1.0f);  // Defines the position (location and orientation) of the camera
    projection_matrix = glm::ortho(-5.0f, 5.0f, -3.75f, 3.75f, -1.0f, 1.0f);  // Defines the characteristics of your camera, such as clip planes, field of view, projection method etc.

    state.player = new Object();
    state.player->set_speed(1.0f);
    state.player->set_type(PLAYER);

    state.player->set_acceleration(glm::vec3(0.f, -1.f, 0.f));
    //state.player->set_texture(load_texture(SPRITESHEET_FILEPATH));

    state.platform = new Object[PLATFORM_COUNT];

    for (int i = 0; i < PLATFORM_COUNT; i++)
    {
        //state.platform[i].texture_id = platform_texture_id;
        state.platform[i].set_position(glm::vec3(i - 1.0f, -3.0f, 0.0f));
        state.platform[i].set_type(WIN); //when we have diff platforms, make lose and win
        state.platform[i].update(0.f, NULL, 0);
    }

    program.SetProjectionMatrix(projection_matrix);
    program.SetViewMatrix(view_matrix);

    glUseProgram(program.programID);

    glClearColor(BG_RED, BG_BLUE, BG_GREEN, BG_OPACITY);
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
/*
		case SDL_KEYDOWN:
			switch (event.key.keysym.sym) {
                /*
			case SDLK_d:
                curr_movement.x += 1.f;
                curr_acc.x = 10.f;
				state.player->set_movement(curr_movement);
                state.player->set_acceleration(curr_acc);
				break;

			case SDLK_a:
                curr_movement.x -= 1.f;
                curr_acc.x = -10.f;
                state.player->set_movement(curr_movement);
                state.player->set_acceleration(curr_acc);
				break;
                
			default:
				break;
			}
*/
		default:
			break;
		}
	}

	const Uint8* key_state = SDL_GetKeyboardState(NULL);

	if (key_state[SDL_SCANCODE_D]) 
    {
        curr_acc.x = 50.f;
        state.player->set_acceleration(curr_acc);
	}
	else if (key_state[SDL_SCANCODE_A]) 
    {
        curr_acc.x = -50.f;
        state.player->set_acceleration(curr_acc);
	}
    else 
    {
        slow_down = true;
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
        state.player->update(FIXED_TIMESTEP, state.platform, PLATFORM_COUNT);
        delta_time -= FIXED_TIMESTEP;
    }

    accumulator = delta_time;
}

void render() 
{
    glClear(GL_COLOR_BUFFER_BIT);

    state.player->render(&program);

    for (int i = 0; i < PLATFORM_COUNT; i++) { state.platform[i].render(&program); }

    SDL_GL_SwapWindow(display_window);
}

void shutdown() 
{ 
    //delete state.player;
    SDL_Quit(); 
}

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
