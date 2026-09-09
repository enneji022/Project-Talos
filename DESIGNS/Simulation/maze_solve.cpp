#include <iostream>
#include <vector>
#include <random>
#include <chrono>
#include <cstdlib>
#include <thread>

constexpr int TOTAL_WIDTH = 424;
constexpr int TOTAL_HEIGHT = 424;

constexpr int PASSAGE_SIZE = 10;
constexpr int WALL_SIZE = 4;
constexpr int BLOCK_SIZE = PASSAGE_SIZE + WALL_SIZE;

constexpr int MAZE_COLS = (TOTAL_WIDTH - WALL_SIZE) / BLOCK_SIZE;
constexpr int MAZE_ROWS = (TOTAL_HEIGHT - WALL_SIZE) / BLOCK_SIZE;

enum Heading { UP = 0, RIGHT = 1, DOWN = 2, LEFT = 3 };

// Direction offsets corresponding to Heading enum [UP, RIGHT, DOWN, LEFT]
const int dr[] = {-1, 0, 1, 0};
const int dc[] = {0, 1, 0, -1};

struct RobotCar {
    int r;
    int c;
    Heading dir = RIGHT;
    char symbol = 'U';
};

void clearScreen() {
#if defined(_WIN32) || defined(_WIN64)
    std::system("cls");
#else
    std::system("clear");
#endif
}

bool isValidMove(const std::vector<std::string>& maze, int nr, int nc) {
    if (nr < 0 || nr >= TOTAL_HEIGHT || nc < 0 || nc >= TOTAL_WIDTH) {
        return false;
    }
    return maze[nr][nc] != '#';
}

void renderMaze(const std::vector<std::string>& maze, const RobotCar& robot) {
    clearScreen();
    for (int r = 0; r < TOTAL_HEIGHT; ++r) {
        for (int c = 0; c < TOTAL_WIDTH; ++c) {
            if (r == robot.r && c == robot.c) {
                std::cout << robot.symbol;
            } else {
                std::cout << maze[r][c];
            }
        }
        std::cout << '\n';
    }
    std::cout << "\nAutonomous Mode Active | Robot Location: (" << robot.r << ", " << robot.c << ")\n";
}

// Autonomous Wall-Following Logic (Right-Hand Rule)
void updateAutonomousNavigation(const std::vector<std::string>& maze, RobotCar& car) {
    // Relative directions to test: Right (+1), Straight (+0), Left (+3), Back (+2)
    const int relativeTurns[] = {1, 0, 3, 2};

    for (int turn : relativeTurns) {
        Heading targetHeading = static_cast<Heading>((car.dir + turn) % 4);
        int nextR = car.r + dr[targetHeading];
        int nextC = car.c + dc[targetHeading];

        if (isValidMove(maze, nextR, nextC)) {
            car.r = nextR;
            car.c = nextC;
            car.dir = targetHeading;
            return;
        }
    }
}

int main() {
    std::ios_base::sync_with_stdio(false);
    std::cin.tie(NULL);

    std::vector<std::string> maze(TOTAL_HEIGHT, std::string(TOTAL_WIDTH, '#'));

    auto carveBlock = [&](int startR, int startC, int height, int width) {
        for (int r = startR; r < startR + height && r < TOTAL_HEIGHT; ++r) {
            for (int c = startC; c < startC + width && c < TOTAL_WIDTH; ++c) {
                maze[r][c] = ' ';
            }
        }
    };

    auto carveCell = [&](int r, int c) {
        int top = WALL_SIZE + r * BLOCK_SIZE;
        int left = WALL_SIZE + c * BLOCK_SIZE;
        carveBlock(top, left, PASSAGE_SIZE, PASSAGE_SIZE);
    };

    auto carveConnection = [&](int r1, int c1, int r2, int c2) {
        int top1 = WALL_SIZE + r1 * BLOCK_SIZE;
        int left1 = WALL_SIZE + c1 * BLOCK_SIZE;
        int top2 = WALL_SIZE + r2 * BLOCK_SIZE;
        int left2 = WALL_SIZE + c2 * BLOCK_SIZE;

        if (r1 == r2) {
            int minC = std::min(left1, left2);
            carveBlock(top1, minC + PASSAGE_SIZE, PASSAGE_SIZE, WALL_SIZE);
        } else if (c1 == c2) {
            int minR = std::min(top1, top2);
            carveBlock(minR + PASSAGE_SIZE, left1, WALL_SIZE, PASSAGE_SIZE);
        }
    };

    std::vector<std::vector<bool>> visited(MAZE_ROWS, std::vector<bool>(MAZE_COLS, false));
    std::vector<std::pair<int, int>> stack;

    std::mt19937 rng(static_cast<unsigned int>(
        std::chrono::steady_clock::now().time_since_epoch().count()));

    visited[0][0] = true;
    carveCell(0, 0);
    stack.push_back({0, 0});

    while (!stack.empty()) {
        auto [r, c] = stack.back();

        std::vector<int> valid_dirs;
        for (int i = 0; i < 4; ++i) {
            int nr = r + dr[i];
            int nc = c + dc[i];

            if (nr >= 0 && nr < MAZE_ROWS && nc >= 0 && nc < MAZE_COLS) {
                if (!visited[nr][nc]) {
                    valid_dirs.push_back(i);
                }
            }
        }

        if (!valid_dirs.empty()) {
            std::uniform_int_distribution<size_t> dist(0, valid_dirs.size() - 1);
            int dir = valid_dirs[dist(rng)];

            int nr = r + dr[dir];
            int nc = c + dc[dir];

            carveCell(nr, nc);
            carveConnection(r, c, nr, nc);

            visited[nr][nc] = true;
            stack.push_back({nr, nc});
        } else {
            stack.pop_back();
        }
    }

    carveBlock(WALL_SIZE, 0, PASSAGE_SIZE, WALL_SIZE);
    maze[WALL_SIZE][0] = 'S';

    int exitR = WALL_SIZE + (MAZE_ROWS - 1) * BLOCK_SIZE;
    carveBlock(exitR, TOTAL_WIDTH - WALL_SIZE, PASSAGE_SIZE, WALL_SIZE);
    maze[exitR + PASSAGE_SIZE - 1][TOTAL_WIDTH - 1] = 'E';

    RobotCar car{WALL_SIZE, WALL_SIZE, RIGHT, 'U'};

    // Autonomous Loop
    do{
        renderMaze(maze, car);

        if (maze[car.r][car.c] == 'E') {
            std::cout << "\nAutonomous Vehicle 'U' successfully reached the Exit ('E')!\n";
            break;
        }

        // Execute self-navigation step
        updateAutonomousNavigation(maze, car);

        // Delay between frame updates (in milliseconds)
        std::this_thread::sleep_for(std::chrono::milliseconds(1000));
    }while (false);

    return 0;
}