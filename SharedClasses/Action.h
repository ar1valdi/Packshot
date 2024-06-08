#pragma once
#include <iostream>
#include <sstream>
using namespace std;

enum ActionCode {
	MOVE_LEFT,
	MOVE_RIGHT,
	MOVE_UP,
	MOVE_DOWN,
	ATTACK,
	READY_IN_LOBBY,
	NOT_READY_IN_LOBBY,
	PRESENT
};

class Action {
public:
	ActionCode actionCode;
    inline string serialize() {
        stringstream ss;
        ss << actionCode;
        return ss.str();
    }
    inline static Action deserialize(const string& ser) {
        Action action;
        stringstream ss(ser);
        int code;
        ss >> code;
        action.actionCode = static_cast<ActionCode>(code);
        return action;
    }
};