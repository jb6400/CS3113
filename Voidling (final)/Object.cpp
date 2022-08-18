#include "Object.h"

Object::Object()
{
    position = glm::vec3(0);
    velocity = glm::vec3(0);
    acceleration = glm::vec3(0);

    movement = glm::vec3(0);

    speed = 0;

    model_matrix = glm::mat4(1.0f);

    spawn_time = (rand() % 1000) + 100;
    sleeping_time = 500;
}

Object::~Object()
{
    delete [] animation_right_walk;
    delete [] animation_left_walk;

    delete [] animation_right_run;
    delete [] animation_left_run; 

    delete [] animation_idle;
    delete [] animation_idle_happy ;

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

void Object::activate_ai(Object* player, Object* objects, int object_count)
{
    switch (ai_type)
    {
    case CAT:
        ai_cat(player, objects, object_count);
        break;

    case MOUSE:
        ai_mouse(player);
        break;

    case DOG:
        ai_dog(player);
        break;

    default:
        break;
    }
}

void Object::ai_cat(Object* player, Object* objects, int object_count)
{
    if (ai_state != RELAXING)
    {
        for (int i = 0; i < object_count; i++)
        {
            if (!objects[i].is_active)
                continue;

            if (objects[i].ai_type == DOG)
            {
                if (level_pos == objects[i].level_pos)
                    current_distance_d = position.x - objects[i].position.x;
                else
                    current_distance_d = (position.x + (level_pos - 1) * 11)
                    - (objects[i].position.x + (objects[i].level_pos - 1) * 11);


                if (distance == 0) distance = current_distance_d;

                if (!(current_distance_d * distance > 0) && turn_around)
                {
                    turn_around = false;
                    count_delay = 0;
                }
                else
                    count_delay = 11;

                if (fabs(current_distance_d) < 3.0f) //&& !turn_around)
                {
                    if (level_pos == objects[i].level_pos)
                    {
                        this->set_indicies(animations[RIGHT_R]);
                        movement = glm::vec3(1.f, 0.f, 0.f);
                        dir = 1;
                        objects[i].set_state(BARKING);
                        ai_state = RUNNING;
                        break;
                    }
            }
        }

        if (objects[i].ai_type == MOUSE)
        {
            if (level_pos == objects[i].level_pos || ai_state == IDLE)
                current_distance_m = position.x - objects[i].position.x;
            else
                current_distance_m = (position.x + (level_pos - 1) * 11)
                - (objects[i].position.x + (objects[i].level_pos - 1) * 11);


            if (distance == 0) distance = current_distance_m;

            if (!(current_distance_m * distance > 0) && turn_around)
            {
                turn_around = false;
                count_delay = 0;
            }
            else
                count_delay = 11;

            if (fabs(current_distance_m) < 3.0f && !turn_around && ai_state != RUNNING)
            {
                if (position.y == -5.f && objects[i].position.y > position.y)
                {
                    movement = glm::vec3(1.f, 0.f, 0.f);
                    dir = 1;
                    ai_state = WALKING;
                }
                else if (position.y >= -2.f && objects[i].position.y < (position.y - 1.f))
                {
                    movement = glm::vec3(-1.f, 0.f, 0.f);
                    dir = -1;
                    ai_state = WALKING;
                }
                else if ((position.y == -5.f && objects[i].position.y <= position.y) ||
                    (position.y >= -2.f && objects[i].position.y >= (position.y - 1.f)))
                    ai_state = CHASING;
            }

            else if (level_pos == objects[i].level_pos && ai_state != RUNNING && !button_down)
                ai_state = IDLE;
        }
    }
    }
    
    switch (ai_state) {
        case IDLE:
            this->set_indicies(animations[IDL]);
            movement = glm::vec3(0.0f, 0.0f, 0.0f);
            break;
    
        case CHASING:

            button_down = false;
            walking_time = 300;

            this->set_indicies(animations[RIGHT_R]);

            if (current_distance_m > 0 && fabs(current_distance_m) < 3) {
                dir = -1;
                movement = glm::vec3(-1.0f, 0.0f, 0.0f);
            }
            else if (current_distance_m < 0 && fabs(current_distance_m) < 3) {
                dir = 1;
                movement = glm::vec3(1.0f, 0.0f, 0.0f);
            }
            else if (fabs(current_distance_m) > 3)
                ai_state = IDLE;

            energy -= 0.06;

            break;
    
        case WALKING:
            this->set_indicies(animations[RIGHT_W]);

            if (button_down && walking_time-- <= 0 ) 
            {
                ai_state = IDLE;
                button_down = false;
                walking_time = 300;
            }

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

            energy -= 0.02;

            break;

        case RUNNING:

            button_down = false;
            walking_time = 300;

            energy -= 0.07;
            break;

        case RELAXING:

            button_down = false;
            walking_time = 300;

            energy += 0.01;
            if (relax_time-- < 0) 
            {
                this->set_indicies(animations[IDLE]);
                ai_state = IDLE;
            }
            break;
        case EATING:

            button_down = false;
            walking_time = 300;

            this->set_indicies(animations[IDL]);
            movement = glm::vec3(0.f, 0.f, 0.f);
            energy += 0.03;

            if (objects[2].food <= 0) 
            {
                objects[2].food -= 0.3f;
                objects[2].set_indicies(objects[2].animations[IDL]);
                ai_state = IDLE;
            }
            break;
        default:
            break;
        }
    if (count_delay++ > 10)
    {
        distance = current_distance_m;
    }
}

void Object::ai_mouse(Object* player) 
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

void Object::ai_dog(Object* player) 
{
    switch (ai_state)
    {
    case WALKING:
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
        break;
    case BARKING:
        break;
    }
}

void Object::activate_dish(Object* player) 
{
    switch (ai_type) 
    {
    case FULL:
        break;
    case EMPTY:
        break;
    default:
        break;
    }
}

void Object::update(float delta_time, Object* player, Object* objects, int object_count, Map* map)
{
    if (!is_active && type == ANIMAL && spawn_time-- > 0)
    {
        return;
    }
    else if (!is_active && type == ANIMAL)
    {
        is_active = true;
        if (ai_type == DOG)
        {
            dir = 1;
            movement = glm::vec3(1.f, 0.f, 0.f);
        }
        else if (ai_type == MOUSE)
        {
            dir = -1;
            movement = glm::vec3(-1.f, 0.f, 0.f);
        }
    }

    if (type == ANIMAL && ai_type == CAT)
    {
        if (ai_state != RUNNING && ai_state != RELAXING && --spawn_time <= 0)
        {
            this->set_indicies(animations[RIGHT_R]);
            dir = 1;
            movement = glm::vec3(1.f, 0.f, 0.f);
            ai_state = RUNNING;
        }
    }

    collided_top = false;
    collided_bottom = false;
    collided_left = false;
    collided_right = false;

    if (type == ANIMAL) activate_ai(player, objects, object_count);

    if (type == DISH) activate_dish(player);

    if (animation_indices != NULL)
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

    // Our character moves from left to right, so they need an initial velocity
    velocity.x = movement.x * speed;
    velocity.y = movement.y * speed;

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
            
            if ((collidable_entity->type == ANIMAL || collidable_entity->type == DISH)) return;

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

        srand(time(NULL));
        int probability = rand() % 100;

        if (check_collision(collidable_entity))
        {
            float x_distance = fabs(position.x - collidable_entity->position.x);
            float x_overlap = fabs(x_distance - (width / 2.0f) - (collidable_entity->width / 2.0f));

            if (collidable_entity->type == DISH && (ai_type == CAT || ai_type == DOG)
                && collidable_entity->food > 0)
            {
                switch (ai_type) 
                {
                case DOG:
                    collidable_entity->food = 0;
                    collidable_entity->set_indicies(collidable_entity->animations[IDL]);
                    break;
                case CAT:
                    ai_state = EATING;
                    break;
                default:
                    break;
                }
                return;
            }
            //maybe add probablility later
            else if (collidable_entity->ai_type == MOUSE && ai_type == CAT && ai_state == CHASING
                && probability <= 30 ) 
            {
                collidable_entity->is_active = false;
                collidable_entity->spawn_time = (rand() % 1000) + 100;
                collidable_entity->laps = 0;

                collidable_entity->position = glm::vec3(11.5f, -3.f, 0.f);

                energy++;
                ai_state = IDLE;
                return;
            }
            else 
                return;

            /*if (collidable_entity->type == ANIMAL) return;*/

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
    if (type == PLAYER) return;

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
    if (type == PLAYER || !is_active) return;

    // Probes for tiles
    glm::vec3 left = glm::vec3(position.x - (width / 2), position.y, position.z);
    glm::vec3 right = glm::vec3(position.x + (width / 2), position.y, position.z);

    float penetration_x = 0;
    float penetration_y = 0;

    if (map->is_solid(left, &penetration_x, &penetration_y) && velocity.x < 0)
    {
        position.x += penetration_x;
        if (type == ANIMAL)
            if (position.x <= 1.f && position.y >= -3.f && position.y <= -1.8f)
            {
                if (ai_type == MOUSE)
                {      
                    position.x = 12.f;
                    position.y = -6.f;
                }
                else if (ai_type == CAT)
                {
                    position.x = 11.5f;
                    position.y = -5.f;
                }
                level_pos = 1;
                return;
            }
            else if (ai_type == DOG && position.x <= 1.f && position.y <= -4.8f && position.y >= -6.f) 
            {
                
                spawn_time = (rand() % 1000) + 100;
                is_active = false;
            }
            else
                turn_around = true;

        collided_left = true;
    }
    if (map->is_solid(right, &penetration_x, &penetration_y) && velocity.x > 0)
    {
        position.x -= penetration_x;
        if (type == ANIMAL)
            if ((position.x >= 12.f && position.y <= -4.8f && position.y >= -6.f)
                && (ai_type == CAT || ai_type == MOUSE))
            {
                if (ai_type == MOUSE)
                {
                    position.y = -3.f;
                    position.x = 1.f;
                }
                else if (ai_type == CAT)
                {
                    position.y = -2.f;
                    position.x = 1.75f;
                }

                level_pos = 2;
                return;

            }
            else if (position.x >= 12.f && position.y >= -3.f && position.y <= -1.8f
                && ai_state == RUNNING)
            {
                dir = -1;
                if (spawn_time <= 0) 
                {
                    this->set_indicies(animations[SLEEP]); //have sleep
                    relax_time = 500;
                    spawn_time = (rand() % 1000) + 1000;
                    ai_state = RELAXING;
                }
                else 
                {
                    this->set_indicies(animations[IDL]);
                    relax_time = 100;
                    ai_state = RELAXING;
                }
            }
            else if (position.x >= 12.f && position.y >= -3.f && position.y <= -1.8f
                && ++laps >= 2 && ai_type == MOUSE)
            {
                spawn_time = (rand() % 1000) + 100;
                laps = 0;
                is_active = false;
            }
            else
                turn_around = true;

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


//GETTERS AND SETTERS
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
int& Object::get_indicies() { return *animation_indices; }
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
AIState Object::get_ai_state() { return ai_state; }

bool Object::get_activity() { return is_active; }
void Object::set_activity(bool act) { is_active = act; }

void Object::reset_delay() { count_delay = 11; }
void Object::clear_turnaround() { turn_around = false; }

int Object::get_lives() { return lives; }
void Object::set_lives(int new_life) { lives = new_life; }

float Object::get_energy() { return energy; }
void Object::set_energy(float new_energ) { energy = new_energ; }

int Object::get_level() { return level_pos; }

bool Object::get_button() { return button_down; }
void Object::set_button(bool button) { button_down = button; }

void Object::set_food(float new_food) { food = new_food; }