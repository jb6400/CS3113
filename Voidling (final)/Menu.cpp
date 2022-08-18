#include "Menu.h"
#include "Utility.h"

GLuint text_id;

Menu::~Menu() 
{
    Mix_FreeMusic(this->state.bgm);
}

void Menu::initialise() 
{
    state.next_scene_id = -1;

    text_id = Utility::load_texture("pixel_font.png");

     //BGM and SFX
    Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 4096);

    state.bgm = Mix_LoadMUS("Starchaser (Chiptune original).wav");
    Mix_PlayMusic(state.bgm, -1);
    Mix_VolumeMusic(MIX_MAX_VOLUME / 4.0f);
}

void Menu::render(ShaderProgram* program)
{
    program->SetModelMatrix(model_matrix);

    float vertices[] = { -5.f, -3.75f, 5.f, -3.75f, 5.f, 3.75f, -5.f, -3.75f, 5.f, 3.75f, -5.f, 3.75f };
    float texCoords[] = { 0.0, 1.0, 1.0, 1.0, 1.0, 0.0, 0.0, 1.0, 1.0, 0.0, 0.0, 0.0 };

    glBindTexture(GL_TEXTURE_2D, Utility::load_texture("start_screen.png"));

    glVertexAttribPointer(program->positionAttribute, 2, GL_FLOAT, false, 0, vertices);
    glEnableVertexAttribArray(program->positionAttribute);

    glVertexAttribPointer(program->texCoordAttribute, 2, GL_FLOAT, false, 0, texCoords);
    glEnableVertexAttribArray(program->texCoordAttribute);

    glDrawArrays(GL_TRIANGLES, 0, 6);

    glDisableVertexAttribArray(program->positionAttribute);
    glDisableVertexAttribArray(program->texCoordAttribute);

    Utility::draw_text(program, text_id,
        "VOIDLING", .5f, .01f,
        glm::vec3(-4.f,
            -.5f, 0.f));

    Utility::draw_text(program, text_id,
        "Press Enter", .5f, .01f,
        glm::vec3(-.5f,
            -3.f, 0.f));
}
