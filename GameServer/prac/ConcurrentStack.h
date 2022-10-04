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

//// popCount ���
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
//		// �����ͺи� -> countüũ �����̸� oldHead�� �и��Ǿ�����
//		if (_popCount == 1)
//		{
//			Node* node = _pendingList.exchange(nullptr);
//
//			// �����̸� ����� �ٸ� �����尡 ���� -> pending�Ǿ��ִ� ���鵵 �Բ� ����
//			if (--_popCount == 0)
//			{
//				DeleteNodes(node);
//			}
//			else if (node)
//			{
//				ChainPendingNodeList(node);
//			}
//
//			// �� �����ʹ� ����
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
//	atomic<uint32> _popCount = 0; //Pop�� �������� ������ ����
//	atomic<Node*> _pendingList; // ���� ����� ����
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
			// ������ ȹ�� (externalCount�� ������ ���� +1 �� �ְ� �̱�)
			IncreaseHeadCount(oldHead);

			// �ּ��� externalCount >= 2 �λ���(�ش� �����Ϳ� ������ �����尡 ����)�� �����ϸ� �ȵ�
			Node* ptr = oldHead.ptr;

			if (ptr == nullptr)
				return shared_ptr<T>();
			
			// ������ ȹ�� (�ٸ� �����尡 �������� ȹ���ϱ� ������ ������ �����尡 ���)
			if (_head.compare_exchange_strong(oldHead, ptr->next))
			{
				shared_ptr<T> res;
				res.swap(ptr->data);
				// countincrease���� �ʱⰪ(1)�� �ڱ��ڽ�(1) ���ؼ� �� 2�� ������ �� -> �ش� ptr�� ������ �ڽ� ���� ������ ������ ������ �ǹ�
				const int32 countincrease = oldHead.externalCount - 2;

				// �������� ȹ���� �����尡 ������ �������� �� ���ǹ��� ����ϸ鼭 ptr delete
				if (ptr->internalCount.fetch_add(countincrease) == -countincrease)
					delete ptr;

				return res;
			}
			// �������� ȹ����� ��������� internalCount�� 1�� ���ҽ�Ű�ٰ� ������ ������� ptr�� delete
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
