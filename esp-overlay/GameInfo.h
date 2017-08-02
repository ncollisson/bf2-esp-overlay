#pragma once
#include "d3dx9.h"

struct PlayerInfo
{
	unsigned int team;
	D3DXVECTOR3 position;
	unsigned int vehicleClass;
	unsigned int isAlive;
	unsigned int isManDown;
};

struct GameInfo
{
	// my camera matrix
	// my player info
	// player info: coords, team, vehicle
	D3DXMATRIX myCamera;
	float myZoom;
	size_t numberOfPlayers;
	PlayerInfo playerInfo[128];
};