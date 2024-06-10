#pragma once
#define UP 'W'
#define DOWN 'S'
#define RIGHT 'D'
#define LEFT 'A'
#define NO_INPUT 0
#define ESCAPE VK_ESCAPE
#define ATTACK_MOVE 5
// BOTH
#define NOT_ENOUGHT_SLOTS "NS"
#define CONNECTED_CLIENT "CC"

#define FETCH_MSG "FETCH"
#define IP_ADDRESS "127.0.0.1"
#define PORT 12345
#define START_GAME "SG"
#define STILL_QUEUE "SQ"
#define FETCH_QUEUE "FQ"

// SERVER
#define SERVER_RECV_BUF 1024
#define MAX_CLIENTS_NUM 4
#define CLIENT_CUTOFF_TIME 500000

// CLIENT
#define CLIENT_RECV_BUF 1024

#define ATTACK_RANGE 1

#define DEATH_COOLDOWN 5000
#define KILL_POINT_GAIN 10
#define FLAG_POINT_GAIN 1

#define GAME_UPDATE_RATE 10
