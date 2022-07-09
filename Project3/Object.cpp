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

void Object::update(float delta_time, Object* colliders, int num_collider)
{
    //collision checker
    for (int i = 0; i < num_collider; i++)
    {
        Object* collider = &colliders[i];

        if (check_collision(collider))
        {
            float y_distance = fabs(position.y - collider->position.y);
            float y_overlap = fabs(y_distance - (height / 2.0f) - (collider->height / 2.0f));
            if (velocity.y > 0) {
                position.y -= y_overlap;
                velocity.y = 0;
            }
            else if (velocity.y < 0) {
                position.y += y_overlap;
                velocity.y = 0;
            }
        }
    }

    // Our character moves from left to right, so they need an initial velocity
    velocity.x = movement.x * speed;
    // Now we add the rest of the gravity physics
    velocity += acceleration * delta_time;
    //_RPTF2(_CRT_WARN, "velocity x, y, z: %f % f %f\n", 
    //    velocity.x, velocity.y, velocity.z);
    //_RPTF2(_CRT_WARN, "acceleration x, y, z: %f % f %f\n", 
    //    acceleration.x, acceleration.y, acceleration.z);
    position += velocity * delta_time;

    //_RPTF2(_CRT_WARN, "position x, y, z: %f % f %f\n",
    //    position.x, position.y, position.z);

    model_matrix = glm::mat4(1.f);
    model_matrix = glm::translate(model_matrix, position);

    movement = glm::vec3(0);
}

void Object::render(ShaderProgram* program) 
{
    program->SetModelMatrix(model_matrix);

    float vertices[] = { -0.5, -0.5, 0.5, -0.5, 0.5, 0.5, -0.5, -0.5, 0.5, 0.5, -0.5, 0.5 };
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