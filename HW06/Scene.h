#pragma once
#define GL_SILENCE_DEPRECATION

#ifdef _WINDOWS
#include <GL/glew.h>
#endif

#define GL_GLEXT_PROTOTYPES 1
#include <SDL_mixer.h>
#include <SDL.h>
#include <SDL_opengl.h>
#include "glm/mat4x4.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "ShaderProgram.h"
#include "Utility.h"
#include "Entity.h"
#include "Map.h"

/**
    Notice that the game's state is now part of the Scene class, not the main file.
*/
struct GameState
{
    // ————— GAME OBJECTS ————— //
    Map *map;
    Entity *player;
    Entity *enemies;
    Entity *milk;
    
    // ————— AUDIO ————— //
    Mix_Music *bgm;
    Mix_Chunk *jump_sfx;
    Mix_Chunk *shoot_sfx;
    Mix_Chunk* collect_sfx;
    Mix_Chunk* winning_sfx;
    Mix_Chunk* losing_sfx;



    
    // ————— POINTERS TO OTHER SCENES ————— //
    int next_scene_id;
};

class Scene {
protected:
    GameState m_game_state;
    GLuint font_texture_id;
public:
    // ————— ATTRIBUTES ————— //
    int m_number_of_enemies = 7;
    bool m_winning_sound_on = false;
    bool m_losing_sound_on = false;


    // ---- Pointer to  refer to the number of lifes ---- //
    int static m_number_of_lives;
    
    // ————— METHODS ————— //
    virtual void initialise() = 0;
    virtual void update(float delta_time) = 0;
    virtual void render(ShaderProgram *program) = 0;
    
    // ————— GETTERS ————— //
    GameState const get_state() const { return m_game_state;             }
    int const get_number_of_enemies() const { return m_number_of_enemies; }
    void const set_next_scene_id (int num) { m_game_state.next_scene_id = num; }
};


