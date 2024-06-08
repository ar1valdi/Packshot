#pragma once
#include <iostream>
using namespace std;

enum ActionCode {
	MOVE_LEFT,
	MOVE_RIGHT,
	MOVE_UP,
	MOVE_DOWN,
	ATTACK
};

class Action {
public:
	ActionCode actionCode;
	string serialize();
	static Action deserialize(string ser);
};