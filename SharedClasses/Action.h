#pragma once
#include <iostream>
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
	string serialize() {
		return "";
	};
	static Action deserialize(const string& ser) { return Action(); };
};