#include "pch.h"
#include "ServerGlobal.h"

GameSessionManager* GSessionManager = nullptr;
shared_ptr<Room> GRoom = nullptr;

void ServerGlobal::Init()
{
	GSessionManager = new GameSessionManager();
	GRoom = MakeShared<Room>();
}
