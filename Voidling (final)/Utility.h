#pragma once
#define GL_SILENCE_DEPRECATION

#ifdef _WINDOWS
#include <GL/glew.h>
#endif

#define GL_GLEXT_PROTOTYPES 1
#include <vector>
#include <SDL.h>
#include <SDL_opengl.h>
#include "glm/mat4x4.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "ShaderProgram.h"

enum Coordinate { x_coordinate, y_coordinate };

class Utility {
public:
    static const int WINDOW_WIDTH = 640,
                     WINDOW_HEIGHT = 480;

    const Coordinate X_COORDINATE = x_coordinate;
    const Coordinate Y_COORDINATE = y_coordinate;

    static const float ORTHO_WIDTH, ORTHO_HEIGHT;
    
    static GLuint load_texture(const char* filepath);
    static void draw_text(ShaderProgram* program, GLuint font_texture_id, std::string text, float screen_size, float spacing, glm::vec3 position);
    static float get_screen_to_ortho(float coordinate, Coordinate axis);
};
