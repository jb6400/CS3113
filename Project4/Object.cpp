#include "Object.h"

Object::Object()
{
    position = glm::vec3(0);
    velocity = glm::vec3(0);
    acceleration = glm::vec3(0);

    movement = glm::vec3(0);

    speed = 0;

    model_matrix = glm::mat4(1.0f);
}

Object::~Object()
{
    delete[] animation_left;
    delete[] animation_right;
    delete[] animations;
}

void Object::draw_sprite_from_texture_atlas(ShaderProgram* program, GLuint textureID, int index) 
{
    // Step 1: Calculate the UV location of the indexed frame
    float u_coord = (float)(index % animation_cols) / (float)animation_cols;
    float v_coord = (float)(index / animation_cols) / (float)animation_rows;

    // Step 2: Calculate its UV size
    float width = 1.0f / (float)animation_cols;
    float height = 1.0f / (float)animation_rows;

    // Step 3: Just as we have done before, match the texture coordinates to the vertices
    float tex_coords[] =
    {
        u_coord, v_coord + height, u_coord + width, v_coord + height, u_coord + width, v_coord,
        u_coord, v_coord + height, u_coord + width, v_coord, u_coord, v_coord
    };

    float vertices[] =
    {
        dir * -0.5, -0.5, dir * 0.5, -0.5, dir * 0.5, 0.5,
        dir * -0.5, -0.5, dir * 0.5,  0.5, dir * -0.5, 0.5
    };

    // Step 4: And render
    glBindTexture(GL_TEXTURE_2D, textureID);

    glVertexAttribPointer(program->positionAttribute, 2, GL_FLOAT, false, 0, vertices);
    glEnableVertexAttribArray(program->positionAttribute);

    glVertexAttribPointer(program->texCoordAttribute, 2, GL_FLOAT, false, 0, tex_coords);
    glEnableVertexAttribArray(program->texCoordAttribute);

    glDrawArrays(GL_TRIANGLES, 0, 6);

    glDisableVertexAttribArray(program->positionAttribute);
    glDisableVertexAttribArray(program->texCoordAttribute);
}

void Object::activate_ai(Object* player, Object* bullets)
{
    switch (ai_type)
    {
    case WALKER:
        ai_walker();
        break;

    case GUARD:
        ai_guard(player);
        break;

    case SHOOTER:
        ai_shooter(player, bullets);
        break;

    default:
        break;
    }
}

void Object::ai_walker()
{
    if (turn_around)
    {
        if (count_delay++ > 10)
        {
            dir = -dir;
            movement.x = -movement.x;
            count_delay = 0;
        }
        turn_around = false;
    }
    else
        count_delay = 11;
}

void Object::ai_guard(Object* player)
{
    float current_distance = position.x - player->position.x;
    if (distance == 0) distance = current_distance;
    if (!(current_distance * distance > 0) && turn_around)
    {
        turn_around = false;
        count_delay = 0;
    }
    else
        count_delay = 11;
    if (fabs(current_distance) < 3.0f && !turn_around) ai_state = WALKING;
    else ai_state = IDLE;

    switch (ai_state) {
        case IDLE:
        
            movement = glm::vec3(0.0f, 0.0f, 0.0f);
            
            //if (glm::distance(position, player->position) < 3.0f) ai_state = WALKING;
            break;

        case WALKING:

            if (position.x > player->get_position().x) {
                dir = -1;
                movement = glm::vec3(-1.0f, 0.0f, 0.0f);
            }
            else if (position.x < player->get_position().x) {
                dir = 1;
                movement = glm::vec3(1.0f, 0.0f, 0.0f);
            }
            break;

        case ATTACKING:
            break;

        default:
            break;
        }
    if (count_delay++ > 10)
    {
        distance = current_distance;
    }
}

void Object::ai_shooter(Object* player, Object* bullets)
{
    switch (ai_state) {
    case IDLE:

        movement = glm::vec3(0.0f, 0.0f, 0.0f);
        
        if (ammo_count < ammo && count_delay++ > 100)
        {
            bullets[ammo_count].set_position(glm::vec3(this->position.x, this->position.y, 0));
            bullets[ammo_count].set_activity(true);
            bullets[ammo_count].set_speed(5.f);
            bullets[ammo_count].set_movement(glm::vec3(-1, 0, 0));

            ammo_count++;
            count_delay = 0;
        }

        //if (glm::distance(position, player->position) < 3.0f) ai_state = ATTACKING;
        break;

    case ATTACKING:
        break;

    default:
        break;
    }
}

void Object::update(float delta_time, Object* player, Object* objects, int object_count, Map* map, Object* bullets)
{
    if (!is_active) return;

    collided_top = false;
    collided_bottom = false;
    collided_left = false;
    collided_right = false;

    if (type == ENEMY) activate_ai(player, bullets);

    if (animation_indices != NULL)
    {
        if (glm::length(movement) != 0)
        {
            animation_time += delta_time;
            float frames_per_second = 1.f / SECONDS_PER_FRAME;

            if (animation_time >= frames_per_second)
            {
                animation_time = 0.0f;
                animation_index++;

                if (animation_index >= animation_frames)
                {
                    animation_index = 0;
                }
            }
        }
    }

    // Our character moves from left to right, so they need an initial velocity
    velocity.x = movement.x * speed;

    // Now we add the rest of the gravity physics
    velocity += acceleration * delta_time;

    position.y += velocity.y * delta_time;
    check_collision_y(objects, object_count);
    check_collision_y(map);

    position.x += velocity.x * delta_time;
    check_collision_x(objects, object_count);
    check_collision_x(map);

    // Jump
    if (is_jumping)
    {
        // STEP 1: Immediately return the flag to its original false state
        is_jumping = false;

        // STEP 2: The player now acquires an upward velocity
        velocity.y += jumping_power;
    }

    model_matrix = glm::mat4(1.0f);
    model_matrix = glm::translate(model_matrix, position);

    if (type == BULLET && position.x == 15) is_active = false;
}

void const Object::check_collision_y(Object* collidable_entities, int collidable_entity_count)
{
    for (int i = 0; i < collidable_entity_count; i++)
    {
        Object* collidable_entity = &collidable_entities[i];

        if (check_collision(collidable_entity))
        {
            float y_distance = fabs(position.y - collidable_entity->position.y);
            float y_overlap = fabs(y_distance - (height / 2.0f) - (collidable_entity->height / 2.0f));
            
            if (collidable_entity->type == ENEMY) is_active = false;

            if (velocity.y > 0) {
                position.y -= y_overlap;
                velocity.y = 0;
                collided_top = true;
            }
            else if (velocity.y < 0) {
                position.y += y_overlap;
                velocity.y = 0;
                collided_bottom = true;
            }
        }
    }
}

void const Object::check_collision_x(Object* collidable_entities, int collidable_entity_count)
{
    for (int i = 0; i < collidable_entity_count; i++)
    {
        Object* collidable_entity = &collidable_entities[i];

        if (check_collision(collidable_entity))
        {
            float x_distance = fabs(position.x - collidable_entity->position.x);
            float x_overlap = fabs(x_distance - (width / 2.0f) - (collidable_entity->width / 2.0f));

            if (type == BULLET) 
            { 
                collidable_entity->is_active = false;
                is_active = false;
            }

            if (type == PLAYER) is_active = false;

            if (type == ENEMYBULLET && collidable_entity->type == PLAYER)
            {
                collidable_entity->is_active = false;
                is_active = false;
            }
           
            if (velocity.x > 0) {
                position.x -= x_overlap;
                velocity.x = 0;
                collided_right = true;
            }
            else if (velocity.x < 0) {
                position.x += x_overlap;
                velocity.x = 0;
                collided_left = true;
            }
        }
    }
}

void const Object::check_collision_y(Map* map)
{
    // Probes for tiles
    glm::vec3 top = glm::vec3(position.x, position.y + (height / 2), position.z);
    glm::vec3 top_left = glm::vec3(position.x - (width / 2), position.y + (height / 2), position.z);
    glm::vec3 top_right = glm::vec3(position.x + (width / 2), position.y + (height / 2), position.z);

    glm::vec3 bottom = glm::vec3(position.x, position.y - (height / 2), position.z);
    glm::vec3 bottom_left = glm::vec3(position.x - (width / 2), position.y - (height / 2), position.z);
    glm::vec3 bottom_right = glm::vec3(position.x + (width / 2), position.y - (height / 2), position.z);
    
    is_touching_bottom_center = false;
    is_touching_bottom_left = false;
    is_touching_bottom_right = false;

    float penetration_x = 0;
    float penetration_y = 0;

    if (map->is_solid(top, &penetration_x, &penetration_y) && velocity.y > 0)
    {
        position.y -= penetration_y;
        velocity.y = 0;
        collided_top = true;
    }
    else if (map->is_solid(top_left, &penetration_x, &penetration_y) && velocity.y > 0)
    {
        position.y -= penetration_y;
        velocity.y = 0;
        collided_top = true;
    }
    else if (map->is_solid(top_right, &penetration_x, &penetration_y) && velocity.y > 0)
    {
        position.y -= penetration_y;
        velocity.y = 0;
        collided_top = true;
    }

    if (map->is_solid(bottom, &penetration_x, &penetration_y) && velocity.y < 0)
    {
        position.y += penetration_y;
        velocity.y = 0;
        collided_bottom = true;

        is_touching_bottom_center = true;
    }
    
    if (map->is_solid(bottom_left, &penetration_x, &penetration_y) )
    {
        position.y += penetration_y;
        velocity.y = 0;
        collided_bottom = true;

        is_touching_bottom_left = true;
    }
    
    if (map->is_solid(bottom_right, &penetration_x, &penetration_y) )
    {
        position.y += penetration_y;
        velocity.y = 0;
        collided_bottom = true;

        is_touching_bottom_right = true;
    }

    if ((is_touching_bottom_center && is_touching_bottom_left && !is_touching_bottom_right) || (is_touching_bottom_center && is_touching_bottom_right && !is_touching_bottom_left))
    {
        turn_around = true;
        is_touching_bottom_center = false;
        is_touching_bottom_left = false;
        is_touching_bottom_right = false;

    }
}

void const Object::check_collision_x(Map* map)
{
    // Probes for tiles
    glm::vec3 left = glm::vec3(position.x - (width / 2), position.y, position.z);
    glm::vec3 right = glm::vec3(position.x + (width / 2), position.y, position.z);

    float penetration_x = 0;
    float penetration_y = 0;

    if (map->is_solid(left, &penetration_x, &penetration_y) && velocity.x < 0)
    {
        position.x += penetration_x;
        if (type == ENEMY)
            if(ai_type == GUARD)  ai_state = IDLE; 
            else turn_around = true;
        if (type == PLAYER) velocity.x = 0;
        if (type == BULLET || type == ENEMYBULLET) is_active = false;
        collided_left = true;
    }
    if (map->is_solid(right, &penetration_x, &penetration_y) && velocity.x > 0)
    {
        position.x -= penetration_x;
        if (type == ENEMY)
            if (ai_type == GUARD)  ai_state = IDLE; 
            else turn_around = true;
        if(type == PLAYER) velocity.x = 0;
        if (type == BULLET || type == ENEMYBULLET) is_active = false;
        collided_right = true;
    }
}

void Object::render(ShaderProgram* program) 
{
    if (!is_active) return;

    program->SetModelMatrix(model_matrix);

    if (animation_indices != NULL)
    {
        draw_sprite_from_texture_atlas(program, textureID, animation_indices[animation_index]);
        return;
    }

    float vertices[] = { width * -0.5, height * -0.5, width * 0.5, height * -0.5, 
                         width * 0.5, height * 0.5, width * -0.5, height * -0.5, 
                         width * 0.5, height * 0.5, width * -0.5, height * 0.5 };
    float texCoords[] = { 0.0, 1.0, 1.0, 1.0, 1.0, 0.0, 0.0, 1.0, 1.0, 0.0, 0.0, 0.0 };

    glBindTexture(GL_TEXTURE_2D, textureID);

    glVertexAttribPointer(program->positionAttribute, 2, GL_FLOAT, false, 0, vertices);
    glEnableVertexAttribArray(program->positionAttribute);

    glVertexAttribPointer(program->texCoordAttribute, 2, GL_FLOAT, false, 0, texCoords);
    glEnableVertexAttribArray(program->texCoordAttribute);

    glDrawArrays(GL_TRIANGLES, 0, 6);

    glDisableVertexAttribArray(program->positionAttribute);
    glDisableVertexAttribArray(program->texCoordAttribute);
}

bool const Object::check_collision(Object* other) 
{
    if (!other->is_active || !is_active) return false;

    float x_distance = fabs(position.x - other->position.x) - ((width + other->width) / 2.0f);
    float y_distance = fabs(position.y - other->position.y) - ((height + other->height) / 2.0f);

    return x_distance < 0.0f && y_distance < 0.0f;
}

bool const Object::check_collision(Object* other) const
{
    // If we are checking with collisions with ourselves, this should be false
    if (other == this) return false;

    // If either entity is inactive, there shouldn't be any collision
    if (!is_active || !other->is_active) return false;

    float x_distance = fabs(position.x - other->position.x) - ((width + other->width) / 2.0f);
    float y_distance = fabs(position.y - other->position.y) - ((height + other->height) / 2.0f);

    return x_distance < 0.0f && y_distance < 0.0f;
}

glm::vec3 Object::get_position() { return position; }
void Object::set_position(glm::vec3 new_pos) { position = new_pos; }

glm::vec3 Object::get_movement() { return movement; }
void Object::set_movement(glm::vec3 new_mov) { movement = new_mov; }

float Object::get_speed() { return speed; }
void Object::set_speed(float new_speed) { speed = new_speed; }

GLuint Object::get_texture() { return textureID; }
void Object::set_texture(GLuint new_tex) { textureID = new_tex; }

glm::vec3 Object::get_acceleration() { return acceleration; }
void Object::set_acceleration(glm::vec3 new_acc) { acceleration = new_acc; }

void Object::set_velocity(glm::vec3 new_vel) { velocity = new_vel; }

float Object::get_width() { return width; }
void Object::set_width(float new_width) { width = new_width; }

float Object::get_height() { return height; }
void Object::set_height(float new_height) { height = new_height; }

ObjectType Object::get_type() { return type; }
void Object::set_type(ObjectType new_type) { type = new_type; }

void Object::set_anim_cols(int num) { animation_cols = num; }
void Object::set_anim_rows(int num) { animation_rows = num; }

void Object::set_indicies(int* new_ind) { animation_indices = new_ind; }
void Object::set_anim_frames(int num) { animation_frames = num; }

float Object::get_dir() { return dir; }
void Object::set_dir(float new_dir) { dir = new_dir; }

bool Object::get_state() { return is_win; }
void Object::set_state(bool new_state) { is_win = new_state; }

bool Object::get_status() { return in_game; }
void Object::set_status(bool new_status) { in_game = new_status; }

bool Object::get_collision(int type) 
{
    switch (type) 
    {
    case 0:
        return collided_top;
    case 1:
        return collided_right;
    case 2:
        return collided_bottom;
    case 3:
        return collided_left;
    }
}

bool Object::get_jump() { return is_jumping; }
void Object::set_jump(bool stat) { is_jumping = stat; }

void Object::set_power(float new_power) { jumping_power = new_power; }

void Object::set_ai_type(AIType new_type) { ai_type = new_type; }
void Object::set_ai_state(AIState new_state) { ai_state = new_state; }

bool Object::get_activity() { return is_active; }
void Object::set_activity(bool act) { is_active = act; }

int Object::get_ammo_count() { return ammo_count; }
void Object::set_ammo_count(int new_count) { ammo_count = new_count; }
void Object::reset_delay() { count_delay = 11; }
void Object::clear_turnaround() { turn_around = false; }