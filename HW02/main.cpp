#define LOG(argument) std::cout << argument << '\n'
#define STB_IMAGE_IMPLEMENTATION
#define GL_SILENCE_DEPRECATION
#define GL_GLEXT_PROTOTYPES 1
#define NUMBER_OF_BALLS 3

#ifdef _WINDOWS
#include <GL/glew.h>
#endif

#include <SDL.h>
#include <SDL_opengl.h>
#include "glm/mat4x4.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "ShaderProgram.h"
#include "stb_image.h"
#include "Entity.h"
#include <vector>
#include <ctime>
#include "cmath"

// ————— STRUCTS AND ENUMS —————//
struct GameState
{
    Entity* player1;
    Entity* player2;
    Entity* bg;
    Entity** balls;
};

enum {LINEAR, NEAREST};

// ————— CONSTANTS ————— //
const int WINDOW_WIDTH = 640,
WINDOW_HEIGHT = 480;

constexpr float BG_RED = 0.9765625f,
BG_GREEN = 0.97265625f,
BG_BLUE = 0.9609375f,
BG_OPACITY = 1.0f;


const int VIEWPORT_X = 0,
VIEWPORT_Y = 0,
VIEWPORT_WIDTH = WINDOW_WIDTH,
VIEWPORT_HEIGHT = WINDOW_HEIGHT;


const char V_SHADER_PATH[] = "shaders/vertex_textured.glsl",
F_SHADER_PATH[] = "shaders/fragment_textured.glsl";

const float MILLISECONDS_IN_SECOND = 1000.0;

const char KNIGHT_FILEPATH[] = "assets/knight1.png",
KNIGHT2_FILEPATH[] = "assets/knight2.png",
BALL_FILEPATH[] = "assets/dragon3.png",
BG_FILEPATH[] = "assets/temple.png",
FONTSHEET_FILEPATH[] = "assets/font1.png";



// ----- SCALE ------ //
constexpr glm::vec3 BG_INITIAL_SCALE(10.0f, 7.5f, 0.0f);
const int NUMBER_OF_TEXTURES = 1;
const GLint LEVEL_OF_DETAIL = 0;
const GLint TEXTURE_BORDER = 0;

// ————— VARIABLES ————— //
GameState g_game_state;

SDL_Window* g_display_window;
bool g_game_is_running = true;

ShaderProgram g_shader_program;
glm::mat4 g_view_matrix, g_projection_matrix;

float g_previous_ticks = 0.0f;

int current_ball_num = 1;

GLuint g_font_texture_id;



// ———— GENERAL FUNCTIONS ———— //
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

void initialise()
{
    SDL_Init(SDL_INIT_VIDEO);
    g_display_window = SDL_CreateWindow("PONG!",
        SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
        WINDOW_WIDTH, WINDOW_HEIGHT,
        SDL_WINDOW_OPENGL);

    SDL_GLContext context = SDL_GL_CreateContext(g_display_window);
    SDL_GL_MakeCurrent(g_display_window, context);

#ifdef _WINDOWS
    glewInit();
#endif

    glViewport(VIEWPORT_X, VIEWPORT_Y, VIEWPORT_WIDTH, VIEWPORT_HEIGHT);

    g_shader_program.load(V_SHADER_PATH, F_SHADER_PATH);

    g_view_matrix = glm::mat4(1.0f);
    g_projection_matrix = glm::ortho(-5.0f, 5.0f, -3.75f, 3.75f, -1.0f, 1.0f);

    g_shader_program.set_projection_matrix(g_projection_matrix);
    g_shader_program.set_view_matrix(g_view_matrix);

    glUseProgram(g_shader_program.get_program_id());

    //----Font Texture----//
    g_font_texture_id = load_texture(FONTSHEET_FILEPATH);

    // ----- BACKGROUND ---- //
    g_game_state.bg = new Entity(load_texture(BG_FILEPATH), BG_INITIAL_SCALE);

    
    // ————— PLAYERS ————— //
    g_game_state.player1 = new Entity();
    g_game_state.player1->set_position(glm::vec3(3.0f, 0.0f, 0.0f));
    g_game_state.player1->set_movement(glm::vec3(0.0f));
    g_game_state.player1->set_speed(1.75f);
    g_game_state.player1->set_texture_id(load_texture(KNIGHT_FILEPATH));


    g_game_state.player2 = new Entity();
    g_game_state.player2->set_position(glm::vec3(-3.0f, 0.0f, 0.0f));
    g_game_state.player2->set_movement(glm::vec3(0.0f));
    g_game_state.player2->set_speed(1.75f);
    g_game_state.player2->set_texture_id(load_texture(KNIGHT2_FILEPATH));


    //----- BALLS -----//
    g_game_state.balls = new Entity * [3];
    GLuint ball_texture_id = load_texture(BALL_FILEPATH);

    for (int i = 0; i < NUMBER_OF_BALLS; i++) {
        g_game_state.balls[i] = new Entity();
        g_game_state.balls[i]->set_position(glm::vec3(0.0f, 0.0f, 0.0f));
        g_game_state.balls[i]->set_movement(glm::vec3(0.25f, 0.1f, 0.0f));
        g_game_state.balls[i]->set_speed(3.0f);
        g_game_state.balls[i]->set_texture_id(load_texture(BALL_FILEPATH));

    }


    glClearColor(BG_RED, BG_BLUE, BG_GREEN, BG_OPACITY);



    // ————— GENERAL ————— //
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
}

constexpr int FONTBANK_SIZE = 16;

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

void process_input()
{
    // VERY IMPORTANT: If nothing is pressed, we don't want to go anywhere
    g_game_state.player1->set_movement(glm::vec3(0.0f));
    if (g_game_state.player2->get_mode())
        g_game_state.player2->set_movement(glm::vec3(0.0f));


    SDL_Event event;
    while (SDL_PollEvent(&event))
    {
        switch (event.type) {
        case SDL_QUIT:
        case SDL_WINDOWEVENT_CLOSE:
            g_game_is_running = false;
            break;

        case SDL_KEYDOWN:
            switch (event.key.keysym.sym) {
            case SDLK_q: g_game_is_running = false;

            //Change to single player game mode
            case SDLK_t: {
                g_game_state.player2->set_mode();
                g_game_state.player2->set_movement(glm::vec3(0.0f, 1.5f, 0.0f));
            }

            default:     break;
            }

        default:
            break;
        }
    }

    const Uint8* key_state = SDL_GetKeyboardState(NULL);

    //check if no one won/lost
    if (g_game_state.player1->get_game_status() && g_game_state.player2->get_game_status())
        if (key_state[SDL_SCANCODE_UP])
        {
            g_game_state.player1->move_up();
        }
        else if (key_state[SDL_SCANCODE_DOWN])
        {
            g_game_state.player1->move_down();
        }

    if (g_game_state.player1->get_game_status() && g_game_state.player2->get_game_status() && g_game_state.player2->get_mode())
        if (key_state[SDL_SCANCODE_W])
        {
            g_game_state.player2->move_up();
        }
        else if (key_state[SDL_SCANCODE_S])
        {
            g_game_state.player2->move_down();
        }


    //change the balls number
    if (key_state[SDL_SCANCODE_1])
    {
        current_ball_num = 1;
    }

    if (key_state[SDL_SCANCODE_2])
    {
        current_ball_num = 2;
    }

    if  (key_state[SDL_SCANCODE_3])
        current_ball_num = 3;


}

void update()
{
    float ticks = (float)SDL_GetTicks() / MILLISECONDS_IN_SECOND;
    float delta_time = ticks - g_previous_ticks;
    g_previous_ticks = ticks;
    g_game_state.bg->update(delta_time);

    g_game_state.player1->update(g_game_state.player2, g_game_state.bg, g_game_state.balls, current_ball_num, delta_time, 3.4f, 5.0f);




}

void render()
{
    glClear(GL_COLOR_BUFFER_BIT);

    g_game_state.bg->render(&g_shader_program);
    g_game_state.player1->render(&g_shader_program);
    g_game_state.player2->render(&g_shader_program);

    for (int i = 0; i < current_ball_num; i++) {
        g_game_state.balls[i]->render(&g_shader_program);
    }

    if (!g_game_state.player2->get_game_status()) {
        draw_text(&g_shader_program, g_font_texture_id, "PLAYER 1 WINS!", 0.5f, 0.05f,
            glm::vec3(-3.5f, 2.0f, 0.0f));
    }

    if (!g_game_state.player1->get_game_status()){
        draw_text(&g_shader_program, g_font_texture_id, "PLAYER 2 WINS!", 0.5f, 0.05f,
            glm::vec3(-3.5f, 2.0f, 0.0f));
    }



    SDL_GL_SwapWindow(g_display_window);
}

void shutdown() { SDL_Quit(); }

// ————— DRIVER GAME LOOP ————— /
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