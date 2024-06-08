#include "Game.h"

string Game::getSerializedGameState() {
	return gs.serialize(1);
}
GameState Game::handleIngameRequest(Action a) {
	GameState gs;
	switch (a.actionCode) {
	case ActionCode::MOVE_UP:
		gs.timer = 1;
		break;
	case ActionCode::MOVE_DOWN:
		gs.timer = 2;
		break;
	case ActionCode::MOVE_RIGHT:
		gs.timer = 3;
		break;
	case ActionCode::MOVE_LEFT:
		gs.timer = 4;
		break;
	}
	return gs;
}