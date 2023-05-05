#include "client.h"
#include "general.h"
#include "sdk.h"
#include "Sync.h"
#include "EndScene.h"

bool customRocketEndPos = true;

bool syncing = false;
bool moveForward = false;
bool doubleSync = false;

int quinting = 1;


int hitWall = 0;
//extern ConVar *quinting;
constexpr int firedelayticks = (int)(firedelay / 0.015f) + 1;

//Vector rocketSource2(Vector* angles, Vector* endPosp, const Vector* Position);
constexpr float latestDistance = 1100.f * (firedelay + (3*0.015f)); // 3 ticks for rocket to hit the ground if you're on the ground...

void sync(CUserCmd* pCommand){
	static std::vector<float>endPosDists;
	static int latestTick;
	static trace_t myTrace;
	static bool waitingForGroundRocket = false;
	static int syncStartTick;
	static bool rocketShot = false;
	static int explodeTick;
	static int ticksAtGround = 3;
	static bool crouched;
	static int latestTick2;
	if (!syncing && GetAsyncKeyState(syncKey[0]) & 1) {
		gInts::cvar->ConsolePrintf("F7 key pressed\n");
		moveForward = true;

	}
		
		if (moveForward && GetGroundEntity(pPlayer) == NULL) {
			if (customRocketEndPos) {
				print("Using custom EndPos: ");
				Vector forward, start;
				Ray_t myRay;
				CTraceFilter filter;
				Weapon_ShootPosition(pPlayer, &start);
				AngleVectors(*EyeAngles(pPlayer), &forward, NULL, NULL);
				myRay.Init(start, start + (forward * 9999));
				gInts::EngineTrace->TraceRay(myRay, 0x46004003, &filter, &myTrace);
				desiredRocketEndPos = myTrace.endpos;
				GetEndPos(false, false, false);
			}
			else
				GetEndPos(true, false, true);
			crouched = pCommand->buttons & 4;
			hitWall = 0;
			syncStartTick = gpGlobals->tickcount;
			
			moveForward = false;
			Vector angles, endPosLocal;
			Vector myRocketSource = rocketSource2(&angles, &endPosLocal, &predictedLandingPos);
			
			Vector explodePos = predictedLandingPos;
			explodePos.z += 1.f;
			
			
			for ( auto position = Positions.begin(); position != Positions.end();  position++) {
				Vector angles;
				Vector vecSrc = rocketSource2(&angles, NULL, &(*position));
				endPosDists.push_back(dist(vecSrc, desiredRocketEndPos));
			}
			
			syncing = true;
			latestTick = 0;
			
			while ((latestTick < Positions.size()) && endPosDists[latestTick] > latestDistance)
				++latestTick;

		}
		if (syncing) {
			/*if (crouched)
				pCommand->buttons |= 4;
			else*/
				pCommand->buttons &= ~4;
			static int moveCounter = -1;
			static std::vector<float>fmoves;
			//pCommand->forwardmove = -450.f;
			if (!waitingForGroundRocket /* && (gpGlobals->tickcount <= (syncStartTick + Positions.size())*/) {

					print("Tick %d\nPredicted Time to Land == %f\nActual Time to Land == %f\n", gpGlobals->tickcount - syncStartTick, timeToLandFn(zVels[gpGlobals->tickcount - syncStartTick], Positions[gpGlobals->tickcount - syncStartTick].z, myTrace.endpos.z), timeToLandFn(g_pMoveData->m_vecVelocity.z, pPlayer->GetAbsOrigin().z, myTrace.endpos.z));
					constexpr float thresholdDiff = 1100.f * 0.015f;// being in this range doesn't necessarily mean the rocket will explode at the same tick.
					static float futureRocketDist;

					if (!doubleSync) {
						static int tickAtClosestDiff;
						static float rocketShotDist;
						static int rocketShotTick;
						static int firstBundleTicks;
						static bool quint2ndBundle;
						static int shot1 = -1, shot2 = -1;
						
						static std::vector<int>endPosDists2;
						static bool initialized = false;
						static int ticksToNextShot = firedelayticks;
						Vector myRocketSource = rocketSource2(&(pCommand->viewangles), NULL, NULL);
						//gInts::Engine->SetViewAngles(pCommand->viewangles);
						pCommand->forwardmove = 0;
						pCommand->sidemove = 0;
						
						if (moveCounter >= 0 && moveCounter < fmoves.size())
							pCommand->forwardmove = fmoves[moveCounter++];
						if (rocketShot) { 
							if ( gpGlobals->tickcount == tickAtClosestDiff + syncStartTick ||  gpGlobals->tickcount == shot1)/* with or without the -1 the last rocket is still always one tick too late*/ { // should it be equals within a range?
								
								
								pCommand->buttons |= 1;//IN_ATTACK;
								shotTick = gpGlobals->tickcount;
								print("endPosDists[tickAtClosestDiff] == %f\n", endPosDists[tickAtClosestDiff]);
								print("futureRocketDist: %f\nfutureRocketDist + (1100*0.015f): %f\n", futureRocketDist, futureRocketDist + (1100 * 0.015f));
							
								Vector angles;
							
								if(!quinting)
									waitingForGroundRocket = true;
							
							}
							else if (quint2ndBundle && gpGlobals->tickcount > tickAtClosestDiff + syncStartTick) {
								--firstBundleTicks;
								if(ticksToNextShot > 0)
									--ticksToNextShot;
								if (firstBundleTicks <= 0) {
									syncing = false;
									rocketShot = false;
									quint2ndBundle = false;
									initialized = false;
									ticksToNextShot = firedelayticks;
									endPosDists2.clear();
								}
							}
							 if (gpGlobals->tickcount == shot2) {
								pCommand->buttons |= 1;
								waitingForGroundRocket = true;
							}
						}
						
						
						if (quint2ndBundle && myGameMovement->mv->m_vecVelocity.z == 0) {
							
						
							if (initialized == false) {
								Vector rocketOrigin;
								float fMove;
								float distance;
								v_customEndPos = desiredRocketEndPos;
								float angle = atan2(v_customEndPos.y - g_pMoveData->m_vecAbsOrigin.y, v_customEndPos.x - g_pMoveData->m_vecAbsOrigin.x) * 180 / M_PI;
								while (angle > 180.f)
									angle -= 360.f;
								while (angle < -180.f)
									angle += 360.f;
								
								pCommand->viewangles[YAW] = angle;
								
								
								gInts::Engine->SetViewAngles(pCommand->viewangles);
								DWORD address = (DWORD)(pPlayer)+offsetForNoNull;
								DWORD value = (DWORD)&noNullPtr;
								writeMemoryWithProtection<DWORD>(address, value);
								writeMemoryWithProtection<float>((DWORD)gpGlobals + 16, 0.015f);
								
								writeMemoryWithProtection<float>((DWORD) & (g_pMoveData->m_vecViewAngles[YAW]), angle);
								writeMemoryWithProtection<float>((DWORD) & (g_pMoveData->m_flForwardMove), 450.f);
								
								endPosDists2.push_back(dist(rocketSource2(NULL, NULL, &myGameMovement->mv->m_vecAbsOrigin), desiredRocketEndPos)); 
								// Don't forget to add the first endpos... ?
								
								do {
									
									myGameMovement->mv->m_vecViewAngles[YAW] = angle;
									myGameMovement->mv->m_flForwardMove = 450.f;
									ProcessMovement(myGameMovement, pPlayer, myGameMovement->mv);
									
									if (hitWall == 1) {
										syncing = false;
										rocketShot = false;
										quint2ndBundle = false;
										initialized = false;
										ticksToNextShot = firedelayticks;
										endPosDists2.clear();
										hitWall = 0;
										return;
										
									} // this is not correct to check for wall collision, one of them could be zero and there could be no collision
									Vector rocketOrigin = rocketSource2(NULL, NULL, &myGameMovement->mv->m_vecAbsOrigin);
									endPosDists2.push_back(dist(rocketOrigin, desiredRocketEndPos) / (1100.f * 0.015f));
									fmoves.push_back(450.f);
									
								} while (GetGroundEntity(pPlayer));

								while (getNextFmove(pCommand->viewangles, fMove, &endPosDists2))
									fmoves.push_back(fMove);
								fmoves.push_back(fMove);
								myGameMovement->mv->m_flForwardMove = 0;
								writeMemoryWithProtection<float>((DWORD) & (g_pMoveData->m_flForwardMove), 0);
								while (myGameMovement->mv->m_vecVelocity.z != 0) {
									
									ProcessMovement(myGameMovement, pPlayer, myGameMovement->mv);
									Vector rocketOrigin = rocketSource2(NULL, NULL, &myGameMovement->mv->m_vecAbsOrigin);
									endPosDists2.push_back(dist(rocketOrigin, desiredRocketEndPos) / (1100.f * 0.015f));
									fmoves.push_back(0);
								}
								initialized = true;
							}
							for (int i = ticksToNextShot, k = firstBundleTicks- ticksToNextShot; i < endPosDists2.size() /* && k >= firedelayticks * 2*/; ++i, --k) {
								if (endPosDists2[i] == k-1) {
									for (int j = i+ firedelayticks, tripleBundleTicks = k-firedelayticks; j < endPosDists2.size() /* && tripleBundleTicks >= firedelayticks*/; ++j, --tripleBundleTicks) // rest of the ticks
										if (endPosDists2[j] == tripleBundleTicks-1) {
											shot1 = i + gpGlobals->tickcount;// -1 cuz the rocket comes out of the position from the next tick???
											shot2 = j + gpGlobals->tickcount;
											moveCounter = 0;
											quint2ndBundle = false;
											initialized = false;
											ticksToNextShot = firedelayticks;
											endPosDists2.clear();
											return;
										}
								}
							}
							
							
						}
						

						else if (!rocketShot){
							
							gInts::Engine->SetViewAngles(pCommand->viewangles);
							float closestDifference = FLT_MAX;
							
								
								int t = (gpGlobals->tickcount - syncStartTick) +/*1*/(int)(firedelay / 0.015f) + 1;
								if (t > latestTick) {
									syncing = false;
									Positions.clear();
									endPosDists.clear();
									return;
								}

							for (; t < Positions.size()  && t < latestTick; ++t) {


								futureRocketDist = dist(myRocketSource, desiredRocketEndPos) - 1100.f * 0.015f * (t - (gpGlobals->tickcount - syncStartTick + 1));
								//print("futureRocketDist - endPosDists[t] == %f\n", rocketDist - endPosDists[t]);
								firstBundleTicks = ((int)(futureRocketDist / (1100.f * 0.015f))) + 1;
								// if you shoot at tick t it will explode in endPosDists[t] / (1100.f * 0.015f))+1 plus the one tick it take for actually shooting the rocket...
								//if(t - gpGlobals->tickcount < endPosDists.size())
								if (firstBundleTicks == ((int)(endPosDists[t] / (1100.f * 0.015f)))+2 /* + 1 commenting this off cuz the rocket seems to consistently be one tick off...*/) {
								
									tickAtClosestDiff = t;
									pCommand->buttons |= 1;//IN_ATTACK;
									shotTick = gpGlobals->tickcount;

									if (quinting) 
										quint2ndBundle = true;
									rocketShot = true;
									// ticksToExplode is off by one anyways...
									explodeTick = 1+t + firstBundleTicks + syncStartTick; // explode tick or disappearance tick? Is there a difference? + 1 tick to shoot the rocket??
									break;

								}
								
							}
						}
					}
					else
						if (!rocketShot) {
							Vector myRocketSource = rocketSource2(&(pCommand->viewangles), NULL, NULL);
							float timeToLand = ((-(g_pMoveData->m_vecVelocity.z) - sqrt(g_pMoveData->m_vecVelocity.z * g_pMoveData->m_vecVelocity.z - (2 * gravity * (/*playerPos.z*/pPlayer->GetAbsOrigin().z - predictedLandingPos.z)))) / gravity);
							float timeToExplode = (dist(myRocketSource, predictedLandingPos) / 1100.f);

							if (/*(dist(playerPos, myTrace.endpos) > dist(Positions[latestTick], myTrace.endpos)) &&*/ timeToExplode > timeToLand  /*+ delay1 && timeToLand>fireRate*/) {// if the rocket will land later than you will


								// you would land a bit in front of this point because of friction
								// pCommand->viewangles = correctAngles;
								//ExplodeTime = gpGlobals->curtime + timeToExplode;
								pCommand->buttons |= 1;//IN_ATTACK;
								//pCommand->viewangles = correctAngles;
								gInts::Engine->SetViewAngles(pCommand->viewangles);
								rocketShot = true;
								gInts::cvar->ConsolePrintf("Rocket Shot\n");
								printVector(predictedLandingPos);
								waitingForGroundRocket = true;
								//syncing = false;
							}
						}

				else {
					print("Error, failed to sync")
					syncing = false;
					Positions.clear();
					zVels.clear();
					endPosDists.clear();

				}
			}
			else {
				if (rocketShot) {
					
					constexpr float groundDelay = 0.015;
					
					if (gpGlobals->tickcount == explodeTick-ticksAtGround/* - 1 for 1 tick to shoot ? And one tick to explode ? ? ? */ /* - groundDelay*/) { // assume it takes two ticks for a rocket to explode after shooting at the ground...
						shotTick = gpGlobals->tickcount;
						pCommand->viewangles.x = 89.f;
						moveCounter = -1;
						fmoves.clear();
						pCommand->buttons = (pCommand->buttons | 1 | 2 | 4); // 1 to shoot, 2 to jump and 4 to crouch (0111 = 7)
						rocketShot = false;
						waitingForGroundRocket = false;
						// Why copy all of this????
						syncing = false;
						Positions.clear();
						zVels.clear();
						endPosDists.clear();
						
					}
				}
				else {
					print("Error, failed to sync\n");
					syncing = false;
					Positions.clear();
					zVels.clear();
					endPosDists.clear();
					

				}
			}
			
		}
}
