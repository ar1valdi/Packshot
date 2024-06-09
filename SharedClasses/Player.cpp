#include "Player.h"

#include <sstream>
using namespace std;

string Player::serialize() {
	stringstream ss;
	ss << id << " " << name << " " << score << " " << deathTimer << " " 
		<< isAlive << " " << attackRange << " " << position.serialize() << " "
		<< spawnPoint.serialize() << " " << numOfFlags;
	return ss.str();
}
Player Player::deserialize(const string& ser) {
	std::string sname, temp1, temp2;
	int sid, sscore, sattackRange, snumOfFlags;
	double sdeathTimer;
	bool sisAlive;
	Position sposition, sspawnPoint;
	stringstream ss(ser);

	ss >> sid;
	ss >> sname;
	ss >> sscore;
	ss >> sdeathTimer;
	ss >> sisAlive;
	ss >> sattackRange;
	ss >> temp1 >> temp2;
	sposition = Position::deserialize(temp1 + " " + temp2);
	ss >> temp1 >> temp2;
	sspawnPoint = Position::deserialize(temp1 + " " + temp2);
	ss >> snumOfFlags;
	Player p(sid, sname, sscore, sdeathTimer, sisAlive, sattackRange, sposition, sspawnPoint, snumOfFlags);

	if (ss.fail()) {
		throw runtime_error("Failed to deserialize Player");
	}

	return p;
}