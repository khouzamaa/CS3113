/**
* Author: Khouzama Bin Jardan
* Assignment: Platformer
* Date due: 2023-11-26, 11:59pm
* I pledge that I have completed this assignment without
* collaborating with anyone else, in conformance with the
* NYU School of Engineering Policies and Procedures on
* Academic Misconduct.
**/


#include "Menu.h"
#include "Utility.h"

#define LEVEL_WIDTH 17
#define LEVEL_HEIGHT 8


constexpr char SPRITESHEET_FILEPATH[] = "assets/images/Charakter.png",
PLATFORM_FILEPATH[] = "assets/images/platformPack_tilesheet.png",
FONT_FILEPATH[] = "assets/images/font1.png";

unsigned int MENU_DATA[] =
{
    3, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,0,0,0,
    3, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,0,0,0,
    3, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,0,0,0,
    3, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,0,0,0,
    3, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,0,0,0,
    3, 0, 0, 0, 0, 0, 0, 0, 0, 2, 2, 2, 2, 2,0,0,6,
    3, 2, 2, 2, 2, 2, 2, 2, 2, 3, 3, 3, 3, 3,0,0,0,
    3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3,0,0,0
};


Menu::~Menu()
{
    delete    m_game_state.player;
    delete    m_game_state.map;
    Mix_FreeChunk(m_game_state.jump_sfx);
    Mix_FreeMusic(m_game_state.bgm);
}

void Menu::initialise()
{
    font_texture_id = Utility::load_texture(FONT_FILEPATH);


    GLuint map_texture_id = Utility::load_texture("assets/images/platformPack_tilesheet.png");
    m_game_state.map = new Map(LEVEL_WIDTH, LEVEL_HEIGHT, MENU_DATA, map_texture_id, 1.0f, 14, 7);

    GLuint player_texture_id = Utility::load_texture(SPRITESHEET_FILEPATH);

    int player_walking_animation[4][4] =
    {
        { 8, 9, 10, 11 },  // for George to move to the left,
        { 12, 13, 14, 15 }, // for George to move to the right,
        { 0, 1, 2, 3 }, // for George to move upwards,
        { 4, 5, 6, 7 }   // for George to move downwards
    };

    glm::vec3 acceleration = glm::vec3(0.0f, -4.81f, 0.0f);

    m_game_state.player = new Entity(
        player_texture_id,         // texture id
        4.0f,                      // speedf
        acceleration,              // acceleration
        5.0f,                      // jumping power
        player_walking_animation,  // animation index sets
        0.0f,                      // animation time
        4,                         // animation frame amount
        0,                         // current animation index
        4,                         // animation column amount
        4,                         // animation row amount
        0.50f,                      // width
        0.50f,                       // height
        PLAYER                      //Entity Type
    );

    m_game_state.player->set_scale(glm::vec3(0.0f, 0.0f, 0.0f));
    /**
     BGM and SFX
     */
    Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 4096);

    m_game_state.bgm = Mix_LoadMUS("assets/sound/dooblydoo.mp3");
    Mix_PlayMusic(m_game_state.bgm, -1);
    Mix_VolumeMusic(100.0f);

    m_game_state.jump_sfx = Mix_LoadWAV("assets/sound/bounce.wav");
}

void Menu::update(float delta_time)
{
    m_game_state.player->update(delta_time, m_game_state.player, NULL, 0, m_game_state.map);

    if (start) m_game_state.next_scene_id = 1;
}


void Menu::render(ShaderProgram* g_shader_program)
{

    m_game_state.map->render(g_shader_program);
    m_game_state.player->render(g_shader_program);

    Utility::draw_text(g_shader_program, font_texture_id, "HELP MOCHI THE KITTEN  ", 0.4f, 0.04f,
        glm::vec3(0.8f, -1.25f, 0.0f));

    Utility::draw_text(g_shader_program, font_texture_id, "COLLECT ALL THE ", 0.4f, 0.04f,
        glm::vec3(2.0f, -2.0f, 0.0f));

    Utility::draw_text(g_shader_program, font_texture_id, "MILK BOTTELS", 0.4f, 0.04f,
        glm::vec3(2.5f, -2.75f, 0.0f));

    Utility::draw_text(g_shader_program, font_texture_id, "use WASD to move", 0.3f, 0.03f,
            glm::vec3(2.75f, -4.0f, 0.0f));

    Utility::draw_text(g_shader_program, font_texture_id, "and F to shoot", 0.3f, 0.03f,
        glm::vec3(3.0f, -4.5f, 0.0f));

    Utility::draw_text(g_shader_program, font_texture_id, "press enter to begin", 0.35f, 0.035f,
        glm::vec3(1.25f, -6.5f, 0.0f));



}
