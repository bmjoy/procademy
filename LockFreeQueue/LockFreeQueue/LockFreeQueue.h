#pragma once

#include "CrashDump.h"

#include <Windows.h>
#include "MemoryPool.h"

//�ֱ� 50000���� ����Ѵ�
//�Ǿ� ����(16����) 1�϶� dequeue
//�Ǿ� ����(16����) 0�϶� enqueue
#define TRACK_MAX 100000

void *track[TRACK_MAX];
unsigned long long trackCur = 0;

template <class T>
class LockFreeQueue
{
private:
	struct NODE
	{
		NODE(T data)
		{
			next = NULL;
			item = data;
		}

		NODE()
		{
			next = NULL;
		}

		T item;
		NODE *next;
	};

	struct END_NODE
	{
		END_NODE()
		{
			node = NULL;// new NODE;
			//node=new NODE();
			//node->next = node;
			check = 0;
		}

		NODE *node;
		long long check;
	};
public:
	LockFreeQueue();
	bool Enqueue(T data);
	bool Dequeue(T *data = NULL);
	unsigned long long GetUseCount() { return _useCount; }
private:
	END_NODE * volatile _head;
	END_NODE * volatile _tail;

	unsigned long long _headCheckNum;
	unsigned long long _tailCheckNum;

	int _useCount;
	int _maxCount;

	MemoryPool<NODE> queuePool;
};

template<class T>
LockFreeQueue<T>::LockFreeQueue()
	:_headCheckNum(0), _tailCheckNum(0), _useCount(0),_maxCount(0)
{
	_head = new END_NODE;
	_head->node = new NODE;
	_tail = new END_NODE;
	_tail->node = _head->node;
}

template<class T>
bool LockFreeQueue<T>::Enqueue(T data)
{
	NODE *newNode = queuePool.Alloc();

	//������
	ULONG trackTemp = InterlockedIncrement((LONG *)&trackCur);
	InterlockedExchange64((LONG64 *)&track[trackTemp % TRACK_MAX], (LONG64)newNode);
	//������


	if (newNode == NULL)
		return false;

	newNode->item = data;
	newNode->next = NULL;

	
	unsigned long long checkNum = InterlockedIncrement64((LONG64 *)&_tailCheckNum);

	

	while (1)
	{
		END_NODE tail;// = _tail;
		tail.check = _tail->check;
		tail.node = _tail->node;
		NODE *next = tail.node->next;

		if (next == NULL)
		{
			//tail�� next�� �״�� NULL�� �� ����
			if (InterlockedCompareExchangePointer((PVOID *)&tail.node->next, newNode, next) == NULL)
			{
				//_tail�� ���ο� node�� ��
				//�����Ѵٸ� �� ���������� _tail�� �̹� ������� => ����� ������ _tail�� ������ ���� ����Ѵ�
				//���� interlock���� ���������� ���ư��⶧���� _tail�ڿ��� �̹� ���ο� node�� ������ ���ɼ��� ����
				//(������ _tail�� ������ ���� tail�� ����Ű�Ƿ� _tail�� next�� null�� �ƴϰԵ�)
				InterlockedCompareExchange128((LONG64 *)_tail, checkNum, (LONG64)tail.node->next, (LONG64 *)&tail);

				break;
			}
		}
		//tail�� next�� NULL�� �ƴ� ��� tail�� �Ű������
		//�� �̰�쿡�� _tail�� atomic�ϰ� ����Ǿ����
		else
		{
			if (InterlockedCompareExchange128((LONG64 *)_tail, checkNum, (LONG64)tail.node->next, (LONG64 *)&tail))
			{
				volatile int test = 1;
				NODE *temp = _tail->node;
				if (temp->next == temp)
				{
					CrashDump::Crash();
				}
			}
			checkNum = InterlockedIncrement64((LONG64 *)&_tailCheckNum);//_tail�� ����ʿ� ���� checkNum�� ����
		}
	}

	InterlockedIncrement((LONG *)&_useCount);

	return true;
}

template<class T>
bool LockFreeQueue<T>::Dequeue(T *data)
{
	//isEmpty�� atomic�� �� �ְ�
	if (InterlockedDecrement((LONG *)&_useCount) < 0)
	{
		InterlockedIncrement((LONG *)&_useCount);
		return false;
	}

	END_NODE h;
	//NODE *newHead = NULL;
	T popData=NULL;
	unsigned long long checkNum = InterlockedIncrement64((LONG64 *)&_headCheckNum);//�� pop������ checkNum�� �Լ� ���� �ÿ� ����

	while (1)
	{
		//tail�� �и��� �ʾ��� �� ��� head�� ������ �� ��� tail�� ���ǵ� �� �ִ�.
		END_NODE tail;// = _tail;
		tail.check = _tail->check;
		tail.node = _tail->node;
		if (tail.node->next != NULL)
		{
			unsigned long long tailCheckNum = InterlockedIncrement64((LONG64 *)&_tailCheckNum);//_tail�� ����ʿ� ���� checkNum�� ����
			if (InterlockedCompareExchange128((LONG64 *)_tail, tailCheckNum, (LONG64)tail.node->next, (LONG64 *)&tail))
			{
				volatile int test = 1;
				NODE *temp = _tail->node;
				if (temp->next == temp)
				{
					CrashDump::Crash();
				}
			}
		}

		//END_NODE h;
		h.check = _head->check;
		h.node = _head->node;
		NODE *next = h.node->next;

		if(next!=NULL)
			popData = next->item;
		if (InterlockedCompareExchange128((LONG64 *)_head, checkNum, (LONG64)next, (LONG64 *)&h))
		{
			break;
		}
	}

	/*
	while (!InterlockedCompareExchange128((LONG64 *)_head,checkNum,(LONG64)_head->node->next,(LONG64 *)&h))
	{
		//tail�� �и��� �ʾ��� �� ��� head�� ������ �� ��� tail�� ���ǵ� �� �ִ�.
		END_NODE tail;// = _tail;
		tail.check = _tail->check;
		tail.node = _tail->node;
		if (tail.node->next != NULL)
		{
			unsigned long long tailCheckNum = InterlockedIncrement64((LONG64 *)&_tailCheckNum);//_tail�� ����ʿ� ���� checkNum�� ����
			if (InterlockedCompareExchange128((LONG64 *)_tail, tailCheckNum, (LONG64)tail.node->next, (LONG64 *)&tail))
			{
				volatile int test = 1;
				NODE *temp = _tail->node;
				if (temp->next == temp)
				{
					CrashDump::Crash();
				}
			}
		}

		newHead = h.node->next;

		//newHead�� null�� ���� _head�� ������� �����̴�. => useCount�� 0�ε� dequeue�� ��쿣 while�� ������ ������ �ȵ�
		//���� newHead�� null�� ��� �ڿ������� _head�� h�� ��ġ�� �� ����(_head�� �̹� �ٲ�����Ƿ�) while������ ���Եȴ�.
		//newHead�� null�� ��쿡�� _head�� h�� �������� �ʴٸ� _head�� ����� ���̹Ƿ� popData�� �߸��Ǽ� ��ȯ�� ���ɼ��� ����
		if(newHead!=NULL)
			popData = newHead->item;
	}
	*/
	//������
	ULONG trackTemp = InterlockedIncrement((LONG *)&trackCur);
	InterlockedExchange64((LONG64 *)&track[trackTemp % TRACK_MAX], (LONG64)h.node | 0x1000000000000000);
	//������

	queuePool.Free(h.node);

	if (data != NULL)
		*data = popData;

	return true;
}