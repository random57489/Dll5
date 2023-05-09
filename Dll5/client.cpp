#include "client.h"
#include "vmthook.h"
#include "offsets.h"
#include "sdk.h"
#include <limits>
#include "general.h"
#include <stdexcept>
extern char SpeedShotKey[];

trace_t myTrace;

std::vector<float>endPosDists;
std::vector<Vector>Positions2;
std::vector<float>frameTimes;



static bool CrouchedSpeedShots = true;

//bool moveForward = false;
bool movingToTarget = false;


float __fastcall SetKeyHook(DWORD _this, DWORD edx, DWORD key) {
	if (*(DWORD*)(key + 8) & 1)//if the the key is down...
		return 0.25;
	return 0.0;
}



Vector correctAngles;

static bool manualAimSpdSht = false;



inline void ClampAngle(Vector& qaAng)
{
	while (qaAng.x > 89)
		qaAng.x -= 180;

	while (qaAng.x < -89)
		qaAng.x += 180;

	while (qaAng.y > 180)
		qaAng.y -= 360;

	while (qaAng.y < -180)
		qaAng.y += 360;

	while (qaAng.z != 0)
		qaAng.z = 0;
}

bool checkForSync = false;
int passes = 3;


int RocketShotPasses = 0;
float ExplodeTime;

int positionCounter;
int numOfPositions = 0;
Vector lastPos;
Vector v_customEndPos; 


bool checkForLandTick2 = false;
int addoneortwo = 0;
float predictedLandTime;



void speedShot(CUserCmd* pCommand);

bool checkForLandTick = false;
int shotTick;
void  __fastcall hookedcreatemove(DWORD ClientMode, DWORD edx, CUserCmd* pCommand) {// NOT ACTUALLY CREATEMOVE

	static float rocketShotTime;
	static Vector rocketShotLocation;
	static bool rocketShot = false;
	static bool syncing = false;
	static bool doubleSync = false;
	static bool customEndPos = false;
	static bool moveToEndPos = true;
	static int syncStartTick;
	static Vector rocketVel;
	static int latestTick;
	static float closestDifference = FLT_MAX;
	static bool waitingForGroundRocket = false;
	static float speedShotStartTick;
	static float closestOfClosestDifferences = FLT_MAX;
	
	static int mySpecialIndex = 0;
	constexpr float firedelay = 0.80000001; // THIS IS THE TRUE FIRERATE: 0.80000001f
	static std::vector<Vector> velPrediction;
	static int ticksToLand;
	static std::vector<int> thresholdTicks;
	static int tickAtClosestDiff = INT_MAX;
	
	static float latestDistance = 1100.f * firedelay;
	static bool moveForward2 = false;
	
	static DWORD pointsTo;
	static CBaseEntity* ent;
	static float averageFrameTime;
	static int divisor;
	
	static int myStartTick;
	static bool getEndPos = false;
	static float lastOrigin;
	static float lastPred;


	static int nextTick = -1;
	static bool bunnyHopping = false;
	if (GetAsyncKeyState('U') & 1) { // U for c-tap...
		pCommand->buttons |= 4; // duck right away...
		// the optimal amount of ticks afterward to jump and to shoot... Should you even do these at the same time?
	}
	if (GetAsyncKeyState('T') & 1)
		bunnyHopping = true;
	if (GetAsyncKeyState('Y') & 1)
		bunnyHopping = false;
	
	if (gpGlobals->tickcount == nextTick) {
		pCommand->buttons |= 2;
		//nextTick = true;
		//writeMemoryWithProtection<BYTE>((DWORD)bSendPacket, 1);
	}
	//static int successfulBhops = 0;


	if(bunnyHopping == true) {
		DWORD address = (DWORD)(pPlayer)+offsetForNoNull;
		DWORD value = (DWORD)&noNullPtr;
		writeMemoryWithProtection<DWORD>(address, value);
		writeMemoryWithProtection<float>((DWORD)gpGlobals + 16, 0.015f);
		ProcessMovement(myGameMovement, pPlayer, myGameMovement->mv);
		if (myGameMovement->mv->m_vecVelocity.z == 0) {
			//writeMemoryWithProtection<BYTE>((DWORD)bSendPacket, 0);
			nextTick = gpGlobals->tickcount + 1;
			//bunnyHopping = false;
			
			//print("bhops = %d\n", ++successfulBhops);
		}
		 //possible sig for editing bsendpacket "89 45 F0 C6 45 FF 01  8B 01 8B 40 18 FF D0";
	}
	

	static int cmdIndex = 0;
	static bool replayMovement = false;
	static CBaseEntity* ent2;
	static DWORD pointsTo2;
	static bool predictionStarted = false;
	static bool moveToPos = false;
	static Vector goToPos;
		
	static float totalFrametimePred;
	static float actualTotalFrametime;
	static float lastPredz;

	try {
		speedShot(pCommand);
		sync(pCommand);
	}
	catch (const std::out_of_range& oor) {
		print("error");
	}

		if (checkForLandTick && gpGlobals->tickcount != speedShotStartTick) {
			if ( g_pMoveData->m_vecVelocity.z == 0) {
				print("Actual landing Tick == %d\nTicks to Land: %d\n", gpGlobals->tickcount, gpGlobals->tickcount-speedShotStartTick);
				checkForLandTick = false;
				print("Actual landingPos == ");
				printVector(pPlayer->GetAbsOrigin());
				/*print("average FrameTime: %f\n", averageFrameTime / divisor);
				divisor = averageFrameTime = 0;*/
				//printVector(myGameMovement->mv->m_vecAbsOrigin);
				print("Predicted landingPos ==");
				printVector(predictedLandingPos);
			}
		}

		CreateMove(ClientMode, pCommand);
		return;
	}
