#include <iostream>
#include <conio.h>   // For _getch() function
#include <windows.h> // For Sleep() function

int main() {
    int position = 0;
    bool isJumping = false;

    while (true) {
        // Check if a key is pressed
        if (_kbhit()) {
            char key = _getch();
            if (key == ' ' && !isJumping) {
                isJumping = true;
                position = 0;
            }
        }

        // Jumping animation
        if (isJumping) {
            position++;
            if (position <= 10) {
                // Print Mario ascending
                std::cout << "Mario Jumping: ";
                for (int i = 0; i < position; i++) {
                    std::cout << "*";
                }
                std::cout << std::endl;
            } else {
                // Print Mario descending
                std::cout << "Mario Descending: ";
                for (int i = 10; i > position - 10; i--) {
                    std::cout << "*";
                }
                std::cout << std::endl;

                // Reset jump state
                isJumping = false;
            }
        } else {
            // Print Mario standing
            std::cout << "Mario Standing: *";
            std::cout << std::endl;
        }

        // Delay for smoother animation
        // Adjust the value (in milliseconds) to modify the speed of animation
        Sleep(50);

        // Clear console screen
        system("cls");
    }

    return 0;
}
