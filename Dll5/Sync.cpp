#include "client.h"
#include "general.h"
#include "sdk.h"
#include "Sync.h"
#include "EndScene.h"

bool customRocketEndPos = true;

bool syncing = false;
bool moveForward = false;
bool doubleSync = false;


int hitWall = 0;

constexpr int firedelayticks = (int)(firedelay / 0.015f) + 1;

//Vector rocketSource2(Vector* angles, Vector* endPosp, const Vector* Position);
constexpr float latestDistance = 1100.f * (firedelay + (3*0.015f)); // 3 ticks for rocket to hit the ground if you're on the ground...

constexpr float latestDistanceTripleMaybe = 1100.f * (2 * firedelay + (3 * 0.015f));

enum syncType type;

static int syncStartTick;
static int explodeTick;
static int latestTick;
static int firstBundleTicks;
static bool rocketShot = false;
static bool quint2ndBundle;
static std::vector<int>explodeTicks;
static int tickAtClosestDiff;
static std::vector<int> shotTicks;
int tripleSync(CUserCmd* pCommand, int t, float distance);

void sync(CUserCmd* pCommand){
	static std::vector<float>endPosDists;
	
	
	static int latestTickTriple;
	static trace_t myTrace;
	static bool waitingForGroundRocket = false;
	//static bool autoDetect = true;
	
	
	static int ticksAtGround = 3;
	static bool crouched;
	static int latestTick2;
	static int ticksToLand;
	static int shot1 = -1;
	
	if (!syncing && GetAsyncKeyState(SyncKey) & 1) {
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
			//Vector myRocketSource = rocketSource2(&angles, &endPosLocal, &predictedLandingPos, false);
			//Vector vecSrc = rocketSource2(&angles, NULL, &Positions[0], false);
			//Vector vecSrc = rocketSource2(&angles, NULL, &Positions[0], false);

			// will auto-detect only work in the case that you move toward the rocket endpos???
			//if(dist(vecSrc, desiredRocketEndPos) > latestDistanceTripleMaybe && )


			Vector explodePos = predictedLandingPos;
			explodePos.z += 1.f;
			
			
			
			for ( auto position = Positions.begin(); position != Positions.end();  position++) {
				Vector angles;
				Vector vecSrc = rocketSource2(&angles, NULL, &(*position), false);
				endPosDists.push_back(dist(vecSrc, desiredRocketEndPos));
				explodeTicks.push_back((int)(dist(vecSrc, desiredRocketEndPos) / (1100.f * 0.015f)) + 1);
			}
			ticksToLand = Positions.size();
			syncing = true;
			latestTick = 0;
			latestTickTriple = 0;
			while ((latestTick < Positions.size()) && endPosDists[latestTick] > latestDistance) {
				++latestTick;
				if (endPosDists[latestTickTriple] > latestDistanceTripleMaybe)
					++latestTickTriple;
			}
			int minExplodeTick = INT_MAX;

			// Trying to check for quint would require that you have aim at crosshair disabled, which would require you to enable/disable something anyway.
			// Could check that you're aiming at a different plane and automatically differentiate between aim at crosshair and aim at endpos...
			//if (type == autoDetect) {
			//	int shot1 = -1, shot2;
			//	for (int s = 0; s < latestTickTriple; ++s) {
			//		int expTick = tripleSync(pCommand, s + (int)(firedelay / 0.015f) + 1, endPosDists[s]);
			//		if (expTick != -1 && expTick < minExplodeTick) {
			//			shot1 = s;
			//			shot2 = tickAtClosestDiff;
			//			minExplodeTick = expTick;
			//		}
			//	}
			//
			//	if (shot1 != -1) {
			//		syncing = false;
			//		shotTicks.push_back(shot1+syncStartTick);
			//		shotTicks.push_back(shot2+syncStartTick);
			//		shotTicks.push_back(minExplodeTick+syncStartTick);
			//		endPosDists.clear();
			//		explodeTicks.clear();
			//		Positions.clear();
			//	}
			//	else 
			//		type = doubling;
			//	
			//}
		}
		//if (type == autoDetect) {
		//	for (int i = 0; i < shotTicks.size(); ++i)
		//		if (shotTicks[i] == gpGlobals->tickcount) {
		//			if (shotTicks.size() == 1)
		//				pCommand->buttons = (pCommand->buttons | 2 | 4);
		//			pCommand->buttons |= 1;
		//			shotTicks.erase(shotTicks.begin() + i);
		//		}
		//}
		if (syncing) {
			/*if (crouched)
				pCommand->buttons |= 4;
			else*/
				pCommand->buttons &= ~4;
			static int moveCounter = -1;
			static std::vector<float>fmoves;
			static std::vector<int>endPosDists2;
			if (!waitingForGroundRocket /* && (gpGlobals->tickcount <= (syncStartTick + Positions.size())*/) {

					//print("Tick %d\nPredicted Time to Land == %f\nActual Time to Land == %f\n", gpGlobals->tickcount - syncStartTick, timeToLandFn(zVels[gpGlobals->tickcount - syncStartTick], Positions[gpGlobals->tickcount - syncStartTick].z, myTrace.endpos.z), timeToLandFn(g_pMoveData->m_vecVelocity.z, pPlayer->GetAbsOrigin().z, myTrace.endpos.z));
					constexpr float thresholdDiff = 1100.f * 0.015f;// being in this range doesn't necessarily mean the rocket will explode at the same tick.
					static float futureRocketDist;

					if (type != doubling) {
						static int tickAtPlatform;
						
						static float rocketShotDist;
						static int rocketShotTick;
						
						
						static int shot2 = -1;
						
						
						static bool initialized = false;
						static int ticksToNextShot = firedelayticks;
						

						Vector myRocketSource = rocketSource2(&(pCommand->viewangles), NULL, NULL, false);
						if (customRocketEndPos/* && !(pCommand->forwardmove == 0 && pCommand->sidemove == 0)*/ ) {
							int currentTick = gpGlobals->tickcount - syncStartTick;
							myGameMovement->mv->m_flForwardMove = pCommand->forwardmove;
							myGameMovement->mv->m_flSideMove = pCommand->sidemove;
							ProcessMovementWrapper();
							Vector rocketSource = rocketSource2(NULL, NULL, &(myGameMovement->mv->m_vecAbsOrigin), false);
							int otherTicks = dist(rocketSource, desiredRocketEndPos) / (1100.f * 0.015f) + 2;
							if (currentTick +1< explodeTicks.size() && (explodeTicks[currentTick+1 ] != otherTicks)) {
								// ensure that you really are in range in case the other option doesn't work.
								pCommand->forwardmove = 0;
								pCommand->sidemove = 0;
								// this doesn't ensure anything...
							}

						}
						else {
							pCommand->forwardmove = 0;
							pCommand->sidemove = 0;
						}
						
						if (moveCounter >= 0 && moveCounter < fmoves.size()) {
							//if (customRocketEndPos) {
							//	int currentTick = gpGlobals->tickcount - tickAtPlatform;
							//	myGameMovement->mv->m_flForwardMove = pCommand->forwardmove;
							//	myGameMovement->mv->m_flSideMove = pCommand->sidemove;
							//	ProcessMovementWrapper();
							//	Vector rocketSource = rocketSource2(NULL, NULL, &(myGameMovement->mv->m_vecAbsOrigin));
							//	int otherTicks =  dist(rocketSource, desiredRocketEndPos) / (1100.f * 0.015f) + 2;
							//	if (currentTick +1< endPosDists2.size() && (endPosDists2[currentTick+1] != otherTicks)) {
							//		// ensure that you really are in range in case the other option doesn't work.
							//		pCommand->forwardmove = fmoves[moveCounter];
							//		pCommand->sidemove = 0;
							//		// this doesn't ensure anything...
							//	}
							//	++moveCounter;
							//}

							//else {
								pCommand->forwardmove = fmoves[moveCounter++];
								pCommand->sidemove = 0;
							//}
						}
						if (rocketShot) { 
							if ( gpGlobals->tickcount == tickAtClosestDiff + syncStartTick ||  gpGlobals->tickcount == shot1)/* with or without the -1 the last rocket is still always one tick too late*/ { // should it be equals within a range?
								pCommand->buttons |= 1;//IN_ATTACK;
								if(type != quint)
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
									endPosDists.clear();
									explodeTicks.clear();
								}
							}
							 if (gpGlobals->tickcount == shot2) {
								pCommand->buttons |= 1;
								waitingForGroundRocket = true;
							}
						}
						
						
						if (quint2ndBundle && myGameMovement->mv->m_vecVelocity.z == 0) {
							tickAtPlatform = gpGlobals->tickcount;
						
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
								
								endPosDists2.push_back(dist(rocketSource2(NULL, NULL, &myGameMovement->mv->m_vecAbsOrigin, false), desiredRocketEndPos)/(1100.f*0.015f)+2); 
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
										explodeTicks.clear();
										
										hitWall = 0;
										return;
										
									} // this is not correct to check for wall collision, one of them could be zero and there could be no collision
									Vector rocketOrigin = rocketSource2(NULL, NULL, &myGameMovement->mv->m_vecAbsOrigin, false);
									endPosDists2.push_back(dist(rocketOrigin, desiredRocketEndPos) / (1100.f * 0.015f)+2);
									fmoves.push_back(450.f);
									
								} while (GetGroundEntity(pPlayer));

								while (getNextFmove(pCommand->viewangles, fMove, &endPosDists2))
									fmoves.push_back(fMove);
								fmoves.push_back(fMove);
								myGameMovement->mv->m_flForwardMove = 0;
								writeMemoryWithProtection<float>((DWORD) & (g_pMoveData->m_flForwardMove), 0);
								while (myGameMovement->mv->m_vecVelocity.z != 0) {
									
									ProcessMovement(myGameMovement, pPlayer, myGameMovement->mv);
									Vector rocketOrigin = rocketSource2(NULL, NULL, &myGameMovement->mv->m_vecAbsOrigin, false);
									endPosDists2.push_back(dist(rocketOrigin, desiredRocketEndPos) / (1100.f * 0.015f)+2);
									fmoves.push_back(0);
								}
								initialized = true;
							 }
							for (int i = ticksToNextShot, k = firstBundleTicks- ticksToNextShot; i < endPosDists2.size() /* && k >= firedelayticks * 2*/; ++i, --k) {
								if (endPosDists2[i] == k) {
									for (int j = i+ firedelayticks, tripleBundleTicks = k-firedelayticks; j < endPosDists2.size() /* && tripleBundleTicks >= firedelayticks*/; ++j, --tripleBundleTicks) // rest of the ticks
										if (endPosDists2[j] == tripleBundleTicks) {
											shot1 = i + gpGlobals->tickcount;// -1 cuz the rocket comes out of the position from the next tick???
											shot2 = j + gpGlobals->tickcount;
											moveCounter = 0;
											quint2ndBundle = false;
											initialized = false;
											ticksToNextShot = firedelayticks;
											explodeTicks.clear();
											return;
										}
								}
							}
							
							
						}
						

						else if (!rocketShot){
							
							gInts::Engine->SetViewAngles(pCommand->viewangles);
								int t = (gpGlobals->tickcount - syncStartTick) +(int)(firedelay / 0.015f) + 1;
								if (t > latestTick) {
									/*if (autoDetect) {

									}*/
									syncing = false;
									Positions.clear();
									endPosDists.clear();
									explodeTicks.clear();
									return;
								}
								
							
							for (; t < Positions.size() && t < latestTick; ++t) {
							

								futureRocketDist = dist(myRocketSource, desiredRocketEndPos) - 1100.f * 0.015f * (t - (gpGlobals->tickcount - syncStartTick + 1));
								//print("futureRocketDist - endPosDists[t] == %f\n", rocketDist - endPosDists[t]);
								firstBundleTicks = ((int)(futureRocketDist / (1100.f * 0.015f))+1) /* + 2*/;
								
								/*if (autoDetect) {
									explodeTick = 1 + t + firstBundleTicks + syncStartTick;
									if (explodeTick < minExplodeTick && explodeTick < nextShot && explodeTick < landingTick) {
										minExplodeTick = explodeTick;
										shootAt = gpGlobals->tickcount + t;
									}

								}*/
								// if you shoot at tick t it will explode in endPosDists[t] / (1100.f * 0.015f))+1 plus the one tick it take for actually shooting the rocket...
								//if(t - gpGlobals->tickcount < endPosDists.size())
								if (firstBundleTicks == explodeTicks[t])//((int)(endPosDists[t] / (1100.f * 0.015f)))+2 /* + 1 commenting this off cuz the rocket seems to consistently be one tick off...*/)
								{
								
									tickAtClosestDiff = t/* - 1*/;
									pCommand->buttons |= 1;//IN_ATTACK;
									shotTick = gpGlobals->tickcount;

									if (type == quint) 
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
							//static int i = 0;
							//Vector myRocketSource = rocketSource2(&(pCommand->viewangles), NULL, endPosDists[i++]);
							
							//float timeToLand = ((-(g_pMoveData->m_vecVelocity.z) - sqrt(g_pMoveData->m_vecVelocity.z * g_pMoveData->m_vecVelocity.z - (2 * gravity * (/*playerPos.z*/pPlayer->GetAbsOrigin().z - predictedLandingPos.z)))) / gravity);
							//int ticksToExplode = (dist(endPosDists[i++], desiredRocketEndPos) / (1100.f * 0.015f)) + 1;
							
							if (ticksToLand < firedelayticks){
								syncing = false;
								//i = 0;
								Positions.clear();
							}
							int currentTick = gpGlobals->tickcount - syncStartTick;
							if (currentTick < explodeTicks.size()) {
								int ticksToExplode = explodeTicks[currentTick];// (endPosDists[i] / (1100.f * 0.015f)) + 1;
								if (ticksToLand-- < ticksToExplode) {// if the rocket will land later than you will

									pCommand->buttons |= 1;//IN_ATTACK;
									//pCommand->viewangles = correctAngles;
									gInts::Engine->SetViewAngles(pCommand->viewangles);
									rocketShot = true;
									gInts::cvar->ConsolePrintf("Rocket Shot\n");
									printVector(predictedLandingPos);
									explodeTick = gpGlobals->tickcount + ticksToExplode;
									endPosDists.clear();
									explodeTicks.clear();
									//i = 0;
									waitingForGroundRocket = true;
								}
							}
							else {
								Positions.clear();
								syncing = false;
								explodeTicks.clear();
							}
							//++i;
						}

				else {
					print("Error, failed to sync")
					syncing = false;
					Positions.clear();
					//zVels.clear();
					endPosDists.clear();
					explodeTicks.clear();

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
						endPosDists.clear();
						endPosDists2.clear();
						explodeTicks.clear();
						
					}
				}
				else {
					print("Error, failed to sync\n");
					syncing = false;
					Positions.clear();
					endPosDists.clear();
					endPosDists2.clear();
					explodeTicks.clear();
					

				}
			}
			
		}
}


// return explodeTick if succesful else return -1
int tripleSync(CUserCmd *pCommand, int t, float distance) {

	for (; t < Positions.size() && t < latestTick; ++t) {
		int futureRocketDist = distance - 1100.f * 0.015f * (t - (gpGlobals->tickcount - syncStartTick + 1));
		firstBundleTicks = ((int)(futureRocketDist / (1100.f * 0.015f))) + 1;
		if (firstBundleTicks == explodeTicks[t])//((int)(endPosDists[t] / (1100.f * 0.015f)))+2 /* + 1 commenting this off cuz the rocket seems to consistently be one tick off...*/)
		{

			tickAtClosestDiff = t;
			//shotTicks.push_back(t);
			//pCommand->buttons |= 1;//IN_ATTACK;
			shotTick = gpGlobals->tickcount;

			if (type == quint)
				quint2ndBundle = true;
			rocketShot = true;
			// ticksToExplode is off by one anyways...
			explodeTick = 1 + t + firstBundleTicks;// explode tick or disappearance tick? Is there a difference? + 1 tick to shoot the rocket??
			
			return explodeTick - 3;
			

		}

	}
	return -1;
}
