#define __SOCKETAPI__
#include <windows.h>
#include <process.h>
#include <stdio.h>
#include <synchapi.h>

#define TOTAL_THREAD_CNT	2

HANDLE hThreadArr[TOTAL_THREAD_CNT];

void Init(void);
void CleanUp(void);

unsigned _stdcall WorkerThread1(void* args);
unsigned _stdcall WorkerThread2(void* args);

long gSpinLock = 0;

SRWLOCK lock = RTL_SRWLOCK_INIT;
int flag[2] = { false, false };
char turn;

unsigned long count = 0;

int thread1After2Cnt = 0;
int thread2After1Cnt = 0;

int thread1Stat = 0;
int thread2Stat = 0;

bool srwlockFlag1 = false;
bool srwlockFlag2 = false;

int otherFlag1;
int otherFlag2;

int tmpCount1;
int tmpCount2;

//#define SPIN_LOCK
#define PETERSON

int main()
{
	Init();

	DWORD result = WaitForMultipleObjects(TOTAL_THREAD_CNT, hThreadArr, true, INFINITE);
	printf("count: %d\n", count);
	printf("thread1After2Cnt: %d\n", thread1After2Cnt);
	printf("thread2After1Cnt: %d\n", thread2After1Cnt);

	CleanUp();
	return 0;
}

void Init(void)
{
	hThreadArr[0] = (HANDLE)_beginthreadex(nullptr, 0, WorkerThread1, nullptr, 0, nullptr);
	hThreadArr[1] = (HANDLE)_beginthreadex(nullptr, 0, WorkerThread2, nullptr, 0, nullptr);
}

void CleanUp(void)
{
	CloseHandle(hThreadArr[0]);
	CloseHandle(hThreadArr[1]);
}

unsigned _stdcall WorkerThread1(void* args)
{
	for (int i = 0; i < 100000000; ++i)
	{
#ifdef PETERSON
		_InterlockedExchange((long*)&thread1Stat, 1);
		flag[0] = true;
		turn = 0;		
		while (flag[1] == true 
			&& (tmpCount1 = _InterlockedCompareExchange((long*)(flag + 1), false, false)) == true
			&& turn == 0);
		_InterlockedExchange((long*)&thread1Stat, 2);
		while ((_InterlockedCompareExchange((long*)&thread2Stat, 2, 2) == 2 || _InterlockedCompareExchange((long*)&thread2Stat, 3, 3) == 3) 
			&& tmpCount1 == false && tmpCount2 == false)
		{
			InterlockedIncrement((long*)&thread2After1Cnt);
		}
		++count;
		_InterlockedExchange((long*)&thread1Stat, 3);
		flag[0] = false;
		_InterlockedExchange((long*)&thread1Stat, 4);
#endif // PETERSON
#ifdef SPIN_LOCK
		while (InterlockedExchange((long*)&gSpinLock, 1) != 0);
		++count;
		InterlockedExchange((long*)&gSpinLock, 0);
#endif // SPIN_LOCK
	}

	return 0;
}

unsigned _stdcall WorkerThread2(void* args)
{
	for (int i = 0; i < 100000000; ++i)
	{
#ifdef PETERSON
		_InterlockedExchange((long*)&thread2Stat, 1);
		flag[1] = true;
		turn = 1;
		/*
			flag[0] 값을 미리 로드 했다.
			미리 로드된 값은 false 이다.
		*/
		while (flag[0] == true 
			&& (tmpCount2 = _InterlockedCompareExchange((long*)flag, false, false)) == true
			&& turn == 1);
		_InterlockedExchange((long*)&thread2Stat, 2);
		while ((_InterlockedCompareExchange((long*)&thread1Stat, 2, 2) == 2 || _InterlockedCompareExchange((long*)&thread1Stat, 3, 3) == 3)
			&& tmpCount2 == false && tmpCount1 == false) // 
		{
			InterlockedIncrement((long*)&thread2After1Cnt);
		}
		++count;
		_InterlockedExchange((long*)&thread2Stat, 3);
		flag[1] = false;
		_InterlockedExchange((long*)&thread2Stat, 4);
#endif // PETERSON
#ifdef SPIN_LOCK
		while (InterlockedExchange((long*)&gSpinLock, 1) != 0);
		++count;
		InterlockedExchange((long*)&gSpinLock, 0);
#endif // SPIN_LOCK
	}

	return 0;
}