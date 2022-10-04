#pragma once
#include "JobQueue.h"

class Room : public JobQueue
{
	friend class EnterJob;
	friend class LeaveJob;
	friend class BroadCastJob;

public:
	
	void	Broadcast(SendBufferRef sendBuffer);

public:
	Vector<GameSessionRef>	_sessions;
};
