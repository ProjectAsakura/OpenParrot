#include "MT6.h"
#include <vector>

#define TS_PIPE_TIMEOUT 16

// FROM GAME
char CMD_RESET[3] = {
	0x01, 0x52, 0x0d
};

char CMD_DIAGNOSTICS[4] = {
	0x01, 0x44, 0x58, 0x0d
};

// TO GAME
char OK_RESPONSE[3] = {
	0x01, 0x30, 0x0d
};

volatile BOOL bHasBooted = FALSE;

volatile unsigned short touchx = 0;
volatile unsigned short touchy = 0;
volatile BOOL touchpressed = FALSE;
volatile BOOL touchlift = FALSE;

void mt6SetTouchParams(unsigned short x, unsigned short y, BOOL down)
{
	if (bHasBooted)
	{
		touchx = x;
		touchy = y;
		touchpressed = down;
	}
}

DWORD mt6WritePort(HANDLE port, char data[], unsigned length)
{
	DWORD numWritten = 0;

	OVERLAPPED ol = { 0, 0, 0, 0, NULL };
	ol.hEvent = CreateEvent(NULL, TRUE, FALSE, NULL);

	BOOL status = WriteFile(port, data, length, &numWritten, &ol);
	DWORD xferBytes = 0;

	if (!status)
	{
		switch (GetLastError())
		{
		case ERROR_SUCCESS:
			break;
		case ERROR_IO_PENDING:
			// Wait for 16ms
			if (WaitForSingleObject(ol.hEvent, TS_PIPE_TIMEOUT) == WAIT_OBJECT_0)
			{
				status = GetOverlappedResult(port, &ol, &xferBytes, FALSE);
			}
			else
			{
				CancelIo(port);
			}
			break;
		}
	}

	CloseHandle(ol.hEvent);

	FlushFileBuffers(port);
	return numWritten;
}

DWORD mt6SerialTouchThread(HANDLE port)
{
	char fileBuf[32];
	puts("starting serial touch thread");

	DWORD times = 0;

	for (;;)
	{
		DWORD bytesRead = 0;
		memset(fileBuf, 0, 32);

		OVERLAPPED ol = { 0, 0, 0, 0, NULL };
		BOOL ret = 0;
		ol.hEvent = CreateEvent(NULL, TRUE, FALSE, NULL);

		BOOL rfResult = ReadFile(port, fileBuf, 32, &bytesRead, &ol);
		DWORD xferBytes = 0;

		if (!rfResult)
		{
			switch (GetLastError())
			{
			case ERROR_SUCCESS:
				break;
			case ERROR_IO_PENDING:
				// Wait for 16ms
				if (WaitForSingleObject(ol.hEvent, TS_PIPE_TIMEOUT) == WAIT_OBJECT_0)
				{
					rfResult = GetOverlappedResult(port, &ol, &xferBytes, FALSE);
				}
				else
				{
					CancelIo(port);
				}
				break;
			}
		}

		CloseHandle(ol.hEvent);

		if (xferBytes > 0)
		{
			printf("IN: xferred %d bytes\n", xferBytes);
		}

		if (bytesRead > 0)
		{
			printf("Read %d bytes: ", bytesRead);
			for (unsigned x = 0; x < bytesRead; x++)
			{
				printf("%02X ", fileBuf[x]);
			}
			printf("\n");

			BOOL packetRecognised = FALSE;
			if (memcmp(fileBuf, CMD_RESET, 3) == 0)
			{
				puts("CMD_RESET");
				bHasBooted = FALSE;
				packetRecognised = TRUE;
				mt6WritePort(port, OK_RESPONSE, 3);
			}

			if (memcmp(fileBuf, CMD_DIAGNOSTICS, 4) == 0)
			{
				puts("CMD_DIAGNOSTICS");
				bHasBooted = TRUE;
				packetRecognised = TRUE;
				mt6WritePort(port, OK_RESPONSE, 3);
			}

			if (!packetRecognised)
			{
				puts("unknown packet, responding with OK");
				mt6WritePort(port, OK_RESPONSE, 3);
			}
		}

		if (touchpressed)
		{
			touchlift = TRUE;
			char touchResp[5];
			memset(touchResp, 0, 5);
			touchResp[0] = (char)0b11000000;
			touchResp[1] = (touchx & 0b01111111);
			touchResp[2] = ((touchx >> 8) & 0b01111111);
			touchResp[3] = (touchy & 0b01111111);
			touchResp[4] = ((touchy >> 8) & 0b01111111);
			mt6WritePort(port, touchResp, 5);
		}
		else
		{
			if (touchlift)
			{
				char touchResp[5];
				memset(touchResp, 0, 5);
				touchResp[0] = (char)0b10000000;
				touchResp[1] = (touchx & 0b01111111);
				touchResp[2] = ((touchx >> 8) & 0b01111111);
				touchResp[3] = (touchy & 0b01111111);
				touchResp[4] = ((touchy >> 8) & 0b01111111);
				mt6WritePort(port, touchResp, 5);
				touchlift = false;
			}
		}

		Sleep(16);
	}
}

DWORD mt6SerialNamedPipeServer(LPVOID _)
{
	puts("initialising 3M Microtouch emulator v2 pipe server");

	HANDLE pipe = CreateNamedPipeW(
		L"\\\\.\\pipe\\mt6-touchemu",
		PIPE_ACCESS_DUPLEX | FILE_FLAG_OVERLAPPED,
		PIPE_TYPE_BYTE | PIPE_WAIT,
		PIPE_UNLIMITED_INSTANCES,
		255,
		255,
		25,
		NULL
	);

	if (!pipe)
	{
		puts("named pipe creation failed!");
		return 1;
	}

	// Why, win32
	BOOL connected = ConnectNamedPipe(pipe, NULL) ? TRUE : (GetLastError() == ERROR_PIPE_CONNECTED);

	if (connected)
	{
		puts("client connection established, spawning thread");
		
		DWORD tid = 0;
		CreateThread(NULL, 0, mt6SerialTouchThread, pipe, 0, &tid);
		printf("thread spawned, tid=%d\n", tid);
	}

	return 0;
}

void mt6SerialTouchInit()
{
	// This is on another thread so we can wait a bit to make sure the pipe server is actually init'd
	CreateThread(NULL, 0, mt6SerialNamedPipeServer, NULL, 0, NULL);
	Sleep(20);
}