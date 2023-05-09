#include "imgui.h"
#include "EndScene.h"
#include "imgui_impl_win32.h"
#include "Sync.h"

#include "sdk.h"

UINT SpeedShotKey = 'E';
UINT SyncKey = 'F';

extern float unitsBack;

UINT* keyToEdit = nullptr;
//struct CUSTOMVERTEX {
//	float x, y, z;
//};
bool showWindow = true;
HRESULT __stdcall EndSceneHook(IDirect3DDevice9* vDevice) {
	static bool initGui = false;
	
	if (!initGui) {
		HWND hWindow=FindWindowA("Valve001", nullptr);
		ImGui::CreateContext();
		ImGuiIO& io = ImGui::GetIO();
		//io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
		ImGui_ImplWin32_Init(hWindow);
		ImGui_ImplDX9_Init(vDevice);
		initGui = true;
		
	}
	/*LPDIRECT3DVERTEXBUFFER9 g_pVB;
	vDevice->CreateVertexBuffer(5*sizeof(CUSTOMVERTEX), D3DUSAGE_DONOTCLIP, 0, D3DPOOL_DEFAULT, &g_pVB, NULL)
	vDevice->DrawPrimitive(D3DPT_LINESTRIP, 0, 5);*/

	if (showWindow) {
		ImGui_ImplDX9_NewFrame();
		ImGui_ImplWin32_NewFrame();
		ImGui::NewFrame();

		//ImGui::ShowDemoWindow(&show_demo_window);

		ImGui::Begin("menu", &showWindow);
		if (ImGui::CollapsingHeader("Sync")) {
			ImGui::RadioButton("Quinting", (int*)&type, quint);
			ImGui::RadioButton("Triple", (int*)&type, triple);
			ImGui::RadioButton("Double", (int*)&type, doubling);
			//ImGui::RadioButton("auto detect", (int*)&type, autoDetect);
			ImGui::Checkbox("Aim at crosshair position", &customRocketEndPos);

		}
		if (ImGui::CollapsingHeader("SpeedShot")) {

			ImGui::SliderFloat("unitsBack", &unitsBack, -30.f, 30.f);
		}
		//ImGui::InputText("Speedshot key", SpeedShotKey, IM_ARRAYSIZE(SpeedShotKey));
		ImGui::Text("Speedshot key %c", SpeedShotKey);
		if (ImGui::Button("Edit key"))
			keyToEdit = &SpeedShotKey;
		ImGui::Text("Sync key %c", SyncKey);
		if (ImGui::Button("Edit key"))
			keyToEdit = &SyncKey;


		//ImGui::InputText("Sync key", syncKey, IM_ARRAYSIZE(SpeedShotKey));
		//ImGui::RadioButton("Double", &doubleSync)

		//ImGui::Checkbox("");
		ImGui::End();
		ImGui::Render();
		ImGui_ImplDX9_RenderDrawData(ImGui::GetDrawData());
	}
	return EndScene(vDevice);
}

LRESULT STDMETHODCALLTYPE hkWndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
	if (keyToEdit && uMsg == WM_KEYDOWN && wParam != VK_INSERT) {
		*keyToEdit = wParam;
		keyToEdit = nullptr;
	}
	if (uMsg == WM_KEYDOWN && wParam == VK_INSERT)
		showWindow = !showWindow;
	if (uMsg == WM_MOUSEWHEEL) {
		int zDelta = GET_WHEEL_DELTA_WPARAM(wParam);
			unitsBack += zDelta / 120.f;
	}
	if(showWindow)
		if (ImGui_ImplWin32_WndProcHandler(hWnd, uMsg, wParam, lParam))
			return true;
	return CallWindowProc(pOldWindowProc, hWnd, uMsg, wParam, lParam);
}