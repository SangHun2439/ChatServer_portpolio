#pragma once
#include "GameSessionManager.h"
#include "Room.h"

extern GameSessionManager*	GSessionManager;
extern shared_ptr<Room>		GRoom;

class ServerGlobal
{
public:
	static void Init();
};