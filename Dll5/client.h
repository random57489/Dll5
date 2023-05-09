#pragma once
#include "sdk.h"
#define print(string, ...) gInts::cvar->ConsolePrintf(string, __VA_ARGS__);
# define pPlayer (gInts::EntList->GetClientEntity(gInts::Engine->GetLocalPlayer()))
//extern Vector endPos;
//extern int starttick;
extern Vector getPosInTime(float);
//constexpr int timeLimit = 10;
//extern int nexttick;
extern CGlobalVarsBase* gpGlobals;
extern Vector poses[];
typedef void(__thiscall* CreateMoveFn)(DWORD, CUserCmd*);
extern CreateMoveFn CreateMove;
void  __fastcall hookedcreatemove(DWORD ClientMode, DWORD edx, CUserCmd* pCommand);
float __fastcall SetKeyHook(DWORD _this, DWORD edx, DWORD key);
extern DWORD clientbase;


//extern inline void _SSE_RSqrtInline(float a, float* out);
//extern float VectorNormalize(Vector& vec);




struct CMoveData {
    BYTE a[12];
    Vector m_vecViewAngles;// THIS DOESN'T seem like the right place for this...
    DWORD F;
    int buttons;
    DWORD G;
    DWORD H;
    DWORD I;
    float m_flForwardMove;
    DWORD d;
    float m_flSideMove;
    BYTE c[12];
    Vector m_vecVelocity;
    BYTE e[156 - 80];
    Vector m_vecAbsOrigin;
};

struct CTFGameMovement {
    BYTE a[8];
    CMoveData* mv;
};


typedef  void(__thiscall* CategorizePositionFn)(CTFGameMovement*);
extern CategorizePositionFn CategorizePosition;
extern CMoveData* g_pMoveData;
//int futureVelocity(Vector& m_vecVelocity, const Vector& m_vecViewAngles);
typedef  Vector* (__thiscall* EyeAnglesFn)(CBaseEntity*);
extern EyeAnglesFn EyeAngles;
typedef DWORD* (__thiscall* GetGroundEntityFn)(CBaseEntity*);
extern GetGroundEntityFn GetGroundEntity;
typedef void(__cdecl* AngleVectorsFn)(const Vector&, Vector*, Vector*, Vector*);
extern AngleVectorsFn AngleVectors2;
typedef void (__thiscall* Weapon_ShootPositionFn)(CBaseEntity*, Vector*);
extern Weapon_ShootPositionFn Weapon_ShootPosition;
typedef void(__thiscall* FullWalkMoveFn)(CTFGameMovement*);
extern FullWalkMoveFn FullWalkMove;
extern CTFGameMovement* myGameMovement;
typedef void(__thiscall* ProcessMovementFn)(CTFGameMovement*, CBaseEntity*, CMoveData*);
extern ProcessMovementFn ProcessMovement;
extern CGlobalVarsBase** othergpGlobals;
extern BYTE* bSendPacket;
//extern ConVar customRocketEndPos;
//extern ConVar doubleSync;
//extern ConVar quinting;
//extern ConVar syncing;
//extern ConVar speedshoting;
//extern ConVar unitsBack;

typedef ConVar*(__thiscall *ConVarFn)(ConVar *convar, const char* pName, const char* pDefaultValue, int flags);
typedef ConVar*(__thiscall* ConVarDestructorFn)(ConVar* convar);
template<typename T>
DWORD protectMemory(DWORD address, DWORD prot)
{
    DWORD oldProt;
    VirtualProtect((LPVOID)address, sizeof(T), prot, &oldProt);
    return oldProt;
}
template<typename T>
void writeMemoryWithProtection(DWORD address, T value)
{
    auto oldProtection = protectMemory<T>(address, PAGE_EXECUTE_READWRITE);
    *((T*)address) = value;
    protectMemory<T>(address, oldProtection);
}
template<typename T>
T readMemory(DWORD address)
{
    return *((T*)address);
}
struct usefulCmds {
    usefulCmds(float afmove, float asmove, int abuttons, const Vector& aviewAngles) :fmove(afmove), smove(asmove), buttons(abuttons), viewAngles(aviewAngles) {}
    float fmove;
    float smove;
    int buttons;
    Vector viewAngles;
};
//extern std::vector<usefulCmds>pastCommands;
extern DWORD frameTimeLocation;

