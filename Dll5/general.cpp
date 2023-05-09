#include "client.h"
#include "general.h"
std::vector<Vector>Positions;

bool CrouchedSpeedShots = true;
Vector predictedLandingPos;
Vector desiredRocketEndPos;
CMoveData noNullPtr;
bool customEndPos = true;
extern float unitsBack;
float unitsBack;

void ProcessMovementWrapper() {
	DWORD address = (DWORD)(pPlayer)+offsetForNoNull; // Do you need to change this offset on each update????
	DWORD value = (DWORD)&noNullPtr;
	writeMemoryWithProtection<DWORD>(address, value);
	writeMemoryWithProtection<float>((DWORD)gpGlobals + 0x10, 0.015f);
	ProcessMovement(myGameMovement, pPlayer, myGameMovement->mv);
}

void  GetEndPos(bool WhenStopped, bool speedShoting, bool desiredPosIsLandingPos) {
	Ray_t myRay;
	Vector start = g_pMoveData->m_vecAbsOrigin;//pPlayer->GetAbsOrigin();
	Vector m_vecVelocity = *(Vector*)((DWORD)pPlayer + 288);
	print("Networked? velocity: ");
	printVector(m_vecVelocity);
	print("g_pMoveData velocity: ")
		printVector(g_pMoveData->m_vecVelocity);
	print("Start pos from getAbsOrigin(): ")
		printVector(pPlayer->GetAbsOrigin());
	//writeMemoryWithProtection<Vector>((DWORD) &(g_pMoveData->m_vecAbsOrigin), pPlayer->GetAbsOrigin());
	print("Start pos from g_pMoveData: ")
		printVector(g_pMoveData->m_vecAbsOrigin);
	int toAdd = 0;
	
	Vector end = start;
	CTraceFilter filter;

	//print("units back: %f\n", unitsBack);
	//DWORD address = (DWORD)(pPlayer)+0x107C;
	DWORD address = (DWORD)(pPlayer)+offsetForNoNull; // Do you need to change this offset on each update????
	//DWORD value = (DWORD)(pPlayer + (0x122C / 4));
	
	DWORD value = (DWORD)&noNullPtr;
	writeMemoryWithProtection<DWORD>(address, value);
	writeMemoryWithProtection<float>((DWORD)gpGlobals + 0x10, 0.015f);
	//writeMemoryWithProtection<Vector>((DWORD)g_pMoveData+68, g
	//writeMemoryWithProtection<Vector>(myGameMovment->mv-)
	bool (*condition)();
	if (WhenStopped)
		condition = [] {return myGameMovement->mv->m_vecVelocity.z != 0 && myGameMovement->mv->m_vecVelocity.y != 0 && myGameMovement->mv->m_vecVelocity.x != 0; };
	else
		condition = [] {return myGameMovement->mv->m_vecVelocity.z != 0; };
	
	while (condition()) {
		ProcessMovement(myGameMovement, pPlayer, myGameMovement->mv);
		Positions.push_back(myGameMovement->mv->m_vecAbsOrigin);
		//zVels.push_back(myGameMovement->mv->m_vecVelocity.z);
	}

	//CPredictionCopy?????
	print("Positions 2 size: %d\n+tickcount == %d\nPredicted landing pos: ", Positions.size(), Positions.size() + gpGlobals->tickcount);
	if (Positions.size() >= 1) {
		printVector(Positions[Positions.size() - 1]);
		predictedLandingPos = Positions[Positions.size() - 1];
	}

	if (speedShoting) {
		float horzVelAng = atan2(m_vecVelocity.y, m_vecVelocity.x);
		desiredRocketEndPos = Vector(predictedLandingPos.x - (cos(horzVelAng) * unitsBack), predictedLandingPos.y - (sin(horzVelAng) * unitsBack), predictedLandingPos.z);
	}
	else if(desiredPosIsLandingPos)
		desiredRocketEndPos = predictedLandingPos;
	
}
void AngleVectors(const Vector& angles, Vector* forward, Vector* right, Vector* up) {
	float sp, sy, cp, cy, sr, cr;

	sy = sinf(DEG2RAD(angles.y)); // yaw
	cy = cosf(DEG2RAD(angles.y));

	sp = sinf(DEG2RAD(angles.x)); // pitch
	cp = cosf(DEG2RAD(angles.x));

	sr = sinf(DEG2RAD(angles.z)); // pitch
	cr = cosf(DEG2RAD(angles.z));

	if (forward) {
		forward->x = cp * cy;
		forward->y = cp * sy;
		forward->z = -sp;
	}
	if (right)
	{
		right->x = (-1 * sr * sp * cy + -1 * cr * -sy);
		right->y = (-1 * sr * sp * sy + -1 * cr * cy);
		right->z = -1 * sr * cp;
	}

	if (up)
	{
		up->x = (cr * sp * cy + -sr * -sy);
		up->y = (cr * sp * sy + -sr * cy);
		up->z = cr * cp;
	}
}
void VectorAngles(const Vector& forward, Vector& angles)
{
	//Assert(s_bMathlibInitialized);
	float	tmp, yaw, pitch;

	if (forward[1] == 0 && forward[0] == 0)
	{
		yaw = 0;
		if (forward[2] > 0)
			pitch = 270;
		else
			pitch = 90;
	}
	else
	{
		yaw = (atan2(forward[1], forward[0]) * 180 / M_PI);
		if (yaw < 0)
			yaw += 360;

		tmp = sqrt(forward[0] * forward[0] + forward[1] * forward[1]);
		pitch = (atan2(-forward[2], tmp) * 180 / M_PI);
		if (pitch < 0)
			pitch += 360;
	}

	angles[0] = pitch;
	angles[1] = yaw;
	angles[2] = 0;
}
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

Vector rocketSource2(Vector* angles, Vector* endPosp, const Vector* Position, bool crouched) {
	if (!Position) {
		DWORD address = (DWORD)(pPlayer)+offsetForNoNull;
		//DWORD address = (DWORD)(pPlayer)+0x107C;
		//DWORD value = (DWORD)(pPlayer + (0x122C / 4));
		DWORD value = (DWORD)&noNullPtr;
		writeMemoryWithProtection<DWORD>(address, value);
		writeMemoryWithProtection<float>((DWORD)gpGlobals + 16, 0.015f);
		ProcessMovement(myGameMovement, pPlayer, myGameMovement->mv);
		writeMemoryWithProtection<Vector>((DWORD)pPlayer + 0x28C, myGameMovement->mv->m_vecAbsOrigin);
	}
	else
		writeMemoryWithProtection<Vector>((DWORD)pPlayer + 0x28C, *Position);
	if (angles) {
		Vector vecShootPos;
		Weapon_ShootPosition(pPlayer, &vecShootPos);

		// THIS IS NOT THE EXPLODE POS
		VectorAngles(desiredRocketEndPos - vecShootPos, *angles);// subtract origin or weapon_shootpos?????
		ClampAngle(*angles);
		writeMemoryWithProtection<Vector>((DWORD)pPlayer + 0x1028, *angles);
	}
	Ray_t myRay;
	CTraceFilter filter;
	trace_t myTraceLocal;
	Vector fwd, right, up;

	CBaseEntity* weapon = gInts::EntList->GetClientEntityFromHandle(*(DWORD*)((DWORD)pPlayer + 3320));
	//CBaseEntity* weapon2 = gInts::EntList->GetClientEntityFromHandle(*(DWORD*)((DWORD)pPlayer + 3512));

	float vecOffsetz = crouched ? -3.f : 8.f;
	//if (*(BYTE*)(pPlayer + 280) & 2) // if you're ducking This if statement is commented out when you're speedshoting cuz it seemed to ruin everything. It didn't work
		//vecOffsetz = 8.0f;
	
	Vector	vecOffset(23.5, 12.0, vecOffsetz);
	Vector vecSrc, angForward;
	typedef void(__thiscall* GetProjectileFireSetupFn)(CBaseEntity*, CBaseEntity*, Vector, Vector*, Vector*, bool, float);
	GetProjectileFireSetupFn GetProjectileFireSetup = *(GetProjectileFireSetupFn*)(*(DWORD*)weapon + 0x63C); // This virtual table offset has changed 0x638 to 0x63C!!! 
// need a better method
	GetProjectileFireSetup(weapon, pPlayer, vecOffset, &vecSrc, &angForward, 0, 2000.f);
		if (endPosp) {
			AngleVectors(angForward, &fwd, nullptr, nullptr);
			myRay.Init(vecSrc, vecSrc + fwd * 9999);
			gInts::EngineTrace->TraceRay(myRay, 0x46004003, &filter, &myTraceLocal);
			*endPosp = myTraceLocal.endpos;
		}
	//GetProjectileFireSetup(weapon2, pPlayer, vecOffset, &vecSrc, &angForward, 0, 2000.f);
	//writeMemoryWithProtection<Vector>((DWORD)pPlayer + 0x28C, originalPos);
	//printVector(vecSrc);
	return vecSrc;
}

inline void _SSE_RSqrtInline(float a, float* out)
{
	const __m128 fThree = _mm_set_ss(3.f);
	const __m128 fOneHalf = _mm_set_ss(0.5f);
	__m128 xx, xr, xt;
	xx = _mm_set_ss(a);
	xr = _mm_rsqrt_ss(xx);
	xt = _mm_mul_ss(xr, xr);
	xt = _mm_mul_ss(xt, xx);
	xt = _mm_sub_ss(fThree, xt);
	xt = _mm_mul_ss(xt, fOneHalf);
	xr = _mm_mul_ss(xr, xt);
	_mm_store_ss(out, xr);
}

float VectorNormalize(Vector& vec) {
	float sqrlen = vec.LengthSqr() + 1.0e-10f, invlen;
	_SSE_RSqrtInline(sqrlen, &invlen);
	vec.x *= invlen;
	vec.y *= invlen;
	vec.z *= invlen;
	return sqrlen * invlen;
}
float dist(const Vector& start, const Vector& end) {
	Vector delta = end - start;
	return sqrt(delta.x * delta.x + delta.y * delta.y + delta.z * delta.z);
}
float floatNegative(float x) { // if the float is already negative, leave it. Else make it negative
	unsigned int* y = (unsigned int*)&x;
	unsigned int ret = (*y | (1 << 31));
	return *(float*)&ret;
}
float timeToLandFn(float zVel, float zPosStart, float zPosEnd) {
	return (((-zVel)+sqrt(zVel * zVel - (2 * gravity * floatNegative(zPosStart - zPosEnd)))) / gravity);
}
bool getNextFmove(const Vector& viewAngles, float& nextFmove, std::vector<int>* ticks) {

	// Should you assume a flat surface????? Probably
	DWORD address = (DWORD)(pPlayer)+offsetForNoNull;
	//DWORD address = (DWORD)(pPlayer)+0x107C;
	//DWORD value = (DWORD)(pPlayer + (0x122C / 4));
	DWORD value = (DWORD)&noNullPtr;
	writeMemoryWithProtection<DWORD>(address, value);

	writeMemoryWithProtection<float>((DWORD) &(g_pMoveData->m_flForwardMove), -240.f);
	writeMemoryWithProtection<float>((DWORD)gpGlobals + 0x10, 0.015f);
	myGameMovement->mv->m_flForwardMove = -240.f;
	Vector lastPos = g_pMoveData->m_vecAbsOrigin;
	Vector lastVel = g_pMoveData->m_vecVelocity;
	ProcessMovement(myGameMovement, pPlayer, myGameMovement->mv);
	// save the positions in case you're predicting the second triple for a quint sync.
	
	float deltax = myGameMovement->mv->m_vecAbsOrigin.x - v_customEndPos.x; //...
	float deltay = myGameMovement->mv->m_vecAbsOrigin.y - v_customEndPos.y;
	float distance = sqrt(deltax * deltax + deltay * deltay);
	float HorzSpeed = sqrt(myGameMovement->mv->m_vecVelocity.x * myGameMovement->mv->m_vecVelocity.x + myGameMovement->mv->m_vecVelocity.y * myGameMovement->mv->m_vecVelocity.y);
	float timeToHorzPos = distance / HorzSpeed;
	int ticksToHorzPos = (int)(timeToHorzPos / 0.015f) + 1;
	float timeToLand = timeToLandFn(myGameMovement->mv->m_vecVelocity.z, myGameMovement->mv->m_vecAbsOrigin.z, v_customEndPos.z);
	//static float* timeToLand2 = NULL;
	//if(timeToLand2 == NULL)


	int ticksToLand = (int)(timeToLand / 0.015f) + 1;
	// ticksToLand ...
	if (timeToHorzPos < timeToLand) {
		// you will go too far. Slow down
		if (ticks) {
			Vector rocketOrigin = rocketSource2(NULL, NULL, &myGameMovement->mv->m_vecAbsOrigin); // to get exact rocket origin, you would need the origin of the rocket the next tick... But you don't
			// know what your fmove next tick will be...???
			ticks->push_back((int)(dist(rocketOrigin, desiredRocketEndPos) / (1100.f * 0.015f)+1));
		}
		nextFmove = -240.f;
		return true;
	}
	else if (timeToHorzPos > timeToLand) {

		float desiredSpeed = distance / timeToLand;
		print("desiredSpeed: %f\n", desiredSpeed);
		//xVel = cos(viewAngles[YAW]) * desiredSpeed;
		//yVel = sin(viewAngles[YAW]) * desiredSpeed; 
		Vector forward;

		AngleVectors(viewAngles, &forward, NULL, NULL);
		VectorNormalize(forward);
		float d = sqrt(forward[1] * forward[1] + forward[0] * forward[0]) * 0.15f; //0.015f * accel;

		float a = d * d * (forward[0] * forward[0] + forward[1] * forward[1]);
		float b = -2 * d * ((lastVel[0] * forward[0]) + (lastVel[1] * forward[1]));
		float c = lastVel[0] * lastVel[0] + lastVel[1] * lastVel[1] - (desiredSpeed * desiredSpeed);
		
		float e = ((-b + sqrt(b * b - 4 * a * c)) / (2 * a));
		float f = ((-b - sqrt(b * b - 4 * a * c)) / (2 * a));
		nextFmove = sqrt(e);
		nextFmove = -sqrt(f);
		
		float wishdir1 = nextFmove * forward[0], wishdir2 = nextFmove * forward[1];
		float wishspeed = sqrt(wishdir1 * wishdir1 * wishdir2 * wishdir2);
		wishdir1 = wishdir1 / wishspeed; wishdir2 = wishdir2 / wishspeed;
		float accelspeed = 0.15 * wishspeed;
		float wishspd = wishspeed > 30.f ? 30.f : wishspeed;
		float currentSpeed = lastVel[0] * wishdir1 + lastVel[1] * wishdir2;
		float addspeed = wishspd - currentSpeed;
		if (accelspeed > addspeed)
			accelspeed = addspeed;
		float newVel0 = lastVel[0] + accelspeed * wishdir1;
		float newVel1 = lastVel[1] + accelspeed * wishdir2;
		print("predicted velocity: %f\n", sqrt(newVel0 * newVel0 + newVel1 * newVel1));


		if (ticks) {
			myGameMovement->mv->m_flForwardMove = nextFmove;
			myGameMovement->mv->m_vecAbsOrigin = lastPos;
			myGameMovement->mv->m_vecVelocity = lastVel;
			writeMemoryWithProtection<Vector>((DWORD) & (g_pMoveData->m_vecAbsOrigin), lastPos);
			writeMemoryWithProtection<Vector>((DWORD) & (g_pMoveData->m_vecVelocity), lastVel);
			writeMemoryWithProtection<float>((DWORD) & (g_pMoveData->m_flForwardMove), nextFmove);
			ProcessMovement(myGameMovement, pPlayer, myGameMovement->mv);
			print("actual speed %f\nDesired Speed: %f\n", sqrt((myGameMovement->mv->m_vecVelocity.x) * (myGameMovement->mv->m_vecVelocity.x) + (myGameMovement->mv->m_vecVelocity.y) * (myGameMovement->mv->m_vecVelocity.y)), desiredSpeed);
			Vector rocketOrigin = rocketSource2(NULL, NULL, &myGameMovement->mv->m_vecAbsOrigin);
			ticks->push_back((int)(dist(rocketOrigin, desiredRocketEndPos) / (1100.f * 0.015f))+1);
		}
		return false;
	}
	else
	{
		nextFmove = -240.f;
		return false;
	}
}
CBaseEntity* checkForRocket() {
	CBaseEntity* ent;
	for (int i = 0; i < gInts::EntList->GetHighestEntityIndex(); ++i) {
		ent = gInts::EntList->GetClientEntity(i);
		if (ent && (!strcmp(ent->GetClientClass()->chName, "CTFProjectile_Rocket")))
			return ent;
	}
}