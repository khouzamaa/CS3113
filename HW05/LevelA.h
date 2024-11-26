#include "Scene.h"

class LevelA : public Scene {
public:

    // ————— STATIC ATTRIBUTES ————— //
    int ENEMY_COUNT = 1;
    
    // ————— DESTRUCTOR ————— //
    ~LevelA();
    
    // ————— METHODS ————— //
    void initialise() override;
    void update(float delta_time) override;
    void render(ShaderProgram *program) override;

    static void setLives(int newLives) { Scene::m_number_of_lives = newLives; }
    static int getLives() { return Scene::m_number_of_lives; }


};