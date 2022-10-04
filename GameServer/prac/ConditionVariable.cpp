#include "pch.h"
#include <iostream>
#include "CorePch.h"

#include <thread>
#include <atomic>
#include <mutex>
#include <windows.h>
#include <future>

mutex m;
queue<int32> q;
HANDLE handle;

condition_variable cv;

void Producer()
{
	while (true)
	{
		{
			unique_lock<mutex> lock(m);
			q.push(100);
		}
		cv.notify_one();
		//this_thread::sleep_for(100ms);
	}

}

void Consumer()
{
	while (true)
	{
		unique_lock<mutex> lock(m);
		cv.wait(lock, []() { return !q.empty(); });
		{
			int32 data = q.front();
			q.pop();
			cout << q.size() << endl;
		}
	}
}
//
//int main()
//{
//	thread t1(Producer);
//	thread t2(Consumer);
//
//	t1.join();
//	t2.join();
//
//}