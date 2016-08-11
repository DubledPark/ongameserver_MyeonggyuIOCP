#include "Thread.h"
#include <process.h>
#include <time.h>

Thread::Thread() : handle(INVALID_HANDLE_VALUE), identifier(0), isStarted(FALSE), runner(NULL)
{
	srand(time(0));
}

Thread::Thread(Runnable* r) : handle(INVALID_HANDLE_VALUE), identifier(0), isStarted(FALSE), runner(r)
{
	srand(time(0));
}

Thread::~Thread()
{
	terminated();
	if(INVALID_HANDLE_VALUE != handle)
		CloseHandle(handle);
}

unsigned __stdcall Thread::handleRunner(void* parameter)
//DWORD WINAPI Thread::handleRunner(LPVOID parameter)
{
	srand(time(0));
	//Thread* aThread = (Thread*)parameter;
	Thread* aThread = reinterpret_cast<Thread*>(parameter);
	if(aThread->runner)
		aThread->runner->run(aThread);
	else
		aThread->run();
	_endthreadex(0);
	return 0;
}

DWORD Thread::resume()
{	
	return ::ResumeThread(handle);
}

void Thread::run()
{
}

void Thread::sleep(long millis)
{
	Sleep(millis);
}

DWORD Thread::start()
{
	if(FALSE != isStarted)
		return ERROR_ALREADY_THREAD;

	if(INVALID_HANDLE_VALUE != handle)
		CloseHandle(handle);

	isStarted = TRUE;
	//handle = CreateThread(0, 0, Thread::handleRunner, (LPVOID)this, 0, &identifier);
	//handle = CreateThread(0, 0, Thread::handleRunner, reinterpret_cast<LPVOID>(this), 0, &identifier);
	handle = (HANDLE)::_beginthreadex(NULL, 0, &Thread::handleRunner, reinterpret_cast<void*>(this), 0, &identifier);
	return 0;
}

void Thread::stop()
{
	isStarted = FALSE;
}

DWORD Thread::suspend()
{
	return ::SuspendThread(handle);
}

void Thread::terminated()
{
	stop();
	TerminateThread(handle, 0);
}

bool Thread::waitFor()
{
	DWORD r;
	r = ::WaitForSingleObject(handle, INFINITE);
	if(r == WAIT_OBJECT_0)
		return true;
	return false;
}
