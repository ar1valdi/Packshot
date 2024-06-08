#pragma once
#include "Position.h"
using namespace std;

class Flag {
public:
	Position position;
	double captureTime;
	int capturingTimer;
	int ownerID;
public:
	static const int fieldsToSerialize = 5;
	Flag();
	Flag(Position pos, double cTime, double cTimer, int owner)
		: position(pos), captureTime(cTime), capturingTimer(cTimer), ownerID(owner) {}

	string serialize();
	static Flag deserialize(const string& ser);
};