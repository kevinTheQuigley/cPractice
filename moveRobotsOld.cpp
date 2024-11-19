#include <SFML/Graphics.hpp>
#include <random>
#include <vector>
#include <cmath>

// Structure for a Robot (ball) in pseudo-3D space
struct Robot {
    sf::CircleShape shape;
    float dx;    // Change in x-direction (scaled by depth)
    float dy;    // Change in y-direction (scaled by depth)
    float dz;    // Change in z-direction (depth)
    float z;     // Current depth
    bool bouncing; // Flag to indicate if the robot is currently bouncing
    int bounceTimer; // Timer to manage the bounce effect duration
};

// Function to generate a random float between -0.0005 and 0.0005 for very slow movement
float randomDirection() {
    static std::random_device rd;
    static std::mt19937 gen(rd());
    static std::uniform_real_distribution<float> dis(-0.005f, 0.005f);  // Very slow x and y movement
    return dis(gen);
}

// Function to generate a random depth (z) velocity value for very slow depth movement
float randomDepthSpeed() {
    static std::random_device rd;
    static std::mt19937 gen(rd());
    static std::uniform_real_distribution<float> depth_dis(-0.00001f, 0.00001f);  // Extremely slow z movement
    return depth_dis(gen);
}

int main() {
    // Set up the SFML window with double the original size
    sf::RenderWindow window(sf::VideoMode(1600, 1200), "Contained Robots in Lower Half");

    // Vector to hold multiple robots
    std::vector<Robot> robots;

    // Create 10 robots within the lower half
    for (int i = 0; i < 10; ++i) {
        Robot robot;
        robot.shape = sf::CircleShape(20);  // Circle with base radius 20
        robot.shape.setFillColor(sf::Color::Green);

        // Initial position within the lower half of the screen
        float startX = 400 + static_cast<float>(rand() % 800);  // Random x position in the center range
        float startY = 600 + static_cast<float>(rand() % 600);  // Random y position in the lower half
        robot.shape.setPosition(startX, startY);

        // Random direction for movement
        robot.dx = randomDirection();
        robot.dy = randomDirection();
        robot.dz = randomDepthSpeed();  // Extremely slow depth movement

        // Random initial depth
        robot.z = 1.5f + static_cast<float>(i) / 20.0f;  // Spread out the robots initially

        // Initialize bounce properties
        robot.bouncing = false;
        robot.bounceTimer = 0;

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
            // Adjust the movement speed based on depth to simulate perspective
            float adjusted_dx = robot.dx / robot.z;  // Slow down x movement as z increases
            float adjusted_dy = robot.dy / robot.z;  // Slow down y movement as z increases

            // Move the robot in x, y, and z (pseudo-3D depth) with depth-scaling
            sf::Vector2f pos = robot.shape.getPosition();
            robot.shape.setPosition(pos.x + adjusted_dx, pos.y + adjusted_dy);
            robot.z += robot.dz;

            // Scale the robot based on its depth to simulate perspective
            float scale_factor = 1.0f / robot.z;
            robot.shape.setScale(scale_factor, scale_factor);

            // Enforce boundary checks for the lower half of the screen
            pos = robot.shape.getPosition();
            float robotWidth = robot.shape.getGlobalBounds().width;
            float robotHeight = robot.shape.getGlobalBounds().height;

            if (pos.x < 0) {
                robot.shape.setPosition(0, pos.y);
                robot.dx = fabs(robot.dx);  // Redirect to stay within bounds
            } else if (pos.x + robotWidth > 1600) {
                robot.shape.setPosition(1600 - robotWidth, pos.y);
                robot.dx = -fabs(robot.dx); // Redirect to stay within bounds
            }
            if (pos.y < 600) {  // Top bound of the lower half
                robot.shape.setPosition(pos.x, 600);
                robot.dy = fabs(robot.dy);  // Redirect to stay within bounds
            } else if (pos.y + robotHeight > 1200) {  // Bottom bound of the screen
                robot.shape.setPosition(pos.x, 1200 - robotHeight);
                robot.dy = -fabs(robot.dy); // Redirect to stay within bounds
            }

            // Apply bounce effect by temporarily increasing size
            if (robot.bouncing) {
                robot.shape.setScale(scale_factor * 1.2f, scale_factor * 1.2f);  // Increase size for bounce effect
                robot.bounceTimer--;

                if (robot.bounceTimer <= 0) {
                    robot.bouncing = false;  // End bounce effect
                }
            } else {
                robot.shape.setScale(scale_factor, scale_factor);  // Return to normal size
            }

            // Draw the robot
            window.draw(robot.shape);
        }

        // Display the updated frame
        window.display();
    }

    return 0;
}
