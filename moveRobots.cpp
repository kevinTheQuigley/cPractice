#include <SFML/Graphics.hpp>
#include <fstream>
#include <iomanip>
#include <sstream>
#include <vector>
#include <chrono>   // For high-resolution timer
#include <random>   // For randomFloat and stuck chance
#include <cstdlib>  // For system()

// Function to generate a random float between min and max
float randomFloat(float min, float max) {
    static std::random_device rd;  // Random device for seeding
    static std::mt19937 gen(rd());  // Mersenne Twister generator
    std::uniform_real_distribution<float> dis(min, max);  // Distribution
    return dis(gen);  // Generate and return random number
}

// Function to simulate a 1/100,000 chance
bool isStuck() {
    static std::random_device rd;  // Random device for seeding
    static std::mt19937 gen(rd());  // Mersenne Twister generator
    std::uniform_int_distribution<int> dis(1, 100000);  // Range 1 to 100,000
    return dis(gen) == 1;  // True if we hit 1
}

// Structure for a Robot
struct Robot {
    int id;  // Unique ID for the robot
    sf::CircleShape shape;
    sf::VertexArray trail;  // Vertex array for the trail
    bool movingDown = true;  // Initially moving down
    bool movingUp = false;   // Initially not moving up
    bool shiftingRight = true;  // Initially shifting right
    bool movingLeft = false;    // Used when bouncing off the right wall
    float targetX = 0;          // Target x-coordinate when shifting
    std::string logFile;        // File to log robot's activity
    std::chrono::time_point<std::chrono::steady_clock> lastLogTime;  // Last log time
    bool stuck = false;         // Whether the robot is stuck
};

// Create a log message with timestamp and robot ID
std::string createLogMessage(int robotID, const std::string& status, const sf::Vector2f& position) {
    std::ostringstream oss;
    auto t = std::time(nullptr);
    auto tm = *std::localtime(&t);

    oss << std::put_time(&tm, "[%Y-%m-%d %H:%M:%S] ")
        << "Robot ID: " << robotID
        << " - Position: (" << position.x << ", " << position.y << ") - Status: " << status;
    return oss.str();
}

// Log message to the robot's file
void logToFile(const std::string& logFile, const std::string& message) {
    std::ofstream file(logFile, std::ios::app);
    if (file.is_open()) {
        file << message << "\n";
    }
}

int main() {
    // Set up the SFML window
    sf::RenderWindow window(sf::VideoMode(1600, 1200), "Mowing Motion with Error Handling");

    // Parameters for the screen
    const int screenWidth = 1600;
    const int screenHeight = 1200;
    const int lowerHalfStart = screenHeight / 2;

    // Ensure the logs directory exists using system command
    std::system("mkdir -p robot_logs");

    // Vector to hold multiple robots
    std::vector<Robot> robots;

    // Create 10 robots
    for (int i = 0; i < 10; ++i) {
        Robot robot;
        robot.id = i + 1;  // Assign a unique ID to each robot
        robot.shape = sf::CircleShape(15);  // Circle with radius 15
        robot.shape.setFillColor(sf::Color::Green);

        // Starting position: One robot at x = 0, others spaced evenly
        float startX = (i == 0) ? 0 : 160 + (i - 1) * 140;
        robot.shape.setPosition(startX, screenHeight / 2);

        // Initialize trail
        robot.trail = sf::VertexArray(sf::LinesStrip);

        // Assign log file
        robot.logFile = "robot_logs/robot" + std::to_string(robot.id) + ".txt";

        // Clear previous logs
        std::ofstream file(robot.logFile, std::ios::trunc);  // Truncate the file to clear it
        file.close();

        // Initialize last log time
        robot.lastLogTime = std::chrono::steady_clock::now();

        // Log initial position
        logToFile(robot.logFile, createLogMessage(robot.id, "Initialized", robot.shape.getPosition()));

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
        window.clear(sf::Color::Black);

        // Update and draw each robot
        for (auto& robot : robots) {
            // Get the current time
            auto currentTime = std::chrono::steady_clock::now();

            // Check if it's been 3 seconds since the last log
            auto timeSinceLastLog = std::chrono::duration_cast<std::chrono::seconds>(currentTime - robot.lastLogTime).count();

            // If the robot is stuck
            if (robot.stuck) {
                // Log the stuck state every 3 seconds
                if (timeSinceLastLog >= 3) {
                    logToFile(robot.logFile, createLogMessage(robot.id, "ERROR: Stuck", robot.shape.getPosition()));
                    robot.lastLogTime = currentTime;
                }
                // Draw the robot (keep it visible)
                window.draw(robot.shape);
                window.draw(robot.trail);
                continue;
            }

            // Check if the robot gets stuck
            if (isStuck()) {
                robot.stuck = true;
                robot.shape.setFillColor(sf::Color::Red);  // Turn red
                logToFile(robot.logFile, createLogMessage(robot.id, "ERROR: Stuck", robot.shape.getPosition()));
                // Draw the robot immediately after it gets stuck
                window.draw(robot.shape);
                window.draw(robot.trail);
                continue;  // Stop moving
            }

            // Move the robot
            sf::Vector2f currentPosition = robot.shape.getPosition();

            // Add the current position to the trail
            robot.trail.append(sf::Vertex(currentPosition, sf::Color::White));

            // Determine movement based on state
            bool changedDirection = false;

            if (robot.movingDown) {
                robot.shape.move(0, 0.1f);  // Move down slower
                if (currentPosition.y >= screenHeight - 30) {
                    robot.movingDown = false;
                    robot.shiftingRight = !robot.movingLeft;
                    robot.movingLeft = false;
                    robot.targetX = currentPosition.x + 30 + randomFloat(-5, 5);  // Add random x component
                    changedDirection = true;
                }
            } else if (robot.movingUp) {
                robot.shape.move(0, -0.1f);  // Move up slower
                if (currentPosition.y <= lowerHalfStart) {
                    robot.movingUp = false;
                    robot.shiftingRight = !robot.movingLeft;
                    robot.movingLeft = false;
                    robot.targetX = currentPosition.x + 30 + randomFloat(-5, 5);  // Add random x component
                    changedDirection = true;
                }
            } else if (robot.shiftingRight) {
                robot.shape.move(0.1f, 0);  // Shift right slower
                if (currentPosition.x >= robot.targetX) {
                    robot.shiftingRight = false;
                    if (currentPosition.y >= screenHeight - 30) {
                        robot.movingUp = true;
                    } else {
                        robot.movingDown = true;
                    }
                    changedDirection = true;
                }
                if (currentPosition.x >= screenWidth - 30) {
                    robot.shiftingRight = false;
                    robot.movingLeft = true;
                    changedDirection = true;
                }
            } else if (robot.movingLeft) {
                robot.shape.move(-0.1f, 0);  // Move left slower
                if (currentPosition.x <= 0) {
                    robot.movingLeft = false;
                    robot.movingDown = true;
                    changedDirection = true;
                }
            }

            // Log direction change immediately
            if (changedDirection) {
                logToFile(robot.logFile, createLogMessage(robot.id, "Changed direction", robot.shape.getPosition()));
            }

            // Log position every 3 seconds
            if (timeSinceLastLog >= 3) {
                logToFile(robot.logFile, createLogMessage(robot.id, "Moving", robot.shape.getPosition()));
                robot.lastLogTime = currentTime;  // Update last log time
            }

            // Draw the trail
            window.draw(robot.trail);

            // Draw the robot
            window.draw(robot.shape);
        }

        // Display the updated frame
        window.display();
    }

    return 0;
}
