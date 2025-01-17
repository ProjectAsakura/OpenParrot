#include <StdInc.h>
#include "Utility/InitFunction.h"
#include "Functions/Global.h"
#include "MinHook.h"
#include <Utility/Hooking.Patterns.h>
#include <thread>
#ifdef _M_AMD64

#pragma optimize("", off)
#pragma comment(lib, "Ws2_32.lib")

extern LPCSTR hookPort;
static uintptr_t imageBase;
static unsigned char hasp_buffer[0xD40];
static bool isFreePlay;
static bool isEventMode2P;
static bool isEventMode4P;
static const char *ipaddr;

// Data for IC card, Force Feedback etc OFF.
static unsigned char settingData[405] = {
	0x1F, 0x8B, 0x08, 0x08, 0x53, 0x6A, 0x8B, 0x5A, 0x00, 0x00, 0x73, 0x65,
	0x74, 0x74, 0x69, 0x6E, 0x67, 0x2E, 0x6C, 0x75, 0x61, 0x00, 0x85, 0x93,
	0xC9, 0x6E, 0xC2, 0x30, 0x10, 0x86, 0xEF, 0x79, 0x0A, 0x5E, 0x80, 0x8A,
	0x90, 0x80, 0xE8, 0xA1, 0x07, 0x08, 0xA4, 0x20, 0x11, 0x81, 0x20, 0x2A,
	0x52, 0x6F, 0xC6, 0x19, 0x88, 0x85, 0x17, 0xE4, 0xD8, 0xAD, 0x78, 0xFB,
	0xDA, 0x59, 0x1D, 0xB5, 0x2A, 0x39, 0x44, 0xF9, 0xBF, 0x59, 0x32, 0x8B,
	0x3D, 0x1C, 0xFE, 0xFF, 0x78, 0xF6, 0x35, 0x28, 0x40, 0x29, 0xC2, 0xAF,
	0x2F, 0x54, 0x23, 0xEF, 0x49, 0xC0, 0xD0, 0xF3, 0x58, 0x84, 0x28, 0x39,
	0xAF, 0x11, 0xCF, 0x28, 0x44, 0xC0, 0x15, 0xC8, 0xC1, 0xDB, 0x20, 0x08,
	0x27, 0xD3, 0x51, 0x6D, 0x9A, 0x63, 0x0C, 0xB4, 0xB5, 0x34, 0x74, 0x21,
	0xD1, 0x0D, 0x7E, 0xD1, 0x44, 0x28, 0x21, 0x5B, 0x3A, 0xF1, 0xFD, 0x9A,
	0xA7, 0x42, 0xE3, 0x7C, 0x0B, 0x17, 0x65, 0xE8, 0x78, 0x14, 0xCE, 0x5C,
	0x7C, 0x20, 0xD7, 0xDC, 0x72, 0x3F, 0x0C, 0x82, 0xA9, 0x6B, 0x48, 0xC5,
	0xFD, 0x2F, 0xBC, 0x10, 0x4A, 0x09, 0xD6, 0x25, 0x12, 0x84, 0x47, 0xB9,
	0x56, 0x60, 0x7D, 0x3D, 0xB6, 0xD0, 0x8F, 0x08, 0xC9, 0x2C, 0x12, 0x85,
	0xCD, 0x19, 0x78, 0xEC, 0x1D, 0x31, 0xA8, 0xD5, 0xD8, 0x7A, 0x73, 0x33,
	0x1B, 0xED, 0x90, 0x58, 0x53, 0x1A, 0x09, 0x2D, 0x8B, 0x86, 0x85, 0x86,
	0x49, 0x80, 0x3D, 0x45, 0x8F, 0x2A, 0xE5, 0x1E, 0x9F, 0x37, 0x59, 0xD5,
	0xE4, 0x06, 0xDB, 0xE4, 0x87, 0x6F, 0x57, 0x7D, 0x00, 0xCF, 0x9A, 0x21,
	0x24, 0x57, 0xD7, 0x1E, 0x0B, 0x89, 0x21, 0x06, 0xC8, 0xCE, 0x08, 0xDF,
	0x2A, 0x74, 0x22, 0xBC, 0x98, 0xF3, 0xEC, 0x00, 0x0C, 0x99, 0xAF, 0x2A,
	0xFF, 0xEA, 0xCB, 0x0C, 0x2C, 0x11, 0x19, 0x54, 0x2E, 0xAD, 0x5C, 0x92,
	0xB2, 0x1E, 0x17, 0x99, 0x42, 0x79, 0x5D, 0x63, 0x44, 0x45, 0x01, 0xE9,
	0xE3, 0x0E, 0x75, 0x63, 0x56, 0x1E, 0x35, 0x37, 0xEA, 0x75, 0x5A, 0xCB,
	0x44, 0xF4, 0x64, 0xAA, 0xC1, 0x95, 0x27, 0xC8, 0x7A, 0xD6, 0x5C, 0xBB,
	0x32, 0x96, 0xC4, 0x95, 0x47, 0xA4, 0x5C, 0xB9, 0x2C, 0x67, 0x63, 0x65,
	0xB9, 0x92, 0x3D, 0xE2, 0x40, 0xAB, 0x52, 0xED, 0xB8, 0x3F, 0x84, 0x15,
	0xBE, 0x51, 0x73, 0xA5, 0x24, 0xC2, 0xAA, 0x03, 0xBB, 0xCB, 0x85, 0x12,
	0x0E, 0x5D, 0xB7, 0x26, 0x1D, 0xBE, 0x19, 0x6A, 0x0E, 0x6D, 0x05, 0x52,
	0xC2, 0xE0, 0x53, 0xF0, 0xA6, 0x35, 0xBB, 0x7B, 0x8B, 0xCC, 0x1F, 0xB7,
	0xF5, 0x41, 0x71, 0x9C, 0xD6, 0x66, 0x71, 0x6D, 0xF0, 0xAC, 0xE3, 0x09,
	0xE1, 0x6E, 0xCE, 0xA3, 0x66, 0x0C, 0xA4, 0x35, 0xF6, 0x02, 0x7A, 0x96,
	0x7E, 0xC8, 0xD3, 0x7B, 0x53, 0xDE, 0xB4, 0xD5, 0x2E, 0x7E, 0xEE, 0xF9,
	0x03, 0x44, 0x94, 0xFB, 0x8E, 0xB5, 0x03, 0x00, 0x00
};

// FOR FREEPLAY
static unsigned char terminalPackage1_Free[79] = {
	0x01, 0x04, 0x4B, 0x00, 0x12, 0x14, 0x0A, 0x00, 0x10, 0x04, 0x18, 0x00,
	0x20, 0x00, 0x28, 0x00, 0x30, 0x00, 0x38, 0x00, 0x40, 0x00, 0x48, 0x00,
	0x50, 0x00, 0x1A, 0x02, 0x5A, 0x00, 0x2A, 0x12, 0x08, 0x12, 0x12, 0x0C,
	0x32, 0x37, 0x32, 0x32, 0x31, 0x31, 0x39, 0x39, 0x30, 0x30, 0x30, 0x32,
	0x18, 0x00, 0x30, 0x03, 0x4A, 0x08, 0x08, 0x01, 0x10, 0x01, 0x18, 0x00,
	0x20, 0x00, 0x52, 0x0B, 0x08, 0x64, 0x10, 0xDE, 0x0F, 0x18, 0x05, 0x20,
	0x00, 0x28, 0x00, 0xEC, 0x72, 0x00, 0x41
};

static unsigned char terminalPackage2_Free[139] = {
	0x01, 0x04, 0x87, 0x00, 0x12, 0x14, 0x0A, 0x00, 0x10, 0x04, 0x18, 0x00,
	0x20, 0x00, 0x28, 0x00, 0x30, 0x00, 0x38, 0x00, 0x40, 0x00, 0x48, 0x00,
	0x50, 0x00, 0x1A, 0x02, 0x5A, 0x00, 0x2A, 0x12, 0x08, 0x14, 0x12, 0x0C,
	0x32, 0x37, 0x32, 0x32, 0x31, 0x31, 0x39, 0x39, 0x30, 0x30, 0x30, 0x32,
	0x18, 0x00, 0x30, 0x03, 0x42, 0x3A, 0x08, 0x01, 0x10, 0x03, 0x18, 0x02,
	0x20, 0x02, 0x28, 0x04, 0x30, 0x01, 0x38, 0x01, 0x40, 0x00, 0x48, 0x00,
	0x50, 0x02, 0x58, 0x60, 0x60, 0x60, 0x68, 0x60, 0x70, 0x60, 0x78, 0x60,
	0x80, 0x01, 0x60, 0x88, 0x01, 0x60, 0x90, 0x01, 0x60, 0x98, 0x01, 0x00,
	0xA0, 0x01, 0xE2, 0xBA, 0xAC, 0xD4, 0x05, 0xA8, 0x01, 0x04, 0xB0, 0x01,
	0x24, 0xB8, 0x01, 0x00, 0x4A, 0x08, 0x08, 0x01, 0x10, 0x01, 0x18, 0x00,
	0x20, 0x00, 0x52, 0x0B, 0x08, 0x64, 0x10, 0xDE, 0x0F, 0x18, 0x05, 0x20,
	0x00, 0x28, 0x00, 0x99, 0x4E, 0xC6, 0x14
};

static unsigned char terminalPackage3_Free[79] = {
	0x01, 0x04, 0x4B, 0x00, 0x12, 0x14, 0x0A, 0x00, 0x10, 0x04, 0x18, 0x00,
	0x20, 0x00, 0x28, 0x00, 0x30, 0x00, 0x38, 0x00, 0x40, 0x00, 0x48, 0x00,
	0x50, 0x00, 0x1A, 0x02, 0x5A, 0x00, 0x2A, 0x12, 0x08, 0x19, 0x12, 0x0C,
	0x32, 0x37, 0x32, 0x32, 0x31, 0x31, 0x39, 0x39, 0x30, 0x30, 0x30, 0x32,
	0x18, 0x00, 0x30, 0x03, 0x4A, 0x08, 0x08, 0x01, 0x10, 0x01, 0x18, 0x00,
	0x20, 0x00, 0x52, 0x0B, 0x08, 0x64, 0x10, 0xDE, 0x0F, 0x18, 0x05, 0x20,
	0x00, 0x28, 0x00, 0x89, 0x93, 0x3A, 0x22
};

static unsigned char terminalPackage4_Free[139] = {
	0x01, 0x04, 0x87, 0x00, 0x12, 0x14, 0x0A, 0x00, 0x10, 0x04, 0x18, 0x00,
	0x20, 0x00, 0x28, 0x00, 0x30, 0x00, 0x38, 0x00, 0x40, 0x00, 0x48, 0x00,
	0x50, 0x00, 0x1A, 0x02, 0x5A, 0x00, 0x2A, 0x12, 0x08, 0x2E, 0x12, 0x0C,
	0x32, 0x37, 0x32, 0x32, 0x31, 0x31, 0x39, 0x39, 0x30, 0x30, 0x30, 0x32,
	0x18, 0x00, 0x30, 0x03, 0x42, 0x3A, 0x08, 0x01, 0x10, 0x03, 0x18, 0x02,
	0x20, 0x02, 0x28, 0x04, 0x30, 0x01, 0x38, 0x01, 0x40, 0x00, 0x48, 0x00,
	0x50, 0x02, 0x58, 0x60, 0x60, 0x60, 0x68, 0x60, 0x70, 0x60, 0x78, 0x60,
	0x80, 0x01, 0x60, 0x88, 0x01, 0x60, 0x90, 0x01, 0x60, 0x98, 0x01, 0x00,
	0xA0, 0x01, 0xF0, 0xBA, 0xAC, 0xD4, 0x05, 0xA8, 0x01, 0x04, 0xB0, 0x01,
	0x24, 0xB8, 0x01, 0x00, 0x4A, 0x08, 0x08, 0x01, 0x10, 0x01, 0x18, 0x00,
	0x20, 0x00, 0x52, 0x0B, 0x08, 0x64, 0x10, 0xDE, 0x0F, 0x18, 0x05, 0x20,
	0x00, 0x28, 0x00, 0x55, 0x42, 0x47, 0xD5
};

static unsigned char terminalPackage5_Free[79] = {
	0x01, 0x04, 0x4B, 0x00, 0x12, 0x14, 0x0A, 0x00, 0x10, 0x04, 0x18, 0x00,
	0x20, 0x00, 0x28, 0x00, 0x30, 0x00, 0x38, 0x00, 0x40, 0x00, 0x48, 0x00,
	0x50, 0x00, 0x1A, 0x02, 0x5A, 0x00, 0x2A, 0x12, 0x08, 0x2F, 0x12, 0x0C,
	0x32, 0x37, 0x32, 0x32, 0x31, 0x31, 0x39, 0x39, 0x30, 0x30, 0x30, 0x32,
	0x18, 0x00, 0x30, 0x03, 0x4A, 0x08, 0x08, 0x01, 0x10, 0x01, 0x18, 0x00,
	0x20, 0x00, 0x52, 0x0B, 0x08, 0x64, 0x10, 0xDE, 0x0F, 0x18, 0x05, 0x20,
	0x00, 0x28, 0x00, 0x9C, 0xC9, 0xE0, 0x73
};

static unsigned char terminalPackage6_Free[139] = {
	0x01, 0x04, 0x87, 0x00, 0x12, 0x14, 0x0A, 0x00, 0x10, 0x04, 0x18, 0x00,
	0x20, 0x00, 0x28, 0x00, 0x30, 0x00, 0x38, 0x00, 0x40, 0x00, 0x48, 0x00,
	0x50, 0x00, 0x1A, 0x02, 0x5A, 0x00, 0x2A, 0x12, 0x08, 0x6A, 0x12, 0x0C,
	0x32, 0x37, 0x32, 0x32, 0x31, 0x31, 0x39, 0x39, 0x30, 0x30, 0x30, 0x32,
	0x18, 0x00, 0x30, 0x03, 0x42, 0x3A, 0x08, 0x01, 0x10, 0x03, 0x18, 0x02,
	0x20, 0x02, 0x28, 0x04, 0x30, 0x01, 0x38, 0x01, 0x40, 0x00, 0x48, 0x00,
	0x50, 0x02, 0x58, 0x60, 0x60, 0x60, 0x68, 0x60, 0x70, 0x60, 0x78, 0x60,
	0x80, 0x01, 0x60, 0x88, 0x01, 0x60, 0x90, 0x01, 0x60, 0x98, 0x01, 0x00,
	0xA0, 0x01, 0xF1, 0xBA, 0xAC, 0xD4, 0x05, 0xA8, 0x01, 0x04, 0xB0, 0x01,
	0x24, 0xB8, 0x01, 0x00, 0x4A, 0x08, 0x08, 0x01, 0x10, 0x01, 0x18, 0x00,
	0x20, 0x00, 0x52, 0x0B, 0x08, 0x64, 0x10, 0xDE, 0x0F, 0x18, 0x05, 0x20,
	0x00, 0x28, 0x00, 0x26, 0xB7, 0x89, 0xD0
};

// FOR COIN ENTRY!
static unsigned char terminalPackage1_Coin[75] = {
	0x01, 0x04, 0x47, 0x00, 0x12, 0x14, 0x0A, 0x00, 0x10, 0x04, 0x18, 0x00,
	0x20, 0x00, 0x28, 0x00, 0x30, 0x00, 0x38, 0x00, 0x40, 0x00, 0x48, 0x00,
	0x50, 0x00, 0x1A, 0x00, 0x2A, 0x12, 0x08, 0x0B, 0x12, 0x0C, 0x32, 0x37,
	0x32, 0x32, 0x31, 0x31, 0x39, 0x39, 0x30, 0x30, 0x30, 0x32, 0x18, 0x00,
	0x4A, 0x08, 0x08, 0x01, 0x10, 0x01, 0x18, 0x00, 0x20, 0x00, 0x52, 0x0B,
	0x08, 0x64, 0x10, 0xDE, 0x0F, 0x18, 0x05, 0x20, 0x00, 0x28, 0x00, 0x09,
	0x06, 0x41, 0x0B
};

static unsigned char terminalPackage2_Coin[135] = {
	0x01, 0x04, 0x83, 0x00, 0x12, 0x14, 0x0A, 0x00, 0x10, 0x04, 0x18, 0x00,
	0x20, 0x00, 0x28, 0x00, 0x30, 0x00, 0x38, 0x00, 0x40, 0x00, 0x48, 0x00,
	0x50, 0x00, 0x1A, 0x00, 0x2A, 0x12, 0x08, 0x39, 0x12, 0x0C, 0x32, 0x37,
	0x32, 0x32, 0x31, 0x31, 0x39, 0x39, 0x30, 0x30, 0x30, 0x32, 0x18, 0x00,
	0x42, 0x3A, 0x08, 0x01, 0x10, 0x03, 0x18, 0x02, 0x20, 0x02, 0x28, 0x04,
	0x30, 0x00, 0x38, 0x01, 0x40, 0x00, 0x48, 0x00, 0x50, 0x02, 0x58, 0x60,
	0x60, 0x60, 0x68, 0x60, 0x70, 0x60, 0x78, 0x60, 0x80, 0x01, 0x60, 0x88,
	0x01, 0x60, 0x90, 0x01, 0x60, 0x98, 0x01, 0x00, 0xA0, 0x01, 0xD5, 0xBE,
	0x8F, 0xD2, 0x05, 0xA8, 0x01, 0x04, 0xB0, 0x01, 0x24, 0xB8, 0x01, 0x00,
	0x4A, 0x08, 0x08, 0x01, 0x10, 0x01, 0x18, 0x00, 0x20, 0x00, 0x52, 0x0B,
	0x08, 0x64, 0x10, 0xDE, 0x0F, 0x18, 0x05, 0x20, 0x00, 0x28, 0x00, 0xF5,
	0xF1, 0x0D, 0xB2
};

static unsigned char terminalPackage3_Coin[75] = {
	0x01, 0x04, 0x47, 0x00, 0x12, 0x14, 0x0A, 0x00, 0x10, 0x04, 0x18, 0x00,
	0x20, 0x00, 0x28, 0x00, 0x30, 0x00, 0x38, 0x00, 0x40, 0x00, 0x48, 0x00,
	0x50, 0x00, 0x1A, 0x00, 0x2A, 0x12, 0x08, 0x3A, 0x12, 0x0C, 0x32, 0x37,
	0x32, 0x32, 0x31, 0x31, 0x39, 0x39, 0x30, 0x30, 0x30, 0x32, 0x18, 0x00,
	0x4A, 0x08, 0x08, 0x01, 0x10, 0x01, 0x18, 0x00, 0x20, 0x00, 0x52, 0x0B,
	0x08, 0x64, 0x10, 0xDE, 0x0F, 0x18, 0x05, 0x20, 0x00, 0x28, 0x00, 0x22,
	0x25, 0x31, 0x0D
};

static unsigned char terminalPackage4_Coin[135] = {
	0x01, 0x04, 0x83, 0x00, 0x12, 0x14, 0x0A, 0x00, 0x10, 0x04, 0x18, 0x00,
	0x20, 0x00, 0x28, 0x00, 0x30, 0x00, 0x38, 0x00, 0x40, 0x00, 0x48, 0x00,
	0x50, 0x00, 0x1A, 0x00, 0x2A, 0x12, 0x08, 0x57, 0x12, 0x0C, 0x32, 0x37,
	0x32, 0x32, 0x31, 0x31, 0x39, 0x39, 0x30, 0x30, 0x30, 0x32, 0x18, 0x00,
	0x42, 0x3A, 0x08, 0x01, 0x10, 0x03, 0x18, 0x02, 0x20, 0x02, 0x28, 0x04,
	0x30, 0x00, 0x38, 0x01, 0x40, 0x00, 0x48, 0x00, 0x50, 0x02, 0x58, 0x60,
	0x60, 0x60, 0x68, 0x60, 0x70, 0x60, 0x78, 0x60, 0x80, 0x01, 0x60, 0x88,
	0x01, 0x60, 0x90, 0x01, 0x60, 0x98, 0x01, 0x00, 0xA0, 0x01, 0xD6, 0xBE,
	0x8F, 0xD2, 0x05, 0xA8, 0x01, 0x04, 0xB0, 0x01, 0x24, 0xB8, 0x01, 0x00,
	0x4A, 0x08, 0x08, 0x01, 0x10, 0x01, 0x18, 0x00, 0x20, 0x00, 0x52, 0x0B,
	0x08, 0x64, 0x10, 0xDE, 0x0F, 0x18, 0x05, 0x20, 0x00, 0x28, 0x00, 0xCA,
	0x8B, 0x15, 0xCB
};

static unsigned char terminalPackage5_Coin[79] = {
	0x01, 0x04, 0x4B, 0x00, 0x12, 0x14, 0x0A, 0x00, 0x10, 0x04, 0x18, 0x00,
	0x20, 0x00, 0x28, 0x00, 0x30, 0x00, 0x38, 0x00, 0x40, 0x00, 0x48, 0x00,
	0x50, 0x00, 0x1A, 0x02, 0x5A, 0x00, 0x2A, 0x12, 0x08, 0x58, 0x12, 0x0C,
	0x32, 0x37, 0x32, 0x32, 0x31, 0x31, 0x39, 0x39, 0x30, 0x30, 0x30, 0x32,
	0x18, 0x00, 0x30, 0x03, 0x4A, 0x08, 0x08, 0x01, 0x10, 0x01, 0x18, 0x00,
	0x20, 0x00, 0x52, 0x0B, 0x08, 0x64, 0x10, 0xDE, 0x0F, 0x18, 0x05, 0x20,
	0x00, 0x28, 0x00, 0x3E, 0xB1, 0xB7, 0x22
};

static unsigned char terminalPackage6_Coin[139] = {
	0x01, 0x04, 0x87, 0x00, 0x12, 0x14, 0x0A, 0x00, 0x10, 0x04, 0x18, 0x00,
	0x20, 0x00, 0x28, 0x00, 0x30, 0x00, 0x38, 0x00, 0x40, 0x00, 0x48, 0x00,
	0x50, 0x00, 0x1A, 0x02, 0x5A, 0x00, 0x2A, 0x12, 0x08, 0x77, 0x12, 0x0C,
	0x32, 0x37, 0x32, 0x32, 0x31, 0x31, 0x39, 0x39, 0x30, 0x30, 0x30, 0x32,
	0x18, 0x00, 0x30, 0x03, 0x42, 0x3A, 0x08, 0x01, 0x10, 0x03, 0x18, 0x02,
	0x20, 0x02, 0x28, 0x04, 0x30, 0x00, 0x38, 0x01, 0x40, 0x00, 0x48, 0x00,
	0x50, 0x02, 0x58, 0x60, 0x60, 0x60, 0x68, 0x60, 0x70, 0x60, 0x78, 0x60,
	0x80, 0x01, 0x60, 0x88, 0x01, 0x60, 0x90, 0x01, 0x60, 0x98, 0x01, 0x00,
	0xA0, 0x01, 0xD7, 0xBE, 0x8F, 0xD2, 0x05, 0xA8, 0x01, 0x04, 0xB0, 0x01,
	0x24, 0xB8, 0x01, 0x00, 0x4A, 0x08, 0x08, 0x01, 0x10, 0x01, 0x18, 0x00,
	0x20, 0x00, 0x52, 0x0B, 0x08, 0x64, 0x10, 0xDE, 0x0F, 0x18, 0x05, 0x20,
	0x00, 0x28, 0x00, 0xBD, 0x07, 0xCF, 0xDC
};


//Event mode 2P
static unsigned char terminalPackage1_Event4P[79] = {
	0x01, 0x04, 0x44, 0x00, 0x12, 0x0e, 0x0a, 0x00, 0x10, 0x04, 0x18, 0x00,
	0x20, 0x00, 0x28, 0x00, 0x30, 0x00, 0x38, 0x00, 0x1a, 0x00, 0x2a, 0x13,
	0x08, 0xd1, 0x0b, 0x12, 0x0c, 0x32, 0x37, 0x32, 0x32, 0x31, 0x31, 0x39,
	0x39, 0x30, 0x30, 0x30, 0x32, 0x18, 0x00, 0x30, 0x00, 0x4a, 0x08, 0x08, 
	0x03, 0x10, 0x01, 0x18, 0x00, 0x20, 0x00, 0x52, 0x0b, 0x08, 0x64, 0x10,
	0xde, 0x0f, 0x18, 0x05, 0x20, 0x00, 0x28, 0x00, 0xc1, 0x96, 0xc9, 0x2e
};

static unsigned char terminalPackage2_Event4P[139] = {
	0x01, 0x04, 0x80, 0x00, 0x12, 0x0e, 0x0a, 0x00, 0x10, 0x04, 0x18, 0x00,
	0x20, 0x00, 0x28, 0x00, 0x30, 0x00, 0x38, 0x00, 0x1a, 0x00, 0x2a, 0x13,
	0x08, 0xd2, 0x0b, 0x12, 0x0c, 0x32, 0x37, 0x32, 0x32, 0x31, 0x31, 0x39,
	0x39, 0x30, 0x30, 0x30, 0x32, 0x18, 0x00, 0x30, 0x00, 0x42, 0x3a, 0x08,
	0x01, 0x10, 0x03, 0x18, 0x02, 0x20, 0x02, 0x28, 0x04, 0x30, 0x01, 0x38,
	0x01, 0x40, 0x01, 0x48, 0x00, 0x50, 0x02, 0x58, 0x60, 0x60, 0x60, 0x68,
	0x60, 0x70, 0x60, 0x78, 0x60, 0x80, 0x01, 0x60, 0x88, 0x01, 0x60, 0x90,
	0x01, 0x60, 0x98, 0x01, 0x00, 0xa0, 0x01, 0xd8, 0xc3, 0xd6, 0xe1, 0x05,
	0xa8, 0x01, 0x04, 0xb0, 0x01, 0x24, 0xb8, 0x01, 0x00, 0x4a, 0x08, 0x08,
	0x03, 0x10, 0x01, 0x18, 0x00, 0x20, 0x00, 0x52, 0x0b, 0x08, 0x64, 0x10,
	0xde, 0x0f, 0x18, 0x05, 0x20, 0x00, 0x28, 0x00, 0x91, 0x74, 0xca, 0x1e

};

static unsigned char terminalPackage3_Event4P[79] = {
	0x01, 0x04, 0x44, 0x00, 0x12, 0x0e, 0x0a, 0x00, 0x10, 0x04, 0x18, 0x00,
	0x20, 0x00, 0x28, 0x00, 0x30, 0x00, 0x38, 0x00, 0x1a, 0x00, 0x2a, 0x13,
	0x08, 0x8d, 0x0c, 0x12, 0x0c, 0x32, 0x37, 0x32, 0x32, 0x31, 0x31, 0x39,
	0x39, 0x30, 0x30, 0x30, 0x32, 0x18, 0x00, 0x30, 0x00, 0x4a, 0x08, 0x08,
	0x03, 0x10, 0x01, 0x18, 0x00, 0x20, 0x00, 0x52, 0x0b, 0x08, 0x64, 0x10,
	0xde, 0x0f, 0x18, 0x05, 0x20, 0x00, 0x28, 0x00, 0x86, 0xb1, 0x27, 0x9e
};

static unsigned char terminalPackage4_Event4P[139] = {
	0x01, 0x04, 0x80, 0x00, 0x12, 0x0e, 0x0a, 0x00, 0x10, 0x04, 0x18, 0x00,
	0x20, 0x00, 0x28, 0x00, 0x30, 0x00, 0x38, 0x00, 0x1a, 0x00, 0x2a, 0x13,
	0x08, 0x8e, 0x0c, 0x12, 0x0c, 0x32, 0x37, 0x32, 0x32, 0x31, 0x31, 0x39,
	0x39, 0x30, 0x30, 0x30, 0x32, 0x18, 0x00, 0x30, 0x00, 0x42, 0x3a, 0x08,
	0x01, 0x10, 0x03, 0x18, 0x02, 0x20, 0x02, 0x28, 0x04, 0x30, 0x01, 0x38,
	0x01, 0x40, 0x01, 0x48, 0x00, 0x50, 0x02, 0x58, 0x60, 0x60, 0x60, 0x68,
	0x60, 0x70, 0x60, 0x78, 0x60, 0x80, 0x01, 0x60, 0x88, 0x01, 0x60, 0x90,
	0x01, 0x60, 0x98, 0x01, 0x00, 0xa0, 0x01, 0xd9, 0xc3, 0xd6, 0xe1, 0x05,
	0xa8, 0x01, 0x04, 0xb0, 0x01, 0x24, 0xb8, 0x01, 0x00, 0x4a, 0x08, 0x08,
	0x03, 0x10, 0x01, 0x18, 0x00, 0x20, 0x00, 0x52, 0x0b, 0x08, 0x64, 0x10,
	0xde, 0x0f, 0x18, 0x05, 0x20, 0x00, 0x28, 0x00, 0xc2, 0x11, 0x2a, 0x66

};

static unsigned char terminalPackage5_Event4P[79] = {
	0x01, 0x04, 0x44, 0x00, 0x12, 0x0e, 0x0a, 0x00, 0x10, 0x04, 0x18, 0x00,
	0x20, 0x00, 0x28, 0x00, 0x30, 0x00, 0x38, 0x00, 0x1a, 0x00, 0x2a, 0x13,
	0x08, 0xc9, 0x0c, 0x12, 0x0c, 0x32, 0x37, 0x32, 0x32, 0x31, 0x31, 0x39,
	0x39, 0x30, 0x30, 0x30, 0x32, 0x18, 0x00, 0x30, 0x00, 0x4a, 0x08, 0x08,
	0x03, 0x10, 0x01, 0x18, 0x00, 0x20, 0x00, 0x52, 0x0b, 0x08, 0x64, 0x10,
	0xde, 0x0f, 0x18, 0x05, 0x20, 0x00, 0x28, 0x00, 0x5d, 0x49, 0x01, 0x1e
};

static unsigned char terminalPackage6_Event4P[139] = {
	0x01, 0x04, 0x80, 0x00, 0x12, 0x0e, 0x0a, 0x00, 0x10, 0x04, 0x18, 0x00,
	0x20, 0x00, 0x28, 0x00, 0x30, 0x00, 0x38, 0x00, 0x1a, 0x00, 0x2a, 0x13,
	0x08, 0xca, 0x0c, 0x12, 0x0c, 0x32, 0x37, 0x32, 0x32, 0x31, 0x31, 0x39,
	0x39, 0x30, 0x30, 0x30, 0x32, 0x18, 0x00, 0x30, 0x00, 0x42, 0x3a, 0x08,
	0x01, 0x10, 0x03, 0x18, 0x02, 0x20, 0x02, 0x28, 0x04, 0x30, 0x01, 0x38,
	0x01, 0x40, 0x01, 0x48, 0x00, 0x50, 0x02, 0x58, 0x60, 0x60, 0x60, 0x68,
	0x60, 0x70, 0x60, 0x78, 0x60, 0x80, 0x01, 0x60, 0x88, 0x01, 0x60, 0x90,
	0x01, 0x60, 0x98, 0x01, 0x00, 0xa0, 0x01, 0xda, 0xc3, 0xd6, 0xe1, 0x05,
	0xa8, 0x01, 0x04, 0xb0, 0x01, 0x24, 0xb8, 0x01, 0x00, 0x4a, 0x08, 0x08,
	0x03, 0x10, 0x01, 0x18, 0x00, 0x20, 0x00, 0x52, 0x0b, 0x08, 0x64, 0x10,
	0xde, 0x0f, 0x18, 0x05, 0x20, 0x00, 0x28, 0x00, 0xd4, 0x80, 0x16, 0xc2
};


//Event mode 2P
static unsigned char terminalPackage1_Event2P[79] = {
	0x01, 0x04, 0x44, 0x00, 0x12, 0x0e, 0x0a, 0x00, 0x10, 0x04, 0x18, 0x00,
	0x20, 0x00, 0x28, 0x00, 0x30, 0x00, 0x38, 0x00, 0x1a, 0x00, 0x2a, 0x13,
	0x08, 0xfe, 0x0e, 0x12, 0x0c, 0x32, 0x37, 0x32, 0x32, 0x31, 0x31, 0x39,
	0x39, 0x30, 0x30, 0x30, 0x32, 0x18, 0x00, 0x30, 0x00, 0x4a, 0x08, 0x08,
	0x03, 0x10, 0x01, 0x18, 0x00, 0x20, 0x00, 0x52, 0x0b, 0x08, 0x64, 0x10,
	0xde, 0x0f, 0x18, 0x05, 0x20, 0x00, 0x28, 0x00, 0xaf, 0xa1, 0x42, 0x3d
};

static unsigned char terminalPackage2_Event2P[139] = {
	0x01, 0x04, 0x80, 0x00, 0x12, 0x0e, 0x0a, 0x00, 0x10, 0x04, 0x18, 0x00,
	0x20, 0x00, 0x28, 0x00, 0x30, 0x00, 0x38, 0x00, 0x1a, 0x00, 0x2a, 0x13,
	0x08, 0xff, 0x0e, 0x12, 0x0c, 0x32, 0x37, 0x32, 0x32, 0x31, 0x31, 0x39,
	0x39, 0x30, 0x30, 0x30, 0x32, 0x18, 0x00, 0x30, 0x00, 0x42, 0x3a, 0x08,
	0x01, 0x10, 0x03, 0x18, 0x02, 0x20, 0x02, 0x28, 0x04, 0x30, 0x01, 0x38,
	0x01, 0x40, 0x02, 0x48, 0x00, 0x50, 0x02, 0x58, 0x60, 0x60, 0x60, 0x68,
	0x60, 0x70, 0x60, 0x78, 0x60, 0x80, 0x01, 0x60, 0x88, 0x01, 0x60, 0x90,
	0x01, 0x60, 0x98, 0x01, 0x00, 0xa0, 0x01, 0xa7, 0xc2, 0xa5, 0xe1, 0x05,
	0xa8, 0x01, 0x02, 0xb0, 0x01, 0x24, 0xb8, 0x01, 0x00, 0x4a, 0x08, 0x08,
	0x03, 0x10, 0x01, 0x18, 0x00, 0x20, 0x00, 0x52, 0x0b, 0x08, 0x64, 0x10,
	0xde, 0x0f, 0x18, 0x05, 0x20, 0x00, 0x28, 0x00, 0xe8, 0x94, 0x41, 0x46

};

static unsigned char terminalPackage3_Event2P[79] = {
	0x01, 0x04, 0x44, 0x00, 0x12, 0x0e, 0x0a, 0x00, 0x10, 0x04, 0x18, 0x00,
	0x20, 0x00, 0x28, 0x00, 0x30, 0x00, 0x38, 0x00, 0x1a, 0x00, 0x2a, 0x13,
	0x08, 0x80, 0x0f, 0x12, 0x0c, 0x32, 0x37, 0x32, 0x32, 0x31, 0x31, 0x39,
	0x39, 0x30, 0x30, 0x30, 0x32, 0x18, 0x00, 0x30, 0x00, 0x4a, 0x08, 0x08,
	0x03, 0x10, 0x01, 0x18, 0x00, 0x20, 0x00, 0x52, 0x0b, 0x08, 0x64, 0x10,
	0xde, 0x0f, 0x18, 0x05, 0x20, 0x00, 0x28, 0x00, 0xa3, 0x94, 0x12, 0x9b
};

static unsigned char terminalPackage4_Event2P[139] = {
	0x01, 0x04, 0x80, 0x00, 0x12, 0x0e, 0x0a, 0x00, 0x10, 0x04, 0x18, 0x00,
	0x20, 0x00, 0x28, 0x00, 0x30, 0x00, 0x38, 0x00, 0x1a, 0x00, 0x2a, 0x13,
	0x08, 0x8d, 0x0f, 0x12, 0x0c, 0x32, 0x37, 0x32, 0x32, 0x31, 0x31, 0x39,
	0x39, 0x30, 0x30, 0x30, 0x32, 0x18, 0x00, 0x30, 0x00, 0x42, 0x3a, 0x08,
	0x01, 0x10, 0x03, 0x18, 0x02, 0x20, 0x02, 0x28, 0x04, 0x30, 0x01, 0x38,
	0x01, 0x40, 0x02, 0x48, 0x00, 0x50, 0x02, 0x58, 0x60, 0x60, 0x60, 0x68,
	0x60, 0x70, 0x60, 0x78, 0x60, 0x80, 0x01, 0x60, 0x88, 0x01, 0x60, 0x90,
	0x01, 0x60, 0x98, 0x01, 0x00, 0xa0, 0x01, 0xa8, 0xc2, 0xa5, 0xe1, 0x05,
	0xa8, 0x01, 0x02, 0xb0, 0x01, 0x24, 0xb8, 0x01, 0x00, 0x4a, 0x08, 0x08,
	0x03, 0x10, 0x01, 0x18, 0x00, 0x20, 0x00, 0x52, 0x0b, 0x08, 0x64, 0x10,
	0xde, 0x0f, 0x18, 0x05, 0x20, 0x00, 0x28, 0x00, 0x8b, 0x02, 0xdf, 0xad

};

static unsigned char terminalPackage5_Event2P[79] = {
	0x01, 0x04, 0x44, 0x00, 0x12, 0x0e, 0x0a, 0x00, 0x10, 0x04, 0x18, 0x00,
	0x20, 0x00, 0x28, 0x00, 0x30, 0x00, 0x38, 0x00, 0x1a, 0x00, 0x2a, 0x13,
	0x08, 0x8e, 0x0f, 0x12, 0x0c, 0x32, 0x37, 0x32, 0x32, 0x31, 0x31, 0x39,
	0x39, 0x30, 0x30, 0x30, 0x32, 0x18, 0x00, 0x30, 0x00, 0x4a, 0x08, 0x08,
	0x03, 0x10, 0x01, 0x18, 0x00, 0x20, 0x00, 0x52, 0x0b, 0x08, 0x64, 0x10,
	0xde, 0x0f, 0x18, 0x05, 0x20, 0x00, 0x28, 0x00, 0xa3, 0xc2, 0x27, 0x9c
};

static unsigned char terminalPackage6_Event2P[139] = {
	0x01, 0x04, 0x80, 0x00, 0x12, 0x0e, 0x0a, 0x00, 0x10, 0x04, 0x18, 0x00,
	0x20, 0x00, 0x28, 0x00, 0x30, 0x00, 0x38, 0x00, 0x1a, 0x00, 0x2a, 0x13,
	0x08, 0xf0, 0x0e, 0x12, 0x0c, 0x32, 0x37, 0x32, 0x32, 0x31, 0x31, 0x39,
	0x39, 0x30, 0x30, 0x30, 0x32, 0x18, 0x00, 0x30, 0x00, 0x42, 0x3a, 0x08,
	0x01, 0x10, 0x03, 0x18, 0x02, 0x20, 0x02, 0x28, 0x04, 0x30, 0x01, 0x38,
	0x01, 0x40, 0x02, 0x48, 0x00, 0x50, 0x02, 0x58, 0x60, 0x60, 0x60, 0x68,
	0x60, 0x70, 0x60, 0x78, 0x60, 0x80, 0x01, 0x60, 0x88, 0x01, 0x60, 0x90,
	0x01, 0x60, 0x98, 0x01, 0x00, 0xa0, 0x01, 0xa6, 0xc2, 0xa5, 0xe1, 0x05,
	0xa8, 0x01, 0x02, 0xb0, 0x01, 0x24, 0xb8, 0x01, 0x00, 0x4a, 0x08, 0x08,
	0x03, 0x10, 0x01, 0x18, 0x00, 0x20, 0x00, 0x52, 0x0b, 0x08, 0x64, 0x10,
	0xde, 0x0f, 0x18, 0x05, 0x20, 0x00, 0x28, 0x00, 0x97, 0xd5, 0x79, 0xa6
};


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
	*hasp_size = 0xD40; // Max addressable size by the game... absmax is 4k
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

typedef int (WINAPI *BIND)(SOCKET, CONST SOCKADDR *, INT);
static BIND pbind = NULL;

static unsigned int WINAPI Hook_bind(SOCKET s, const sockaddr *addr, int namelen) {
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


static unsigned char saveData[0x2000];
static bool saveOk = false;
static unsigned char carData[0xFF];
static int SaveOk()
{
	saveOk = true;
	return 1;
}

static char carFileName[0xFF];
static bool loadOk = false;
static bool customCar = false;

static int SaveGameData()
{
	if (!saveOk)
		return 1;

	memset(saveData, 0, 0x2000);
	uintptr_t value = *(uintptr_t*)(imageBase + 0x1948F10);
	value = *(uintptr_t*)(value + 0x108);
	memcpy(saveData, (void *)value, 0x340);
	FILE* file = fopen("openprogress.sav", "wb");
	fwrite(saveData, 1, 0x2000, file);
	fclose(file);

	// Car Profile saving
	memset(carData, 0, 0xFF);
	memset(carFileName, 0, 0xFF);
	memcpy(carData, (void *)*(uintptr_t*)(*(uintptr_t*)(imageBase + 0x1948F10) + 0x180 + 0xa8 + 0x18), 0xE0);
	CreateDirectoryA("OpenParrot_Cars", nullptr);
	if(customCar)
	{
		sprintf(carFileName, ".\\OpenParrot_Cars\\custom.car");
	}
	else
	{
		sprintf(carFileName, ".\\OpenParrot_Cars\\%08X.car", *(DWORD*)(*(uintptr_t*)(*(uintptr_t*)(imageBase + 0x1948F10) + 0x180 + 0xa8 + 0x18) + 0x2C));
	}
	FILE *carSave = fopen(carFileName, "wb");
	fwrite(carData, 1, 0xE0, file);
	fclose(carSave);
	saveOk = false;
	return 1;
}

static uintptr_t saveGameOffset;

static int LoadGameData()
{
	saveOk = false;
	memset(saveData, 0x0, 0x2000);
	FILE* file = fopen("openprogress.sav", "rb");
	if (file)
	{
		fseek(file, 0, SEEK_END);
		int fsize = ftell(file);
		if (fsize == 0x2000)
		{
			fseek(file, 0, SEEK_SET);
			fread(saveData, fsize, 1, file);
			uintptr_t value = *(uintptr_t*)(imageBase + 0x1948F10);
			value = *(uintptr_t*)(value + 0x108);

			// First page
			//memcpy((void *)(value), saveData, 0x48);
			memcpy((void *)(value + 0x10), saveData + 0x10, 0x20);
			memcpy((void *)(value + 0x40), saveData + 0x40, 0x08);
			//memcpy((void *)(value + 0x48 + 8), saveData + 0x48 + 8, 0x20);
			memcpy((void *)(value + 0x48 + 8), saveData + 0x48 + 8, 0x08);
			memcpy((void *)(value + 0x48 + 24), saveData + 0x48 + 24, 0x08);
			memcpy((void *)(value + 0x48 + 32), saveData + 0x48 + 32, 0x08);

			// Second page
			value += 0x110;
			memcpy((void *)(value), saveData + 0x110, 0x90);
			value -= 0x110;

			// Third Page
			value += 0x1B8;
			memcpy((void *)(value), saveData + 0x1B8, 0x48);
			memcpy((void *)(value + 0x48 + 8), saveData + 0x1B8 + 0x48 + 8, 0x28);
			value -= 0x1B8;

			// Fourth page
			value += 0x240;
			memcpy((void *)(value), saveData + 0x240, 0x68);
			value -= 0x240;

			// Fifth page
			value += 0x2B8;
			memcpy((void *)(value), saveData + 0x2B8, 0x88);
			value -= 0x2B8;

			loadOk = true;
		}
		fclose(file);
	}
	return 1;
}

static BOOL FileExists(char *szPath)
{
	DWORD dwAttrib = GetFileAttributesA(szPath);

	return (dwAttrib != INVALID_FILE_ATTRIBUTES &&
		!(dwAttrib & FILE_ATTRIBUTE_DIRECTORY));
}

static void LoadWmmt5CarData()
{
	if (!loadOk)
		return;
	customCar = false;
	memset(carData, 0, 0xFF);
	memset(carFileName, 0, 0xFF);
	CreateDirectoryA("OpenParrot_Cars", nullptr);

	// check for custom car
	sprintf(carFileName, ".\\OpenParrot_Cars\\custom.car");
	if (FileExists(carFileName))
	{
		FILE* file = fopen(carFileName, "rb");
		if (file)
		{
			fseek(file, 0, SEEK_END);
			int fsize = ftell(file);
			if (fsize == 0xE0)
			{
				fseek(file, 0, SEEK_SET);
				fread(carData, fsize, 1, file);
				uintptr_t carSaveLocation = *(uintptr_t*)((*(uintptr_t*)(imageBase + 0x1948F10)) + 0x180 + 0xa8 + 0x18);
				memcpy((void *)(carSaveLocation + 0x08), carData + 0x08, 8);
				memcpy((void *)(carSaveLocation + 0x10), carData + 0x10, 8);
				memcpy((void *)(carSaveLocation + 0x20), carData + 0x20, 8);
				memcpy((void *)(carSaveLocation + 0x28), carData + 0x28, 8);
				memcpy((void *)(carSaveLocation + 0x30), carData + 0x30, 8);
				memcpy((void *)(carSaveLocation + 0x38), carData + 0x38, 8);
				memcpy((void *)(carSaveLocation + 0x40), carData + 0x40, 8);
				memcpy((void *)(carSaveLocation + 0x50), carData + 0x50, 8);
				memcpy((void *)(carSaveLocation + 0x58), carData + 0x58, 8);
				memcpy((void *)(carSaveLocation + 0x68), carData + 0x68, 8);
				//				memcpy((void *)(carSaveLocation + 0x70), carData + 0x70, 8);
				memcpy((void *)(carSaveLocation + 0x80), carData + 0x80, 8);
				memcpy((void *)(carSaveLocation + 0x88), carData + 0x88, 8);
				memcpy((void *)(carSaveLocation + 0x90), carData + 0x90, 8);
				memcpy((void *)(carSaveLocation + 0x98), carData + 0x98, 8);
				memcpy((void *)(carSaveLocation + 0xA0), carData + 0xA0, 8);
				memcpy((void *)(carSaveLocation + 0xA8), carData + 0xA8, 8);
				memcpy((void *)(carSaveLocation + 0xB8), carData + 0xB8, 8);
				memcpy((void *)(carSaveLocation + 0xC8), carData + 0xC8, 8);
				memcpy((void *)(carSaveLocation + 0xD8), carData + 0xD8, 8);
				//memcpy((void *)(carSaveLocation + 0xE0), carData + 0xE0, 8);
				customCar = true;
			}
			loadOk = false;
			fclose(file);
			return;
		}
	}

	memset(carFileName, 0, 0xFF);
	// Load actual car if available
	sprintf(carFileName, ".\\OpenParrot_Cars\\%08X.car", *(DWORD*)(*(uintptr_t*)(*(uintptr_t*)(imageBase + 0x1948F10) + 0x180 + 0xa8 + 0x18) + 0x2C));
	if(FileExists(carFileName))
	{
		FILE* file = fopen(carFileName, "rb");
		if (file)
		{
			fseek(file, 0, SEEK_END);
			int fsize = ftell(file);
			if (fsize == 0xE0)
			{
				fseek(file, 0, SEEK_SET);
				fread(carData, fsize, 1, file);
				uintptr_t carSaveLocation = *(uintptr_t*)((*(uintptr_t*)(imageBase + 0x1948F10)) + 0x180 + 0xa8 + 0x18);
				memcpy((void *)(carSaveLocation + 0x08), carData + 0x08, 8);
				memcpy((void *)(carSaveLocation + 0x10), carData + 0x10, 8);
				memcpy((void *)(carSaveLocation + 0x20), carData + 0x20, 8);
				memcpy((void *)(carSaveLocation + 0x28), carData + 0x28, 8);
				memcpy((void *)(carSaveLocation + 0x30), carData + 0x30, 8);
				memcpy((void *)(carSaveLocation + 0x38), carData + 0x38, 8);
				memcpy((void *)(carSaveLocation + 0x40), carData + 0x40, 8);
				memcpy((void *)(carSaveLocation + 0x50), carData + 0x50, 8);
				memcpy((void *)(carSaveLocation + 0x58), carData + 0x58, 8);
				memcpy((void *)(carSaveLocation + 0x68), carData + 0x68, 8);
//				memcpy((void *)(carSaveLocation + 0x70), carData + 0x70, 8);
				memcpy((void *)(carSaveLocation + 0x80), carData + 0x80, 8);
				memcpy((void *)(carSaveLocation + 0x88), carData + 0x88, 8);
				memcpy((void *)(carSaveLocation + 0x90), carData + 0x90, 8);
				memcpy((void *)(carSaveLocation + 0x98), carData + 0x98, 8);
				memcpy((void *)(carSaveLocation + 0xA0), carData + 0xA0, 8);
				memcpy((void *)(carSaveLocation + 0xA8), carData + 0xA8, 8);
				memcpy((void *)(carSaveLocation + 0xB8), carData + 0xB8, 8);
				memcpy((void *)(carSaveLocation + 0xC8), carData + 0xC8, 8);
				memcpy((void *)(carSaveLocation + 0xD8), carData + 0xD8, 8);
				//memcpy((void *)(carSaveLocation + 0xE0), carData + 0xE0, 8);
			}
			fclose(file);
		}
	}
	loadOk = false;
}

static int ReturnTrue()
{
	return 1;
}

static BYTE GenerateChecksum(unsigned char* myArray, int index, int length)
{
	BYTE crc = 0;
	for (int i = 0; i < length; i++)
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
	if(isTerminal)
	{
		memcpy(hasp_buffer + 0xD00, "276411292430", 12);
		hasp_buffer[0xD3E] = GenerateChecksum(hasp_buffer, 0xD00, 62);
		hasp_buffer[0xD3F] = hasp_buffer[0xD3E] ^ 0xFF;
	}
	else
	{
		memcpy(hasp_buffer + 0xD00, "276413292430", 12);
		hasp_buffer[0xD3E] = GenerateChecksum(hasp_buffer, 0xD00, 62);
		hasp_buffer[0xD3F] = hasp_buffer[0xD3E] ^ 0xFF;
	}
}

static char customName[256];


static DWORD WINAPI SpamCustomName(LPVOID)
{
	while (true)
	{
		Sleep(50);
		void *value = (void *)(imageBase + 0x194C230);
		memcpy(value, customName, strlen(customName) + 1);
	}
}

static DWORD WINAPI SpamMulticast(LPVOID)
{
	WSADATA wsaData;
	WSAStartup(MAKEWORD(2, 2), &wsaData);

	SOCKET sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);

	int ttl = 255;
	setsockopt(sock, IPPROTO_IP, IP_MULTICAST_TTL, (char*)&ttl, sizeof(ttl));

	int reuse = 1;
	setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, (char*)&reuse, sizeof(reuse));

	setsockopt(sock, IPPROTO_IP, IP_MULTICAST_LOOP, (char*)&reuse, sizeof(reuse));

	sockaddr_in bindAddr = { 0 };
	bindAddr.sin_family = AF_INET;
	bindAddr.sin_addr.s_addr = inet_addr(ipaddr);
	bindAddr.sin_port = htons(50765);
	bind(sock, (sockaddr*)&bindAddr, sizeof(bindAddr));
	

	ip_mreq mreq;
	mreq.imr_multiaddr.s_addr = inet_addr("225.0.0.1");
	mreq.imr_interface.s_addr = inet_addr(ipaddr);

	setsockopt(sock, IPPROTO_IP, IP_ADD_MEMBERSHIP, (char*)&mreq, sizeof(mreq));

	const uint8_t* byteSequences_Free[] = {
		terminalPackage1_Free,
		terminalPackage2_Free,
		terminalPackage3_Free,
		terminalPackage4_Free,
		terminalPackage5_Free,
		terminalPackage6_Free,
	};

	const size_t byteSizes_Free[] = {
		sizeof(terminalPackage1_Free),
		sizeof(terminalPackage2_Free),
		sizeof(terminalPackage3_Free),
		sizeof(terminalPackage4_Free),
		sizeof(terminalPackage5_Free),
		sizeof(terminalPackage6_Free),
	};

	const uint8_t* byteSequences_Event2P[] = {
		terminalPackage1_Event2P,
		terminalPackage2_Event2P,
		terminalPackage3_Event2P,
		terminalPackage4_Event2P,
		terminalPackage5_Event2P,
		terminalPackage6_Event2P,
	};

	const size_t byteSizes_Event2P[] = {
		sizeof(terminalPackage1_Event2P),
		sizeof(terminalPackage2_Event2P),
		sizeof(terminalPackage3_Event2P),
		sizeof(terminalPackage4_Event2P),
		sizeof(terminalPackage5_Event2P),
		sizeof(terminalPackage6_Event2P),
	};

	const uint8_t* byteSequences_Event4P[] = {
		terminalPackage1_Event4P,
		terminalPackage2_Event4P,
		terminalPackage3_Event4P,
		terminalPackage4_Event4P,
		terminalPackage5_Event4P,
		terminalPackage6_Event4P,
	};

	const size_t byteSizes_Event4P[] = {
		sizeof(terminalPackage1_Event4P),
		sizeof(terminalPackage2_Event4P),
		sizeof(terminalPackage3_Event4P),
		sizeof(terminalPackage4_Event4P),
		sizeof(terminalPackage5_Event4P),
		sizeof(terminalPackage6_Event4P),
	};

	const uint8_t* byteSequences_Coin[] = {
		terminalPackage1_Coin,
		terminalPackage2_Coin,
		terminalPackage3_Coin,
		terminalPackage4_Coin,
		terminalPackage5_Coin,
		terminalPackage6_Coin,
	};

	const size_t byteSizes_Coin[] = {
		sizeof(terminalPackage1_Coin),
		sizeof(terminalPackage2_Coin),
		sizeof(terminalPackage3_Coin),
		sizeof(terminalPackage4_Coin),
		sizeof(terminalPackage5_Coin),
		sizeof(terminalPackage6_Coin),
	};

	sockaddr_in toAddr = { 0 };
	toAddr.sin_family = AF_INET;
	toAddr.sin_addr.s_addr = inet_addr("225.0.0.1");
	toAddr.sin_port = htons(50765);
	

	isFreePlay = ToBool(config["General"]["FreePlay"]);
	isEventMode2P = ToBool(config["TerminalEmuConfig"]["2P Event Mode"]);
	isEventMode4P = ToBool(config["TerminalEmuConfig"]["4P Event Mode"]);

	if (isFreePlay)
	{
		if (isEventMode2P) {
			while (true) for (int i = 0; i < _countof(byteSequences_Event2P); i++)
			{
				sendto(sock, (const char*)byteSequences_Event2P[i], byteSizes_Event2P[i], 0, (sockaddr*)&toAddr, sizeof(toAddr));
				Sleep(8);
			}
		}
		else if (isEventMode4P) {
			while (true) for (int i = 0; i < _countof(byteSequences_Event4P); i++)
			{
				sendto(sock, (const char*)byteSequences_Event4P[i], byteSizes_Event4P[i], 0, (sockaddr*)&toAddr, sizeof(toAddr));
				Sleep(8);
			}
		}
		else {
			while (true) for (int i = 0; i < _countof(byteSequences_Free); i++)
			{
				sendto(sock, (const char*)byteSequences_Free[i], byteSizes_Free[i], 0, (sockaddr*)&toAddr, sizeof(toAddr));
				Sleep(8);
			}
		}
	}
	
	while (true) for (int i = 0; i < _countof(byteSequences_Coin); i++)
	{
		sendto(sock, (const char*)byteSequences_Coin[i], byteSizes_Coin[i], 0, (sockaddr*)&toAddr, sizeof(toAddr));
		Sleep(8);
	}
}

extern int* ffbOffset;
extern int* ffbOffset2;
extern int* ffbOffset3;
extern int* ffbOffset4;

static DWORD WINAPI Wmmt5FfbCollector(void* ctx)
{
	uintptr_t imageBase = (uintptr_t)GetModuleHandleA(0);
	while (true)
	{
		//*ffbOffset = *(DWORD *)(imageBase + 0x196F188);
		//*ffbOffset2 = *(DWORD *)(imageBase + 0x196F18c);
		//*ffbOffset3 = *(DWORD *)(imageBase + 0x196F190);
		//*ffbOffset4 = *(DWORD *)(imageBase + 0x196F194);
		Sleep(10);
	}
}

static InitFunction Wmmt5DXFunc([]()
{
	// folder for path redirections
	CreateDirectoryA(".\\OpenParrot", nullptr);

	FILE* fileF = _wfopen(L".\\OpenParrot\\setting.lua.gz", L"r");
	if (fileF == NULL)
	{
		FILE* settingsF = _wfopen(L".\\OpenParrot\\setting.lua.gz", L"wb");
		fwrite(settingData, 1, sizeof(settingData), settingsF);
		fclose(settingsF);
	}
	else
	{
		fclose(fileF);
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
	MH_CreateHookApi(L"hasp_windows_x64_98199.dll", "hasp_write", Hook_hasp_write, NULL);
	MH_CreateHookApi(L"hasp_windows_x64_98199.dll", "hasp_read", Hook_hasp_read, NULL);
	MH_CreateHookApi(L"hasp_windows_x64_98199.dll", "hasp_get_size", Hook_hasp_get_size, NULL);
	MH_CreateHookApi(L"hasp_windows_x64_98199.dll", "hasp_decrypt", Hook_hasp_decrypt, NULL);
	MH_CreateHookApi(L"hasp_windows_x64_98199.dll", "hasp_encrypt", Hook_hasp_encrypt, NULL);
	MH_CreateHookApi(L"hasp_windows_x64_98199.dll", "hasp_logout", Hook_hasp_logout, NULL);
	MH_CreateHookApi(L"hasp_windows_x64_98199.dll", "hasp_login", Hook_hasp_login, NULL);
	MH_CreateHookApi(L"WS2_32", "bind", Hook_bind, reinterpret_cast<LPVOID*>(&pbind));

	//load banapass emu
	LoadLibraryA(".\\openBanaW5p.dll");

	GenerateDongleData(isTerminal);

	injector::WriteMemory<uint8_t>(imageBase + 0x61773E, 0, true);
	injector::MakeNOP(imageBase + 0x6185DB, 6, true);
	injector::WriteMemory<uint8_t>(hook::get_pattern("83 FA 04 0F 8C 1E 01 00 00 4C 89 44 24 18 4C 89 4C 24 20", 2), 0, true);
		
	// Skip weird camera init that stucks entire pc on certain brands. TESTED ONLY ON 05!!!!
	if (ToBool(config["General"]["WhiteScreenFix"]))
	{
		injector::WriteMemory<DWORD>(hook::get_pattern("48 8B C4 55 57 41 54 41 55 41 56 48 8D 68 A1 48 81 EC 90 00 00 00 48 C7 45 D7 FE FF FF FF 48 89 58 08 48 89 70 18 45 33 F6 4C 89 75 DF 33 C0 48 89 45 E7", 0), 0x90C3C032, true);
	}

	injector::MakeNOP(hook::get_pattern("45 33 C0 BA 65 09 00 00 48 8D 4D B0 E8 ? ? ? ? 48 8B 08", 12), 5);

	{
		auto location = hook::get_pattern<char>("41 3B C7 74 0E 48 8D 8F B8 00 00 00 BA F6 01 00 00 EB 6E 48 8D 8F A0 00 00 00");
		// Patch some jnz
		injector::WriteMemory<uint8_t>(location + 3, 0xEB, true);

		// Skip some jnz
		injector::MakeNOP(location + 0x22, 2);

		// Skip some jnz
		injector::MakeNOP(location + 0x33, 2);
	}

	// Skip DebugBreak on MFStartup fail
	{
		auto location = hook::get_pattern<char>("48 83 EC 28 33 D2 B9 70 00 02 00 E8 ? ? ? ? 85 C0 79 06");
		injector::WriteMemory<uint8_t>(location + 0x12, 0xEB, true);
	}

	if (isTerminal)
	{
		// Patch some func to 1
		safeJMP(hook::get_pattern("0F B6 41 05 2C 30 3C 09 77 04 0F BE C0 C3 83 C8 FF C3"), ReturnTrue);
	}
	else
	{
		{
			auto location = imageBase + 0x8CCEDD;
			injector::MakeNOP(location + 6, 6); // 6
			injector::MakeNOP(location + 0x16, 2); // 0xF
			injector::MakeNOP(location + 0x1C, 2); // 0x15
		}

		// spam thread
		if (ToBool(config["General"]["TerminalEmulator"]))
		{
			CreateThread(0, 0, SpamMulticast, 0, 0, 0);			
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
		safeJMP(imageBase + 0x85D7A0, ReturnTrue);
	}


	std::string value = config["General"]["CustomName"];
	if (!value.empty())
	{
		memset(customName, 0, 256);
		strcpy(customName, value.c_str());
		//CreateThread(0, 0, SpamCustomName, 0, 0, 0);
	}

	MH_EnableHook(MH_ALL_HOOKS);

}, GameID::WMMT5DX);
#endif
#pragma optimize("", on)