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

//Level::Level(unsigned int* NEW_LEVEL_DATA, int size)
//{
//    for (int i = 0; i < size; i++) LEVEL_DATA[i] = NEW_LEVEL_DATA[i];
//    player_init_pos = glm::vec3(2.0f, 0.0f, 0.0f);
//    player_retry_pos = glm::vec3(2.f, -4.f, 0.f);
//
//    enem1_init_pos = glm::vec3(4.5f, -3.0f, 0.0f);
//    enem2_init_pos = glm::vec3(10.f, -4.0f, 0.0f);
//    enem3_init_pos = glm::vec3(10.5f, -1.0f, 0.0f);
//}

Level::Level(unsigned int* NEW_LEVEL_DATA, int size_l,
             glm::vec3* NEW_OBJ_POS, int size_o)
{
    for (int i = 0; i < size_l; i++) LEVEL_DATA[i] = NEW_LEVEL_DATA[i];
    player_init_pos = NEW_OBJ_POS[0];
    player_retry_pos = NEW_OBJ_POS[1];

    enem1_init_pos = NEW_OBJ_POS[2];
    enem2_init_pos = NEW_OBJ_POS[3];
    enem3_init_pos = NEW_OBJ_POS[4];
}

Level::~Level()
{
    if(!(state.enemies == NULL))  delete[]  this->state.enemies;
    if (!(state.player == NULL)) delete    this->state.player;
    if (!(state.map == NULL)) delete    this->state.map;

    if (!(state.bullets == NULL))  delete[]  this->state.bullets;
    if (!(state.enemy_bullets == NULL))  delete[]  this->state.enemy_bullets;
    //Mix_FreeChunk(this->state.jump_sfx);
    //Mix_FreeMusic(this->state.bgm);
}

void Level::initialise()
{
    //what's next?
    state.next_scene_id = -1;

    //MAP
    GLuint map_texture_id = Utility::load_texture(PLATFORM_FILEPATH);
    state.map = new Map(LEVEL_WIDTH, LEVEL_HEIGHT, LEVEL_DATA, map_texture_id, 1.0f, 4, 1);

    //PLAYER
    //Existing
    state.player = new Object();

    state.player->set_type(PLAYER);

    state.player->set_position(player_init_pos);
    state.player->set_movement(glm::vec3(0.0f));
    state.player->set_speed(1.5f);
    state.player->set_acceleration(glm::vec3(0.0f, -5.5f, 0.0f));

    state.player->set_texture(Utility::load_texture(SPRITESHEET_FILEPATH));

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
    GLuint enemy_texture_id = Utility::load_texture(ENEMY_FILEPATH);

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

    state.enemies[0].set_position(enem1_init_pos);
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

    state.enemies[1].set_position(enem2_init_pos);
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

    state.enemies[2].set_position(enem3_init_pos);
    state.enemies[2].set_movement(glm::vec3(0.0f));
    state.enemies[2].set_speed(1.0f);
    state.enemies[2].set_acceleration(glm::vec3(0.0f, -5.5f, 0.0f));

    state.bullets = new Object[AMMO];

    for (int i = 0; i < AMMO; i++)
    {
        state.bullets[i].set_texture(Utility::load_texture(BULLET_FILEPATH));
        state.bullets[i].set_type(BULLET);

        state.bullets[i].set_width(0.5f);
        state.bullets[i].set_height(0.5f);

        state.bullets[i].set_activity(false);
    }

    state.enemy_bullets = new Object[AMMO];

    for (int i = 0; i < AMMO; i++)
    {
        state.enemy_bullets[i].set_texture(Utility::load_texture(ENEMY_BULLET_FILEPATH));
        state.enemy_bullets[i].set_type(ENEMYBULLET);

        state.enemy_bullets[i].set_width(0.5f);
        state.enemy_bullets[i].set_height(0.5f);

        state.enemy_bullets[i].set_activity(false);
    }


    //BGM and SFX
    Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 4096);

    state.bgm = Mix_LoadMUS("Zalza - Flybird.wav");
    Mix_PlayMusic(state.bgm, -1);
    Mix_VolumeMusic(MIX_MAX_VOLUME / 4.0f);

    state.jump_sfx = Mix_LoadWAV("391670__jeckkech__jump.wav");
    state.enemy_die_sfx = Mix_LoadWAV("391660__jeckkech__projectile.wav");
 } 

void Level::update(float delta_time)
{
    state.player->update(delta_time, state.player, state.enemies, ENEMY_COUNT, state.map, state.enemy_bullets);

    for (int i = 0; i < ENEMY_COUNT; i++) state.enemies[i].update(delta_time, state.player, state.enemies, 0, state.map, state.enemy_bullets);
    for (int i = 0; i < AMMO; i++) state.bullets[i].update(delta_time, state.player, state.enemies, ENEMY_COUNT, state.map, NULL);
    for (int i = 0; i < AMMO; i++) state.enemy_bullets[i].update(delta_time, state.player, state.player, 1, state.map, NULL);
}

void Level::render(ShaderProgram* program)
{
    state.player->render(program);
    state.map->render(program);

    for (int i = 0; i < ENEMY_COUNT; i++) state.enemies[i].render(program);
    for (int i = 0; i < AMMO; i++) state.bullets[i].render(program);
    for (int i = 0; i < AMMO; i++) state.enemy_bullets[i].render(program);
}

void Level::restart() 
{
    state.player->set_position(player_retry_pos);
    state.player->set_activity(true);
    state.player->set_ammo_count(0);

    state.enemies[0].set_position(enem1_init_pos);
    state.enemies[0].set_activity(true);
    state.enemies[0].set_ai_state(IDLE);
    state.enemies[0].set_ammo_count(0);

    state.enemies[1].set_position(enem2_init_pos);
    state.enemies[1].set_ai_state(IDLE);
    state.enemies[1].set_activity(true);
    state.enemies[1].reset_delay();
    state.enemies[1].clear_turnaround();

    state.enemies[2].set_position(enem3_init_pos);
    state.enemies[2].set_ai_state(IDLE);
    state.enemies[2].set_activity(true);
    state.enemies[2].clear_turnaround();

    for (int i = 0; i < AMMO; i++) state.bullets[i].set_activity(false);
}

int Level::get_id() { return id; }
void Level::set_id(int new_id) { id = new_id; }