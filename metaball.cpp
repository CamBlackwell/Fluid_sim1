#include <SFML/Graphics.hpp>
#include <vector>
#include <cstdlib>
#include <ctime>
#include <cmath>
#include <thread>
#include <mutex>

const char* metaballFragmentShader = R"(
uniform vec2 ballPositions[1000];
uniform int ballCount;
uniform float threshold;
uniform float radius;
uniform vec2 windowSize;

void main() {
    vec2 pixelPos = vec2(gl_FragCoord.x, windowSize.y - gl_FragCoord.y);
    float sum = 0.0;

    for(int i = 0; i < ballCount; i++){
        vec2 diff = pixelPos - ballPositions[i];
        float distSquared = dot(diff, diff);

        if(distSquared > 9000.0) continue;

        distSquared = max(distSquared, 25.0);

        if(distSquared > 0.0001) {
            sum += (radius * radius) / distSquared;
        }
        if(sum > threshold * 1.5) break;
    }
    
    if(sum > threshold) {
        float intensity = smoothstep(threshold, threshold * 20.0, sum);
        vec3 colour1 = vec3(0.0, 0.502, 0.502); //light blue
        vec3 colour2 = vec3(0.0, 0.2, 1.0); //Blue
        vec3 finalColour = mix(colour1, colour2, intensity);
        gl_FragColor = vec4(finalColour, 1.0);
    } else {
        discard; //transparent 
    }
}
)";

const int NUM_THREADS = std::thread::hardware_concurrency();

struct Ball {
    sf::CircleShape shape;
    float x_vol = 3.f;
    float y_vol = 3.f;
    int blue = 255;
    int red = 0;
};

std::vector<Ball> makeBalls(int num){
    std::vector<Ball> balls;
    std::srand(1);
    for (int i = 0; i < num; i++){
        Ball ball;
        ball.shape.setRadius(4.f);
        
        ball.shape.setPosition({std::rand() % 700 + 50.f, std::rand() % 500 + 50.f});
        //ball.shape.setFillColor({sf::Color(std::rand() % 256, std::rand() % 256, std::rand() % 256)});
        ball.blue = rand() % (255 + 1 - 150) + 150;
        ball.shape.setFillColor(sf::Color(0, 0, ball.blue));
        //ball.shape.setFillColor(sf::Color(0,0, ball.colour));

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
    std::vector<std::vector<std::vector<int>>> grid;


    float scalar = 10.f;
    std::vector<Ball> balls = makeBalls(1000);

    sf::Shader metaballShader;
    if(!metaballShader.loadFromMemory(metaballFragmentShader, sf::Shader::Type::Fragment)) {
        return -1;
    }

    sf::RenderTexture renderTexture;
    if (!renderTexture.resize({window_width / 2, window_height / 2})) {
        return -1;
    }

    renderTexture.display();

    sf::Sprite sprite(renderTexture.getTexture());
    sprite.setScale(sf::Vector2f(2.0f, 2.0f));
    
    while (window.isOpen()){

        while (const std::optional event = window.pollEvent()){
            if (event->is<sf::Event::Closed>())
                window.close();
            if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Escape)){
                window.close();
            }

            if (event->is<sf::Event::Resized>()) {
                auto size = event->getIf<sf::Event::Resized>();
                sf::FloatRect visibleArea(sf::Vector2f(0, 0), sf::Vector2f(size->size.x, size->size.y));
                window.setView(sf::View(visibleArea));

                if (!renderTexture.resize({size->size.x / 2, size->size.y / 2})) {  
                    return -1;
                }
                renderTexture.setView(sf::View(sf::FloatRect(sf::Vector2f(0, 0), sf::Vector2f(size->size.x / 2, size->size.y / 2))));

                renderTexture.display();
                sprite.setTexture(renderTexture.getTexture());
                sprite.setTextureRect(sf::IntRect(sf::Vector2i(0, 0), sf::Vector2i(size->size.x / 2, size->size.y / 2)));
                sprite.setPosition(sf::Vector2f(0, 0));
                sprite.setScale(sf::Vector2f(2.0f, 2.0f));
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
            float speedSquared = ball.x_vol * ball.x_vol + ball.y_vol * ball.y_vol;
            /*int newColour = std::min(255, static_cast<int>(speedSquared * 0.2));
            if (std::abs(newColour - ball.red) > 5){
                ball.red = newColour;
                ball.shape.setFillColor(sf::Color(ball.red, 0, ball.blue));
            }*/
        }


        const int GRID_WIDTH = windowSize.x / CELL_SIZE;
        const int GRID_HEIGHT = windowSize.y / CELL_SIZE;

        //resize grid if needed (handles window resize too)
        if (grid.size() != GRID_WIDTH || (grid.size() > 0 && grid[0].size() != GRID_HEIGHT)) {
            grid.resize(GRID_WIDTH);
            for (auto& row : grid) {
                row.resize(GRID_HEIGHT);
            }
        }

        for(auto& row : grid) {
            for(auto& cell : row) {
                cell.clear();
            }
}

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

        std::vector<std::thread> threads;
        int ballsPerThread = balls.size() / NUM_THREADS;

        for (int t = 0; t < NUM_THREADS; t++) {
            int start = t * ballsPerThread;
            int end = (t == NUM_THREADS - 1)? balls.size() : (t + 1) * ballsPerThread;

                threads.emplace_back([start, end, &balls, &grid, GRID_HEIGHT, GRID_WIDTH, CELL_SIZE] (){
                    for (int i = start; i < end; i++){

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
                });
            }
            for (auto& thread : threads) {
                thread.join();
            }
            /*for (const auto& ball : balls) {

                window.draw(ball.shape);
            }*/



            std::vector<float> positions;
            for (const auto& ball : balls){
                sf::Vector2f pos = ball.shape.getPosition();
                float radius = ball.shape.getRadius();
                positions.push_back((pos.x + radius) / 2.0f);
                positions.push_back((pos.y + radius) / 2.0f);
            }

            sf::Vector2u currentSize = window.getSize();

            metaballShader.setUniformArray("ballPositions", reinterpret_cast<sf::Glsl::Vec2*>(positions.data()), balls.size());
            metaballShader.setUniform("ballCount", static_cast<int>(balls.size()));
            metaballShader.setUniform("threshold", 1.5f);
            metaballShader.setUniform("radius", 7.f);
            metaballShader.setUniform("windowSize", sf::Vector2f(currentSize.x / 2.0f, currentSize.y / 2.0f));
            
            renderTexture.clear(sf::Color(248,248,255));
            sf::RectangleShape fullscreen(sf::Vector2f(currentSize.x / 2.0f, currentSize.y / 2.0f));
            renderTexture.draw(fullscreen, &metaballShader);
            renderTexture.display();

            window.clear(sf::Color(248,248,255));
            window.draw(sprite);
        window.display();
    }
}


// cd /c/Users/camer/OneDrive/Desktop/Projects/Programming_projects/Fluid_sim1