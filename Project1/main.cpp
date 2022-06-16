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

//vars for openGL
const int WINDOW_WIDTH = 640,
WINDOW_HEIGHT = 480;

const float BG_RED = 0.573f,
            BG_BLUE = 0.784f,
            BG_GREEN = 0.678f,
            BG_OPACITY = 1.0f;

const int VIEWPORT_X = 0,
          VIEWPORT_Y = 0,
          VIEWPORT_WIDTH = WINDOW_WIDTH,
          VIEWPORT_HEIGHT = WINDOW_HEIGHT;

const char V_SHADER_PATH[] = "shaders/vertex_textured.glsl",
           F_SHADER_PATH[] = "shaders/fragment_textured.glsl";

SDL_Window* display_window;

ShaderProgram program;

//constants and float values
const float SCALE_FACTOR = 1.f;
float g_scale = 1;
const float ROT_ANGLE = 90.f;
const float TRANS_VALUE = .75f;

const float MILLISECONDS_IN_SECOND = 1000.0;
const float DEGREES_PER_SECOND = 90.0f;

const int NUMBER_OF_TEXTURES = 1; // to be generated, that is
const GLint LEVEL_OF_DETAIL = 0;  // base image level; Level n is the nth mipmap reduction image
const GLint TEXTURE_BORDER = 0;   // this value MUST be zero

float previous_ticks = 0.0f;

//appx location of objs in scene
float vertices_sun[] = //sun
{
    -4.f, 2.f, -3.f, 2.f, -3.f, 3.f, //triangle 1
    -4.f, 2.f, -3.f, 3.f, -4.f, 3.f  //triangle 2
};

float vertices_fairy[] = //fairy
{
    -0.5f, 0.5f, 0.5f, 0.5f, 0.5f, 1.5f, //triangle 1
    -0.5f, 0.5f, 0.5f, 1.5f, -0.5f, 1.5f //triangle 2
};

float verticesbg[] = //background elements that take up the whole screen
{
    -5.f, -3.75f, 5.f, -3.75f, 5.f, 3.75f, //triangle 1
    -5.f, -3.75f, 5.f, 3.75f, -5.f, 3.75f //triangle 2
};

float vertices_mtree[] = //medium tree
{
    2.f, -3.75f, 2.5f, -3.75f, 2.5f, 3.75f, //triangle 1
    2.f, -3.75f, 2.5f, 3.75f, 2.f, 3.75f //triangle 2
};

float vertices_ltree[] = //large tree
{
    -3.f, -3.75f, -.25f, -3.75f, -.25f, 3.75f, //triangle 1
    -3.f, -3.75f, -.25f, 3.75f, -3.f, 3.75f //triangle 2
};

float vertices_grass[] = //foreground grass
{
    -5.f, -3.75f, 5.f, -3.75f, 5.f,-1.125f, //triangle 1
    -5.f, -3.75f, 5.f, -1.125f, -5.f, -1.125f //triangle 2
};

float texture_coordinates[] = //uv coordinates that remain consistent for each sprite
{
    0.f, 1.f, 1.f, 1.f, 1.f, 0.f, //triangle 1
    0.f, 1.f, 1.f, 0.f, 0.f, 0.f //triangle 2
};

//sprite images
const char FAIRY_SPRITE[] = "fairy-sprite.png";
const char SUN_SPRITE[] = "sun-sprite.png";
const char BACKGROUND_SPRITE[] = "background-sprite.png";
const char SUNLIGHT_SPRITE[] = "sunlight-sprite.png";
const char MED_TREE_SPRITE[] = "med-tree-sprite.png";
const char LARGE_TREE_SPRITE[] = "big-tree-sprite.png";
const char GRASS_SPRITE[] = "front-grass-sprite.png";

//texture ids
GLuint fairy_texture_id, sun_texture_id, 
       background_texture_id, sunlight_texture_id,
       med_tree_texture_id, large_tree_texture_id,
       grass_texture_id;

//bools
bool is_growing = true;
bool hit_boundary_x_max = false;
bool hit_boundary_y_max = false;
bool game_is_running = true;

//matrices
glm::mat4 view_matrix, model_matrix2, model_matrix1, model_matrix_bg, projection_matrix; //need for now bc we don't use the class yet
const glm::mat4 MIN_SIZE = glm::mat4(1.0f);
const glm::mat4 MAX_SIZE = glm::mat4(4.0f);
glm::mat4 GOAL_SIZE = MAX_SIZE;

//function prototypes
GLuint load_texture(const char* filepath);
void initialise();
void process_input();
void update();
void draw_object(glm::mat4& object_model_matrix, GLuint& object_texture_id);
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

GLuint load_texture(const char* filepath) {
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

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    stbi_image_free(image);

    return textureID;
}

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

    //enable blending
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    
    //load all the sprites
    fairy_texture_id = load_texture(FAIRY_SPRITE);
    sun_texture_id = load_texture(SUN_SPRITE);
    background_texture_id = load_texture(BACKGROUND_SPRITE);
    sunlight_texture_id = load_texture(SUNLIGHT_SPRITE);
    med_tree_texture_id = load_texture(MED_TREE_SPRITE);
    large_tree_texture_id = load_texture(LARGE_TREE_SPRITE);
    grass_texture_id = load_texture(GRASS_SPRITE);

    //matrix definition
    view_matrix = glm::mat4(1.0f);  // Defines the position (location and orientation) of the camera

    model_matrix1 = glm::mat4(1.0f);  // Defines every translation, rotations, or scaling applied to sun object
    model_matrix2= glm::mat4(1.0f);  // Defines every translation, rotations, or scaling applied to fairy object
    model_matrix_bg = glm::mat4(1.0f);  // Defines every translation, rotations, or scaling applied bg objects

    projection_matrix = glm::ortho(-5.0f, 5.0f, -3.75f, 3.75f, -1.0f, 1.0f);  // Defines the characteristics of your camera, such as clip planes, field of view, projection method etc.

    program.SetProjectionMatrix(projection_matrix);
    program.SetViewMatrix(view_matrix);

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
    //defining values based off of global vars
    float _TRANS_VAL_X = TRANS_VALUE, _TRANS_VAL_Y = TRANS_VALUE;

    //for delta time
    float ticks = (float)SDL_GetTicks() / MILLISECONDS_IN_SECOND; // get the current number of ticks
    float delta_time = ticks - previous_ticks; // the delta time is the difference from the last frame
    previous_ticks = ticks;

    //sun transformations
  
    //rotation on own axis
    glm::vec3 pivot = glm::vec3(-3.5f, 2.5f, 0);
    auto res_rotate = glm::translate(glm::mat4(1.f), pivot) * glm::rotate(glm::mat4(1.f), 
                      glm::radians(ROT_ANGLE*delta_time), 
                      glm::vec3(0, 0, 1.f)) * glm::translate(glm::mat4(1.f), -pivot);
    model_matrix1 = model_matrix1 * res_rotate;

    //scaling (beating exercise from class)
    auto scale_action = (is_growing ? 1 + SCALE_FACTOR*delta_time : 1 - SCALE_FACTOR*delta_time);
    g_scale *= scale_action;
     if (g_scale>2) {
         g_scale = 2;
         is_growing = !is_growing;
     }
     if (g_scale < 1) {
         g_scale = 1;
         is_growing = !is_growing;
     }
    glm::vec3 scale_vector = glm::vec3(scale_action, scale_action, 1.f);
    auto res_scale = glm::translate(glm::mat4(1.f), pivot) * glm::scale(glm::mat4(1.f), scale_vector) * glm::translate(glm::mat4(1.f), -pivot);
    model_matrix1 = model_matrix1 * res_scale;

    //fairy transformations
 
    //translation and bouncing around screen 
    //borders
    if (model_matrix2[3].x + vertices_fairy[2] >= 5.f) {
        hit_boundary_x_max = true;
    }
    if (model_matrix2[3].x + vertices_fairy[0] <= -5.f) {
        hit_boundary_x_max = false;
    }
    if (model_matrix2[3].y + vertices_fairy[5] >= 3.75f){
        hit_boundary_y_max = true;
    }
    if (model_matrix2[3].y + vertices_fairy[1] <= -3.75f){
        hit_boundary_y_max = false;
    }

    //modifying value
    if (hit_boundary_x_max == true)
        _TRANS_VAL_X = -TRANS_VALUE;
    if (hit_boundary_y_max == true)
        _TRANS_VAL_Y = -TRANS_VALUE;

    model_matrix2 = glm::translate(model_matrix2,  
                                   glm::vec3(_TRANS_VAL_X * delta_time, 
                                   _TRANS_VAL_Y * delta_time, 0.0f));
 
}

void draw_object(glm::mat4& object_model_matrix, GLuint& object_texture_id)
{
    program.SetModelMatrix(object_model_matrix);
    glBindTexture(GL_TEXTURE_2D, object_texture_id);
    glDrawArrays(GL_TRIANGLES, 0, 6); // we are now drawing 2 triangles, so we use 6 instead of 3
}

void render() {
    glClear(GL_COLOR_BUFFER_BIT);

    //drawing background elements behind sprites
    glVertexAttribPointer(program.positionAttribute, 2, GL_FLOAT, false, 0, verticesbg);
    glEnableVertexAttribArray(program.positionAttribute);

    glVertexAttribPointer(program.texCoordAttribute, 2, GL_FLOAT, false, 0, texture_coordinates);
    glEnableVertexAttribArray(program.texCoordAttribute);

    draw_object(model_matrix_bg, background_texture_id);

    glVertexAttribPointer(program.positionAttribute, 2, GL_FLOAT, false, 0, verticesbg);
    glEnableVertexAttribArray(program.positionAttribute);

    glVertexAttribPointer(program.texCoordAttribute, 2, GL_FLOAT, false, 0, texture_coordinates);
    glEnableVertexAttribArray(program.texCoordAttribute);

    draw_object(model_matrix_bg, sunlight_texture_id);

    //draw sun sprite
    glVertexAttribPointer(program.positionAttribute, 2, GL_FLOAT, false, 0, vertices_sun);
    glEnableVertexAttribArray(program.positionAttribute);

    glVertexAttribPointer(program.texCoordAttribute, 2, GL_FLOAT, false, 0, texture_coordinates);
    glEnableVertexAttribArray(program.texCoordAttribute);

    draw_object(model_matrix1, sun_texture_id);

    //trees behind fairy sprite when going right
    if (hit_boundary_x_max == false){
        glVertexAttribPointer(program.positionAttribute, 2, GL_FLOAT, false, 0, vertices_mtree);
        glEnableVertexAttribArray(program.positionAttribute);

        glVertexAttribPointer(program.texCoordAttribute, 2, GL_FLOAT, false, 0, texture_coordinates);
        glEnableVertexAttribArray(program.texCoordAttribute);

        draw_object(model_matrix_bg, med_tree_texture_id);

        glVertexAttribPointer(program.positionAttribute, 2, GL_FLOAT, false, 0, vertices_ltree);
        glEnableVertexAttribArray(program.positionAttribute);

        glVertexAttribPointer(program.texCoordAttribute, 2, GL_FLOAT, false, 0, texture_coordinates);
        glEnableVertexAttribArray(program.texCoordAttribute);

        draw_object(model_matrix_bg, large_tree_texture_id);
    }

    //draw fairy sprite
    glVertexAttribPointer(program.positionAttribute, 2, GL_FLOAT, false, 0, vertices_fairy);
    glEnableVertexAttribArray(program.positionAttribute);

    glVertexAttribPointer(program.texCoordAttribute, 2, GL_FLOAT, false, 0, texture_coordinates);
    glEnableVertexAttribArray(program.texCoordAttribute);
    
    draw_object(model_matrix2, fairy_texture_id);

    //foreground objects

    //trees in front of fairy sprite when going left
    if(hit_boundary_x_max == true){
        glVertexAttribPointer(program.positionAttribute, 2, GL_FLOAT, false, 0, vertices_mtree);
        glEnableVertexAttribArray(program.positionAttribute);

        glVertexAttribPointer(program.texCoordAttribute, 2, GL_FLOAT, false, 0, texture_coordinates);
        glEnableVertexAttribArray(program.texCoordAttribute);

        draw_object(model_matrix_bg, med_tree_texture_id);

        glVertexAttribPointer(program.positionAttribute, 2, GL_FLOAT, false, 0, vertices_ltree);
        glEnableVertexAttribArray(program.positionAttribute);

        glVertexAttribPointer(program.texCoordAttribute, 2, GL_FLOAT, false, 0, texture_coordinates);
        glEnableVertexAttribArray(program.texCoordAttribute);

        draw_object(model_matrix_bg, large_tree_texture_id);
    }
    //grass
    glVertexAttribPointer(program.positionAttribute, 2, GL_FLOAT, false, 0, vertices_grass);
    glEnableVertexAttribArray(program.positionAttribute);

    glVertexAttribPointer(program.texCoordAttribute, 2, GL_FLOAT, false, 0, texture_coordinates);
    glEnableVertexAttribArray(program.texCoordAttribute);

    draw_object(model_matrix_bg, grass_texture_id);

    glDisableVertexAttribArray(program.positionAttribute);
    glDisableVertexAttribArray(program.texCoordAttribute);

    SDL_GL_SwapWindow(display_window);
}

void shutdown() { SDL_Quit(); }