#pragma once

struct KeysActive {
    bool q {false};
    bool w {false};
    bool e {false};
    bool a {false};
    bool s {false};
    bool d {false};

    bool shift {false}; // left shift key

    // Use arrow keys, because couldn't use the mouse yet
    bool up {false};
    bool down {false};
    bool left {false};
    bool right {false};
};

/*
struct MouseInput {
    double dx {0.0};
    double dy {0.0};
};
*/

class IInputListener {
public:
    virtual KeysActive const & getKeysActive() const = 0;
    //virtual MouseInput const & getMouseInput() const = 0;
};