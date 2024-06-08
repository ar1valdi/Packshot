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

	bool operator==(const Position& other) const {
		return (x == other.x && y == other.y);
	}

	bool operator!=(const Position& other) const {
		return (x != other.x || y != other.y);
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

		if (ss.fail()) {
			throw runtime_error("Failed to deserialize Position");
		}

		return Position(sx, sy);
	}
};