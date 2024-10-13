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
#include "Entity.h"


// Default constructor
Entity::Entity()
    : m_position(0.0f), m_movement(0.0f), m_scale(1.0f, 1.0f, 0.0f), m_model_matrix(1.0f),
    m_speed(0.0f), m_texture_id(0)
{

}



// Simpler Parameterized constructor
Entity::Entity(GLuint texture_id, float speed)
    : m_position(0.0f), m_movement(0.0f), m_scale(1.0f, 1.0f, 0.0f), m_model_matrix(1.0f),
    m_speed(speed), m_texture_id(texture_id)
{
}

Entity::Entity(GLuint texture_id, glm::vec3 scale)
    : m_position(0.0f), m_movement(0.0f), m_scale(scale), m_model_matrix(1.0f),
    m_speed(0.0f), m_texture_id(texture_id)
{
}

Entity::~Entity() { }



void Entity::update(float delta_time)
{
    // check if player is should move automatically (game is in single player mode)
    if (!m_is_playing) {
        if (m_position.y == 10)
            m_movement.y = 1;
        else if (m_position.y == -10)
            m_movement.y = -1;

    }

    m_position += m_movement * m_speed * delta_time;
    m_model_matrix = glm::mat4(1.0f);
    m_model_matrix = glm::translate(m_model_matrix, m_position);
    m_model_matrix = glm::scale(m_model_matrix, m_scale);


}




void Entity::update(Entity* player2, Entity* bg, Entity** balls, int balls_num, float delta_time, float height, float width)
{
    // check if player is should move automatically (game is in single player mode)
    //if (!is_playing) {
    //    if (m_position.y == 10)
    //        m_movement.y = 1;
    //    else if (m_position.y == -10)
    //        m_movement.y = -1;

    //}

    //Reset model matrix
    m_model_matrix = glm::mat4(1.0f);
    player2->m_model_matrix = glm::mat4(1.0f);
    bg->m_model_matrix = glm::mat4(1.0f);


    for (int i = 0; i < balls_num; i++) {

        //check collision
        if (check_collision(balls[i])) {
            balls[i]->m_movement.x = -0.25;
            balls[i]->m_movement.y = -0.17;
        }
        else
            if (player2->check_collision(balls[i])) {
                balls[i]->m_movement.x = 0.25;
                balls[i]->m_movement.y = 0.1;
            }

        //Check if ball is colliding with the window
        if (balls[i]->check_window_collision_top(height) || balls[i]->check_window_collision_bottom(height)) {
            balls[i]->m_movement *= -1;
        }



        //Check if the ball entered left/right side of the window (losing condition) 
        if (balls[i]->check_window_collision_left(width + 0.5)) {
            player2->m_game_status = false;
            balls[i]->m_movement = glm::vec3(0.0f);
        }

        if (balls[i]->check_window_collision_right(width + 0.5)) {
            m_game_status = false;
            balls[i]->m_movement = glm::vec3(0.0f);
        }

        //Change position (accumlate if the ball did not hit the right/left end of the window)
        if (m_game_status && player2->m_game_status)
            balls[i]->m_position += balls[i]->m_movement * balls[i]->m_speed * delta_time;
        else
            balls[i]->m_scale = glm::vec3(0.0f);



        balls[i]->m_model_matrix = glm::mat4(1.0f);
        balls[i] ->m_model_matrix = glm::translate(balls[i]->m_model_matrix, balls[i]->m_position);


    }


    //Check if single player mode
    if (!player2->m_is_playing) {
        if (player2->check_window_collision_top(height - 1)) {
            player2->m_movement.y = -1;  // Reverse movement if collision at the top
        }
        else if (player2->check_window_collision_bottom(height - 1)) {
            player2->m_movement.y = 1;   // Reverse movement if collision at the bottom
        }
    }


    

    //Change position (accumlate if the ball did not hit the right/left end of the window)

    if (m_game_status && player2->m_game_status) {

        //do not accumlate if the player collides with the top or the bottom of the window
        if (check_window_collision_bottom(3.3))
            m_movement.y = 1;
        if (check_window_collision_top(3.3))
            m_movement.y = -1;

        //same logic for player 2
        if (player2->check_window_collision_bottom(3.3))
            player2 -> m_movement.y = 1;
        if (player2->check_window_collision_top(3.3))
            player2 -> m_movement.y = -1;

        m_position += m_movement * m_speed * delta_time;
        player2->m_position += player2->m_movement * player2->m_speed * delta_time;
    }
   


    //Translate
    m_model_matrix = glm::translate(m_model_matrix, m_position);
    player2->m_model_matrix = glm::translate(player2->m_model_matrix, player2->m_position);


    //Scale
    m_model_matrix = glm::scale(m_model_matrix, m_scale);
    player2->m_model_matrix = glm::scale(player2->m_model_matrix, player2->m_scale);
    bg->m_model_matrix = glm::scale(bg->m_model_matrix, bg->m_scale);




}

void Entity::render(ShaderProgram *program)
{
    program->set_model_matrix(m_model_matrix);
    
    
    float vertices[]   = { -0.5, -0.5, 0.5, -0.5, 0.5, 0.5, -0.5, -0.5, 0.5, 0.5, -0.5, 0.5 };
    float tex_coords[] = {  0.0,  1.0, 1.0,  1.0, 1.0, 0.0,  0.0,  1.0, 1.0, 0.0,  0.0, 0.0 };
    
    glBindTexture(GL_TEXTURE_2D, m_texture_id);
    
    glVertexAttribPointer(program->get_position_attribute(), 2, GL_FLOAT, false, 0, vertices);
    glEnableVertexAttribArray(program->get_position_attribute());
    glVertexAttribPointer(program->get_tex_coordinate_attribute(), 2, GL_FLOAT, false, 0, tex_coords);
    glEnableVertexAttribArray(program->get_tex_coordinate_attribute());
    
    glDrawArrays(GL_TRIANGLES, 0, 6);
    
    glDisableVertexAttribArray(program->get_position_attribute());
    glDisableVertexAttribArray(program->get_tex_coordinate_attribute());
}

bool const Entity::check_collision(Entity* other) const
{
    float x_distance = fabs(m_position.x - other->m_position.x) - ((m_scale.x + other->m_scale.x) / 2.0f);
    float y_distance = fabs(m_position.y - other->m_position.y) - ((m_scale.y + other->m_scale.y) / 2.0f);

    return x_distance < 0.0f && y_distance < 0.0f;
}


bool const Entity::check_window_collision_top(float height) const {
    //Check if the ball has collided with the top or bottom and return
    return m_position.y >= height;
  
}


bool const Entity::check_window_collision_bottom(float height) const {
    //Check if the ball has collided with the top or bottom and return
    return m_position.y <= -height;
}


bool const Entity::check_window_collision_right(float width) const {
    return m_position.x >= width;
}

bool const Entity::check_window_collision_left(float width) const {
    return m_position.x <= -width;
}



