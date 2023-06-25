#include <iostream>
#include <vector>
#include <ctime>
#include <cmath>
#include <cstdlib>

struct Tile {
    float elevation;
    float latitude;
    float temperature;
    float humidity;
    float windDirection; // In degrees
    float windSpeed;
};

float calculateTemperature(float latitude, float elevation) {
    float temperature = 30 - (fabs(latitude) * 0.3) - (elevation * 0.0065);
    return temperature;
}

int main() {
    std::cout << "Start" << std::endl;
    const int width = 10;
    const int height = 10;
    const int numIterations = 10;
    const float convergenceThreshold = 0.01;

    std::srand(std::time(0));

    // Initialize the map
    std::vector<std::vector<Tile>> map(height, std::vector<Tile>(width));

// Randomly assign elevation, wind direction, and wind speed
for (int i = 0; i < height; ++i) {
    for (int j = 0; j < width; ++j) {
        map[i][j].elevation = std::rand() % 21; // Elevation ranks between 0 and 20
        map[i][j].latitude = i * 180.0 / height - 90.0;
        map[i][j].temperature = calculateTemperature(map[i][j].latitude, map[i][j].elevation * 50); // Assuming each rank represents 50 meters
        map[i][j].windDirection = std::rand() % 360;
        map[i][j].windSpeed = std::rand() % 10;

        // Discretize temperature
        map[i][j].temperature = std::round((map[i][j].temperature + 90) / 10); // Assuming the temperature range is -90 to 30
        if (map[i][j].temperature > 20) {
            map[i][j].temperature = 20;
        }

        // Assign high humidity to tiles with low elevation
        if (map[i][j].elevation < 4) { // Adjust the threshold value as needed
            map[i][j].humidity = 16 + std::rand() % 5; // Humidity ranks between 16 and 20
        } else {
            map[i][j].humidity = std::rand() % 16; // Humidity ranks between 0 and 15
        }
    }
}



    // Assign moisture
    for (int iteration = 0; iteration < numIterations; ++iteration) {
        bool converged = true;
        for (int i = 0; i < height; ++i) {
            for (int j = 0; j < width; ++j) {
                float oldHumidity = map[i][j].humidity;
                float transferredMoisture = 0.0;

                // Calculate transferred moisture from neighboring tiles
                for (int x = -1; x <= 1; ++x) {
                    for (int y = -1; y <= 1; ++y) {
                        if (x == 0 && y == 0) continue;
                        int ni = i + x;
                        int nj = j + y;

                        if (ni >= 0 && ni < height && nj >= 0 && nj < width) {
                            float windDirection = map[ni][nj].windDirection;
                            float windSpeed = map[ni][nj].windSpeed;
                            float deltaX = x * cos(windDirection * M_PI / 180.0);
                            float deltaY = y * sin(windDirection * M_PI / 180.0);

                            if (deltaX + deltaY > 0) {
                                transferredMoisture += map[ni][nj].humidity * windSpeed * (deltaX + deltaY);
                            }
                        }
                    }
                }

                // Update humidity
                map[i][j].humidity += transferredMoisture;

                // Check if the change in humidity has converged
                if (fabs(map[i][j].humidity - oldHumidity) > convergenceThreshold) {
                    converged = false;
                }
            }
        }

        if (converged) {
            break;
        }
    }

        // Print the final humidity values
        for (int i = 0; i < height; ++i) {
            for (int j = 0; j < width; ++j) {
                std::cout << "Tile (" << i << ", " << j << ") Humidity: " << map[i][j].humidity << std::endl;
            }
        }
    

    return 0;
}
