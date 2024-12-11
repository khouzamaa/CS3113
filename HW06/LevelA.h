#include "Scene.h"
#define BULLET_COUNT 10  // Define the number of bullets available in this level

class LevelA : public Scene {
public:
    Entity* bullets[BULLET_COUNT];      // Array to store bullet entities
    bool bullet_active[BULLET_COUNT];  // Array to track active bullets
    // ————— STATIC ATTRIBUTES ————— //
    int ENEMY_COUNT = 7;
    
    // ————— DESTRUCTOR ————— //
    ~LevelA();
    
    // ————— METHODS ————— //
    void initialise() override;
    void update(float delta_time) override;
    void render(ShaderProgram *program) override;

    static void setLives(int newLives) { Scene::m_number_of_lives = newLives; }
    static int getLives() { return Scene::m_number_of_lives; }


};
