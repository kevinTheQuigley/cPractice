#include <SFML/Graphics.hpp>
#include <random>
#include <vector>

// Structure for a Robot
struct Robot {
    sf::CircleShape shape;
    float dx;  // Change in x-direction
    float dy;  // Change in y-direction
};

// Function to generate random float between -0.5 and 0.5 for slower movement
float randomDirection() {
    static std::random_device rd;
    static std::mt19937 gen(rd());
    static std::uniform_real_distribution<float> dis(-0.005f, 0.005f);  // Reduced range for slower movement
    return dis(gen);
}

int main() {
    // Set up the SFML window with double the original size
    sf::RenderWindow window(sf::VideoMode(1600, 1200), "Moving Robots");

    // Vector to hold multiple robots
    std::vector<Robot> robots;

    // Create 10 robots
    for (int i = 0; i < 10; ++i) {
        Robot robot;
        robot.shape = sf::CircleShape(20);  // Circle with radius 20
        robot.shape.setFillColor(sf::Color::Green);

        // Starting position in the lower half of the screen
        robot.shape.setPosition(800, 900);  // Centered in x and lower half in y

        // Random direction for movement
        robot.dx = randomDirection();
        robot.dy = randomDirection();
        robots.push_back(robot);
    }

    // Main loop
    while (window.isOpen()) {
        sf::Event event;
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed)
                window.close();
        }

        // Clear the window
        window.clear();

        // Update and draw each robot
        for (auto& robot : robots) {
            // Move the robot
            robot.shape.move(robot.dx, robot.dy);

            // Check bounds and reverse direction if needed
            sf::Vector2f pos = robot.shape.getPosition();
            if (pos.x <= 0 || pos.x >= 1560) robot.dx = -robot.dx;  // Reverse x-direction within new width
            if (pos.y <= 600 || pos.y >= 1180) robot.dy = -robot.dy;  // Reverse y-direction within lower half

            // Draw the robot
            window.draw(robot.shape);
        }

        // Display the updated frame
        window.display();
    }

    return 0;
}
