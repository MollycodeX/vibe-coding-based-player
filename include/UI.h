// UI.h

#ifndef UI_H
#define UI_H

class UI {
public:
    virtual void draw() = 0;  // Pure virtual function
    virtual void update() = 0; // Pure virtual function
};

#endif // UI_H