#pragma once
#include <sstream>
using namespace std;

struct Position {
	int x;
	int y;
	Position() {
		this->x = 0;
		this->y = 0;
	}
	Position(int x, int y) {
		this->x = x;
		this->y = y;
	}
	string serialize() {
		stringstream ss;
		ss << x << " " << y;
		return ss.str();
	}
	static Position deserialize(const string& ser) {
		stringstream ss(ser);
		int sx, sy;
		ss >> sx;
		ss >> sy;
		return Position(sx, sy);
	}
};