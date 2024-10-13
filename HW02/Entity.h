#include <vector>
class Entity
{
private:
    //------PROPERTIES------//
    float m_height = 1.0f;
    float m_width = 1.0f;
    // ————— TRANSFORMATIONS ————— //
    glm::vec3 m_movement;
    glm::vec3 m_position;
    glm::vec3 m_scale;
    
    glm::mat4 m_model_matrix;
    
    float     m_speed;
    bool m_is_playing = 1;
    bool m_game_status = true;


    // ————— TEXTURES ————— //
    GLuint    m_texture_id;

    
public:
    // ————— STATIC VARIABLES ————— //
    static constexpr int SECONDS_PER_FRAME = 4;

    // ————— METHODS ————— //
    Entity();
    Entity(GLuint texture_id, float speed);
    Entity(GLuint texture_id, glm::vec3 scale);
    ~Entity();

    void draw_sprite_from_texture_atlas(ShaderProgram *program, GLuint texture_id, 
                                        int index);
    void update(float delta_time);
    void update(Entity* player2, Entity* bg, Entity** balls, int balls_num, float delta_time, float height, float width);
    void render(ShaderProgram *program);
    void move_up() { m_movement.y = 1; };
    void move_down() { m_movement.y = -1; }
    

    bool const check_collision(Entity* other) const;
    bool const check_window_collision_top(float height) const;
    bool const check_window_collision_bottom(float height) const;

    bool const check_window_collision_left(float width) const;
    bool const check_window_collision_right(float width) const;


 

    void normalise_movement() { m_movement = glm::normalize(m_movement); };

    


    // ————— GETTERS ————— //
    glm::vec3 const get_position()   const { return m_position;   }
    glm::vec3 const get_movement()   const { return m_movement;   }
    glm::vec3 const get_scale()      const { return m_scale;      }
    GLuint    const get_texture_id() const { return m_texture_id; }
    float     const get_speed()      const { return m_speed;      }
    bool const get_mode() const { return m_is_playing; }
    bool const get_game_status() const { return m_game_status; }




    // ————— SETTERS ————— //
    void const set_position(glm::vec3 new_position)  { m_position   = new_position;     }
    void const set_movement(glm::vec3 new_movement)  { m_movement   = new_movement;     }
    void const set_scale(glm::vec3 new_scale)        { m_scale      = new_scale;        }
    void const set_texture_id(GLuint new_texture_id) { m_texture_id = new_texture_id;   }
    void const set_speed(float new_speed)           { m_speed      = new_speed;        }
    void set_mode() {
        m_is_playing = !m_is_playing;
        }

    void const set_game_status(bool status) { m_game_status = status; }


};
