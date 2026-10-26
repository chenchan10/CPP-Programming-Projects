
#include <iostream>
#include <windows.h>
#include <conio.h>
#include <cstdlib>
#include <ctime>
#include <string>
#include <algorithm>

using namespace std;

// ============================================================
// GAME CONFIGURATION
// ============================================================

constexpr int SCREEN_WIDTH = 79;
constexpr int SCREEN_HEIGHT = 25;

constexpr int GAME_TOP = 2;
constexpr int HURDLE_COUNT = 4;
constexpr int HURDLE_WIDTH = 3;
constexpr int HURDLE_GAP = 9;
constexpr int HURDLE_SPACING = 29;

constexpr int BIRD_X = 12;
constexpr int BIRD_WIDTH = 7;
constexpr int BIRD_HEIGHT = 3;

// Movement settings
constexpr int JUMP_STRENGTH = 6;
constexpr int GRAVITY_INTERVAL = 2;
constexpr int FRAME_DELAY = 80;

// ============================================================
// WINDOWS CONSOLE UTILITIES
// ============================================================

HANDLE consoleHandle = GetStdHandle(STD_OUTPUT_HANDLE);

void goToXY(int x, int y) {

    COORD position = {
        static_cast<SHORT>(x),
        static_cast<SHORT>(y)
    };

    SetConsoleCursorPosition(
        consoleHandle,
        position
    );
}

void setCursorVisible(bool visible) {

    CONSOLE_CURSOR_INFO cursorInfo;

    if (GetConsoleCursorInfo(consoleHandle, &cursorInfo)) {

        cursorInfo.bVisible = visible;

        SetConsoleCursorInfo(
            consoleHandle,
            &cursorInfo
        );
    }
}

void configureConsole() {

    // Try to enlarge the console buffer when necessary.
    // The terminal host may reject resizing.

    CONSOLE_SCREEN_BUFFER_INFO info;

    if (GetConsoleScreenBufferInfo(consoleHandle, &info)) {

        COORD requiredSize = {
            static_cast<SHORT>(
                max<int>(info.dwSize.X, SCREEN_WIDTH + 1)
            ),
            static_cast<SHORT>(
                max<int>(info.dwSize.Y, SCREEN_HEIGHT + 4)
            )
        };

        SetConsoleScreenBufferSize(
            consoleHandle,
            requiredSize
        );
    }
}

// ============================================================
// FLAPPY BIRD CLASS
// ============================================================

class Flappy_Bird {

private:

    struct Hurdle {

        int x;
        int gapTop;
        bool scored;
    };

    Hurdle hurdles[HURDLE_COUNT];

    int birdY = 12;
    int score = 0;
    int frameCount = 0;

    bool gameOver = false;
    bool previousSpaceState = false;

    string screen[SCREEN_HEIGHT + 3];

    // ========================================================
    // RANDOM OBSTACLE POSITION
    // ========================================================

    int randomGapTop() {

        int minTop = GAME_TOP + 3;

        int maxTop =
            SCREEN_HEIGHT - HURDLE_GAP - 3;

        return minTop +
            rand() % (maxTop - minTop + 1);
    }

    // ========================================================
    // INITIALIZE OBSTACLES
    // ========================================================

    void initializeHurdles() {

        for (int i = 0; i < HURDLE_COUNT; i++) {

            hurdles[i].x =
                SCREEN_WIDTH + i * HURDLE_SPACING;

            hurdles[i].gapTop = randomGapTop();

            hurdles[i].scored = false;
        }
    }

    // ========================================================
    // CLEAR FRAME BUFFER
    // ========================================================

    void clearFrame() {

        for (int y = 0; y < SCREEN_HEIGHT + 3; y++) {

            screen[y] = string(
                SCREEN_WIDTH,
                ' '
            );
        }
    }

    // ========================================================
    // DRAW TEXT INTO FRAME BUFFER
    // ========================================================

    void drawText(
        int x,
        int y,
        const string& text
    ) {

        if (
            y < 0 ||
            y >= SCREEN_HEIGHT + 3
        ) {
            return;
        }

        for (size_t i = 0; i < text.size(); i++) {

            int drawX = x + static_cast<int>(i);

            if (
                drawX >= 0 &&
                drawX < SCREEN_WIDTH
            ) {

                screen[y][drawX] = text[i];
            }
        }
    }

    // ========================================================
    // DRAW SCOREBOARD
    // ========================================================

    void printScore() {

        drawText(
            2,
            0,
            "FLAPPY BIRD"
        );

        string scoreText =
            "Score: " + to_string(score);

        drawText(
            SCREEN_WIDTH -
                static_cast<int>(scoreText.size()) - 2,
            0,
            scoreText
        );

        drawText(
            0,
            1,
            string(SCREEN_WIDTH, '-')
        );
    }

    // ========================================================
    // DRAW BIRD
    // ========================================================

    void printBird() {

        drawText(
            BIRD_X,
            birdY,
            "  __"
        );

        drawText(
            BIRD_X,
            birdY + 1,
            " /-o\\"
        );

        drawText(
            BIRD_X,
            birdY + 2,
            " \\_\\-/"
        );
    }

    // ========================================================
    // DRAW HURDLES
    // ========================================================

    void printHurdles() {

        for (int i = 0; i < HURDLE_COUNT; i++) {

            int hurdleX = hurdles[i].x;

            int gapTop = hurdles[i].gapTop;

            int gapBottom =
                gapTop + HURDLE_GAP - 1;

            for (
                int y = GAME_TOP;
                y < SCREEN_HEIGHT;
                y++
            ) {

                if (
                    y >= gapTop &&
                    y <= gapBottom
                ) {
                    continue;
                }

                string obstacle = "|||";

                if (
                    y == gapTop - 1 ||
                    y == gapBottom + 1
                ) {

                    obstacle = "===";
                }

                drawText(
                    hurdleX,
                    y,
                    obstacle
                );
            }
        }
    }

    // ========================================================
    // DRAW GROUND
    // ========================================================

    void printRoad() {

        drawText(
            0,
            SCREEN_HEIGHT,
            string(SCREEN_WIDTH, '_')
        );

        drawText(
            0,
            SCREEN_HEIGHT + 1,
            string(SCREEN_WIDTH, '/')
        );

        drawText(
            0,
            SCREEN_HEIGHT + 2,
            string(SCREEN_WIDTH, '=')
        );
    }

    // ========================================================
    // KEYBOARD INPUT
    // ========================================================

    void handleInput() {

        // Read a Spacebar press as one jump.
        // The key must be released before another jump.

        bool spaceDown =
            (GetAsyncKeyState(VK_SPACE) & 0x8000) != 0;

        if (
            spaceDown &&
            !previousSpaceState
        ) {

            birdY -= JUMP_STRENGTH;

            if (birdY < GAME_TOP) {
                birdY = GAME_TOP;
            }
        }

        previousSpaceState = spaceDown;
    }

    // ========================================================
    // APPLY GRAVITY
    // ========================================================

    void applyGravity() {

        frameCount++;

        if (
            frameCount % GRAVITY_INTERVAL == 0
        ) {

            birdY++;
        }
    }

    // ========================================================
    // MOVE HURDLES AND UPDATE SCORE
    // ========================================================

    void updateHurdles() {

        for (int i = 0; i < HURDLE_COUNT; i++) {

            hurdles[i].x--;

            int hurdleRight =
                hurdles[i].x + HURDLE_WIDTH - 1;

            // Count only successfully passed obstacles.

            if (
                !hurdles[i].scored &&
                hurdleRight < BIRD_X
            ) {

                score++;
                hurdles[i].scored = true;
            }

            // Recycle obstacle.

            if (hurdleRight < 0) {

                int furthestX = SCREEN_WIDTH;

                for (int j = 0; j < HURDLE_COUNT; j++) {

                    if (j != i) {

                        furthestX = max(
                            furthestX,
                            hurdles[j].x
                        );
                    }
                }

                hurdles[i].x =
                    furthestX + HURDLE_SPACING;

                hurdles[i].gapTop = randomGapTop();

                hurdles[i].scored = false;
            }
        }
    }

    // ========================================================
    // COLLISION DETECTION
    // ========================================================

    bool collisionCheck() {

        int birdBottom =
            birdY + BIRD_HEIGHT - 1;

        int birdRight =
            BIRD_X + BIRD_WIDTH - 1;

        // Screen boundaries.

        if (
            birdY < GAME_TOP ||
            birdBottom >= SCREEN_HEIGHT
        ) {

            return true;
        }

        // Obstacle collisions.

        for (int i = 0; i < HURDLE_COUNT; i++) {

            int hurdleX = hurdles[i].x;

            int hurdleRight =
                hurdleX + HURDLE_WIDTH - 1;

            int gapTop = hurdles[i].gapTop;

            int gapBottom =
                gapTop + HURDLE_GAP - 1;

            bool horizontalOverlap =
                birdRight >= hurdleX &&
                BIRD_X <= hurdleRight;

            bool verticalCollision =
                birdY < gapTop ||
                birdBottom > gapBottom;

            if (
                horizontalOverlap &&
                verticalCollision
            ) {

                return true;
            }
        }

        return false;
    }

    // ========================================================
    // RENDER COMPLETE FRAME
    // ========================================================

    void render() {

        clearFrame();

        printScore();
        printHurdles();
        printBird();
        printRoad();

        goToXY(0, 0);

        for (int y = 0; y < SCREEN_HEIGHT + 3; y++) {

            cout << screen[y];

            // Avoid scrolling after the final row.

            if (y < SCREEN_HEIGHT + 2) {
                cout << '\n';
            }
        }

        cout.flush();
    }

    // ========================================================
    // GAME OVER SCREEN
    // ========================================================

    void showGameOver() {

        clearFrame();

        printScore();
        printRoad();

        drawText(
            25,
            10,
            "=========================="
        );

        drawText(
            25,
            12,
            "        GAME OVER!"
        );

        drawText(
            25,
            14,
            "     Final Score: " +
                to_string(score)
        );

        drawText(
            25,
            16,
            "=========================="
        );

        drawText(
            18,
            19,
            "Do you want to play again? (Y/N): "
        );

        goToXY(0, 0);

        for (int y = 0; y < SCREEN_HEIGHT + 3; y++) {

            cout << screen[y];

            if (y < SCREEN_HEIGHT + 2) {
                cout << '\n';
            }
        }

        cout.flush();

        goToXY(53, 19);

        setCursorVisible(true);
    }

public:

    // ========================================================
    // CONSTRUCTOR
    // ========================================================

    Flappy_Bird() {

        initializeHurdles();
    }

    // ========================================================
    // GAME LOOP
    // ========================================================

    void play() {

        system("cls");

        configureConsole();

        setCursorVisible(false);

        gameOver = false;

        while (!gameOver) {

            handleInput();

            applyGravity();

            updateHurdles();

            gameOver = collisionCheck();

            render();

            if (!gameOver) {
                Sleep(FRAME_DELAY);
            }
        }

        showGameOver();
    }
};

// ============================================================
// MAIN FUNCTION
// ============================================================

int main() {

    srand(
        static_cast<unsigned int>(time(nullptr))
    );

    while (true) {

        Flappy_Bird game;

        game.play();

        char choice;

        cin >> choice;

        if (
            choice == 'N' ||
            choice == 'n'
        ) {

            break;
        }

        if (
            choice != 'Y' &&
            choice != 'y'
        ) {

            break;
        }
    }

    system("cls");

    cout << "Thanks for playing Flappy Bird!" << endl;

    return 0;
}
