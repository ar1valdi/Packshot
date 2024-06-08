#pragma once
#include "Position.h"

class Flag {
	Position flagPos;
	double captureTime;
	double capturingTimer;
	int ownerID;
public:
	string serialize();
	static Flag deserialize();
};