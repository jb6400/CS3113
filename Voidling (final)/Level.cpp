#include "Level.h"
#include "Utility.h"

#define LEVEL_WIDTH 14
#define LEVEL_HEIGHT 8

const char SPRITESHEET_FILEPATH[] = "zoella_sprite_sheet_64.png";
const char PLATFORM_FILEPATH[] = "tileset_blue.png";
const char ENEMY_FILEPATH[] = "Ghostie_64.png";

const char BULLET_FILEPATH[] = "Zoella_Shuriken.png";
const char ENEMY_BULLET_FILEPATH[] = "Zoella_Spike_Proj.png";

//Level::Level() 
//{
//    player_init_pos = glm::vec3(2.0f, 0.0f, 0.0f);
//    player_retry_pos = glm::vec3(2.f, -4.f, 0.f);
//
//    enem1_init_pos = glm::vec3(4.5f, -3.0f, 0.0f);
//    enem2_init_pos = glm::vec3(10.f, -4.0f, 0.0f);
//    enem3_init_pos = glm::vec3(10.5f, -1.0f, 0.0f);
//}

Level::Level(unsigned int* NEW_LEVEL_DATA, int size)
{
    for (int i = 0; i < size; i++) LEVEL_DATA[i] = NEW_LEVEL_DATA[i];
}


Level::~Level()
{
    delete state.map;

    delete state.player;
    delete state.pet;
    delete[] state.enemies;
    Mix_FreeChunk(this->state.click_sfx);
    Mix_FreeChunk(this->state.meow_sfx);
}

void Level::initialise()
{
    //what's next?
    state.next_scene_id = -1;

    //MAP
    GLuint map_texture_id = Utility::load_texture("home_tileset.png");
    state.map = new Map(LEVEL_WIDTH, LEVEL_HEIGHT, LEVEL_DATA, map_texture_id, 1.0f, 6, 1);
    
    //CURSOR (the player)
    state.player = new Object();
    state.player->set_speed(4.5f);
    state.player->set_acceleration(glm::vec3(0.f, 0.f, 0.f));
    state.player->set_type(PLAYER);
    state.player->set_texture(Utility::load_texture("cursor_sprite.png"));

    //PET (cat)
    state.pet = new Object();
    state.pet->set_position(glm::vec3(2.5f, -5.f, 0.f));

    state.pet->set_speed(2.f);
    state.pet->set_acceleration(glm::vec3(0.f, -10.f, 0.f));

    state.pet->set_texture(Utility::load_texture("cat_sprite_sheet.png"));

    // Animations
    state.pet->animations[state.pet->IDL]     = new int[5]{ 0, 1, 2, 3, 4 };
    state.pet->animations[state.pet->IDL_H]   = new int[5]{ 5, 6, 7, 8, 9 };
    state.pet->animations[state.pet->LEFT_W]  = new int[5]{ 10, 11, 12, 13, 14 };
    state.pet->animations[state.pet->RIGHT_W] = new int[5]{ 10, 11, 12, 13, 14 };
    state.pet->animations[state.pet->LEFT_R]  = new int[5]{ 15, 16, 17, 18, 19 };
    state.pet->animations[state.pet->RIGHT_R] = new int[5]{ 15, 16, 17, 18, 19 };
    state.pet->animations[state.pet->SLEEP]   = new int[5]{ 20, 21, 22, 23, 24 };

    state.pet->set_indicies(state.pet->animations[state.pet->IDL]);
    state.pet->set_anim_frames(5);
    state.pet->set_anim_cols(5);
    state.pet->set_anim_rows(5);
    state.pet->set_height(1.f);
    state.pet->set_width(1.f);

    state.pet->set_type(ANIMAL);
    state.pet->set_ai_type(CAT);

    //ANIMALS (mouse and dog)
    state.enemies = new Object[NUM_ANIM + 1];

    state.enemies[0].set_type(ANIMAL);
    state.enemies[0].set_ai_type(MOUSE);

    state.enemies[0].set_texture(Utility::load_texture("mouse_sprite_sheet.png"));

    // Animations
    state.enemies[0].animations[state.pet->RIGHT_W] = new int[5]{ 0, 1, 2, 3, 4 };

    state.enemies[0].set_indicies(state.pet->animations[state.enemies[0].RIGHT_W]);
    state.enemies[0].set_anim_frames(5);
    state.enemies[0].set_anim_cols(5);
    state.enemies[0].set_anim_rows(1);
    state.enemies[0].set_width(0.5f);
    state.enemies[0].set_height(0.5f);

    state.enemies[0].set_speed(3.f);
    state.enemies[0].set_acceleration(glm::vec3(0.f, -10.f, 0.f));
    state.enemies[0].set_position(glm::vec3(6.5f, -6.f, 0.f));
    state.enemies[0].set_movement(glm::vec3(1.f, 0.f, 0.f));


    state.enemies[1].set_type(ANIMAL);
    state.enemies[1].set_ai_type(DOG);
    state.enemies[1].set_ai_state(WALKING);
    state.enemies[1].set_texture(Utility::load_texture("dog_sprite_sheet.png"));

    // Animations
    state.enemies[1].animations[state.pet->RIGHT_W] = new int[5]{ 0, 1, 2, 3, 4 };
    state.enemies[1].animations[state.pet->IDL]     = new int[5]{ 5, 6, 7, 8, 9 };

    state.enemies[1].set_indicies(state.pet->animations[state.enemies[1].RIGHT_W]);
    state.enemies[1].set_anim_frames(5);
    state.enemies[1].set_anim_cols(5);
    state.enemies[1].set_anim_rows(2);

    state.enemies[1].set_speed(1.f);
    state.enemies[1].set_acceleration(glm::vec3(0.f, -10.f, 0.f));
    state.enemies[1].set_position(glm::vec3(1.5f, -5.f, 0.f));
    state.enemies[1].set_movement(glm::vec3(1.f, 0.f, 0.f));

    state.enemies[1].set_activity(false);

    //DISH
    state.enemies[2].set_type(DISH);
    state.enemies[2].set_position(glm::vec3(9.5f, -5.f, 0.f));

    state.enemies[2].set_texture(Utility::load_texture("miska_sprite_sheet.png"));

    // Sprite states (stored as animations)
    state.enemies[2].animations[state.pet->IDL]   = new int[1]{ 0 };
    state.enemies[2].animations[state.pet->IDL_H] = new int[1]{ 1 };

    state.enemies[2].set_indicies(state.enemies[2].animations[state.enemies[2].IDL]);
    state.enemies[2].set_anim_frames(1);
    state.enemies[2].set_anim_cols(2);
    state.enemies[2].set_anim_rows(1);

    //BGM and SFX
    Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 4096);

    state.click_sfx = Mix_LoadWAV("270304__littlerobotsoundfactory__collect-point-00.wav");
    state.meow_sfx = Mix_LoadWAV("262313__steffcaffrey__cat-meow2.wav");
 } 

void Level::update(float delta_time)
{
    if (timer >= 0 && !is_game_over) 
    {
        if (state.pet->get_energy() <= 0) 
        {
            state.pet->set_indicies(state.pet->animations[state.pet->SLEEP]);
            is_game_over = true;
            Mix_HaltMusic();
        }
        state.player->update(delta_time, state.player, NULL, 0, state.map);
        state.pet->update(delta_time, state.player, state.enemies, 3, state.map);
        for (int i = 0; i < NUM_ANIM + 1; i++) state.enemies[i].update(delta_time, state.player, &state.enemies[2], 1, state.map);
        /*state.enemies[0].update(delta_time, state.player, NULL, 0, state.map);*/
    }
    else if (!is_game_over)
    {
        is_win = true;
        is_game_over = true;
    }
}

void Level::render(ShaderProgram* program)
{
    state.map->render(program);

    state.pet->render(program);
    for(int i = 0; i < NUM_ANIM + 1; i++) state.enemies[i].render(program);
    state.player->render(program);
    /*state.enemies[0].render(program);*/
}

void Level::restart() 
{

}

int Level::get_id() { return id; }
void Level::set_id(int new_id) { id = new_id; }

bool Level::get_game() { return is_game_over; }
void Level::set_game(bool new_stat) { is_game_over = new_stat; }
//bool Level::get_lose() { return is_lose; }
bool Level::get_win()  { return is_win; }
float Level::get_timer() { return timer; }
void Level::set_timer(float curr_time) { timer = curr_time; }