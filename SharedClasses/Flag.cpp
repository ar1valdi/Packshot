#include "Flag.h"
#include <sstream>

Flag::Flag() {
	flagPos = Position();
	captureTime = 0;
	capturingTimer = 0;
	ownerID = -1;
}
string Flag::serialize() {
	stringstream ss;
	ss << flagPos.serialize() << " ";
	ss << captureTime << " ";
	ss << capturingTimer << " ";
	ss << ownerID;
	return ss.str();
}
Flag Flag::deserialize(const string& ser) {
	stringstream ss(ser);
	string temp1, temp2;
	Position sflagPos;
	double scTime, scTimer;
	int sownerID;
	ss >> temp1;
	ss >> temp2;
	ss >> scTime;
	ss >> scTimer;
	ss >> sownerID;
	sflagPos = Position::deserialize(temp1 + " " + temp2);

	if (ss.fail()) {
		throw runtime_error("Failed to deserialize Flag");
	}

	return Flag(sflagPos, scTime, scTimer, sownerID);
}