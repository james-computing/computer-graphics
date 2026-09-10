#pragma once

struct KeysActive {
    bool q {false};
    bool w {false};
    bool e {false};
    bool a {false};
    bool s {false};
    bool d {false};
};

class IInputListener {
public:
    virtual bool getKeyActive(char const c) const = 0;
};