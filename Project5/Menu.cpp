#include "Menu.h"
#include "Utility.h"

GLuint text_id;

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
    Utility::draw_text(program, text_id,
        "Yuurei: ", .5f, .01f,
        glm::vec3(-1.5f,
            1.5f, 0.f));

    Utility::draw_text(program, text_id,
        "The Platformer", .5f, .01f,
        glm::vec3(-3.f,
            0.f, 0.f));

    Utility::draw_text(program, text_id,
        "Press Enter to Play", .5f, .01f,
        glm::vec3(-4.5f,
            -2.f, 0.f));
}
