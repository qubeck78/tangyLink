#define _CRT_SECURE_NO_DEPRECATE


#include <stdio.h>
#include <stdlib.h>
#include "CSerial.h"


CSerial com;


int sendFileInfo(CSerial* com, char* fileName, int fileLength)
{
	char	buf[1024];
	char	buf2[32];

	int		i;
	int		fnameLength;

	if (fileName == NULL)
	{
		return 1;
	}
	if (com == NULL)
	{
		return 1;
	}

	strcpy( buf, ":00" );

	fnameLength = strlen( fileName );

	sprintf( buf2, "%02X", fnameLength );

	strcat( buf, buf2 );
	
	for (i = 0; i < fnameLength; i++)
	{
		sprintf( buf2, "%02X", fileName[i] );
		strcat( buf, buf2 );
	}

	sprintf( buf2, "%08X", fileLength );

	strcat( buf, buf2 );

	strcat( buf, "00\n" );

	//send line via uart
	return com->write( (uint8_t*)buf, strlen( buf ) );
}

int main(int argc, char* argv[])
{
	int		 i;
	int		 newLineCount;

	int rv;
	wchar_t  comPortWs[256];
	char	 fileNameBuf[256];
	uint8_t	 fileBuf[32];	
	FILE	*in;
	int		 fileLength;

	char	 txBuf[512];
	char     numBuf[16];

	int		 rb;
	uint8_t	 rxb;
	uint8_t	 lrc;

	int		 retransmission;
	int		 fatalError;
	int		 baudRate;

	printf("tangyLink B20240410 -#qUBECk#-\n");

	if (argc < 4)
	{
		printf("usage: tangyLink file comPort baudrate\n\nExample: tangyLink image.jpg com1 230400\n\n");
		return 20;
	}

	swprintf( comPortWs, 256, L"%hs", argv[2] );
	baudRate = atoi( argv[3] );

	if (baudRate == 0)
	{
		baudRate = 460800;
	}

	rv = com.open(comPortWs, baudRate);
	if (rv)
	{
		printf("ERROR: can't open com port\n");
		return 21;
	}

	in = fopen(argv[1], "rb");
	if (!in)
	{
		printf("ERROR: can't open input file\n");
		com.close();
		return 22;

	}

	fseek( in, 0L, SEEK_END );

	fileLength = ftell( in );

	fseek( in, 0L, SEEK_SET );

	//send file name
	sendFileInfo( &com, argv[1], fileLength );
	
	if (!com.readByte(&rxb))
	{
		printf( "%c", rxb );
	}

	if (rxb != '*')
	{
		printf( "\nComm error\n" );

		fclose(in);

		com.close();

		return 1;
	}

	
	fatalError		= 0;
	newLineCount	= 0;

	while (!feof(in))
	{
		rb = fread(fileBuf, 1, sizeof( fileBuf), in);

		retransmission = 0;

		if ( rb > 0)
		{

			do
			{
				//send line via uart

				lrc = (uint8_t)':';
				lrc ^= rb;

					
				sprintf( txBuf, ":01%02X", rb );
				for (i = 0; i < rb; i++)
				{
					sprintf( numBuf, "%02X", fileBuf[i] );
					strcat( txBuf, numBuf );

					lrc ^= fileBuf[i];

				}
				sprintf( numBuf, "%02X\n", lrc );
				strcat( txBuf, numBuf );

				com.write( (uint8_t*)txBuf, strlen( txBuf ) );

				if (!com.readByte(&rxb))
				{
					printf( "%c", rxb );
					//fflush( stdout );

					newLineCount++;

					if( ( newLineCount % 80 == 0 ) )
					{
						printf( "\n" );
						fflush( stdout );
					}

					if (rb == 'r')
					{
						retransmission = 1;
					}
					else if (rb == '!')
					{
						fatalError = 1;
						retransmission = 0;
					}
				}
				else
				{

					printf("\nNo response\n");
					fatalError = 1;
					break;
				}
			} while (retransmission);
		}
		if (fatalError)
		{
			printf("\nFatal error reported\n");
			fclose(in);
			com.close();
			return 23;

		}
	}

	fclose(in);

	strcpy( txBuf, ":0200\n" );

	com.write( (uint8_t*)txBuf, strlen( txBuf ) );

	com.close();

	printf( "\nFile sent.\n" );

	return 0;
}

