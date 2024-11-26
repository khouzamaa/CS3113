/**
* Author: Khouzama Bin Jardan
* Assignment: Platformer
* Date due: 2023-11-26, 11:59pm
* I pledge that I have completed this assignment without
* collaborating with anyone else, in conformance with the
* NYU School of Engineering Policies and Procedures on
* Academic Misconduct.
**/


#include "LevelB.h"
#include "Utility.h"

#define LEVEL_WIDTH 18
#define LEVEL_HEIGHT 8

constexpr char SPRITESHEET_FILEPATH[] = "assets/images/Charakter.png",
PLATFORM_FILEPATH[] = "assets/images/platformPack_tilesheet.png",
ENEMY_FILEPATH[] = "assets/images/mace.png",
FONT_FILEPATH[] = "assets/images/font1.png";


//CHANGE AI TYPE

unsigned int LEVELB_DATA[] =
{
    3, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    3, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0,
    3, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 2, 3, 0, 0, 6, 0,
    3, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 2, 3, 3, 0, 0, 0, 0,
    3, 0, 0, 0, 0, 0, 0, 0, 0, 0, 2, 3, 3, 3, 0, 0, 0, 0,
    3, 0, 0, 0, 0, 0, 0, 0, 0, 2, 3, 3, 3, 3, 0, 0, 0, 0,
    3, 2, 2, 2, 2, 2, 2, 0, 2, 3, 3, 3, 3, 3, 0, 0, 0, 0,
    3, 3, 3, 3, 3, 3, 3, 0, 3, 3, 3, 3, 3, 3, 0, 0, 0, 0,
};


LevelB::~LevelB()
{
    delete[] m_game_state.enemies;
    delete    m_game_state.player;
    delete    m_game_state.map;
    Mix_FreeChunk(m_game_state.jump_sfx);
    Mix_FreeMusic(m_game_state.bgm);
}

void LevelB::initialise()
{

    font_texture_id = Utility::load_texture(FONT_FILEPATH);

    GLuint map_texture_id = Utility::load_texture("assets/images/platformPack_tilesheet.png");
    m_game_state.map = new Map(LEVEL_WIDTH, LEVEL_HEIGHT, LEVELB_DATA, map_texture_id, 1.0f, 14, 7);

    GLuint player_texture_id = Utility::load_texture(SPRITESHEET_FILEPATH);

    int player_walking_animation[4][4] =
    {
        { 8, 9, 10, 11 },  // for George to move to the left,
        { 12, 13, 14, 15 }, // for George to move to the right,
        { 0, 1, 2, 3 }, // for George to move upwards,
        { 1, 5, 9, 13 }   // for George to move downwards
    };

    glm::vec3 acceleration = glm::vec3(0.0f, -4.81f, 0.0f);

    m_game_state.player = new Entity(
        player_texture_id,         // texture id
        4.0f,                      // speed
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

    m_game_state.player->set_position(glm::vec3(5.0f, 0.0f, 0.0f));
    m_game_state.player->set_scale(glm::vec3(1.5f, 1.5f, 0.0f));
    m_game_state.player->number_of_lives = getLives();




    /**
     Enemies' stuff */
    GLuint enemy_texture_id = Utility::load_texture(ENEMY_FILEPATH);

    m_game_state.enemies = new Entity[ENEMY_COUNT];

    for (int i = 0; i < ENEMY_COUNT; i++)
    {
        m_game_state.enemies[i] = Entity(enemy_texture_id, 1.0f, 0.75f, 0.75f, ENEMY, WALKER, IDLE);

    }
    m_game_state.enemies[0].set_scale(glm::vec3(0.85f, 0.85f, 0.0f));
    m_game_state.enemies[0].set_position(glm::vec3(8.0f, -2.0f, 0.0f));
    m_game_state.enemies[0].set_movement(glm::vec3(0.0f));
    m_game_state.enemies[0].set_acceleration(glm::vec3(0.0f, 0.0f, 0.0f));

    /**
     BGM and SFX
     */
    Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 4096);

    m_game_state.bgm = Mix_LoadMUS("assets/sound/dooblydoo.mp3");
    Mix_PlayMusic(m_game_state.bgm, -1);
    Mix_VolumeMusic(100.0f);

    m_game_state.jump_sfx = Mix_LoadWAV("assets / sound / bounce.wav");
}

void LevelB::update(float delta_time)
{
    m_game_state.player->update(delta_time, m_game_state.player, NULL, 0, m_game_state.map);

    for (int i = 0; i < ENEMY_COUNT; i++)
    {
        m_game_state.enemies[i].update(delta_time, m_game_state.player, NULL, 0, m_game_state.map);
    }

    if (m_game_state.player->get_position().x >= 16.0f) 
    {
        m_game_state.next_scene_id = 1;
        m_game_state.player->level_finished();
    }

    setLives(m_game_state.player->number_of_lives);

}


void LevelB::render(ShaderProgram* g_shader_program)
{
    m_game_state.map->render(g_shader_program);
    m_game_state.player->render(g_shader_program);
    for (int i = 0; i < m_number_of_enemies; i++)
        m_game_state.enemies[i].render(g_shader_program);

    Utility::draw_text(g_shader_program, font_texture_id, "LIVES: " + std::to_string(Scene::m_number_of_lives), 0.3f, 0.03f,
        glm::vec3(m_game_state.player->get_position().x - 1.3, m_game_state.player->get_position().y + 1.0f, 0.0f));
}
