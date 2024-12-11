/**
* Author: Khouzama Bin Jardan
* Assignment: Platformer
* Date due: 2023-11-26, 11:59pm
* I pledge that I have completed this assignment without
* collaborating with anyone else, in conformance with the
* NYU School of Engineering Policies and Procedures on
* Academic Misconduct.
**/


#define GL_SILENCE_DEPRECATION
#define GL_GLEXT_PROTOTYPES 1
#define FIXED_TIMESTEP 0.0166666f
#define LEVEL1_WIDTH 14
#define LEVEL1_HEIGHT 8
#define LEVEL1_LEFT_EDGE 5.0f


#ifdef _WINDOWS
#include <GL/glew.h>
#endif

#include <SDL_mixer.h>
#include <SDL.h>
#include <SDL_opengl.h>
#include "glm/mat4x4.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "ShaderProgram.h"
#include "cmath"
#include <ctime>
#include <vector>
#include "Entity.h"
#include "Map.h"
#include "Utility.h"
#include "Scene.h"
#include "Menu.h"
#include "LevelA.h"
#include "LevelB.h"
#include "LevelC.h"



// ————— CONSTANTS ————— //
constexpr int WINDOW_WIDTH  = 960,
          WINDOW_HEIGHT = 720;

constexpr float BG_RED = 0.68f,
                BG_BLUE = 0.90f,
                BG_GREEN = 0.85f,
                BG_OPACITY = 1.0f;

constexpr int VIEWPORT_X = 0,
          VIEWPORT_Y = 0,
          VIEWPORT_WIDTH  = WINDOW_WIDTH,
          VIEWPORT_HEIGHT = WINDOW_HEIGHT;

constexpr char V_SHADER_PATH[] = "shaders/vertex_textured.glsl",
           F_SHADER_PATH[] = "shaders/fragment_textured.glsl";

constexpr float MILLISECONDS_IN_SECOND = 1000.0;

enum AppStatus { RUNNING, TERMINATED };

int g_is_dead = 0;



// ————— GLOBAL VARIABLES ————— //
Scene *g_current_scene;
Menu *g_menu;
LevelA *g_level_a;
LevelB* g_level_b;
LevelC* g_level_c;


Scene* g_levels[4]; //change to 3 when adding the new scence



SDL_Window* g_display_window;

AppStatus g_app_status = RUNNING;
ShaderProgram g_shader_program;
glm::mat4 g_view_matrix, g_projection_matrix;

float g_previous_ticks = 0.0f;
float g_accumulator = 0.0f;

constexpr char   FONT_FILEPATH[] = "assets/images/font1.png";

GLuint font_texture_id;

void switch_to_scene(Scene *scene)
{
    g_current_scene = scene;
    g_current_scene->initialise();
}

void initialise();
void process_input();
void update();
void render();
void shutdown();


void initialise()
{
    // ————— VIDEO ————— //
    SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO);
    g_display_window = SDL_CreateWindow("Mochi's Milk Quest",
                                      SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
                                      WINDOW_WIDTH, WINDOW_HEIGHT,
                                      SDL_WINDOW_OPENGL);
    
    SDL_GLContext context = SDL_GL_CreateContext(g_display_window);
    SDL_GL_MakeCurrent(g_display_window, context);
    if (context == nullptr)
    {
        shutdown();
    }
    
#ifdef _WINDOWS
    glewInit();
#endif
    
    // ————— GENERAL ————— //
    glViewport(VIEWPORT_X, VIEWPORT_Y, VIEWPORT_WIDTH, VIEWPORT_HEIGHT);
    
    g_shader_program.load(V_SHADER_PATH, F_SHADER_PATH);
    
    g_view_matrix = glm::mat4(1.0f);
    g_projection_matrix = glm::ortho(-5.0f, 5.0f, -3.75f, 3.75f, -1.0f, 1.0f);
    
    g_shader_program.set_projection_matrix(g_projection_matrix);
    g_shader_program.set_view_matrix(g_view_matrix);

    glUseProgram(g_shader_program.get_program_id());
    
    glClearColor(BG_RED, BG_GREEN, BG_BLUE, BG_OPACITY);
    
    // ---- MENU SETUP ----- //
    g_menu = new Menu();
    
    // ————— LEVES SETUP ————— //
    g_level_a = new LevelA();
    g_level_b = new LevelB();
    g_level_c = new LevelC();



    g_levels[0] = g_menu;
    g_levels[1] = g_level_a;
    g_levels[2] = g_level_b;
    g_levels[3] = g_level_c;

    switch_to_scene(g_levels[0]);
    
    // ————— BLENDING ————— //
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    font_texture_id = Utility::load_texture("assets/images/font1.png");


    
    g_shader_program.set_is_dead(g_is_dead);

}

void process_input()
{
    g_current_scene->get_state().player->set_movement(glm::vec3(0.0f));


    SDL_Event event;
    while (SDL_PollEvent(&event))
    {
        // ————— KEYSTROKES ————— //
        switch (event.type) {
            // ————— END GAME ————— //
            case SDL_QUIT:
            case SDL_WINDOWEVENT_CLOSE:
                g_app_status = TERMINATED;
                break;
                
            case SDL_KEYDOWN:
                switch (event.key.keysym.sym) {
                    case SDLK_q:
                        // Quit the game with a keystroke
                        g_app_status = TERMINATED;
                        break;
                        
                    case SDLK_SPACE:
                        // ————— JUMPING ————— //
                        if (g_current_scene->get_state().player->get_collided_bottom())
                        {
                            g_current_scene->get_state().player->jump();
                            Mix_PlayChannel(-1,  g_current_scene->get_state().jump_sfx, 0);
                        }
                         break;

                    case SDLK_RETURN:
                        if (g_current_scene == g_menu)
                        {
                            g_menu->start = true;
                        }
                        break;

                    case SDLK_f:
                        g_current_scene->get_state().player->shoot();
                        Mix_PlayChannel(-1, g_current_scene->get_state().shoot_sfx, 0);

                        break;


                    default:
                        break;
                }
                
            default:
                break;
        }
    }
    
    // ————— KEY HOLD ————— //
    const Uint8 *key_state = SDL_GetKeyboardState(NULL);

    if (key_state[SDL_SCANCODE_A])        g_current_scene->get_state().player->move_left();
    else if (key_state[SDL_SCANCODE_D])  g_current_scene->get_state().player->move_right();
    else if (key_state[SDL_SCANCODE_W])  g_current_scene->get_state().player->move_up();
    else if (key_state[SDL_SCANCODE_S])  g_current_scene->get_state().player->move_down();

     
    if (glm::length( g_current_scene->get_state().player->get_movement()) > 1.0f)
        g_current_scene->get_state().player->normalise_movement();
   
 
}

void update()
{
    // ————— DELTA TIME / FIXED TIME STEP CALCULATION ————— //
    float ticks = (float)SDL_GetTicks() / MILLISECONDS_IN_SECOND;
    float delta_time = ticks - g_previous_ticks;
    g_previous_ticks = ticks;
    
    delta_time += g_accumulator;
    
    if (delta_time < FIXED_TIMESTEP)
    {
        g_accumulator = delta_time;
        return;
    }
    
    while (delta_time >= FIXED_TIMESTEP) {
        // ————— UPDATING THE SCENE (i.e. map, character, enemies...) ————— //
        g_current_scene->update(FIXED_TIMESTEP);
        
        delta_time -= FIXED_TIMESTEP;
    }
    
    g_accumulator = delta_time;
    
    
    // ————— PLAYER CAMERA ————— //
    g_view_matrix = glm::mat4(1.0f);
    
    if (g_current_scene->get_state().player->get_position().x > 0|| g_current_scene->get_state().player->get_position().y < -4.0f) {
        g_view_matrix = glm::translate(g_view_matrix, glm::vec3(-g_current_scene->get_state().player->get_position().x, -g_current_scene->get_state().player->get_position().y, 0));
    } else {
        g_view_matrix = glm::translate(g_view_matrix, glm::vec3(-5, 4.0, 0));
    }


    if (g_current_scene == g_menu && g_current_scene->get_state().next_scene_id == 1) switch_to_scene(g_level_a);

    if (g_current_scene == g_level_a && g_current_scene->get_state().player->m_is_milk_collected)
    {
        Mix_PlayChannel(-1, g_current_scene->get_state().collect_sfx, 0);

        g_level_b->player_starting_pos = g_current_scene->get_state().player->get_position();

        switch_to_scene(g_level_b);
    }
    if (g_current_scene == g_level_b && g_current_scene->get_state().player->m_is_milk_collected)
    {
        Mix_PlayChannel(-1, g_current_scene->get_state().collect_sfx, 0);

        g_level_c->player_starting_pos = g_current_scene->get_state().player->get_position();
        switch_to_scene(g_level_c);
    }


    if (g_current_scene == g_level_c && g_current_scene->get_state().player->m_is_milk_collected && !g_current_scene->m_winning_sound_on)
    {
        Mix_PlayChannel(-1, g_current_scene->get_state().collect_sfx, 0);
        Mix_PlayChannel(-1, g_current_scene->get_state().winning_sfx, 0);

        g_current_scene->m_winning_sound_on = true;
    }



    if (g_current_scene->m_number_of_lives == 0 && !g_current_scene->m_losing_sound_on) {
        g_is_dead = 1;
        Mix_PlayChannel(-1, g_current_scene->get_state().losing_sfx, 0);
        g_current_scene->m_losing_sound_on = true;
    }

    g_shader_program.set_is_dead(g_is_dead);




}

void render()
{
    g_shader_program.set_view_matrix(g_view_matrix);

    glClear(GL_COLOR_BUFFER_BIT);

    // ————— RENDERING THE SCENE (i.e. map, character, enemies...) ————— //

    g_current_scene->render(&g_shader_program);

    if (g_current_scene->m_number_of_lives <= 0)
    {
        // Y
        Utility::draw_text(&g_shader_program, font_texture_id, "Y", 0.5f, 0.05f,
            glm::vec3(g_current_scene->get_state().player->get_position().x - 1.6f, -3.0f, 0.0f));

        // O
        Utility::draw_text(&g_shader_program, font_texture_id, "O", 0.5f, 0.05f,
            glm::vec3(g_current_scene->get_state().player->get_position().x - 1.1f, -3.0f, 0.0f));

        // U
        Utility::draw_text(&g_shader_program, font_texture_id, "U", 0.5f, 0.05f,
            glm::vec3(g_current_scene->get_state().player->get_position().x - 0.6f, -3.0f, 0.0f));

        // L (1.0 spacing between U and L)
        Utility::draw_text(&g_shader_program, font_texture_id, "L", 0.5f, 0.05f,
            glm::vec3(g_current_scene->get_state().player->get_position().x + 0.4f, -3.0f, 0.0f));

        // O
        Utility::draw_text(&g_shader_program, font_texture_id, "O", 0.5f, 0.05f,
            glm::vec3(g_current_scene->get_state().player->get_position().x + 0.9f, -3.0f, 0.0f));

        // S
        Utility::draw_text(&g_shader_program, font_texture_id, "S", 0.5f, 0.05f,
            glm::vec3(g_current_scene->get_state().player->get_position().x + 1.4f, -3.0f, 0.0f));

        // E
        Utility::draw_text(&g_shader_program, font_texture_id, "E", 0.5f, 0.05f,
            glm::vec3(g_current_scene->get_state().player->get_position().x + 1.9f, -3.0f, 0.0f));


    }



    SDL_GL_SwapWindow(g_display_window);
}


void shutdown()
{    
    SDL_Quit();
    
    // ————— DELETING LEVEL A DATA (i.e. map, character, enemies...) ————— //
    delete g_level_a;
}

// ————— GAME LOOP ————— //
int main(int argc, char* argv[])
{
    initialise();
    
    while (g_app_status == RUNNING)
    {
        process_input();
        update();
        render();
    }

    if (g_current_scene->get_state().next_scene_id > 0) switch_to_scene(g_levels[g_current_scene->get_state().next_scene_id]);

    
    shutdown();
    return 0;
}
