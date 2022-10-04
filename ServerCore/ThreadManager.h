#pragma once

#include <thread>
#include <functional>


/*-------------------
* 
*	ThreadManager
* 
-------------------*/
class ThreadManager
{
public:
	ThreadManager();
	~ThreadManager();

	void	Launch(function<void(void)> callback);
	void	Join();

	static void	initTLS();
	static void	DestroyTLS();

	static void	DoGlobalQueueWork();
	static void DistributeReservedJobs();

private:
	Mutex			_lock;
	vector<thread>	_threads;

};

