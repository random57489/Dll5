#include "imgui.h"
#include "EndScene.h"
#include "imgui_impl_win32.h"
#include "Sync.h"

char SpeedShotKey[] = "E";
char syncKey[] = "F";
float unitsBack = 30;
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
	

	ImGui_ImplDX9_NewFrame();
	ImGui_ImplWin32_NewFrame();
	ImGui::NewFrame();
	bool show_demo_window = true;
	ImGui::ShowDemoWindow(&show_demo_window);
	ImGui::Begin("Hello, world!");
	if(ImGui::CollapsingHeader("Sync")) {
		ImGui::RadioButton("Quinting", &quinting, 1);
		ImGui::RadioButton("Triple", &quinting, 0);
		ImGui::InputText("Sync key", syncKey, IM_ARRAYSIZE(SpeedShotKey));
	}
	if (ImGui::CollapsingHeader("SpeedShot")) {
		ImGui::InputText("Speedshot key", SpeedShotKey, IM_ARRAYSIZE(SpeedShotKey));
		ImGui::SliderFloat("slider float", &unitsBack, -50.f, 50.f, "unitsBack %.3f");
	}
	//ImGui::RadioButton("Double", &doubleSync)
	ImGui::Checkbox("Aim at crosshair position", &customRocketEndPos);
	//ImGui::Checkbox("");
	ImGui::End();
	ImGui::Render();
	ImGui_ImplDX9_RenderDrawData(ImGui::GetDrawData());
	return EndScene(vDevice);
}

LRESULT STDMETHODCALLTYPE hkWndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
	if (ImGui_ImplWin32_WndProcHandler(hWnd, uMsg, wParam, lParam))
		return true;
	return CallWindowProc(pOldWindowProc, hWnd, uMsg, wParam, lParam);
}