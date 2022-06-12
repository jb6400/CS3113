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
#include "stb_image.h"
#include <thread>
#include <chrono>

//variables
const int WINDOW_WIDTH = 640,
WINDOW_HEIGHT = 480;

//can change bg color OR add image
const float BG_RED = 0.1922f,
BG_BLUE = 0.549f,
BG_GREEN = 0.9059f,
BG_OPACITY = 1.0f;

const int VIEWPORT_X = 0,
VIEWPORT_Y = 0,
VIEWPORT_WIDTH = WINDOW_WIDTH,
VIEWPORT_HEIGHT = WINDOW_HEIGHT;

//when we add sprites, add "_textured" 
const char V_SHADER_PATH[] = "shaders/vertex.glsl",
F_SHADER_PATH[] = "shaders/fragment.glsl";

//delete once we move to sprites
const float TRIANGLE_RED = 1.0f,
TRIANGLE_BLUE = 0.4f,
TRIANGLE_GREEN = 0.4f,
TRIANGLE_OPACITY = 1.0f;

//prof vars
const float GROWTH_FACTOR = 1.01f;
const float SHRINK_FACTOR = 0.99f;
const int MAX_FRAME = 40;
const float ROT_ANGLE = glm::radians(1.5f);
const float TRANS_VALUE = 0.025f;
const float TRANS_RANGE = 10.f;

const float MILLISECONDS_IN_SECOND = 1000.0;
const float DEGREES_PER_SECOND = 90.0f;

const int NUMBER_OF_TEXTURES = 1; // to be generated, that is
const GLint LEVEL_OF_DETAIL = 0;  // base image level; Level n is the nth mipmap reduction image
const GLint TEXTURE_BORDER = 0;   // this value MUST be zero

float triangle_x = 0.0f;
float triangle_rotate = 0.0f;
float previous_ticks = 0.0f;

//object 1
float vertices[] = //appx location of first obj
{
    -3.0f, 2.0f,
    -3.5f, 3.0f,
    -4.0f, 2.0f
};

float vertices2[] =
{
    0.5f, 0.5f,
    0.0f, 1.5f,
    -0.5f, 0.5f
};

GLuint player_texture_id;

//cont. of mine
//int frame_counter = 0;
//bool is_growing = true;

SDL_Window* display_window;
bool game_is_running = true;

bool hit_boundary_x_max = false;
bool hit_boundary_x_min = false;
bool hit_boundary_y_max = false;
bool hit_boundary_y_min = false;

ShaderProgram program;
glm::mat4 view_matrix, model_matrix2, model_matrix1, projection_matrix; //need for now bc we don't use the class yet

//functions
void initialise()
{
    SDL_Init(SDL_INIT_VIDEO);
    display_window = SDL_CreateWindow("Project 1!",
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

    view_matrix = glm::mat4(1.0f);  // Defines the position (location and orientation) of the camera
    model_matrix1 = glm::mat4(1.0f);  // Defines every translation, rotations, or scaling applied to an object
    model_matrix2= glm::mat4(1.0f);  // Defines every translation, rotations, or scaling applied to an object

    projection_matrix = glm::ortho(-5.0f, 5.0f, -3.75f, 3.75f, -1.0f, 1.0f);  // Defines the characteristics of your camera, such as clip planes, field of view, projection method etc.

    program.SetProjectionMatrix(projection_matrix);
    program.SetViewMatrix(view_matrix);
    // Notice we haven't set our model matrix yet!

    program.SetColor(TRIANGLE_RED, TRIANGLE_BLUE, TRIANGLE_GREEN, TRIANGLE_OPACITY);

    glUseProgram(program.programID);

    glClearColor(BG_RED, BG_BLUE, BG_GREEN, BG_OPACITY);
}

void process_input()
{
    SDL_Event event;
    while (SDL_PollEvent(&event))
    {
        if (event.type == SDL_QUIT || event.type == SDL_WINDOWEVENT_CLOSE)
        {
            game_is_running = false;
        }
    }
}

void update()
{
    //make it so one rotates and one translates SEPERATELY!
    float _TRANS_VAL_X = TRANS_VALUE, _TRANS_VAL_Y = TRANS_VALUE;
    //for delta time
    float ticks = (float)SDL_GetTicks() / MILLISECONDS_IN_SECOND; // get the current number of ticks
    float delta_time = ticks - previous_ticks; // the delta time is the difference from the last frame
    previous_ticks = ticks;

    //functions that do all that we can (based on lessons) 
    ///glm::vec3 vertex_coordinate;
    if (model_matrix2[3].x + vertices2[2] - 1.5f >= 3.f) {
        hit_boundary_x_max = true;
        //hit_boundary_x_min = false;
    }
    if (model_matrix2[3].x + vertices2[4] + 2.f <= -3.f) {
        hit_boundary_x_max = false;
        //hit_boundary_x_min = true;
    }
    if (model_matrix2[3].y + vertices2[1] >= 3.f) {
        hit_boundary_y_max = true;
        //hit_boundary_y_min = false;
    }
    if (model_matrix2[3].y + vertices2[1] + 1 <= -3.f) {
        hit_boundary_y_max = false;
        //hit_boundary_y_min = true;
    }

    if (hit_boundary_x_max == true)
        _TRANS_VAL_X = -TRANS_VALUE;
    if (hit_boundary_y_max == true)
        _TRANS_VAL_Y = -TRANS_VALUE;

    model_matrix2 = glm::translate(model_matrix2, glm::vec3(_TRANS_VAL_X, _TRANS_VAL_Y, 0.0f));
    //glm::vec3 world_coordinate = glm::vec3(model_matrix2 * glm::vec4(vertex_coordinate, 1.f));
    _RPTF2(_CRT_WARN,"coord x: %f, y: %f z: %f\n", model_matrix2[3].x, model_matrix2[3].y, model_matrix2[3].z);
    //model_matrix = glm::scale(model_matrix, scale_vecor);
    //model_matrix1 = glm::rotate(model_matrix1, ROT_ANGLE, glm::vec3(0, 0, 1));
    glm::vec3 pivot = glm::vec3(-3.5f, 2.5f, 0);
    auto res = glm::translate(glm::mat4(1.f), pivot)*glm::rotate(glm::mat4(1.f), ROT_ANGLE, glm::vec3(0, 0, 0.1f))* glm::translate(glm::mat4(1.f), -pivot);
    model_matrix1 = model_matrix1 * res;
    /*-3.0f, 2.0f,
    -3.5f, 3.0f,
    -4.0f, 2.0f*/

}

void render() {
    glClear(GL_COLOR_BUFFER_BIT);

    program.SetModelMatrix(model_matrix1);

    glVertexAttribPointer(program.positionAttribute, 2, GL_FLOAT, false, 0, vertices);
    glEnableVertexAttribArray(program.positionAttribute);
    
    glDrawArrays(GL_TRIANGLES, 0, 3);

    //object 2
    program.SetModelMatrix(model_matrix2);

    glVertexAttribPointer(program.positionAttribute, 2, GL_FLOAT, false, 0, vertices2);
    glEnableVertexAttribArray(program.positionAttribute);
    glDrawArrays(GL_TRIANGLES, 0, 3);

    glDisableVertexAttribArray(program.positionAttribute);

    SDL_GL_SwapWindow(display_window);
}

void shutdown() { SDL_Quit(); }

/**
 Start here—we can see the general structure of a game loop without worrying too much about the details yet.
 */
int main(int argc, char* argv[])
{
    initialise();

    while (game_is_running)
    {
        process_input();
        update();
        render();
        std::this_thread::sleep_for(std::chrono::milliseconds(30));
    }

    shutdown();
    return 0;
}