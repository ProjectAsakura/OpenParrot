#include <StdInc.h>
#include "Utility/InitFunction.h"
#include "Functions/Global.h"
#include "MinHook.h"
#include <Utility/Hooking.Patterns.h>
#include <thread>
#include <iostream>
#include <Windowsx.h>
#include <Utility/TouchSerial/MT6.h>
#include <fstream>
#ifdef _M_AMD64
#pragma optimize("", off)
#pragma comment(lib, "Ws2_32.lib")

extern LPCSTR hookPort;
uintptr_t imageBase;
static unsigned char hasp_buffer[0xD40];
static bool isFreePlay;
static bool isEventMode2P;
static bool isEventMode4P;
const char* ipaddr;

// Data for IC card, Force Feedback etc OFF.
unsigned char settingData[408] = {
	0x1F, 0x8B, 0x08, 0x08, 0x53, 0x6A, 0x8B, 0x5A, 0x00, 0x03, 0x46, 0x73,
	0x65, 0x74, 0x74, 0x69, 0x6E, 0x67, 0x2E, 0x6C, 0x75, 0x61, 0x00, 0x85,
	0x93, 0x5B, 0x6F, 0x82, 0x30, 0x14, 0xC7, 0xDF, 0xF9, 0x14, 0x7E, 0x01,
	0x17, 0x11, 0xE7, 0xDC, 0xC3, 0x1E, 0x14, 0x65, 0x9A, 0x48, 0x66, 0x94,
	0x68, 0xB2, 0xB7, 0x5A, 0x8E, 0xD2, 0xD8, 0x8B, 0x29, 0xED, 0x16, 0xBF,
	0xFD, 0x5A, 0xA8, 0x50, 0xB2, 0x65, 0xF2, 0x40, 0xF8, 0xFF, 0xCE, 0x85,
	0x73, 0x69, 0xFB, 0xFD, 0xFF, 0x9F, 0xC0, 0xBE, 0x7A, 0x25, 0x28, 0x45,
	0xF8, 0xF9, 0x89, 0x6A, 0x14, 0x3C, 0x08, 0xE8, 0x07, 0x01, 0x8B, 0x11,
	0x25, 0xC7, 0x25, 0xE2, 0x39, 0x85, 0x18, 0xB8, 0x02, 0xD9, 0x7B, 0xEB,
	0x45, 0xC3, 0x97, 0xF1, 0xC4, 0x99, 0xA6, 0x18, 0x03, 0x6D, 0x2C, 0x03,
	0x47, 0x67, 0x12, 0x5D, 0xE0, 0x17, 0x4D, 0x85, 0x12, 0xB2, 0xA1, 0xCF,
	0x61, 0xE8, 0x78, 0x26, 0x34, 0x2E, 0xD6, 0x70, 0x52, 0x86, 0x0E, 0x07,
	0xA3, 0x89, 0x8F, 0xB7, 0xE4, 0x5C, 0x58, 0x1E, 0x8E, 0xA2, 0x68, 0xEC,
	0x1B, 0x32, 0x71, 0xFD, 0x0B, 0xCF, 0x84, 0x52, 0x82, 0xB5, 0x89, 0x04,
	0xE1, 0x71, 0xA1, 0x15, 0x58, 0xDF, 0x80, 0xCD, 0xF4, 0x2D, 0x46, 0x32,
	0x8F, 0x45, 0x69, 0x73, 0x46, 0x01, 0x7B, 0x47, 0x0C, 0x9C, 0x1A, 0x5A,
	0x6F, 0x6E, 0x66, 0xA3, 0x3D, 0x92, 0x68, 0x4A, 0x63, 0xA1, 0x65, 0x79,
	0x67, 0x23, 0xC3, 0x24, 0xC0, 0x86, 0xA2, 0x5B, 0x9D, 0x72, 0x83, 0x8F,
	0xAB, 0xBC, 0x6E, 0x72, 0x85, 0x6D, 0xF2, 0xED, 0xB7, 0xAF, 0xF6, 0xC0,
	0xF3, 0xFB, 0x10, 0xD2, 0xB3, 0x6F, 0x4F, 0x84, 0xC4, 0x90, 0x00, 0xE4,
	0x47, 0x84, 0x2F, 0x35, 0x3A, 0x10, 0x5E, 0x4E, 0x79, 0xBE, 0x05, 0x86,
	0xCC, 0x57, 0x9D, 0x7F, 0xF1, 0x65, 0x06, 0x96, 0x8A, 0x1C, 0x6A, 0x97,
	0x46, 0xCE, 0x49, 0x55, 0x8F, 0x8F, 0x4C, 0xA1, 0xDC, 0xD5, 0x18, 0x53,
	0x51, 0x42, 0x76, 0xBB, 0x82, 0x6B, 0xCC, 0xCA, 0x9D, 0xE6, 0x46, 0xBD,
	0x8E, 0x9D, 0x4C, 0x45, 0x47, 0x66, 0x1A, 0x7C, 0x79, 0x80, 0xBC, 0x63,
	0x2D, 0xB4, 0x2F, 0x13, 0x49, 0x7C, 0xB9, 0x43, 0xCA, 0x97, 0xF3, 0x6A,
	0x36, 0x56, 0x56, 0x2B, 0xD9, 0x20, 0x0E, 0xB4, 0x2E, 0xD5, 0x8E, 0x7B,
	0x2F, 0xAC, 0x08, 0x8D, 0x9A, 0x2A, 0x25, 0x11, 0x56, 0x2D, 0xF8, 0x38,
	0x9D, 0x28, 0xE1, 0xD0, 0x76, 0x6B, 0xD2, 0xE1, 0x8B, 0xA1, 0xE6, 0xD0,
	0xD6, 0x20, 0x23, 0x0C, 0x3E, 0x05, 0xBF, 0xB7, 0x66, 0x77, 0x6F, 0x91,
	0xF9, 0xE3, 0xDA, 0x1D, 0x14, 0xCF, 0x69, 0x69, 0x16, 0xD7, 0x04, 0x4F,
	0x5A, 0x9E, 0x12, 0xEE, 0xE7, 0xDC, 0x69, 0xC6, 0x40, 0x5A, 0x63, 0x27,
	0xA0, 0x63, 0xE9, 0x86, 0x3C, 0xBC, 0x37, 0xD5, 0x4D, 0x5B, 0x7C, 0x24,
	0x8F, 0x3D, 0x7F, 0x00, 0x10, 0x1E, 0x34, 0xD9, 0xB5, 0x03, 0x00, 0x00
};


#define HASP_STATUS_OK 0
unsigned int Hook_hasp_login(int feature_id, void* vendor_code, int hasp_handle) {
#ifdef _DEBUG
	OutputDebugStringA("hasp_login\n");
#endif
	return HASP_STATUS_OK;
}

unsigned int Hook_hasp_logout(int hasp_handle) {
#ifdef _DEBUG
	OutputDebugStringA("hasp_logout\n");
#endif
	return HASP_STATUS_OK;
}

unsigned int Hook_hasp_encrypt(int hasp_handle, unsigned char* buffer, unsigned int buffer_size) {
#ifdef _DEBUG
	OutputDebugStringA("hasp_encrypt\n");
#endif
	return HASP_STATUS_OK;
}

unsigned int Hook_hasp_decrypt(int hasp_handle, unsigned char* buffer, unsigned int buffer_size) {
#ifdef _DEBUG
	OutputDebugStringA("hasp_decrypt\n");
#endif
	return HASP_STATUS_OK;
}

unsigned int Hook_hasp_get_size(int hasp_handle, int hasp_fileid, unsigned int* hasp_size) {
#ifdef _DEBUG
	OutputDebugStringA("hasp_get_size\n");
#endif
	* hasp_size = 0xD40; // Max addressable size by the game... absmax is 4k
	return HASP_STATUS_OK;
}

unsigned int Hook_hasp_read(int hasp_handle, int hasp_fileid, unsigned int offset, unsigned int length, unsigned char* buffer) {
#ifdef _DEBUG
	OutputDebugStringA("hasp_read\n");
#endif
	memcpy(buffer, hasp_buffer + offset, length);
	return HASP_STATUS_OK;
}

unsigned int Hook_hasp_write(int hasp_handle, int hasp_fileid, unsigned int offset, unsigned int length, unsigned char* buffer) {
	return HASP_STATUS_OK;
}

// ******************************************** //
// ************ Debug Data Logging ************ //
// ******************************************** //

// ************* Global Variables ************* //

// **** String Variables

// Debugging event log file
std::string logfile = "wmmt5_errors.txt";

// writeLog(filename: String, message: String): Int
// Given a filename string and a message string, appends
// the message to the given file.
static int writeLog(std::string filename, std::string message)
{
	// Log file to write to
	std::ofstream eventLog;

	// Open the filename provided (append mode)
	eventLog.open(filename, std::ios_base::app);

	// File open success
	if (eventLog.is_open())
	{
		// Write the message to the file
		eventLog << message;

		// Close the log file handle
		eventLog.close();

		// Success
		return 0;
	}
	else // File open failed
	{
		// Failure
		return 1;
	}
}

static int ReturnTrue()
{
	return 1;
}

void GenerateDongleData(bool isTerminal)
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
		memcpy(hasp_buffer + 0xD00, "272211990002", 12);
		hasp_buffer[0xD3E] = 0x63;
		hasp_buffer[0xD3F] = 0x9C;
	}
	else
	{
		memcpy(hasp_buffer + 0xD00, "272213990002", 12);
		hasp_buffer[0xD3E] = 0x65;
		hasp_buffer[0xD3F] = 0x9A;
	}
}

static HWND mt6Hwnd;

typedef BOOL(WINAPI* ShowWindow_t)(HWND, int);
static ShowWindow_t pShowWindow;


// Hello Win32 my old friend...
typedef LRESULT(WINAPI* WindowProcedure_t)(HWND, UINT, WPARAM, LPARAM);
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
		mt6SetDisplayParams(hwnd);
		gotWindowSize = TRUE;
	}

	if (msg == WM_LBUTTONDOWN ||
		msg == WM_LBUTTONUP)
	{
		mt6SetTouchData(lParam, msg == WM_LBUTTONDOWN, false);
		return 0;
	}

	if (msg == WM_POINTERDOWN ||
		msg == WM_POINTERUP)
	{
		mt6SetTouchData(lParam, msg == WM_POINTERDOWN, true);
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

typedef int (WINAPI* BIND)(SOCKET, CONST SOCKADDR*, INT);
BIND pbind = NULL;

unsigned int WINAPI Hook_bind(SOCKET s, const sockaddr* addr, int namelen) {
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

extern int* ffbOffset;
extern int* ffbOffset2;
extern int* ffbOffset3;
extern int* ffbOffset4;

DWORD WINAPI Wmmt5FfbCollector(void* ctx)
{
	uintptr_t imageBase = (uintptr_t)GetModuleHandleA(0);
	while (true)
	{
		*ffbOffset = *(DWORD*)(imageBase + 0x196F188);
		*ffbOffset2 = *(DWORD*)(imageBase + 0x196F18c);
		*ffbOffset3 = *(DWORD*)(imageBase + 0x196F190);
		*ffbOffset4 = *(DWORD*)(imageBase + 0x196F194);
		Sleep(10);
	}
}

static InitFunction Wmmt5Func([]()
{
	// Alloc debug console
	FreeConsole();
	AllocConsole();

#ifdef NDEBUG
	// Hide the debug console
	mt6Hwnd = FindWindowA("ConsoleWindowClass", NULL);
	ShowWindow(mt6Hwnd, 0);
#endif

	SetConsoleTitle(L"Maxitune5 Console");

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

	FILE* fileF = _wfopen(L"Fsetting.lua.gz", L"r");
	if (fileF == NULL)
	{
		FILE* settingsF = _wfopen(L"Fsetting.lua.gz", L"wb");
		fwrite(settingData, 1, sizeof(settingData), settingsF);
		fclose(settingsF);
	}
	else
	{
		fclose(fileF);
	}

	FILE* fileG = _wfopen(L"Gsetting.lua.gz", L"r");
	if (fileG == NULL)
	{
		FILE* settingsG = _wfopen(L"Gsetting.lua.gz", L"wb");
		fwrite(settingData, 1, sizeof(settingData), settingsG);
		fclose(settingsG);
	}
	else
	{
		fclose(fileG);
	}


	bool isTerminal = false;
	if (ToBool(config["General"]["TerminalMode"]))
	{
		isTerminal = true;
	}

	std::string networkip = config["General"]["NetworkAdapterIP"];
	if (!networkip.empty())
	{
		//strcpy(ipaddr, networkip.c_str());
		ipaddr = networkip.c_str();
	}

	hookPort = "COM3";
	imageBase = (uintptr_t)GetModuleHandleA(0);

	MH_Initialize();

	// Hook dongle funcs
	MH_CreateHookApi(L"hasp_windows_x64_109906.dll", "hasp_write", Hook_hasp_write, NULL);
	MH_CreateHookApi(L"hasp_windows_x64_109906.dll", "hasp_read", Hook_hasp_read, NULL);
	MH_CreateHookApi(L"hasp_windows_x64_109906.dll", "hasp_get_size", Hook_hasp_get_size, NULL);
	MH_CreateHookApi(L"hasp_windows_x64_109906.dll", "hasp_decrypt", Hook_hasp_decrypt, NULL);
	MH_CreateHookApi(L"hasp_windows_x64_109906.dll", "hasp_encrypt", Hook_hasp_encrypt, NULL);
	MH_CreateHookApi(L"hasp_windows_x64_109906.dll", "hasp_logout", Hook_hasp_logout, NULL);
	MH_CreateHookApi(L"hasp_windows_x64_109906.dll", "hasp_login", Hook_hasp_login, NULL);
	MH_CreateHookApi(L"WS2_32", "bind", Hook_bind, reinterpret_cast<LPVOID*>(&pbind));

	// Give me the HWND please maxitune
	MH_CreateHookApi(L"user32", "ShowWindow", Hook_ShowWindow, reinterpret_cast<LPVOID*>(&pShowWindow));
	//pMaxituneWndProc = (WindowProcedure_t)(imageBasedxplus + 0xB78B90);
	pMaxituneWndProc = (WindowProcedure_t)(hook::get_pattern("48 89 5C 24 08 48 89 6C 24 10 48 89 74 24 18 57 48 83 EC 30 8B EA BA EB FF FF FF 49 8B F9 49 8B F0 48 8B D9 FF 15 ? ? ? 00 48 85 C0 74 1D 4C", 0));

	//load banapass emu
	LoadLibraryA(".\\openBanaW5p.dll");

	// disable assert errors
	uintptr_t wassertPtr = (*(uintptr_t*)(imageBase + 0xc840c0));
	safeJMP(wassertPtr, ReturnTrue);

	GenerateDongleData(isTerminal);
		
	injector::WriteMemory<uint8_t>(hook::get_pattern("0F 94 C0 84 C0 0F 94 C0 84 C0 75 05 45 32 ? EB", 0x13), 0, true);
	injector::MakeNOP(hook::get_pattern("83 C0 FD 83 F8 01 0F 87 B4 00 00 00", 6), 6);

	// Skip weird camera init that stucks entire pc on certain brands. TESTED ONLY ON 05!!!!
	if (ToBool(config["General"]["WhiteScreenFix"]))
	{
		injector::WriteMemory<DWORD>(hook::get_pattern("48 8B C4 55 57 41 54 41 55 41 56 48 8D 68 A1 48 81 EC 90 00 00 00 48 C7 45 D7 FE FF FF FF 48 89 58 08 48 89 70 18 45 33 F6 4C 89 75 DF 33 C0 48 89 45 E7", 0), 0x90C3C032, true);
	}

	{
		auto location = hook::get_pattern<char>("41 3B C7 74 0E 48 8D 8F B8 00 00 00 BA F6 01 00 00 EB 6E 48 8D 8F A0 00 00 00");
			
		injector::WriteMemory<uint8_t>(location + 3, 0xEB, true);

		injector::MakeNOP(location + 0x22, 2);

		injector::MakeNOP(location + 0x33, 2);
	}

	{
		auto location = hook::get_pattern<char>("48 83 EC 28 33 D2 B9 70 00 02 00 E8 ? ? ? ? 85 C0 79 06");
		injector::WriteMemory<uint8_t>(location + 0x12, 0xEB, true);
	}

	if (isTerminal)
	{
		safeJMP(hook::get_pattern("0F B6 41 05 2C 30 3C 09 77 04 0F BE C0 C3 83 C8 FF C3"), ReturnTrue);

		safeJMP(hook::get_pattern("40 53 48 83 EC 20 48 83 39 00 48 8B D9 75 28 48 8D ? ? ? ? 00 48 8D ? ? ? ? 00 41 B8 ? ? 00 00 FF 15 ? ? ? ? 4C 8B 1B 41 0F B6 43 78"), ReturnTrue);
	}
	else
	{
		{
			auto location = hook::get_pattern<char>("48 8B 18 48 3B D8 0F 84 88 00 00 00 39 7B 1C 74 60 80 7B 31 00 75 4F 48 8B 43 10 80 78 31 00");
			injector::MakeNOP(location + 6, 6); // 6
			injector::MakeNOP(location + 0xF, 2); // 0xF
			injector::MakeNOP(location + 0x15, 2); // 0x15
		}
	}

	auto chars = { 'F', 'G' };

	for (auto cha : chars)
	{
		auto patterns = hook::pattern(va("%02X 3A 2F", cha));

		if (patterns.size() > 0)
		{
			for (int i = 0; i < patterns.size(); i++)
			{
				char* text = patterns.get(i).get<char>(0);
				std::string text_str(text);

				std::string to_replace = va("%c:/", cha);
				std::string replace_with = va("./%c", cha);

				std::string replaced = text_str.replace(0, to_replace.length(), replace_with);

				injector::WriteMemoryRaw(text, (char*)replaced.c_str(), replaced.length() + 1, true);
			}
		}
	}

	if (ToBool(config["General"]["SkipMovies"]))
	{
		// Skip movies fuck you wmmt5
		safeJMP(imageBase + 0x806020, ReturnTrue);
	}

	// Save story stuff (only 05)
	{
		// Fix F and G drive crash on mileage screen
		injector::MakeNOP(imageBase + 0xCE5DF, 5);
		CreateThread(0, 0, Wmmt5FfbCollector, 0, 0, 0);
	}

	MH_EnableHook(MH_ALL_HOOKS);

}, GameID::WMMT5);
#endif
#pragma optimize("", on)