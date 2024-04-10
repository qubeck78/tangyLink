//---------------------------------------------------------------------------

#pragma hdrstop

#include "CSerial.h"
//---------------------------------------------------------------------------
#pragma package(smart_init)

//https://etutorials.org/Programming/Pocket+pc+network+programming/Chapter+5.+Using+Serial+and+Infrared+Ports/Serial+Communications/
//https://aticleworld.com/serial-port-programming-using-win32-api/
//https://learn.microsoft.com/en-us/windows/win32/api/winbase/nf-winbase-transmitcommchar

CSerial::CSerial()
{

	portOpen = false;
	QueryPerformanceFrequency(&ticksPerSecond);


}

int CSerial::open( wchar_t *portName, int baudRate )
{
	wchar_t			winPortName[256];
	COMMTIMEOUTS	timeouts;


	swprintf_s( winPortName, 256, L"\\\\.\\%s", portName );

	hComm = CreateFile(	winPortName,
						GENERIC_READ | GENERIC_WRITE,      // Read/Write Access
						0,                                 // No Sharing, ports cant be shared
						NULL,                              // No Security
						OPEN_EXISTING,                     // Open existing port only
						0,                                 // Non Overlapped I/O
						NULL );                             // Null for Comm Devices

	if( hComm == INVALID_HANDLE_VALUE )
	{
		portOpen = false;

		return 1;   //error
	}

	dcbSerialParams.DCBlength = sizeof( dcbSerialParams );

	//get current port parameters

	if( GetCommState( hComm, &dcbSerialParams ) == FALSE )
	{
		CloseHandle( hComm );

		portOpen = false;

		return 1;
	}


	dcbSerialParams.BaudRate 	= baudRate;		//500000;	//CBR_115200;      //BaudRate
	dcbSerialParams.ByteSize 	= 8;             //ByteSize = 8
	dcbSerialParams.StopBits 	= ONESTOPBIT;    //StopBits = 1
	dcbSerialParams.Parity 		= NOPARITY;      //Parity = None


	//set new port parameters

	if( SetCommState( hComm, &dcbSerialParams ) == FALSE )
	{
		CloseHandle( hComm );

		portOpen = false;

		return 1;
	}


	//Set timeouts

	timeouts.ReadIntervalTimeout			= 200;
	timeouts.ReadTotalTimeoutConstant		= 10;
	timeouts.ReadTotalTimeoutMultiplier		= 200;
	timeouts.WriteTotalTimeoutConstant		= 10;
	timeouts.WriteTotalTimeoutMultiplier	= 10;

	if( SetCommTimeouts( hComm, &timeouts ) == FALSE )
	{
		CloseHandle( hComm );

		portOpen = false;

		return 1;

	}


	//Set rx char event mask
	if( SetCommMask( hComm, EV_RXCHAR ) == FALSE )
	{
		CloseHandle( hComm );

		portOpen = false;

		return 1;
	}

	portOpen = true;

	return 0;
}

int CSerial::readByte(uint8_t* rxb)
{
	uint8_t buf[16];
	DWORD dwRead;

	if (!portOpen)
	{
		return 21;
	}
	dwRead = 0;

	ReadFile(hComm, &buf, 1, &dwRead, NULL);
	
	if (dwRead == 1)
	{
		*rxb = buf[0];
		return 0;
	}
	else
	{
		return 1;
	}
}

int CSerial::write( uint8_t *buffer, uint32_t numBytesToWrite )
{
	DWORD   numBytesWritten;

	if( !portOpen )
	{
		return 1;
	}


	/*for (numBytesWritten = 0; numBytesWritten < numBytesToWrite; numBytesWritten++)
	{
		TransmitCommChar( hComm, buffer[numBytesWritten] );
		
		//nanoSleep(100);
	}
	*/




	if(	WriteFile(	hComm,						// Handle to the Serialport
					buffer,            			// Data to be written to the port
					numBytesToWrite,   			// No of bytes to write into the port
					&numBytesWritten,  			// No of bytes written to the port
					NULL ) == FALSE )
	{

		//error
		return 2;
	}
  
	return 0;
}

int CSerial::close()
{
	CloseHandle(hComm);

	portOpen = false;

	return 0;
}


int CSerial::nanoSleep(LONGLONG ns)
{
	LONGLONG ticksPerNs = ticksPerSecond.QuadPart / 1000000;
	LONGLONG numTicsToWait;

	LARGE_INTEGER currentTics;
	LARGE_INTEGER startTics;

	numTicsToWait = ticksPerNs * ns;

	QueryPerformanceCounter(&startTics);

	do
	{
		QueryPerformanceCounter(&currentTics);


	} while (currentTics.QuadPart < startTics.QuadPart + numTicsToWait);


	return 0;
}

