#pragma once
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include "Header1.h"

void* CaptureInterface(const char* module_name, const char* interface_name)
{
    auto create_interface_fn = reinterpret_cast<void* (*)(const char* pName, int* pReturnCode)>(GetProcAddress(GetModuleHandleA(module_name), "CreateInterface"));
    return create_interface_fn(interface_name, nullptr);
}

IPanel* panel = reinterpret_cast<IPanel*>(CaptureInterface("vgui2.dll", "VGUI_Panel009"));