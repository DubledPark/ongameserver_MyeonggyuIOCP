#include <windows.h>
#include "DataBuffer.h"
#include "Synchronized.h"
#include "maths.h"
#include "Queue.h"

void CT_DataBuffer()
{
	struct samp
	{
		int a;
		int b;
		short sa;
	};

	Queue<int> sampqueue;

	DataBufferT<1024> dbt;

	int& ia = dbt.addTypeAndGetRefers<int>();
	samp& sa = dbt.addTypeAndGetRefers<samp>();
}