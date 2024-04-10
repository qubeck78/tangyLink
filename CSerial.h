//---------------------------------------------------------------------------

#ifndef CSerialH
#define CSerialH
//---------------------------------------------------------------------------


#include <Windows.h>
#include <stdio.h>
#include <string.h>
#include < stdlib.h >

#include "types.h"

class CSerial
{
	private:

	HANDLE	hComm;
	DCB 	dcbSerialParams;
	LARGE_INTEGER	ticksPerSecond;


	bool    portOpen;


	public:

	CSerial();

	int open( wchar_t *portName, int baudRate );
	int readByte(uint8_t* rxb);
	int write( uint8_t *buffer, uint32_t numBytesToWrite );
	int close();
	int nanoSleep(LONGLONG ns);




};

#endif
