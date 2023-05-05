// dllmain.cpp : Defines the entry point for the DLL application.
#define WIN32_LEAN_AND_MEAN

#include <Windows.h>
#include <iostream>
#include <chrono>
#include <thread>
#include "vmthook.h"
#include "sdk.h"
#include "client.h"
#include "signature.h"
#include "offsets.h"
#include <math.h>
#include <Vector>
#include "imgui_impl_dx9.h"
#include "imgui_impl_win32.h"
#include "EndScene.h"


#define MakeConVar(name, defaultValue) ConVar *name = (ConVar*)malloc(sizeof(ConVar)); ConVarCnstr(name, #name, #defaultValue, 0); ConVars.push_back(name)

DWORD protectMemory(DWORD address, DWORD prot, DWORD size) {
    DWORD oldProt;
    VirtualProtect((LPVOID)address, size, prot, &oldProt);
    return oldProt;
}


template<typename T>
void writeMemory(DWORD address, T value)
{
    *((T*)address) = value;
}


template<typename T>
T readAndWrite(DWORD address, T value) {
    T temp = *((T*)address);
    *((T*)address) = value;
    return temp;
}

void writeMemoryWithProtection(DWORD address, BYTE* values, int size) {
    auto oldProtection = protectMemory(address, PAGE_EXECUTE_READWRITE, size);
    memcpy((DWORD*)address, values, size);
    protectMemory(address, oldProtection, size);
}
std::vector<std::pair<DWORD, DWORD>>myOriginalFuncs;

DWORD callHookAtE8(DWORD hookAt, DWORD newFunc)
{
    DWORD newOffset = newFunc - hookAt - 5;
    //auto oldProtection = protectMemory<DWORD>(hookAt + 1, PAGE_EXECUTE_READWRITE);
    DWORD originalOffset = readMemory<DWORD>(hookAt+1);
    writeMemoryWithProtection<DWORD>(hookAt + 1, newOffset);
    //protectMemory<DWORD>(hookAt + 1, oldProtection);
    myOriginalFuncs.push_back(std::make_pair(hookAt+1, originalOffset));
    return originalOffset + hookAt + 5;
}


typedef DWORD(__thiscall* GetProjectileFireSetup)(DWORD gun, DWORD player, Vector offset, Vector* vecSrc, Vector* angForward, BYTE hitTeammates, float flEndDist);
typedef DWORD(__thiscall* FireProjectileFn)(DWORD gun, DWORD player, Vector offset, Vector* vecSrc, Vector* angForward, BYTE hitTeammates, DWORD idk);


DWORD hookVF(DWORD classInst, DWORD funcIndex, DWORD newFunc)
{
    DWORD VFTable = readMemory<DWORD>(classInst);
    DWORD hookAt = VFTable + funcIndex * sizeof(DWORD);
    auto oldProtection =
        protectMemory<DWORD>(hookAt, PAGE_READWRITE);
    DWORD originalFunc = readMemory<DWORD>(hookAt);
    writeMemory<DWORD>(hookAt, newFunc);
    protectMemory<DWORD>(hookAt, oldProtection);
    myOriginalFuncs.push_back(std::make_pair(hookAt, originalFunc));
    return originalFunc;
}

ICvar* gInts::cvar;
CEntList* gInts::EntList;
EngineClient* gInts::Engine;
IEngineTrace* gInts::EngineTrace;
CNetVars gNetVars;

struct myClass {
    DWORD address;
    BYTE a;
    DWORD b;
    BYTE* c;
    int size;
};
std::vector<myClass> idk2;


DWORD hookWithJump(DWORD hookAt, DWORD newFunc, int size)
{
    myClass idk;
    if (size > 12) // shouldn't ever have to replace 12+ bytes
        return 0;
    DWORD newOffset = newFunc - hookAt - 5;
    auto oldProtection =
        protectMemory(hookAt, PAGE_EXECUTE_READWRITE, size);
    idk.a = readAndWrite<BYTE>(hookAt, 0xE9);

    idk.b = readAndWrite<DWORD>(hookAt + 1, newOffset);
    if (size - 5 > 0) {
        idk.c = new BYTE[size - 5];
        for (int i = 5; i < size; i++)
            idk.c[i - 5] = readAndWrite<BYTE>(hookAt + i, 0x90);
    }
    idk.size = size;
    idk.address = hookAt;
    idk2.push_back(idk);
    protectMemory(hookAt, oldProtection, size);
    return hookAt + size;
}
    


DWORD restoreJumpHook = 0;
DWORD idk = 0;

void restoreHook(DWORD hookAt){
    for (auto& abc : idk2)
    {
        auto oldProt = protectMemory(abc.address, PAGE_EXECUTE_READWRITE, abc.size);
        writeMemory<BYTE>(abc.address, abc.a);
        writeMemory<DWORD>(abc.address+1, abc.b);
        if (abc.size > 5) {
            for (int i = 5; i < abc.size; ++i)
                writeMemory<BYTE>(abc.address + 5, abc.c[i - 5]);
            delete[abc.size - 5]abc.c;
        }
        protectMemory(abc.address, oldProt, abc.size);
        // pop from idk2???
    }
    for (auto &pair : myOriginalFuncs) 
        writeMemoryWithProtection<DWORD>(pair.first, pair.second);
}


DWORD clientbase;

inline void exit(void* hModule) {
    restoreHook(clientbase + 0x5D9FA4);
    pastCommands.clear();

    FreeLibraryAndExitThread((HMODULE)hModule, 0);
}




CGlobalVarsBase* gpGlobals;


typedef float(__fastcall * SetKeyFn)(DWORD _this, DWORD edx, DWORD a1);

inline float VectorLength(const Vector& v)
{
    return sqrtf(v.x * v.x + v.y * v.y + v.z * v.z);
}

CMoveData* g_pMoveData;
EyeAnglesFn EyeAngles;
Weapon_ShootPositionFn Weapon_ShootPosition;
GetGroundEntityFn GetGroundEntity;
AngleVectorsFn AngleVectors2;
CTFGameMovement* myGameMovement;
FullWalkMoveFn FullWalkMove;
ProcessMovementFn ProcessMovement;
CGlobalVarsBase** othergpGlobals;
CategorizePositionFn CategorizePosition;
DWORD offsetForNoNull;
EndSceneFn EndScene;
DWORD GetNextPosRetAddr;
BYTE* bSendPacket;


DWORD frameTimeLocation;


DWORD retAddr;
extern int hitWall;

void __declspec (naked) WalkMoveTramp() {
    _asm {
        mov hitWall, 1
        jmp[retAddr]
    }
}
CreateMoveFn CreateMove;
WNDPROC pOldWindowProc;


unsigned int _stdcall startup(void* hModule) {

    DWORD dwClientModeAddress = gSignatures.GetClientSignature("8B 0D ? ? ? ? 8B 02 D9 05");
    CreateInterface_t ClientFactory = (CreateInterfaceFn)GetProcAddress(gSignatures.GetModuleHandleSafe(L"client.dll"), "CreateInterface");
    CreateInterface_t EngineFactory = (CreateInterfaceFn)GetProcAddress(gSignatures.GetModuleHandleSafe(L"engine.dll"), "CreateInterface");
    CreateInterface_t CvarFactory = (CreateInterfaceFn)GetProcAddress(gSignatures.GetModuleHandleSafe(L"vstdlib.dll"), "CreateInterface");
    gInts::Engine = (EngineClient*)EngineFactory("VEngineClient013", NULL);
    gInts::cvar = (ICvar*)CvarFactory("VEngineCvar004", NULL);
    gInts::EntList = (CEntList*)ClientFactory("VClientEntityList003", NULL);
    gInts::EngineTrace = (IEngineTrace*)EngineFactory("EngineTraceClient003", NULL);

    
    DWORD serverbase = (DWORD)GetModuleHandle(L"server.dll");
    clientbase = (DWORD)GetModuleHandle(L"client.dll");


    DWORD mySig = gSignatures.GetClientSignature("B9 ?? ?? ?? ?? E8 ?? ?? ?? ?? 68 ?? ?? ?? ?? C7 05 ?? ?? ?? ?? ?? ?? ?? ?? C7 05 ?? ?? ?? ?? 00 00 00 00 C6 05 ?? ?? ?? ?? 00 E8 ?? ?? ?? ?? 59 C3") + 1;
    myGameMovement = *(CTFGameMovement**)(mySig);
    ProcessMovement = *(ProcessMovementFn*)(*(DWORD*)myGameMovement+0x4);
    CategorizePosition = *(CategorizePositionFn*)(*(DWORD*)myGameMovement + 0x84);
    FullWalkMove = *(FullWalkMoveFn*)(*(DWORD*)myGameMovement + 0x5C);
    bSendPacket = (BYTE*)(gSignatures.GetEngineSignature("89 45 F0 C6 45 FF 01 8B 01 8B 40 18 FF D0")+6);

    DWORD CmputFwdMoveAddress = gSignatures.GetClientSignature("D8 6E 20 D9 5E 20 E8 ?? ?? ?? ??") + 6; // +6 NOT +7 !!!!!!!
    g_pMoveData = **(CMoveData***)(gSignatures.GetClientSignature("FF 35 ?? ?? ?? ?? 8B 4D FC FF 75 10") + 2);
    mySig = gSignatures.GetClientSignature("8B 4E 04 E8 ? ? ? ? 8B CE 85 C0");
    GetGroundEntity = (GetGroundEntityFn)(*(DWORD*)(mySig + 4) + (mySig + 8));

    othergpGlobals = *(CGlobalVarsBase***)(gSignatures.GetClientSignature("A1 ?? ?? ?? ?? 0F 57 C9 8B 4D") + 1);
 
    frameTimeLocation = gSignatures.GetClientSignature("F3 0F 10 40 ?? A1 ?? ?? ?? ?? 57 F3 0F 11 40 ?? 8B 0D");
    IDirect3DDevice9 *pD3DDevice = **reinterpret_cast<IDirect3DDevice9***>(gSignatures.FindPatternEx(L"shaderapidx9.dll", "A1 ? ? ? ? 50 8B 08 FF 51 0C") + 1);
    

    

    EndScene = (EndSceneFn)hookVF((DWORD)pD3DDevice, 42, (DWORD)EndSceneHook);
    gpGlobals = *othergpGlobals;
    print("gpGlobals: %x\n", gpGlobals);
    mySig = (gSignatures.GetClientSignature("E8 ?? ?? ?? ?? 8B 4F 08 0F 57 D2"));
    AngleVectors2 = (AngleVectorsFn)((*(DWORD*)(mySig + 1))+(mySig+5));
    EyeAngles = *(EyeAnglesFn*)(*(DWORD*)pPlayer + 0x238);
    offsetForNoNull = *(DWORD*)(gSignatures.GetClientSignature("8B F8 74 24 8B 56 04 8B 8A ?? ?? ?? ?? 8B 71 04 8D 4A 08")+9);
    DWORD Weapon_ShootPositionOffset = *(DWORD*)(gSignatures.GetClientSignature("83 C4 ? 8B 07 51 8B CF FF 90 ? ? ? ? 80 7b ? ?")+10);
    Weapon_ShootPosition = *(Weapon_ShootPositionFn*)(*(DWORD*)pPlayer + Weapon_ShootPositionOffset);
    CreateMove = (CreateMoveFn)callHookAtE8(gSignatures.GetClientSignature("8B CF 56 E8 ? ? ? ? 8B CE E8 ? ? ? ? 89 47") + 3, (DWORD)hookedcreatemove);
    CMoveData myMoveData;

    gInts::cvar->ConsolePrintf( "pPlayer == %x\n((DWORD)&myMoveData + 0x44): %x, (DWORD) & (myMoveData.m_vecVelocity): %x, &(myMoveData.m_vecVelocity)-&(myMoveData): %x", pPlayer, (DWORD)(& myMoveData + 0x44), (DWORD)& (myMoveData.m_vecVelocity), (DWORD)&(myMoveData.m_vecVelocity) - (DWORD) & (myMoveData));

    DWORD enginebase = (DWORD)GetModuleHandle(L"engine.dll");

    gInts::cvar->ConsolePrintf("injected\n");
    ConVarFn ConVarCnstr = (ConVarFn)gSignatures.GetClientSignature("55 8B EC D9 EE 56 6A 00 51 D9 14 24 6A 00 51 D9 14 24 6A 00 51 D9 14 24 6A 00 51 D9 1C 24 6A 00 8B F1 6A 00 FF 75 10 FF");
    std::vector<ConVar*>ConVars;

    HWND hWindow = FindWindowA("Valve001", nullptr);
    pOldWindowProc = reinterpret_cast<WNDPROC>(SetWindowLongPtr(hWindow, GWLP_WNDPROC, LONG_PTR(hkWndProc)));

    retAddr = hookWithJump((DWORD)FullWalkMove + 0x861, (DWORD)WalkMoveTramp, 6);
    DWORD ClientMode2 = **(DWORD**)(dwClientModeAddress + 2);

   
    while (!(GetAsyncKeyState(VK_HOME) & 1))
        ;
        /*ImGui_ImplDX9_Shutdown();
        ImGui_ImplWin32_Shutdown();
        ImGui::DestroyContext(); These should be called, but it always crashes when it does (haven't tested them individually...*/
        SetWindowLongPtr(hWindow, GWLP_WNDPROC, LONG_PTR(pOldWindowProc));
        for (auto conVar : ConVars) {
            gInts::cvar->UnregisterConCommand((ConCommandBase*)conVar);
                //conVar = (**(ConVarDestructorFn**)conVar)(conVar);
            free((char*)((DWORD)conVar + 0x24)); // there is a string at 0x24...
            free(conVar);
        }
        gInts::cvar->ConsolePrintf("bye\n");
        //somehook->Unhook();
        exit(hModule);
        return 0;
    }
    enum
    {
        PITCH = 0,	// up / down
        YAW,		// left / right
        ROLL		// fall over
    };




BOOL APIENTRY DllMain( HMODULE hModule,
                       DWORD  ul_reason_for_call,
                       LPVOID lpReserved
                     )
{
    switch (ul_reason_for_call)
    {
    case DLL_PROCESS_ATTACH:
        CreateThread(0, 0, (LPTHREAD_START_ROUTINE)startup, hModule, 0, 0);
    case DLL_THREAD_ATTACH:
    case DLL_THREAD_DETACH:
    case DLL_PROCESS_DETACH:
        break;
    }
    return TRUE;
}

