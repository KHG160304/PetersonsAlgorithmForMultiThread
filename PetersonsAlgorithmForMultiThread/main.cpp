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

SRWLOCK lock = RTL_SRWLOCK_INIT;
bool flag[2] = { false, false };
char turn;
int tmpCount1;
int tmpCount2;
unsigned long count = 0;

int thread1After2Cnt = 0;
int thread2After1Cnt = 0;

int thread1Stat = 0;
int thread2Stat = 0;

bool srwlockFlag1 = false;
bool srwlockFlag2 = false;

int main()
{
	Init();

	DWORD result = WaitForMultipleObjects(TOTAL_THREAD_CNT, hThreadArr, true, INFINITE);
	printf("count: %d\n", count);
	printf("tmpCount1: %d\n", tmpCount1);
	printf("tmpCount2: %d\n", tmpCount2);

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
		flag[0] = true;
		turn = 0;
		while (flag[1] == true && turn == 0);
		//AcquireSRWLockExclusive(&lock);
		srwlockFlag1 = true;
		if (srwlockFlag2 == false)/*상대가 임계점 진입 안했음*/
		{
			if (srwlockFlag2 == true)/*상대가 임계점 진입 함*/
			{
				++tmpCount1;
			}
			//++count;
			InterlockedIncrement((long*)&count);
		}
		srwlockFlag1 = false;
		flag[0] = false;
		//ReleaseSRWLockExclusive(&lock);
	}

	return 0;
}

unsigned _stdcall WorkerThread2(void* args)
{
	for (int i = 0; i < 100000000; ++i)
	{
		flag[1] = true;
		turn = 1;
		while (flag[0] == true && turn == 1);
		//AcquireSRWLockExclusive(&lock);
		srwlockFlag2 = true;
		if (srwlockFlag1 == false)/*상대가 임계점 진입 안했음*/
		{
			if (srwlockFlag1 == true)/*상대가 임계점 진입 함*/
			{
				++tmpCount2;
			}
			//++count;
			InterlockedIncrement((long*)&count);
		}
		srwlockFlag2 = false;
		flag[1] = false;
		//ReleaseSRWLockExclusive(&lock);
	}

	return 0;
}