#include "pch.h"
#include <iostream>
#include "CorePch.h"

#include <thread>
#include <atomic>
#include <mutex>
#include <windows.h>
#include <future>

thread_local int32 LThreadid = 0;

void ThreadMain(int32 threadid)
{
	LThreadid = threadid;

	while (true)
	{
		cout << "I am Thread " << LThreadid << endl;
		this_thread::sleep_for(1s);
	}
}

int main()
{
	vector<thread> threads;

	for (int32 i = 0; i < 10; ++i)
	{
		int32 threadid = i + 1;
		threads.push_back(thread(ThreadMain, threadid));
	}

	for (thread& t : threads)
		t.join();
}