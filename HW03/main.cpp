/**
* Author: Khouzama Bin Jardan
* Assignment: Lunar Lander
* Date due: 2024-10-27, 11:59pm
* I pledge that I have completed this assignment without
* collaborating with anyone else, in conformance with the
* NYU School of Engineering Policies and Procedures on
* Academic Misconduct.
**/


#define GL_SILENCE_DEPRECATION
#define STB_IMAGE_IMPLEMENTATION
#define LOG(argument) std::cout << argument << '\n'
#define GL_GLEXT_PROTOTYPES 1
#define FIXED_TIMESTEP 0.0166666f
#define GROUND_COUNT 4
#define WATER_COUNT 2
#define FONTBANK_SIZE 16

#ifdef _WINDOWS
#include <GL/glew.h>
#endif

#include <SDL.h>
#include <SDL_opengl.h>
#include "glm/mat4x4.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "ShaderProgram.h"
#include "stb_image.h"
#include "cmath"
#include <ctime>
#include <vector>
#include "Entity.h"

// ––––– STRUCTS AND ENUMS ––––– //
struct GameState
{
    Entity* player;
    Entity* ground;
    Entity* water;
    Entity* bg1;
    Entity* bg2;
    Entity* bg3;
    Entity* bg4;

};

// ––––– CONSTANTS ––––– //
const int WINDOW_WIDTH = 640,
WINDOW_HEIGHT = 480;

const float BG_RED = 0.1922f,
BG_BLUE = 0.549f,
BG_GREEN = 0.9059f,
BG_OPACITY = 1.0f;

const int VIEWPORT_X = 0,
VIEWPORT_Y = 0,
VIEWPORT_WIDTH = WINDOW_WIDTH,
VIEWPORT_HEIGHT = WINDOW_HEIGHT;

const char V_SHADER_PATH[] = "shaders/vertex_textured.glsl",
F_SHADER_PATH[] = "shaders/fragment_textured.glsl";

const float MILLISECONDS_IN_SECOND = 1000.0;
const char SPRITESHEET_FILEPATH[] = "assets/Charakter.png";
const char PLATFORM_FILEPATH[] = "assets/grass.png";
const char WATER_FILEPATH[] = "assets/water.png";
const char FONT_FILEPATH[] = "assets/font1.png";
const char BG_LAYER1[] = "assets/Background/bg_layer1.png";
const char BG_LAYER2[] = "assets/Background/bg_layer2.png";
const char BG_LAYER3[] = "assets/Background/bg_layer3.png";
const char BG_LAYER4[] = "assets/Background/bg_layer4.png";





const int NUMBER_OF_TEXTURES = 1;
const GLint LEVEL_OF_DETAIL = 0;
const GLint TEXTURE_BORDER = 0;
GLuint font_texture_id;


// ––––– GLOBAL VARIABLES ––––– //
GameState g_state;

SDL_Window* g_display_window;
bool g_game_is_running = true;

ShaderProgram g_program;
glm::mat4 g_view_matrix, g_projection_matrix;

float g_previous_ticks = 0.0f;
float g_accumulator = 0.0f;
bool g_starting = true;

// ––––– GENERAL FUNCTIONS ––––– //
GLuint load_texture(const char* filepath)
{
    int width, height, number_of_components;
    unsigned char* image = stbi_load(filepath, &width, &height, &number_of_components, STBI_rgb_alpha);

    if (image == NULL)
    {
        LOG("Unable to load image. Make sure the path is correct.");
        assert(false);
    }

    GLuint textureID;
    glGenTextures(NUMBER_OF_TEXTURES, &textureID);
    glBindTexture(GL_TEXTURE_2D, textureID);
    glTexImage2D(GL_TEXTURE_2D, LEVEL_OF_DETAIL, GL_RGBA, width, height, TEXTURE_BORDER, GL_RGBA, GL_UNSIGNED_BYTE, image);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);


    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

    stbi_image_free(image);

    return textureID;
}


void draw_text(ShaderProgram* program, GLuint font_texture_id, std::string text,
    float font_size, float spacing, glm::vec3 position)
{
    // Scale the size of the fontbank in the UV-plane
    // We will use this for spacing and positioning
    float width = 1.0f / FONTBANK_SIZE;
    float height = 1.0f / FONTBANK_SIZE;

    // Instead of having a single pair of arrays, we'll have a series of pairs—one for
    // each character. Don't forget to include <vector>!
    std::vector<float> vertices;
    std::vector<float> texture_coordinates;

    // For every character...
    for (int i = 0; i < text.size(); i++) {
        // 1. Get their index in the spritesheet, as well as their offset (i.e. their
        //    position relative to the whole sentence)
        int spritesheet_index = (int)text[i];  // ascii value of character
        float offset = (font_size + spacing) * i;

        // 2. Using the spritesheet index, we can calculate our U- and V-coordinates
        float u_coordinate = (float)(spritesheet_index % FONTBANK_SIZE) / FONTBANK_SIZE;
        float v_coordinate = (float)(spritesheet_index / FONTBANK_SIZE) / FONTBANK_SIZE;

        // 3. Inset the current pair in both vectors
        vertices.insert(vertices.end(), {
            offset + (-0.5f * font_size), 0.5f * font_size,
            offset + (-0.5f * font_size), -0.5f * font_size,
            offset + (0.5f * font_size), 0.5f * font_size,
            offset + (0.5f * font_size), -0.5f * font_size,
            offset + (0.5f * font_size), 0.5f * font_size,
            offset + (-0.5f * font_size), -0.5f * font_size,
            });

        texture_coordinates.insert(texture_coordinates.end(), {
            u_coordinate, v_coordinate,
            u_coordinate, v_coordinate + height,
            u_coordinate + width, v_coordinate,
            u_coordinate + width, v_coordinate + height,
            u_coordinate + width, v_coordinate,
            u_coordinate, v_coordinate + height,
            });
    }

    // 4. And render all of them using the pairs
    glm::mat4 model_matrix = glm::mat4(1.0f);
    model_matrix = glm::translate(model_matrix, position);

    program->set_model_matrix(model_matrix);
    glUseProgram(program->get_program_id());

    glVertexAttribPointer(program->get_position_attribute(), 2, GL_FLOAT, false, 0,
        vertices.data());
    glEnableVertexAttribArray(program->get_position_attribute());
    glVertexAttribPointer(program->get_tex_coordinate_attribute(), 2, GL_FLOAT, false, 0,
        texture_coordinates.data());
    glEnableVertexAttribArray(program->get_tex_coordinate_attribute());

    glBindTexture(GL_TEXTURE_2D, font_texture_id);
    glDrawArrays(GL_TRIANGLES, 0, (int)(text.size() * 6));

    glDisableVertexAttribArray(program->get_position_attribute());
    glDisableVertexAttribArray(program->get_tex_coordinate_attribute());
}

void initialise()
{
    SDL_Init(SDL_INIT_VIDEO);
    g_display_window = SDL_CreateWindow("LAND!",
        SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
        WINDOW_WIDTH, WINDOW_HEIGHT,
        SDL_WINDOW_OPENGL);

    SDL_GLContext context = SDL_GL_CreateContext(g_display_window);
    SDL_GL_MakeCurrent(g_display_window, context);

#ifdef _WINDOWS
    glewInit();
#endif

    glViewport(VIEWPORT_X, VIEWPORT_Y, VIEWPORT_WIDTH, VIEWPORT_HEIGHT);

    g_program.load(V_SHADER_PATH, F_SHADER_PATH);

    g_view_matrix = glm::mat4(1.0f);
    g_projection_matrix = glm::ortho(-5.0f, 5.0f, -3.75f, 3.75f, -1.0f, 1.0f);

    g_program.set_projection_matrix(g_projection_matrix);
    g_program.set_view_matrix(g_view_matrix);
    
    glUseProgram(g_program.get_program_id());

    glClearColor(BG_RED, BG_BLUE, BG_GREEN, BG_OPACITY);

    // ––––– ground ––––– //
    GLuint ground_texture_id = load_texture(PLATFORM_FILEPATH);

    g_state.ground = new Entity[GROUND_COUNT];

    g_state.ground[0].m_texture_id = ground_texture_id;
    g_state.ground[0].set_position(glm::vec3(0.0f, -2.0f, 0.0f));
    g_state.ground[0].set_scale(glm::vec3(1.5f, 0.6f, 0.0f));
    g_state.ground[0].update(0.0f, NULL, 0, NULL, 0);

    g_state.ground[1].m_texture_id = ground_texture_id;
    g_state.ground[1].set_position(glm::vec3(1.2f, 0.2f, 0.0f));
    g_state.ground[1].set_scale(glm::vec3(1.5f, 0.6f, 0.0f));
    g_state.ground[1].update(0.0f, NULL, 0, NULL, 0);

    g_state.ground[GROUND_COUNT - 1].m_texture_id = ground_texture_id;
    g_state.ground[GROUND_COUNT - 1].set_position(glm::vec3(-3.0f, -2.0f, 0.0f));
    g_state.ground[GROUND_COUNT - 1].set_scale(glm::vec3(1.5f, 0.6f, 0.0f));
    g_state.ground[GROUND_COUNT - 1].update(0.0f, NULL, 0, NULL, 0);

    g_state.ground[GROUND_COUNT - 2].m_texture_id = ground_texture_id;
    g_state.ground[GROUND_COUNT - 2].set_position(glm::vec3(3.5f, -1.0f, 0.0f));
    g_state.ground[GROUND_COUNT - 2].set_scale(glm::vec3(1.5f, 0.6f, 0.0f));;
    g_state.ground[GROUND_COUNT - 2].update(0.0f, NULL, 0, NULL, 0);


    //-----WATER----//
    GLuint water_texture_id = load_texture(WATER_FILEPATH);
    g_state.water = new Entity[WATER_COUNT];

    g_state.water[0].m_texture_id = water_texture_id;
    g_state.water[0].set_position(glm::vec3(-1.5f, -2.0f, 0.0f));
    g_state.water[0].set_scale(glm::vec3(1.5f, 0.6f, 0.0f));;
    g_state.water[0].update(0.0f, NULL, 0, NULL, 0);

    g_state.water[1].m_texture_id = water_texture_id;
    g_state.water[1].set_position(glm::vec3(2.25f, -1.0f, 0.0f));
    g_state.water[1].set_scale(glm::vec3(1.5f, 0.6f, 0.0f));;
    g_state.water[1].update(0.0f, NULL, 0, NULL,0);





    // ––––– PLAYER ––––– //
    // Existing
    g_state.player = new Entity();
    g_state.player->set_position(glm::vec3(-3.0f,2.5f,0.0f));
    g_state.player->set_movement(glm::vec3(0.0f));
    g_state.player->set_scale(glm::vec3(1.5f, 1.5f, 0.0f));
    g_state.player->m_speed = 1.0f;
    g_state.player->set_acceleration(glm::vec3(0.0f, -0.025f, 0.0f));
    g_state.player->m_texture_id = load_texture(SPRITESHEET_FILEPATH);

    // Walking
    g_state.player->m_walking[g_state.player->LEFT] = new int[4] { 8, 9, 10, 11 };
    g_state.player->m_walking[g_state.player->RIGHT] = new int[4] { 12, 13, 14, 15 };
    g_state.player->m_walking[g_state.player->UP] = new int[4] { 0, 1, 2, 3 };
    g_state.player->m_walking[g_state.player->DOWN] = new int[4] { 1, 5, 9, 13 };


    g_state.player->m_animation_indices = g_state.player->m_walking[g_state.player->UP];  // start George looking up
    g_state.player->m_animation_frames = 4;
    g_state.player->m_animation_index = 0;
    g_state.player->m_animation_time = 0.0f;
    g_state.player->m_animation_cols = 4;
    g_state.player->m_animation_rows = 4;
    g_state.player->set_height(0.9f);
    g_state.player->set_width(0.9f);

    // Jumping
    g_state.player->m_jumping_power = 0.015f;

    //FONT
    font_texture_id  = load_texture(FONT_FILEPATH);

    //BG
    GLuint bg1 = load_texture(BG_LAYER1);
    GLuint bg2 = load_texture(BG_LAYER2);
    GLuint bg3 = load_texture(BG_LAYER3);
    GLuint bg4 = load_texture(BG_LAYER4);
    g_state.bg1 = new Entity();
    g_state.bg1->m_texture_id = load_texture(BG_LAYER1);
    g_state.bg1->set_scale(glm::vec3(10.0f, 10.0f, 0.0f));


    g_state.bg2 = new Entity();
    g_state.bg2->m_texture_id = load_texture(BG_LAYER2);
    g_state.bg2->set_scale(glm::vec3(10.0f, 10.0f, 0.0f));


    g_state.bg3 = new Entity();
    g_state.bg3->m_texture_id = load_texture(BG_LAYER3);
    g_state.bg3->set_scale(glm::vec3(10.0f, 10.0f, 0.0f));


    g_state.bg4 = new Entity();
    g_state.bg4->m_texture_id = load_texture(BG_LAYER4);
    g_state.bg4->set_scale(glm::vec3(10.0f, 10.0f, 0.0f));



    // ––––– GENERAL ––––– //
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
}

void process_input()
{
    g_state.player->set_movement(glm::vec3(0.0f));

    SDL_Event event;
    while (SDL_PollEvent(&event))
    {
        switch (event.type) {
            // End game
        case SDL_QUIT:
        case SDL_WINDOWEVENT_CLOSE:
            g_game_is_running = false;
            break;

        case SDL_KEYDOWN:
            g_starting = false;
            break;
            switch (event.key.keysym.sym) {
            case SDLK_q:
                // Quit the game with a keystroke
                g_game_is_running = false;
                break;

            case SDLK_SPACE:
                // Jump
                //if (g_state.player->m_collided_bottom) g_state.player->m_is_jumping = true;
                //break;

            default:
                break;
            }

        default:
            break;
        }
    }

    const Uint8* key_state = SDL_GetKeyboardState(NULL);
    
    if (key_state[SDL_SCANCODE_UP]) {
        if (g_state.player->get_fuel() > 0)
        {
            g_state.player->m_is_jumping = true;
            g_state.player->m_animation_indices = g_state.player->m_walking[g_state.player->UP];
            g_state.player->fuel -= 0.01;
        }
        
    }
    else {
        g_state.player->m_animation_indices = g_state.player->m_walking[g_state.player->DOWN];
        g_state.player->m_acceleration.y = -0.05f;
    }

    if (key_state[SDL_SCANCODE_LEFT])
    {
        if (g_state.player->get_fuel() > 0) 
        {
            g_state.player->m_acceleration.x = -10.0f;
            g_state.player->m_acceleration.y = -0.05f;
            g_state.player->m_animation_indices = g_state.player->m_walking[g_state.player->LEFT];
            g_state.player->fuel -= 0.01;
        }
    }
    else if (key_state[SDL_SCANCODE_RIGHT])
    {
        if (g_state.player->get_fuel() > 0)
        {
            g_state.player->m_acceleration.x = 10.0f;
            g_state.player->m_acceleration.y = -0.05f;
            g_state.player->m_animation_indices = g_state.player->m_walking[g_state.player->RIGHT];
            g_state.player->fuel -= 0.01;
        }
    }
    else {
        g_state.player->m_animation_indices = g_state.player->m_walking[g_state.player->DOWN];
        g_state.player->m_acceleration.y = -0.05f;
    }



    if (glm::length(g_state.player->m_movement) > 1.0f)
    {
        g_state.player->m_movement = glm::normalize(g_state.player->m_movement);
    }
}

void update()
{
    float ticks = (float)SDL_GetTicks() / MILLISECONDS_IN_SECOND;
    float delta_time = ticks - g_previous_ticks;
    g_previous_ticks = ticks;

    delta_time += g_accumulator;

    if (delta_time < FIXED_TIMESTEP)
    {
        g_accumulator = delta_time;
        return;
    }

    while (delta_time >= FIXED_TIMESTEP)
    {
        g_state.player->update(FIXED_TIMESTEP, g_state.ground, GROUND_COUNT, g_state.water, WATER_COUNT);
        delta_time -= FIXED_TIMESTEP;
    }

    g_accumulator = delta_time;
    g_state.bg1->update(delta_time, NULL, 0, NULL, 0);
    g_state.bg2->update(delta_time, NULL, 0, NULL, 0);
    g_state.bg3->update(delta_time, NULL, 0, NULL, 0);
    g_state.bg4->update(delta_time, NULL, 0, NULL, 0);


}

void render()
{
    glClear(GL_COLOR_BUFFER_BIT);
    g_state.bg1->render(&g_program);
    g_state.bg2->render(&g_program);
    g_state.bg3->render(&g_program);
    g_state.bg4->render(&g_program);




    if(g_starting)
        draw_text(&g_program, font_texture_id, "LAND ON WATER!", 0.5f, 0.05f,
            glm::vec3(-3.5f, 2.0f, 0.0f));

    g_state.player->render(&g_program);

    for (int i = 0; i < GROUND_COUNT; i++) g_state.ground[i].render(&g_program);

    for (int i = 0; i < WATER_COUNT; i++) g_state.water[i].render(&g_program);

    
    if(!g_state.player->get_losig_status() && g_state.player->get_winning_status())
        draw_text(&g_program, font_texture_id, "YOU LANDED SAFELY!", 0.5f, 0.05f,
            glm::vec3(-4.5f, 2.0f, 0.0f));

    if (g_state.player->get_losig_status() && !g_state.player->get_winning_status())
        draw_text(&g_program, font_texture_id, "DIED FROM IMPACT!", 0.5f, 0.05f,
            glm::vec3(-4.3f, 2.0f, 0.0f));

    std::string fuelMeter = "FUEL: " + std::to_string((int)g_state.player->fuel);

    draw_text(&g_program, font_texture_id, fuelMeter, 0.3f, 0.005f,
        glm::vec3(-4.75f, 3.5f, 0.0f));

    SDL_GL_SwapWindow(g_display_window);
}

void shutdown()
{
    SDL_Quit();

    delete[] g_state.ground;
    delete[] g_state.water;
    delete g_state.player;
}

// ––––– GAME LOOP ––––– //
int main(int argc, char* argv[])
{
    initialise();

    while (g_game_is_running)
    {
        process_input();
        update();
        render();
    }

    shutdown();
    return 0;
}