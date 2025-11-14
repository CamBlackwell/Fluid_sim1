#include <SFML/Graphics.hpp>
#include <vector>
#include <cstdlib>
#include <ctime>
#include <cmath>

struct Ball {
    sf::CircleShape shape;
    float x_vol = 3.f;
    float y_vol = 3.f;
};

std::vector<Ball> makeBalls(int num){
    std::vector<Ball> balls;
    std::srand(1);
    for (int i = 0; i < num; i++){
        Ball ball;
        ball.shape.setRadius(6.f);
        
        ball.shape.setPosition({std::rand() % 700 + 50.f, std::rand() % 500 + 50.f});
        //ball.shape.setFillColor({sf::Color(std::rand() % 256, std::rand() % 256, std::rand() % 256)});
        ball.shape.setFillColor(sf::Color{0, 0, std::rand() % 256});

        balls.push_back(ball);
    }
    return balls;
}

int main(){
    const unsigned int window_width = 800;
    const unsigned int window_height = 600;
    int SLOW_DOWN_VALUE = 1.f;
    float GRAVITY = 1.5;
    float wall_absoribtion = 0.8;
    sf::RenderWindow window(sf::VideoMode({window_width, window_height}), "HAHA hello :)"); //create main window
    window.setVerticalSyncEnabled(true); //applications refresh rate to sync with monitors refresh rate
    //window.setFramerateLimit(60); //set framerate to 60 DONT MIX WITH setVerticalSync!!!!!!!

    const int CELL_SIZE = 100; //100 x 100 pixels
    const int GRID_WIDTH = window_width / CELL_SIZE;
    const int GRID_HEIGHT = window_height / CELL_SIZE;

    float scalar = 10.f;
    std::vector<Ball> balls = makeBalls(2050);

    
    while (window.isOpen()){
        while (const std::optional event = window.pollEvent()){
            if (event->is<sf::Event::Closed>())
                window.close();
            if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Escape)){
                window.close();
            }
        }

        sf::Vector2u windowSize = window.getSize();
        for (auto& ball : balls) {
            ball.shape.move({ball.x_vol, ball.y_vol});
            sf::Vector2f position = ball.shape.getPosition();
            float radius = ball.shape.getRadius();

            if (position.x < 0){
                position.x = 0;
                ball.x_vol = -ball.x_vol * wall_absoribtion;
            }
            if (position.y < 0) {
                position.y = 0;
                ball.y_vol = -ball.y_vol * wall_absoribtion;
            }
            if (position.x + radius * 2 > windowSize.x)
            {
            position.x = windowSize.x - radius * 2;
            ball.x_vol = -ball.x_vol * wall_absoribtion;
            }
            
            if (position.y + radius * 2 > windowSize.y){
            position.y = windowSize.y - radius * 2;
            ball.y_vol = -ball.y_vol * wall_absoribtion;
            }
            
            ball.shape.setPosition(position);

            if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Left)){
                ball.x_vol -= GRAVITY;
            }
            if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Right)){
                ball.x_vol += GRAVITY;
            }
            if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Up)){
                ball.y_vol -= GRAVITY;
            }
            if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Down)){
                ball.y_vol += GRAVITY;
            }
        }

        std::vector<std::vector<std::vector<int>>> grid(GRID_WIDTH, std::vector<std::vector<int>>(GRID_HEIGHT)); //[x][y] vector info of  all balls in each cell

        for (int i = 0; i < balls.size(); i++){
            sf::Vector2f pos = balls[i].shape.getPosition();
            float radius = balls[i].shape.getRadius();

            int centerX = pos.x + radius;
            int centerY = pos.y + radius;
            int cellX = centerX / CELL_SIZE;
            int cellY = centerY / CELL_SIZE;

            if (cellX >= 0 && cellX < GRID_WIDTH && cellY >= 0 &&cellY < GRID_HEIGHT) {
                grid[cellX][cellY].push_back(i);
            }
        }

        for (int i = 0; i < balls.size(); i++){
            sf::Vector2f pos = balls[i].shape.getPosition();
            float radius = balls[i].shape.getRadius();

            int centerX = pos.x + radius;
            int centerY = pos.y + radius;
            int cellX = centerX / CELL_SIZE;
            int cellY = centerY / CELL_SIZE;

            for (int offset_x = -1; offset_x <= 1; offset_x++){
                for (int offset_y = -1; offset_y <= 1; offset_y++){
                    int xneighbour = cellX + offset_x;
                    int yneighbour = cellY + offset_y;

                    if (xneighbour < 0 || xneighbour >= GRID_WIDTH || yneighbour < 0 || yneighbour >= GRID_HEIGHT){
                        continue;
                    }

                    for (int j : grid[xneighbour][yneighbour]) {
                        if (i >= j) continue; //skip doing same pair


                        sf::Vector2f position1 = balls[i].shape.getPosition();
                        sf::Vector2f position2 = balls[j].shape.getPosition();
                        float r1 = balls[i].shape.getRadius();
                        float r2 = balls[j].shape.getRadius();

                        float x_ball_distance = (position1.x + r1) - (position2.x + r2);
                        float y_ball_distance = (position1.y + r1) - (position2.y + r2);
                        float distance = std::sqrt(x_ball_distance * x_ball_distance + y_ball_distance * y_ball_distance);

                        if (distance < r1 + r2) {

                            float overlap = (r1 + r2) - distance;
    
                            if (distance == 0) distance = 0.01f; //for division by zero
                            
                            //calculate direction vector (normalized)
                            float dx = x_ball_distance / distance;
                            float dy = y_ball_distance / distance;

                            balls[i].shape.move({dx * overlap * 0.5f, dy * overlap * 0.5f});
                            balls[j].shape.move({-dx * overlap * 0.5f, -dy * overlap * 0.5f});

                            float x_relative_velocity = balls[i].x_vol - balls[j].x_vol;
                            float y_relative_velocity = balls[i].y_vol - balls[j].y_vol;

                            float velocity_along_normal = x_relative_velocity * dx + y_relative_velocity * dy;

                            if (velocity_along_normal > 0) continue;

                            float impulse = velocity_along_normal;
                            
                            balls[i].x_vol -= impulse * dx;
                            balls[i].y_vol -= impulse * dy;
                            balls[j].x_vol += impulse * dx;
                            balls[j].y_vol += impulse * dy;

                        }
                    }

                    }
                }
            }


        
            window.clear();
        
        for (const auto& ball : balls) {

            window.draw(ball.shape);
        }
        
        window.display();
    }
}