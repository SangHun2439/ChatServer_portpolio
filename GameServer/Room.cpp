#include "pch.h"
#include "Room.h"
#include "GameSession.h"

void Room::Broadcast(SendBufferRef sendBuffer)
{
	for (auto& session : _sessions)
	{
		session->Send(sendBuffer);
	}
}
