// UI.cpp

#include <iostream>

class UI {
public:
    void displayMenu() {
        std::cout << "1. Start Game\n";
        std::cout << "2. Load Game\n";
        std::cout << "3. Exit\n";
    }
};

int main() {
    UI ui;
    ui.displayMenu();
    return 0;
}