#include "pch.h"
#include <iostream>
#include "CorePch.h"

#include <thread>
#include <atomic>
#include <mutex>
#include <windows.h>
#include <future>

int64 result;

int64 Calculate()
{
	int64 sum = 0;

	for (int32 i = 0; i < 100'000; ++i)
		sum += i;

	return sum;
}

void PromiseWorker(std::promise<string>&& promise)
{
	promise.set_value("Secre Message");
}

void TaskWorker(std::packaged_task<int64(void)>&& task)
{
	task();
}

int main()
{
	// std::future
	{
		// 1) deferred -> lazy evaluation 지연해서 실행하세요
		// 2) async -> 별도의 쓰레드를 만들어서 실행하세요
		// 3) deferred | async -> 둘중 알아서 골라주세요

		// 언젠가 미래에 결과물을 사용
		std::future<int64> future = std::async(std::launch::async, Calculate);

		// TODO
		// future 상태를 확인
		std::future_status status = future.wait_for(1ms);
		if (status == std::future_status::ready)
		{
			// TODO
		}
		int64 sum = future.get();
	}
	{
		// 클래스 멤버 함수 호출방법
		class Knight
		{
		public:
			int64 GetHp() { return 100; }
		};
		// 객체를 우선 생성
		Knight knight;
		std::future<int64> future2 = std::async(std::launch::async, &Knight::GetHp, knight);
	}

	// std::promise
	{
		// 미래(std::future)에 결과물을 반환해줄거라 약속(std::promise)
		std::promise<string> promise;
		std::future<string> future = promise.get_future();

		thread t(PromiseWorker, std::move(promise));

		string message = future.get();
		cout << message << endl;

		t.join();
	}

	// std::packaged_task
	{
		std::packaged_task<int64(void)> task(Calculate);
		std::future<int64> future = task.get_future();

		std::thread t(TaskWorker, std::move(task));

		int64 sum = future.get();
		cout << sum << endl;

		t.join();
	}

}