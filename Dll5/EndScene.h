#pragma once
#include <Windows.h>
#include "imgui_impl_dx9.h"

typedef HRESULT(__stdcall* EndSceneFn)(IDirect3DDevice9*);
extern EndSceneFn EndScene;
HRESULT __stdcall EndSceneHook(IDirect3DDevice9* vDevice);
extern LRESULT STDMETHODCALLTYPE hkWndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
extern WNDPROC pOldWindowProc;
extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
extern char SpeedShotKey[] ;
extern char syncKey[];

extern float unitsBack;