#pragma once

#include <mutex>

template<typename T>
class LockStack
{
public:
	LockStack() { }

	LockStack(const LockStack&) = delete;
	LockStack& operator=(const LockStack&) = delete;

	void Push(T value)
	{
		lock_guard<mutex> lock(_mutex);
		_stack.push(std::move(value));
		_condVar.notify_one();
	}

	bool TryPop(T& value)
	{
		lock_guard<mutex> lock(_mutex);
		if (_stack.empty())
			return false;

		value = std::move(_stack.top());
		_stack.pop();
		return true;
	}

	bool Empty()
	{
		lock_guard<mutex> lock(_mutex);
		return _stack.empty();
	}

	void WaitPop(T& value)
	{
		unique_lock<mutex> lock(_mutex);
		_condVar.wait(lock, [this] { return _stack.empty() == false; });
		value = std::move(_stack.top());
		_stack.pop();
	}

private:
	stack<T> _stack;
	mutex _mutex;
	condition_variable _condVar;
};

//// popCount 기반
//template<typename T>
//class LockFreeStack
//{
//	struct Node
//	{
//		Node(const T& value) : data(value), next(nullptr)
//		{
//
//		}
//
//		T data;
//		Node* next;
//	};
//
//public:
//	void Push(const T& value)
//	{
//		Node* node = new Node(value);
//		node->next = _head;
//
//		while (_head.compare_exchange_weak(node->next, node) == false)
//		{
//
//		}
//	}
//
//	bool TryPop(T& value)
//	{
//		++_popCount;
//
//		Node* oldHead = _head;
//
//		while (oldHead && _head.compare_exchange_weak(oldHead, oldHead->next) == false)
//		{
//		}
//
//		if (oldHead == nullptr)
//		{
//			--_popCount;
//			return false;
//		}
//
//		value = oldHead->data;
//		TryDelete(oldHead);
//
//		return true;
//	}
//
//	void TryDelete(Node* oldHead)
//	{
//		// 데이터분리 -> count체크 성공이면 oldHead는 분리되어있음
//		if (_popCount == 1)
//		{
//			Node* node = _pendingList.exchange(nullptr);
//
//			// 성공이면 끼어든 다른 쓰레드가 없음 -> pending되어있던 노드들도 함께 삭제
//			if (--_popCount == 0)
//			{
//				DeleteNodes(node);
//			}
//			else if (node)
//			{
//				ChainPendingNodeList(node);
//			}
//
//			// 내 데이터는 삭제
//			delete oldHead;
//		}
//		else
//		{
//			ChainPendingNode(oldHead);
//			--_popCount;
//		}
//	}
//
//	void ChainPendingNodeList(Node* first, Node* last)
//	{
//		last->next = _pendingList;
//
//		while (_pendingList.compare_exchange_weak(last->next, first) == false)
//		{
//		}
//	}
//
//	void ChainPendingNodeList(Node* node)
//	{
//		Node* last = node;
//		while (last->next)
//			last = last->next;
//		ChainPendingNodeList(node, last);
//	}
//
//	void ChainPendingNode(Node* node)
//	{
//		ChainPendingNodeList(node, node);
//	}
//
//	static void DeleteNodes(Node* node)
//	{
//		while (node)
//		{
//			Node* next = node->next;
//			delete node;
//			node = next;
//		}
//	}
//private:
//	atomic<Node*> _head;
//
//	atomic<uint32> _popCount = 0; //Pop을 실행중인 쓰레드 개수
//	atomic<Node*> _pendingList; // 삭제 대기중 노드들
//};

 
template<typename T>
class LockFreeStack
{
	struct Node;

	struct CountedNodePtr
	{
		int32 externalCount = 0;
		Node* ptr = nullptr;
	};

	struct Node
	{
		Node(const T& value) : data(make_shared<T>(value))
		{

		}

		shared_ptr<T> data;
		atomic<int32> internalCount = 0;
		CountedNodePtr next;
	};

public:
	void Push(const T& value)
	{
		CountedNodePtr node;
		node.ptr = new Node(value);
		node.externalCount = 1;
		
		node.ptr->next = _head;
		while (_head.compare_exchange_weak(node.ptr->next, node) == false)
		{
		}
	}

	shared_ptr<T> TryPop()
	{
		CountedNodePtr oldHead = _head;
		while (true)
		{
			// 참조권 획득 (externalCount를 현시점 기준 +1 한 애가 이김)
			IncreaseHeadCount(oldHead);

			// 최소한 externalCount >= 2 인상태(해당 포인터에 접근한 스레드가 존재)라서 삭제하면 안됨
			Node* ptr = oldHead.ptr;

			if (ptr == nullptr)
				return shared_ptr<T>();
			
			// 소유권 획득 (다른 스레드가 참조권을 획득하기 이전에 도달한 스레드가 통과)
			if (_head.compare_exchange_strong(oldHead, ptr->next))
			{
				shared_ptr<T> res;
				res.swap(ptr->data);
				// countincrease값은 초기값(1)과 자기자신(1) 합해서 총 2를 제외한 값 -> 해당 ptr에 접근한 자신 제외 나머지 스레드 갯수를 의미
				const int32 countincrease = oldHead.externalCount - 2;

				// 소유권을 획득한 쓰레드가 마지막 쓰레드일 때 조건문을 통과하면서 ptr delete
				if (ptr->internalCount.fetch_add(countincrease) == -countincrease)
					delete ptr;

				return res;
			}
			// 소유권을 획득못한 스레드들은 internalCount를 1씩 감소시키다가 마지막 쓰레드는 ptr을 delete
			else if (ptr->internalCount.fetch_sub(1) == 1)
			{
				delete ptr;
			}
		}
	}

private:

	void IncreaseHeadCount(CountedNodePtr& oldCounter)
	{
		while (true)
		{
			CountedNodePtr newCounter = oldCounter;
			newCounter.externalCount++;

			if (_head.compare_exchange_strong(oldCounter, newCounter))
			{
				oldCounter.externalCount = newCounter.externalCount;
				break;
			}
		}
	}

	atomic<CountedNodePtr> _head;
};
