#pragma once
#include "getvfunc.h"
#include <Windows.h>
#include "CNetVars.h"
#include <assert.h>
#include <corecrt_math_defines.h>
#include "signature.h"
#include "xmmintrin.h"

typedef void* (__cdecl* CreateInterface_t)(const char*, int*);
typedef void* (*CreateInterfaceFn)(const char* pName, int* pReturnCode);

#define PI 3.14159265358979323846f
#define DEG2RAD( x ) ( ( float )( x ) * ( float )( ( float )( PI ) / 180.0f ) )

//CSignature gSignatures;
//class Vector;
//typedef player 
//class C_BasePlayer_or_C_TfPlayer_idk {
//	float HorizontalDistance(Vector endPos);
//};

class CGlobalVarsBase
{
public:

	// Absolute time (per frame still - Use Plat_FloatTime() for a high precision real time 
	//  perf clock, but not that it doesn't obey host_timescale/host_framerate)
	float			realtime;
	// Absolute frame counter
	int				framecount;
	// Non-paused frametime
	float			absoluteframetime;

	// Current time 
	//
	// On the client, this (along with tickcount) takes a different meaning based on what
	// piece of code you're in:
	// 
	//   - While receiving network packets (like in PreDataUpdate/PostDataUpdate and proxies),
	//     this is set to the SERVER TICKCOUNT for that packet. There is no interval between
	//     the server ticks.
	//     [server_current_Tick * tick_interval]
	//
	//   - While rendering, this is the exact client clock 
	//     [client_current_tick * tick_interval + interpolation_amount]
	//
	//   - During prediction, this is based on the client's current tick:
	//     [client_current_tick * tick_interval]
	float			curtime;

	// Time spent on last server or client frame (has nothing to do with think intervals)
	float			frametime;
	// current maxplayers setting
	int				maxClients;

	// Simulation ticks
	int				tickcount;

	// Simulation tick interval
	float			interval_per_tick;

	// interpolation amount ( client-only ) based on fraction of next tick which has elapsed
	float			interpolation_amount;
	int				simTicksThisFrame;

	int				network_protocol;

	// current saverestore data
	DWORD* pSaveData;

private:
	// Set to true in client code.
	bool			m_bClient;

	// 100 (i.e., tickcount is rounded down to this base and then the "delta" from this base is networked
	int				nTimestampNetworkingBase;
	// 32 (entindex() % nTimestampRandomizeWindow ) is subtracted from gpGlobals->tickcount to set the networking basis, prevents
	//  all of the entities from forcing a new PackedEntity on the same tick (i.e., prevents them from getting lockstepped on this)
	int				nTimestampRandomizeWindow;

};
class Vector
{
public:
	Vector(float X, float Y, float  Z) : x{ X }, y{ Y }, z{ Z }{}
	void Init(float ix = 0.0f, float iy = 0.0f, float iz = 0.0f);
	inline float operator[](int i) const;
	inline float& operator[](int i);
	__forceinline Vector operator*(float fl);
	__forceinline Vector operator+(const Vector& vec);
	__forceinline Vector operator+(const float fl);
	__forceinline Vector operator-(const Vector& vec)const;
	__forceinline Vector& operator*=(float fl);
	__forceinline Vector operator/(float i);
	__forceinline void operator+=(const Vector& vec);
	__forceinline float Dot(const Vector&);
	__forceinline float Length2D();
	Vector() : x{ 0 }, y{ 0 }, z{ 0 }{}
	float x, y, z;
	__forceinline float LengthSqr(void) const
	{
		return (x * x + y * y + z * z);
	}
};

__forceinline float Vector::Length2D() {
	return sqrt(x * x + y * y);
}
__forceinline float Vector::Dot(const Vector& v) {
	return x * v.x + y * v.y+ z * v.z;
}
inline void Vector::Init(float ix, float iy, float iz)
{
	x = ix; y = iy; z = iz;
}
__forceinline Vector Vector::operator*(float fl)
{
	return Vector(x * fl, y * fl, z * fl);
}
__forceinline Vector& Vector::operator*=(float fl)
{
	x *= fl;
	y *= fl;
	z *= fl;
	return *this;
}
__forceinline Vector Vector::operator+(const Vector& vec)
{
	return Vector(vec.x + x, vec.y + y, vec.z + z);
}
__forceinline Vector Vector::operator+(const float fl)
{
	return Vector(x + fl, y + fl, z + fl);
}
__forceinline void Vector::operator+=(const Vector& vec) {
	x += vec.x;
	y += vec.y;
	z += vec.z;
}
__forceinline Vector Vector::operator-(const Vector& vec) const
{
	return Vector(x - vec.x, y - vec.y, z - vec.z);
}
inline float Vector::operator[](int i) const
{
	assert((i >= 0) && (i < 3));
	return ((float*)this)[i];
}
inline float& Vector::operator[](int i)
{
	assert((i >= 0) && (i < 3));
	return ((float*)this)[i];
}
inline Vector Vector::operator/(float i)
{
	return Vector(x / i, y / i, z / i);
}

class ClientClass
{
private:
	BYTE _chPadding[8];
public:
	char* chName;
	RecvTable* Table;
	ClientClass* pNextClass;
	int iClassID;
};

class CBaseEntity {
public:
	bool IsDormant()
	{
		PVOID pNetworkable = (PVOID)(this + 0x8);
		typedef bool(__thiscall* OriginalFn)(PVOID);
		return getvfunc<OriginalFn>(pNetworkable, 8)(pNetworkable);
	}
	ClientClass* GetClientClass()
	{
		PVOID pNetworkable = (PVOID)(this + 0x8);
		typedef ClientClass* (__thiscall* OriginalFn)(PVOID);
		return getvfunc<OriginalFn>(pNetworkable, 2)(pNetworkable);
	}
	Vector& GetAbsOrigin()
	{
		typedef Vector& (__thiscall* OriginalFn)(PVOID);
		return getvfunc<OriginalFn>(this, 9)(this);
	}
	void SetViewAngles(Vector& va)
	{
		typedef void(__thiscall* OriginalFn)(PVOID, Vector& va);
		return getvfunc<OriginalFn>(this, 20)(this, va);
	}

	Vector GetVelocity()
	{
		typedef void(__thiscall* EstimateAbsVelocityFn)(CBaseEntity*, Vector&);
		static DWORD dwFn = gSignatures.GetClientSignature("E8 ? ? ? ? F3 0F 10 4D ? 8D 85 ? ? ? ? F3 0F 10 45 ? F3 0F 59 C9 56 F3 0F 59 C0 F3 0F 58 C8 0F 2F 0D ? ? ? ? 76 07") + 0x1;
		static DWORD dwEstimate = ((*(PDWORD)(dwFn)) + dwFn + 0x4);
		EstimateAbsVelocityFn vel = (EstimateAbsVelocityFn)dwEstimate;
		Vector v;
		vel(this, v);
		return v;
	}
};
class C_BaseCombatCharacter {
public:
	DWORD GetActiveWeapon() {
		typedef DWORD(__thiscall* OriginalFn)(PVOID);
		return getvfunc<OriginalFn>(this, 288)(this);
	}
};

class IAppSystem
{
public:
	// Here's where the app systems get to learn about each other
	virtual bool Connect(CreateInterfaceFn factory) = 0;
	virtual void Disconnect() = 0;

	// Here's where systems can access other interfaces implemented by this object
	// Returns NULL if it doesn't implement the requested interface
	virtual void* QueryInterface(const char* pInterfaceName) = 0;

	// Init, shutdown
	virtual int Init() = 0;
	virtual void Shutdown(char* reason) = 0;

};
class ConCommandBase
{
};
struct ConVar {
	// how to get size, write constexpr size_t sizeOfConvar = sizeof(ConVar) in source code leak, hover over sizeOfConvar, size is 92U??
	// in IDA, the largest offset from ecx is 0x58... (I didn't check all functions just the 
	BYTE a[0x24];
	int m_nValue;
	BYTE b[0x58 - 0x28];
	operator bool() const {
		return m_nValue;
	}
};
class ConCommand {

};
typedef void(*FnChangeCallback_t);
class Color{};


class ICvar : public IAppSystem
{
public:
	// Allocate a unique DLL identifier
	virtual int AllocateDLLIdentifier() = 0;

	// Register, unregister commands
	virtual void			RegisterConCommand(ConCommandBase* pCommandBase) = 0;
	virtual void			UnregisterConCommand(ConCommandBase* pCommandBase) = 0;
	virtual void			UnregisterConCommands(int id) = 0;

	// If there is a +<varname> <value> on the command line, this returns the value.
	// Otherwise, it returns NULL.
	virtual const char* GetCommandLineValue(const char* pVariableName) = 0;

	// Try to find the cvar pointer by name
	virtual ConCommandBase* FindCommandBase(const char* name) = 0;
	virtual const ConCommandBase* FindCommandBase(const char* name) const = 0;
	virtual ConVar* FindVar(const char* var_name) = 0;
	virtual const ConVar* FindVar(const char* var_name) const = 0;
	virtual ConCommand* FindCommand(const char* name) = 0;
	virtual const ConCommand* FindCommand(const char* name) const = 0;

	// Get first ConCommandBase to allow iteration
	virtual ConCommandBase* GetCommands(void) = 0;
	virtual const ConCommandBase* GetCommands(void) const = 0;

	// Install a global change callback (to be called when any convar changes)
	virtual void			InstallGlobalChangeCallback(FnChangeCallback_t callback) = 0;
	virtual void			RemoveGlobalChangeCallback(FnChangeCallback_t callback) = 0;
	virtual void			CallGlobalChangeCallbacks(ConVar* var, const char* pOldString, float flOldValue) = 0;

	// Install a console printer
	virtual void			InstallConsoleDisplayFunc(void* pDisplayFunc) = 0;
	virtual void			RemoveConsoleDisplayFunc(void* pDisplayFunc) = 0;
	virtual void			ConsoleColorPrintf(const Color& clr, const char* pFormat, ...) const = 0;
	virtual void			ConsolePrintf(const char* pFormat, ...) const = 0; // index 19?
	/*void ConsolePrintf(const char* pFormat, ...) {
		return getvfunc<void(_stdcall*)(const char* pFormat, ...)>(this, 19)(pFormat, ...);
	}*/
	virtual void			ConsoleDPrintf(const char* pFormat, ...) const = 0;

	// Reverts cvars which contain a specific flag
	virtual void			RevertFlaggedConVars(int nFlag) = 0;

	// Method allowing the engine ICvarQuery interface to take over
	// A little hacky, owing to the fact the engine is loaded
	// well after ICVar, so we can't use the standard connect pattern
	virtual void			InstallCVarQuery(void* pQuery) = 0;

#if defined( _X360 )
	virtual void			PublishToVXConsole() = 0;
#endif
	virtual bool			IsMaterialThreadSetAllowed() const = 0;
	virtual void			QueueMaterialThreadSetValue(ConVar* pConVar, const char* pValue) = 0;
	virtual void			QueueMaterialThreadSetValue(ConVar* pConVar, int nValue) = 0;
	virtual void			QueueMaterialThreadSetValue(ConVar* pConVar, float flValue) = 0;
	virtual bool			HasQueuedMaterialThreadConVarSets() const = 0;
	virtual int				ProcessQueuedMaterialThreadConVarSets() = 0;

protected:	class ICVarIteratorInternal;
public:
	/// Iteration over all cvars.
	/// (THIS IS A SLOW OPERATION AND YOU SHOULD AVOID IT.)
	/// usage:
	/// { ICVar::Iterator iter(g_pCVar);
	///   for ( iter.SetFirst() ; iter.IsValid() ; iter.Next() )
	///   {
	///       ConCommandBase *cmd = iter.Get();
	///   }
	/// }
	/// The Iterator class actually wraps the internal factory methods
	/// so you don't need to worry about new/delete -- scope takes care
	//  of it.
	/// We need an iterator like this because we can't simply return a
	/// pointer to the internal data type that contains the cvars --
	/// it's a custom, protected class with unusual semantics and is
	/// prone to change.
	class Iterator
	{
	public:
		inline Iterator(ICvar* icvar);
		inline ~Iterator(void);
		inline void		SetFirst(void);
		inline void		Next(void);
		inline bool		IsValid(void);
		inline ConCommandBase* Get(void);
	private:
		ICVarIteratorInternal* m_pIter;
	};

protected:
	// internals for  ICVarIterator
	class ICVarIteratorInternal
	{
	public:
		// warning: delete called on 'ICvar::ICVarIteratorInternal' that is abstract but has non-virtual destructor [-Wdelete-non-virtual-dtor]
		virtual ~ICVarIteratorInternal()
		{
		}
		virtual void		SetFirst(void) = 0;
		virtual void		Next(void) = 0;
		virtual	bool		IsValid(void) = 0;
		virtual ConCommandBase* Get(void) = 0;
	};

	virtual ICVarIteratorInternal* FactoryInternalIterator(void) = 0;
	friend class Iterator;
};


class CUserCmd
{
public:
	virtual ~CUserCmd() {}; //Destructor 0
	int command_number; //4
	int tick_count; //8
	Vector viewangles; //C
	float forwardmove; //18
	float sidemove; //1C
	float upmove; //20
	int	buttons; //24
	BYTE impulse; //28
	int weaponselect; //2C
	int weaponsubtype; //30
	int random_seed; //34
	short mousedx; //38
	short mousedy; //3A
	bool hasbeenpredicted; //3C;
};
enum TraceType_t
{
	TRACE_EVERYTHING = 0,
	TRACE_WORLD_ONLY,				// NOTE: This does *not* test static props!!!
	TRACE_ENTITIES_ONLY,			// NOTE: This version will *not* test static props
	TRACE_EVERYTHING_FILTER_PROPS,	// NOTE: This version will pass the IHandleEntity for props through the filter, unlike all other filters
};

enum SurfaceFlags_t
{
	DISPSURF_FLAG_SURFACE = (1 << 0),
		DISPSURF_FLAG_WALKABLE = (1 << 1),
		DISPSURF_FLAG_BUILDABLE = (1 << 2),
		DISPSURF_FLAG_SURFPROP1 = (1 << 3),
		DISPSURF_FLAG_SURFPROP2 = (1 << 4),
};
class ITraceFilter
{
public:
	virtual bool ShouldHitEntity(void* pEntity, int contentsMask) = 0;
	virtual TraceType_t	GetTraceType() const = 0;
};

class CTraceFilter : public ITraceFilter
{
public:
	virtual bool ShouldHitEntity(void* pEntityHandle, int contentsMask)
	{
		CBaseEntity* pEntity = (CBaseEntity*)pEntityHandle;

		switch (pEntity->GetClientClass()->iClassID)
		{
		case 55: // Portal Window
		case 64: // Spawn Door visualizers
		case 117: // Sniper Dots
		case 225: // Medigun Shield
			return false;
			break;
		}

		return !(pEntityHandle == pSkip);
	}

	virtual TraceType_t	GetTraceType() const
	{
		return TRACE_EVERYTHING;
	}

	void* pSkip;
};

struct cplane_t
{
	Vector normal;
	float	dist;
	BYTE	type;
	BYTE	signbits;
	BYTE	pad[2];
};
struct csurface_t
{
	const char* name;
	short			surfaceProps;
	unsigned short	flags;
};
class CBaseTrace
{
public:
	bool IsDispSurface(void) { return ((dispFlags & DISPSURF_FLAG_SURFACE) != 0); }
	bool IsDispSurfaceWalkable(void) { return ((dispFlags & DISPSURF_FLAG_WALKABLE) != 0); }
	bool IsDispSurfaceBuildable(void) { return ((dispFlags & DISPSURF_FLAG_BUILDABLE) != 0); }
	bool IsDispSurfaceProp1(void) { return ((dispFlags & DISPSURF_FLAG_SURFPROP1) != 0); }
	bool IsDispSurfaceProp2(void) { return ((dispFlags & DISPSURF_FLAG_SURFPROP2) != 0); }

public:
	Vector			startpos;
	Vector			endpos;
	cplane_t		plane;

	float			fraction;

	int				contents;
	unsigned short	dispFlags;

	bool			allsolid;
	bool			startsolid;

	CBaseTrace() {}

private:
	CBaseTrace(const CBaseTrace& vOther);
};


class CGameTrace : public CBaseTrace
{
public:
	bool DidHitWorld() const;

	bool DidHitNonWorldEntity() const;

	int GetEntityIndex() const;

	bool DidHit() const
	{
		return fraction < 1 || allsolid || startsolid;
	}

public:
	float			fractionleftsolid;
	csurface_t		surface;

	int				hitgroup;

	short			physicsbone;

	CBaseEntity *m_pEnt;
	int				hitbox;

	CGameTrace() {}
	CGameTrace(const CGameTrace& vOther);
};
typedef CGameTrace trace_t;
class __declspec(align(16))VectorAligned : public Vector
{
public:
	inline VectorAligned(void) {};

	inline VectorAligned(float X, float Y, float Z)
	{
		Init(X, Y, Z);
	}

	explicit VectorAligned(const Vector& vOther)
	{
		Init(vOther.x, vOther.y, vOther.z);
	}

	VectorAligned& operator=(const Vector& vOther)
	{
		Init(vOther.x, vOther.y, vOther.z);
		return *this;
	}

	float w;
};
struct Ray_t
{
	VectorAligned   m_Start;
	VectorAligned   m_Delta;
	VectorAligned   m_StartOffset;
	VectorAligned   m_Extents;

	bool    m_IsRay;
	bool    m_IsSwept;

	void Init(const Vector& start, const Vector& end)
	{
		m_Delta = end - start;

		m_IsSwept = (m_Delta.LengthSqr() != 0);

		m_Extents.Init();
		m_IsRay = true;

		m_StartOffset.Init();
		m_Start = start;
	}

	void Init(Vector& start, Vector& end, Vector& mins, Vector& maxs)
	{
		m_Delta = end - start;

		m_IsSwept = (m_Delta.LengthSqr() != 0);

		m_Extents = maxs - mins;
		m_Extents *= 0.5f;
		m_IsRay = (m_Extents.LengthSqr() < 1e-6);

		m_StartOffset = mins + maxs;
		m_StartOffset *= 0.5f;
		m_Start = start - m_StartOffset;
		m_StartOffset *= -1.0f;
	}
};

class IEngineTrace
{
public:	  //We really only need this I guess...
	void TraceRay(const Ray_t& ray, unsigned int fMask, ITraceFilter* pTraceFilter, trace_t* pTrace)
	{
		typedef void(__thiscall* TraceRayFn)(void*, const Ray_t&, unsigned int, ITraceFilter*, trace_t*);
		return getvfunc<TraceRayFn>(this, 4)(this, ray, fMask, pTraceFilter, pTrace);
	}
};


class QAngle{};
//class C_TFWeaponBase {
//	DWORD GetProjectileFireSetup(DWORD* pPlayer, Vector vecOffset, Vector* vecSrc , QAngle* angForward, bool bHitTeammates, float idk) {
//		typedef DWORD(__thiscall* OriginalFn)(PVOID, DWORD*, Vector, Vector*, QAngle*, bool, float);
//		return getvfunc<OriginalFn>(this, 233)(this, pPlayer, vecOffset, vecSrc, angForward, bHitTeammates, idk);
//	}
//};
//class C_BasePlayer:public CBaseEntity{
//	
//
//};
//inline Vector Vector::operator-(const Vector& v) const
//{
//	Vector res;
//	res.x = x - v.x;
//	res.y = y - v.y;
//	res.z = z - v.z;
//	return res;
//}
class IClientUnknown {
public:
	const Vector& GetCollisionOrigin() {
		return getvfunc<Vector& (__thiscall*)()>(this, 10)();
	} // 10
	//virtual const QAngle& GetCollisionAngles() const = 0; // 11
};
class IClientEntity : public IClientUnknown {


};
class CEntList
{
public:
	IClientEntity* GetClientEntity2(int entnum)
	{
		typedef IClientEntity* (__thiscall* OriginalFn)(PVOID, int);
		return getvfunc<OriginalFn>(this, 3)(this, entnum);
	}
	CBaseEntity* GetClientEntity(int entnum)
	{
		typedef CBaseEntity* (__thiscall* OriginalFn)(PVOID, int);
		return getvfunc<OriginalFn>(this, 3)(this, entnum);
	}
	CBaseEntity* GetClientEntityFromHandle(int hEnt)
	{
		typedef CBaseEntity* (__thiscall* OriginalFn)(PVOID, int);
		return getvfunc<OriginalFn>(this, 4)(this, hEnt);
	}
	int GetHighestEntityIndex(void)
	{
		typedef int(__thiscall* OriginalFn)(PVOID);
		return getvfunc<OriginalFn>(this, 6)(this);
	}
};
class EngineClient {
public:
	int GetLocalPlayer(void)
	{
		typedef int(__thiscall* OriginalFn)(PVOID);
		return getvfunc<OriginalFn>(this, 12)(this);
	}
	void SetViewAngles(Vector& va)
	{
		typedef void(__thiscall* OriginalFn)(PVOID, Vector& va);
		return getvfunc<OriginalFn>(this, 20)(this, va);
	}

};
namespace gInts {
	extern CEntList* EntList;
	extern ICvar* cvar;
	extern EngineClient* Engine;
	//extern IClientDLLSharedAppSystems* dlls;
	extern IEngineTrace* EngineTrace;
	//extern ClientModeShared* ClientMode;
}
inline void printVector(const Vector& v) {
	gInts::cvar->ConsolePrintf("%f %f %f\n", v.x, v.y, v.z);
}
class ClientModeShared {

};
class CBase_Rocket {
	
};/*
class  IClientDLLSharedAppSystems
{
public:
	virtual int	Count() = 0;
	virtual char const* GetDllName(int idx) = 0;
	virtual char const* GetInterfaceName(int idx) = 0;
};*/
