#pragma once
extern void GetEndPos(bool WhenStopped = false, bool speedShoting = false, bool desiredPosIsLandingPos = true);
extern std::vector<Vector>Positions;
extern std::vector<float>zVels;
extern void AngleVectors(const Vector& angles, Vector* forward, Vector* right, Vector* up);
constexpr float firedelay = 0.80000001;
extern Vector rocketSource2(Vector* angles, Vector* endPosp, const Vector* Position);
extern void VectorAngles(const Vector& forward, Vector& angles);
extern inline void ClampAngle(Vector& qaAng);
extern Vector predictedLandingPos;
extern float dist(const Vector& start, const Vector& end);
extern bool getNextFmove(const Vector& viewAngles, float& nextFmove, std::vector<int>*);
constexpr float gravity = 800.f;
float timeToLandFn(float zVel, float zPosStart, float zPosEnd);
extern Vector v_customEndPos;
extern void sync(CUserCmd* pCommand);
extern bool checkForLandTick;
extern int shotTick;
extern DWORD offsetForNoNull;
extern Vector desiredRocketEndPos;
enum {
	PITCH, YAW, ROLL
};
extern CMoveData noNullPtr;