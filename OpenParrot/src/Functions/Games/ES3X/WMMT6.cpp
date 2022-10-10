#include <StdInc.h>
#include "Utility/InitFunction.h"
#include "Functions/Global.h"
#include "MinHook.h"
#include <Utility/Hooking.Patterns.h>
#include <thread>
#include <iostream>
#include <Windowsx.h>
#include <Utility/TouchSerial/MT6.h>
#ifdef _M_AMD64
#pragma optimize("", off)
#pragma comment(lib, "Ws2_32.lib")

extern LPCSTR hookPort;
static uintptr_t imageBase;
static unsigned char hasp_buffer[0xD40];
static bool isFreePlay;
static bool isEventMode2P;
static bool isEventMode4P;
static bool ForceFullTune;
static bool ForceNeon;
static bool CarTuneNeonThread;
static const char* ipaddr;

static DWORD mileageValue = 0;
static int NeonColour;

#define HASP_STATUS_OK 0
static unsigned int Hook_hasp_login(int feature_id, void* vendor_code, int hasp_handle) {
#ifdef _DEBUG
	OutputDebugStringA("hasp_login\n");
#endif
	return HASP_STATUS_OK;
}

static unsigned int Hook_hasp_logout(int hasp_handle) {
#ifdef _DEBUG
	OutputDebugStringA("hasp_logout\n");
#endif
	return HASP_STATUS_OK;
}

static unsigned int Hook_hasp_encrypt(int hasp_handle, unsigned char* buffer, unsigned int buffer_size) {
#ifdef _DEBUG
	OutputDebugStringA("hasp_encrypt\n");
#endif
	return HASP_STATUS_OK;
}

static unsigned int Hook_hasp_decrypt(int hasp_handle, unsigned char* buffer, unsigned int buffer_size) {
#ifdef _DEBUG
	OutputDebugStringA("hasp_decrypt\n");
#endif
	return HASP_STATUS_OK;
}

static unsigned int Hook_hasp_get_size(int hasp_handle, int hasp_fileid, unsigned int* hasp_size) {
#ifdef _DEBUG
	OutputDebugStringA("hasp_get_size\n");
#endif
	* hasp_size = 0xD40; // Max addressable size by the game... absmax is 4k
	return HASP_STATUS_OK;
}

static unsigned int Hook_hasp_read(int hasp_handle, int hasp_fileid, unsigned int offset, unsigned int length, unsigned char* buffer) {
#ifdef _DEBUG
	OutputDebugStringA("hasp_read\n");
#endif
	memcpy(buffer, hasp_buffer + offset, length);
	return HASP_STATUS_OK;
}

static unsigned int Hook_hasp_write(int hasp_handle, int hasp_fileid, unsigned int offset, unsigned int length, unsigned char* buffer) {
	return HASP_STATUS_OK;
}

typedef int (WINAPI* BIND)(SOCKET, CONST SOCKADDR*, INT);
static BIND pbind = NULL;

static unsigned int WINAPI Hook_bind(SOCKET s, const sockaddr* addr, int namelen) {
	sockaddr_in bindAddr = { 0 };
	bindAddr.sin_family = AF_INET;
	bindAddr.sin_addr.s_addr = inet_addr("192.168.96.20");
	bindAddr.sin_port = htons(50765);
	if (addr == (sockaddr*)&bindAddr) {
		sockaddr_in bindAddr2 = { 0 };
		bindAddr2.sin_family = AF_INET;
		bindAddr2.sin_addr.s_addr = inet_addr(ipaddr);
		bindAddr2.sin_port = htons(50765);
		return pbind(s, (sockaddr*)&bindAddr2, namelen);
	}
	else {
		return pbind(s, addr, namelen);

	}
}

static BOOL FileExists(char* szPath)
{
	DWORD dwAttrib = GetFileAttributesA(szPath);

	return (dwAttrib != INVALID_FILE_ATTRIBUTES &&
		!(dwAttrib & FILE_ATTRIBUTE_DIRECTORY));
}

static int ReturnTrue()
{
	return 1;
}

static BYTE GenerateChecksum(unsigned char *myArray, int index, int length)
{
	BYTE crc = 0;
	for(int i = 0; i < length; i++)
	{
		crc += myArray[index + i];
	}
	return crc & 0xFF;
}

static void GenerateDongleData(bool isTerminal)
{
	memset(hasp_buffer, 0, 0xD40);
	hasp_buffer[0] = 0x01;
	hasp_buffer[0x13] = 0x01;
	hasp_buffer[0x17] = 0x0A;
	hasp_buffer[0x1B] = 0x04;
	hasp_buffer[0x1C] = 0x3B;
	hasp_buffer[0x1D] = 0x6B;
	hasp_buffer[0x1E] = 0x40;
	hasp_buffer[0x1F] = 0x87;

	hasp_buffer[0x23] = 0x01;
	hasp_buffer[0x27] = 0x0A;
	hasp_buffer[0x2B] = 0x04;
	hasp_buffer[0x2C] = 0x3B;
	hasp_buffer[0x2D] = 0x6B;
	hasp_buffer[0x2E] = 0x40;
	hasp_buffer[0x2F] = 0x87;
	if (isTerminal)
	{
		memcpy(hasp_buffer + 0xD00, "280811990002", 12); // not sure these are OK, since its from google lol.
		hasp_buffer[0xD3E] = GenerateChecksum(hasp_buffer, 0xD00, 62);
		hasp_buffer[0xD3F] = hasp_buffer[0xD3E] ^ 0xFF;
	}
	else
	{
		memcpy(hasp_buffer + 0xD00, "280813990002", 12);
		hasp_buffer[0xD3E] = GenerateChecksum(hasp_buffer, 0xD00, 62);
		hasp_buffer[0xD3F] = hasp_buffer[0xD3E] ^ 0xFF;
	}
}

static HWND mt6Hwnd;

typedef BOOL (WINAPI* ShowWindow_t)(HWND, int);
static ShowWindow_t pShowWindow;


// Hello Win32 my old friend...
typedef LRESULT (WINAPI* WindowProcedure_t)(HWND, UINT, WPARAM, LPARAM);
static WindowProcedure_t pMaxituneWndProc;

static BOOL gotWindowSize = FALSE;
static unsigned displaySizeX = 0;
static unsigned displaySizeY = 0;
static float scaleFactorX = 0.0f;
static float scaleFactorY = 0.0f;

static LRESULT Hook_WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	if (!gotWindowSize)
	{
		displaySizeX = GetSystemMetrics(SM_CXSCREEN);
		displaySizeY = GetSystemMetrics(SM_CYSCREEN);
		scaleFactorX = static_cast<float>(displaySizeX) / 1360.0f;
		scaleFactorY = static_cast<float>(displaySizeY) / 768.0f;
		printf("display is %dx%d (scale factor of %f, %f)\n", displaySizeX, displaySizeY, scaleFactorX, scaleFactorY);
		gotWindowSize = TRUE;
	}

	if (msg == WM_LBUTTONDOWN ||
		msg == WM_LBUTTONUP)
	{
		unsigned short mx = GET_X_LPARAM(lParam);
		unsigned short my = GET_Y_LPARAM(lParam);

		//unsigned short trueMy = 768 - my;

		float scaledMx = static_cast<float>(mx) / 1360.f;
		float scaledMy = static_cast<float>(my) / 768.f;

		scaledMy = 1.0f - scaledMy;

		scaledMx *= scaleFactorX;
		scaledMy *= scaleFactorY;

		unsigned short trueMx = static_cast<int>(scaledMx * 16383.0f);
		unsigned short trueMy = static_cast<int>(scaledMy * 16383.0f);
		trueMy += 9500; // Cheap hack, todo do the math better!!
		
		//mx *= (16383 / 1360);
		//trueMy *= (16383 / 1360);

		printf("%d %d\n", trueMx, trueMy);
		mt6SetTouchParams(trueMx, trueMy, msg == WM_LBUTTONDOWN);

		printf("MOUSE %s (%d, %d)\n", msg == WM_LBUTTONDOWN ? "DOWN" : "UP  ", mx, my);
		return 0;
	}

	return pMaxituneWndProc(hwnd, msg, wParam, lParam);
}

static BOOL Hook_ShowWindow(HWND hwnd, int nCmdShow)
{
	SetWindowLongPtrW(hwnd, -4, (LONG_PTR)Hook_WndProc);
	ShowCursor(1);

	mt6Hwnd = hwnd;
	return pShowWindow(hwnd, nCmdShow);
}

typedef void (WINAPI* OutputDebugStringA_t)(LPCSTR);

static void Hook_OutputDebugStringA(LPCSTR str)
{
	printf("debug> %s", str);
}

extern int* ffbOffset;
extern int* ffbOffset2;
extern int* ffbOffset3;
extern int* ffbOffset4;

static __int64(__fastcall* g_origMileageFix)(__int64);

static __int64 __fastcall MileageFix(__int64 a1)
{
	//*(DWORD*)(a1 + 224) = mileageValue;
	//auto result = g_origMileageFix(a1);
	//mileageValue += *(DWORD*)(a1 + 228);
	return g_origMileageFix(a1);
}

static InitFunction Wmmt6Func([]()
{
	// Alloc debug console
	FreeConsole();
	AllocConsole();
	SetConsoleTitle(L"Maxitune6 Console");

	FILE* pNewStdout = nullptr;
	FILE* pNewStderr = nullptr;
	FILE* pNewStdin = nullptr;

	::freopen_s(&pNewStdout, "CONOUT$", "w", stdout);
	::freopen_s(&pNewStderr, "CONOUT$", "w", stderr);
	::freopen_s(&pNewStdin, "CONIN$", "r", stdin);
	std::cout.clear();
	std::cerr.clear();
	std::cin.clear();
	std::wcout.clear();
	std::wcerr.clear();
	std::wcin.clear();

	puts("hello there, maxitune");

	// folder for path redirections
	CreateDirectoryA(".\\TP", nullptr);

	/*
	FILE* fileF = _wfopen(L".\\TP\\setting.lua.gz", L"r");
	if (fileF == NULL)
	{
		FILE* settingsF = _wfopen(L".\\TP\\setting.lua.gz", L"wb");
		fwrite(settingData, 1, sizeof(settingData), settingsF);
		fclose(settingsF);
	}
	else
	{
		fclose(fileF);
	}
	*/

	bool isTerminal = false;
	if (ToBool(config["General"]["TerminalMode"]))
	{
		isTerminal = true;
	}

	std::string networkip = config["General"]["NetworkAdapterIP"];
	if (!networkip.empty())
	{
		ipaddr = networkip.c_str();
	}

	hookPort = "COM3";
	imageBase = (uintptr_t)GetModuleHandleA(0);
	MH_Initialize();

	// Hook dongle funcs
	MH_CreateHookApi(L"hasp_windows_x64_28756.dll", "hasp_write", Hook_hasp_write, NULL);
	MH_CreateHookApi(L"hasp_windows_x64_28756.dll", "hasp_read", Hook_hasp_read, NULL);
	MH_CreateHookApi(L"hasp_windows_x64_28756.dll", "hasp_get_size", Hook_hasp_get_size, NULL);
	MH_CreateHookApi(L"hasp_windows_x64_28756.dll", "hasp_decrypt", Hook_hasp_decrypt, NULL);
	MH_CreateHookApi(L"hasp_windows_x64_28756.dll", "hasp_encrypt", Hook_hasp_encrypt, NULL);
	MH_CreateHookApi(L"hasp_windows_x64_28756.dll", "hasp_logout", Hook_hasp_logout, NULL);
	MH_CreateHookApi(L"hasp_windows_x64_28756.dll", "hasp_login", Hook_hasp_login, NULL);
	MH_CreateHookApi(L"WS2_32", "bind", Hook_bind, reinterpret_cast<LPVOID*>(&pbind));
	MH_CreateHook((void*)(imageBase + 0x35AAC0), MileageFix, (void**)&g_origMileageFix);

	MH_CreateHookApi(L"kernel32", "OutputDebugStringA", Hook_OutputDebugStringA, NULL);
	// CreateFile* hooks are in the JVS FILE

	// Give me the HWND please maxitune
	MH_CreateHookApi(L"user32", "ShowWindow", Hook_ShowWindow, reinterpret_cast<LPVOID*>(&pShowWindow));
	//MH_CreateHookApi(L"kernel32", "ReadFile", Hook_ReadFile, reinterpret_cast<LPVOID*>(&pReadFile));

	// Hook the window procedure
	// (The image starts at 0x140000000)
	//MH_CreateHook((void*)(imageBase + 0xB7C030), Hook_WndProc, (void**)&pMaxituneWndProc);
	pMaxituneWndProc = (WindowProcedure_t)(imageBase + 0xB7C030);

	GenerateDongleData(isTerminal);

	// resolves a system error
	injector::WriteMemory<uint8_t>(hook::get_pattern("0F 94 C0 84 C0 0F 94 C0 84 C0 75 05 45 32 ? EB", 0x13), 0, true);

	// Skip weird camera init that stucks entire pc on certain brands. TESTED ONLY ON 05!!!!
	if (ToBool(config["General"]["WhiteScreenFix"]))
	{
		injector::WriteMemory<DWORD>(hook::get_pattern("48 8B C4 55 57 41 54 41 55 41 56 48 8D 68 A1 48 81 EC 90 00 00 00 48 C7 45 D7 FE FF FF FF 48 89 58 08 48 89 70 18 45 33 F6 4C 89 75 DF 33 C0 48 89 45 E7", 0), 0x90C3C032, true);
	}

	// Best LAN setting by doomertheboomer
	injector::WriteMemory<BYTE>(imageBase + 0xA36CAA, 0xEB, true); //content router patch
	injector::MakeNOP(imageBase + 0x690876, 2, true);
	
	// wtf is this?
	//injector::MakeNOP(hook::get_pattern("45 33 C0 BA 65 09 00 00 48 8D 4D B0 E8 ? ? ? ? 48 8B 08", 12), 5);

	auto location = hook::get_pattern<char>("48 83 EC 28 33 D2 B9 70 00 02 00 E8 ? ? ? ? 85 C0 79 06");
	//injector::WriteMemory<uint8_t>(location + 0x12, 0xEB, true);

	// First auth error skip
	//injector::WriteMemory<BYTE>(imageBase + 0x6A0077, 0xEB, true);

	if (isTerminal)
	{
		// I don't know what these do but they stop the game from
		// throwing a fit on the dongle error
		// so I'm leaving them in here.

		// Dongle error?
		//safeJMP(hook::get_pattern("0F B6 41 05 2C 30 3C 09 77 04 0F BE C0 C3 83 C8 FF C3"), ReturnTrue);
	
		// More dongle error shit?
		safeJMP(hook::get_pattern("8B 01 0F B6 40 78 C3 CC CC CC CC"), ReturnTrue);
	}
	else
	{
		// Terminal on same machine check.
		injector::MakeNOP(hook::get_pattern("74 ? 80 7B 31 00 75 ? 48 8B 43 10 80 78 31 00 75 1A 48 8B D8 48 8B 00 80 78 31 00 75 ? 48 8B D8"), 2);

		/*
		injector::WriteMemory<WORD>(imageBase + 0x6A0C87, 0x00D1, true);		
		injector::WriteMemory<BYTE>(imageBase + 0x20B88A, 0x90, true);
		injector::WriteMemory<BYTE>(imageBase + 0x20B88B, 0x90, true);
		injector::WriteMemory<BYTE>(imageBase + 0x20B89B, 0x90, true);
		injector::WriteMemory<BYTE>(imageBase + 0x20B89C, 0x90, true);
		injector::WriteMemory<BYTE>(imageBase + 0x20B8A1, 0x90, true);
		injector::WriteMemory<BYTE>(imageBase + 0x20B8A2, 0x90, true);

		// spam thread
		if (ToBool(config["General"]["TerminalEmulator"]))
		{
			CreateThread(0, 0, SpamMulticast, 0, 0, 0);
		}
		*/
	}

	// path fixes
	injector::WriteMemoryRaw(imageBase + 0x12C5248, "TP", 2, true);
	injector::WriteMemoryRaw(imageBase + 0x12C5268, "TP", 2, true);
	injector::WriteMemoryRaw(imageBase + 0x12C5288, "TP", 2, true);
	injector::WriteMemoryRaw(imageBase + 0x12C52A8, "TP", 2, true);
	injector::WriteMemoryRaw(imageBase + 0x12C52C8, "TP", 2, true);
	injector::WriteMemoryRaw(imageBase + 0x12C52E8, "TP", 2, true);
	injector::WriteMemoryRaw(imageBase + 0x12C5308, "TP", 2, true);
	injector::WriteMemoryRaw(imageBase + 0x12C5328, "TP", 2, true);
	injector::WriteMemoryRaw(imageBase + 0x12C5348, "TP", 2, true);
	injector::WriteMemoryRaw(imageBase + 0x12C5360, "TP", 2, true);
	injector::WriteMemoryRaw(imageBase + 0x135A708, "TP", 2, true);
	injector::WriteMemoryRaw(imageBase + 0x135A720, "TP", 2, true);
	injector::WriteMemoryRaw(imageBase + 0x135A738, "TP", 2, true);
	injector::WriteMemoryRaw(imageBase + 0x135A760, "TP", 2, true);
	injector::WriteMemoryRaw(imageBase + 0x135A788, "TP", 2, true);
	injector::WriteMemoryRaw(imageBase + 0x135A7A0, "TP", 2, true);
	injector::WriteMemoryRaw(imageBase + 0x135A7B8, "TP", 2, true);
	injector::WriteMemoryRaw(imageBase + 0x135A7C8, "TP", 2, true);
	injector::WriteMemoryRaw(imageBase + 0x135A7D8, "TP", 2, true);
	injector::WriteMemoryRaw(imageBase + 0x135A7F0, "TP", 2, true);
	injector::WriteMemoryRaw(imageBase + 0x135A808, "TP", 2, true);
	injector::WriteMemoryRaw(imageBase + 0x135A828, "TP", 2, true);
	injector::WriteMemoryRaw(imageBase + 0x135A848, "TP", 2, true);
	injector::WriteMemoryRaw(imageBase + 0x135A858, "TP", 2, true);
	injector::WriteMemoryRaw(imageBase + 0x135A868, "TP", 2, true);
	injector::WriteMemoryRaw(imageBase + 0x135A880, "TP", 2, true);
	injector::WriteMemoryRaw(imageBase + 0x135A898, "TP", 2, true);
	injector::WriteMemoryRaw(imageBase + 0x135A8B0, "TP", 2, true);
	injector::WriteMemoryRaw(imageBase + 0x135A8C8, "TP", 2, true);
	injector::WriteMemoryRaw(imageBase + 0x135A8E0, "TP", 2, true);
	injector::WriteMemoryRaw(imageBase + 0x135A8F8, "TP", 2, true);
	injector::WriteMemoryRaw(imageBase + 0x135A910, "TP", 2, true);
	injector::WriteMemoryRaw(imageBase + 0x135A928, "TP", 2, true);
	injector::WriteMemoryRaw(imageBase + 0x135A940, "TP", 2, true);
	injector::WriteMemoryRaw(imageBase + 0x1362D48, "TP", 2, true); // F:/contents/
	injector::WriteMemoryRaw(imageBase + 0x13B2890, "TP/contents/", 12, true); // F:contents/
	injector::WriteMemoryRaw(imageBase + 0x13B28A0, "TP/contents/", 12, true);	// G:contents/
	injector::WriteMemoryRaw(imageBase + 0x1401030, "TP", 2, true);
	injector::WriteMemoryRaw(imageBase + 0x1401048, "TP", 2, true);
	injector::WriteMemoryRaw(imageBase + 0x1401E08, "TP", 2, true);
	injector::WriteMemoryRaw(imageBase + 0x1401E20, "TP", 2, true);
	injector::WriteMemoryRaw(imageBase + 0x1401E38, "TP", 2, true);
	injector::WriteMemoryRaw(imageBase + 0x1401E60, "TP", 2, true);
	injector::WriteMemoryRaw(imageBase + 0x1401E88, "TP", 2, true);
	injector::WriteMemoryRaw(imageBase + 0x1401EA0, "TP", 2, true);
	injector::WriteMemoryRaw(imageBase + 0x14028E0, "TP", 2, true);
	injector::WriteMemoryRaw(imageBase + 0x1402900, "TP", 2, true);
	injector::WriteMemoryRaw(imageBase + 0x1401DDC, "TP", 2, true); // F:
	injector::WriteMemoryRaw(imageBase + 0x13652B8, "TP", 2, true);
	injector::WriteMemoryRaw(imageBase + 0x1365AC8, "TP", 2, true);

	std::string value = config["General"]["CustomName"];
	if (!value.empty())
	{
		/*
		if (value.size() > 5)
		{
			memset(customName, 0, 256);
			strcpy(customName, value.c_str());
			CreateThread(0, 0, SpamCustomName, 0, 0, 0);
		}

		injector::WriteMemory<BYTE>(imageBase + 0x10942E8, 0xFF, true);
		injector::WriteMemory<BYTE>(imageBase + 0x10F5428, 0xFF, true);
		injector::WriteMemory<BYTE>(imageBase + 0x12B3EB0, 0xFF, true);
		injector::WriteMemory<BYTE>(imageBase + 0x12B75A0, 0xFF, true);
		injector::WriteMemory<BYTE>(imageBase + 0x12CE688, 0xFF, true);
		injector::WriteMemory<BYTE>(imageBase + 0x13C4BF0, 0xFF, true);
		injector::WriteMemory<BYTE>(imageBase + 0x13C4C00, 0xFF, true);
		injector::WriteMemory<BYTE>(imageBase + 0x13C4C10, 0xFF, true);
		injector::WriteMemory<BYTE>(imageBase + 0x10942EA, 0xFF, true);
		injector::WriteMemory<BYTE>(imageBase + 0x10F542A, 0xFF, true);
		injector::WriteMemory<BYTE>(imageBase + 0x12B3EB2, 0xFF, true);
		injector::WriteMemory<BYTE>(imageBase + 0x12B75A2, 0xFF, true);
		injector::WriteMemory<BYTE>(imageBase + 0x12CE68A, 0xFF, true);
		injector::WriteMemory<BYTE>(imageBase + 0x13C4BF2, 0xFF, true);
		injector::WriteMemory<BYTE>(imageBase + 0x13C4C02, 0xFF, true);
		injector::WriteMemory<BYTE>(imageBase + 0x13C4C12, 0xFF, true);
		injector::WriteMemory<BYTE>(imageBase + 0x10942EC, 0xFF, true);
		injector::WriteMemory<BYTE>(imageBase + 0x10F542C, 0xFF, true);
		injector::WriteMemory<BYTE>(imageBase + 0x12B3EB4, 0xFF, true);
		injector::WriteMemory<BYTE>(imageBase + 0x12B75A4, 0xFF, true);
		injector::WriteMemory<BYTE>(imageBase + 0x12CE68C, 0xFF, true);
		injector::WriteMemory<BYTE>(imageBase + 0x13C4BF4, 0xFF, true);
		injector::WriteMemory<BYTE>(imageBase + 0x13C4C04, 0xFF, true);
		injector::WriteMemory<BYTE>(imageBase + 0x13C4C14, 0xFF, true);
		injector::WriteMemory<BYTE>(imageBase + 0x10942EE, 0xFF, true);
		injector::WriteMemory<BYTE>(imageBase + 0x10F542E, 0xFF, true);
		injector::WriteMemory<BYTE>(imageBase + 0x12B3EB6, 0xFF, true);
		injector::WriteMemory<BYTE>(imageBase + 0x12B75A6, 0xFF, true);
		injector::WriteMemory<BYTE>(imageBase + 0x12CE68E, 0xFF, true);
		injector::WriteMemory<BYTE>(imageBase + 0x13C4BF6, 0xFF, true);
		injector::WriteMemory<BYTE>(imageBase + 0x13C4C06, 0xFF, true);
		injector::WriteMemory<BYTE>(imageBase + 0x13C4C16, 0xFF, true);
		injector::WriteMemory<BYTE>(imageBase + 0x10942F0, 0xFF, true);
		injector::WriteMemory<BYTE>(imageBase + 0x10F5430, 0xFF, true);
		injector::WriteMemory<BYTE>(imageBase + 0x12B3EB8, 0xFF, true);
		injector::WriteMemory<BYTE>(imageBase + 0x12B75A8, 0xFF, true);
		injector::WriteMemory<BYTE>(imageBase + 0x12CE690, 0xFF, true);
		injector::WriteMemory<BYTE>(imageBase + 0x13C4BF8, 0xFF, true);
		injector::WriteMemory<BYTE>(imageBase + 0x13C4C08, 0xFF, true);
		injector::WriteMemory<BYTE>(imageBase + 0x13C4C18, 0xFF, true);

		char NameChar;
		for (int i = 0; i < value.size(); i++) {
			NameChar = value.at(i) - 0x20;

			switch (i)
			{
			case 0x00:
				injector::WriteMemory<BYTE>(imageBase + 0x10942E8, NameChar, true);
				injector::WriteMemory<BYTE>(imageBase + 0x10F5428, NameChar, true);
				injector::WriteMemory<BYTE>(imageBase + 0x12B3EB0, NameChar, true);
				injector::WriteMemory<BYTE>(imageBase + 0x12B75A0, NameChar, true);
				injector::WriteMemory<BYTE>(imageBase + 0x12CE688, NameChar, true);
				injector::WriteMemory<BYTE>(imageBase + 0x13C4BF0, NameChar, true);
				injector::WriteMemory<BYTE>(imageBase + 0x13C4C00, NameChar, true);
				injector::WriteMemory<BYTE>(imageBase + 0x13C4C10, NameChar, true);
				break;
			case 0x01:
				injector::WriteMemory<BYTE>(imageBase + 0x10942EA, NameChar, true);
				injector::WriteMemory<BYTE>(imageBase + 0x10F542A, NameChar, true);
				injector::WriteMemory<BYTE>(imageBase + 0x12B3EB2, NameChar, true);
				injector::WriteMemory<BYTE>(imageBase + 0x12B75A2, NameChar, true);
				injector::WriteMemory<BYTE>(imageBase + 0x12CE68A, NameChar, true);
				injector::WriteMemory<BYTE>(imageBase + 0x13C4BF2, NameChar, true);
				injector::WriteMemory<BYTE>(imageBase + 0x13C4C02, NameChar, true);
				injector::WriteMemory<BYTE>(imageBase + 0x13C4C12, NameChar, true);
				break;
			case 0x02:
				injector::WriteMemory<BYTE>(imageBase + 0x10942EC, NameChar, true);
				injector::WriteMemory<BYTE>(imageBase + 0x10F542C, NameChar, true);
				injector::WriteMemory<BYTE>(imageBase + 0x12B3EB4, NameChar, true);
				injector::WriteMemory<BYTE>(imageBase + 0x12B75A4, NameChar, true);
				injector::WriteMemory<BYTE>(imageBase + 0x12CE68C, NameChar, true);
				injector::WriteMemory<BYTE>(imageBase + 0x13C4BF4, NameChar, true);
				injector::WriteMemory<BYTE>(imageBase + 0x13C4C04, NameChar, true);
				injector::WriteMemory<BYTE>(imageBase + 0x13C4C14, NameChar, true);
				break;
			case 0x03:
				injector::WriteMemory<BYTE>(imageBase + 0x10942EE, NameChar, true);
				injector::WriteMemory<BYTE>(imageBase + 0x10F542E, NameChar, true);
				injector::WriteMemory<BYTE>(imageBase + 0x12B3EB6, NameChar, true);
				injector::WriteMemory<BYTE>(imageBase + 0x12B75A6, NameChar, true);
				injector::WriteMemory<BYTE>(imageBase + 0x12CE68E, NameChar, true);
				injector::WriteMemory<BYTE>(imageBase + 0x13C4BF6, NameChar, true);
				injector::WriteMemory<BYTE>(imageBase + 0x13C4C06, NameChar, true);
				injector::WriteMemory<BYTE>(imageBase + 0x13C4C16, NameChar, true);
				break;
			case 0x04:
				injector::WriteMemory<BYTE>(imageBase + 0x10942F0, NameChar, true);
				injector::WriteMemory<BYTE>(imageBase + 0x10F5430, NameChar, true);
				injector::WriteMemory<BYTE>(imageBase + 0x12B3EB8, NameChar, true);
				injector::WriteMemory<BYTE>(imageBase + 0x12B75A8, NameChar, true);
				injector::WriteMemory<BYTE>(imageBase + 0x12CE690, NameChar, true);
				injector::WriteMemory<BYTE>(imageBase + 0x13C4BF8, NameChar, true);
				injector::WriteMemory<BYTE>(imageBase + 0x13C4C08, NameChar, true);
				injector::WriteMemory<BYTE>(imageBase + 0x13C4C18, NameChar, true);
				break;
			}
		}
		injector::WriteMemory<BYTE>(imageBase + 0x10942E9, 0xFF, true);
		injector::WriteMemory<BYTE>(imageBase + 0x10942EB, 0xFF, true);
		injector::WriteMemory<BYTE>(imageBase + 0x10942ED, 0xFF, true);
		injector::WriteMemory<BYTE>(imageBase + 0x10942EF, 0xFF, true);
		injector::WriteMemory<BYTE>(imageBase + 0x10942F1, 0xFF, true);
		injector::WriteMemory<BYTE>(imageBase + 0x10F5429, 0xFF, true);
		injector::WriteMemory<BYTE>(imageBase + 0x10F542B, 0xFF, true);
		injector::WriteMemory<BYTE>(imageBase + 0x10F542D, 0xFF, true);
		injector::WriteMemory<BYTE>(imageBase + 0x10F542F, 0xFF, true);
		injector::WriteMemory<BYTE>(imageBase + 0x10F5431, 0xFF, true);
		injector::WriteMemory<BYTE>(imageBase + 0x12B3EB1, 0xFF, true);
		injector::WriteMemory<BYTE>(imageBase + 0x12B3EB3, 0xFF, true);
		injector::WriteMemory<BYTE>(imageBase + 0x12B3EB5, 0xFF, true);
		injector::WriteMemory<BYTE>(imageBase + 0x12B3EB7, 0xFF, true);
		injector::WriteMemory<BYTE>(imageBase + 0x12B3EB9, 0xFF, true);
		injector::WriteMemory<BYTE>(imageBase + 0x12B75A1, 0xFF, true);
		injector::WriteMemory<BYTE>(imageBase + 0x12B75A3, 0xFF, true);
		injector::WriteMemory<BYTE>(imageBase + 0x12B75A5, 0xFF, true);
		injector::WriteMemory<BYTE>(imageBase + 0x12B75A7, 0xFF, true);
		injector::WriteMemory<BYTE>(imageBase + 0x12B75A9, 0xFF, true);
		injector::WriteMemory<BYTE>(imageBase + 0x12CE689, 0xFF, true);
		injector::WriteMemory<BYTE>(imageBase + 0x12CE68B, 0xFF, true);
		injector::WriteMemory<BYTE>(imageBase + 0x12CE68D, 0xFF, true);
		injector::WriteMemory<BYTE>(imageBase + 0x12CE68F, 0xFF, true);
		injector::WriteMemory<BYTE>(imageBase + 0x12CE691, 0xFF, true);
		injector::WriteMemory<BYTE>(imageBase + 0x13C4BF1, 0xFF, true);
		injector::WriteMemory<BYTE>(imageBase + 0x13C4BF3, 0xFF, true);
		injector::WriteMemory<BYTE>(imageBase + 0x13C4BF5, 0xFF, true);
		injector::WriteMemory<BYTE>(imageBase + 0x13C4BF7, 0xFF, true);
		injector::WriteMemory<BYTE>(imageBase + 0x13C4BF9, 0xFF, true);
		injector::WriteMemory<BYTE>(imageBase + 0x13C4C01, 0xFF, true);
		injector::WriteMemory<BYTE>(imageBase + 0x13C4C03, 0xFF, true);
		injector::WriteMemory<BYTE>(imageBase + 0x13C4C05, 0xFF, true);
		injector::WriteMemory<BYTE>(imageBase + 0x13C4C07, 0xFF, true);
		injector::WriteMemory<BYTE>(imageBase + 0x13C4C09, 0xFF, true);
		injector::WriteMemory<BYTE>(imageBase + 0x13C4C11, 0xFF, true);
		injector::WriteMemory<BYTE>(imageBase + 0x13C4C13, 0xFF, true);
		injector::WriteMemory<BYTE>(imageBase + 0x13C4C15, 0xFF, true);
		injector::WriteMemory<BYTE>(imageBase + 0x13C4C17, 0xFF, true);
		injector::WriteMemory<BYTE>(imageBase + 0x13C4C19, 0xFF, true);
		*/
	}

	ForceFullTune = (ToBool(config["Tune"]["Force Full Tune"]));
	ForceNeon = (ToBool(config["Tune"]["Force Neon"]));

	if (ForceNeon)
	{
		/*
		if (strcmp(config["Tune"]["Select Neon"].c_str(), "Green") == 0)
			NeonColour = 0x01;
		if (strcmp(config["Tune"]["Select Neon"].c_str(), "Blue") == 0)
			NeonColour = 0x02;
		if (strcmp(config["Tune"]["Select Neon"].c_str(), "Red") == 0)
			NeonColour = 0x03;
		if (strcmp(config["Tune"]["Select Neon"].c_str(), "Yellow") == 0)
			NeonColour = 0x04;
		if (strcmp(config["Tune"]["Select Neon"].c_str(), "Purple") == 0)
			NeonColour = 0x05;
		if (strcmp(config["Tune"]["Select Neon"].c_str(), "Green Pattern") == 0)
			NeonColour = 0x06;
		if (strcmp(config["Tune"]["Select Neon"].c_str(), "Blue Pattern") == 0)
			NeonColour = 0x07;
		if (strcmp(config["Tune"]["Select Neon"].c_str(), "Red Pattern") == 0)
			NeonColour = 0x08;
		if (strcmp(config["Tune"]["Select Neon"].c_str(), "Yellow Pattern") == 0)
			NeonColour = 0x09;
		if (strcmp(config["Tune"]["Select Neon"].c_str(), "Purple Pattern") == 0)
			NeonColour = 0x0A;
		*/
	}

	// Fix dongle error (can be triggered by various USB hubs, dongles
	injector::MakeNOP(imageBase + 0x8C140F, 2, true);

	//Fix crash when saving story mode and Time attack, if the error isn't handled then it doesnt crash?????
	injector::WriteMemory<uint8_t>(imageBase + 0x8A6B5F, 0xEB, true);
	injector::WriteMemory<uint8_t>(imageBase + 0x8A6AE8, 0x38EB, true);

	// Save story stuff (only 05)
	{
		/*
		// skip erasing of temp card data
		injector::WriteMemory<uint8_t>(imageBase + 0xA54F13, 0xEB, true);
		// Skip erasing of temp card
		safeJMP(imageBase + 0x647FB0, LoadGameData);
		safeJMP(imageBase + 0x65ED40, ReturnTrue);
		safeJMP(imageBase + 0x682A00, ReturnTrue);
		safeJMP(imageBase + 0x68CD40, ReturnTrue);

		safeJMP(imageBase + 0xACEA10, ReturnTrue);
		safeJMP(imageBase + 0x65F1F0, ReturnTrue);
		safeJMP(imageBase + 0x6856F0, ReturnTrue);

		// Skip more
		safeJMP(imageBase + 0x641950, ReturnTrue);
		safeJMP(imageBase + 0xACDCE0, ReturnTrue);
		safeJMP(imageBase + 0x6B7030, ReturnTrue);
		safeJMP(imageBase + 0x6C73D0, ReturnTrue);
		safeJMP(imageBase + 0xA85F20, ReturnTrue);
		safeJMP(imageBase + 0x64F600, ReturnTrue);
		safeJMP(imageBase + 0x61BD00, ReturnTrue);

		safeJMP(imageBase + 0x6C8818, LoadWmmt5CarData);

		// Save progress trigger
		injector::WriteMemory<WORD>(imageBase + 0x655154, 0xB848, true);
		injector::WriteMemory<uintptr_t>(imageBase + 0x655154 + 2, (uintptr_t)SaveOk, true);
		injector::WriteMemory<DWORD>(imageBase + 0x655154 + 0xA, 0x9090D0FF, true);

		// Try save later!
		injector::MakeNOP(imageBase + 0x399A56, 0x12);
		injector::WriteMemory<WORD>(imageBase + 0x399A56, 0xB848, true);
		injector::WriteMemory<uintptr_t>(imageBase + 0x399A56 + 2, (uintptr_t)SaveGameData, true);
		injector::WriteMemory<DWORD>(imageBase + 0x399A60, 0x3348D0FF, true);
		injector::WriteMemory<WORD>(imageBase + 0x399A60 + 4, 0x90C0, true);
		*/
	}

	MH_EnableHook(MH_ALL_HOOKS);
}, GameID::WMMT6);
#endif
#pragma optimize("", on)