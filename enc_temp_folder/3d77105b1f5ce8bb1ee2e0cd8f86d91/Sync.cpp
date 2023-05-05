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
			//ticksAtGround = 2;//(int)(dist(myRocketSource, endPosLocal) / (1100.f * 0.015f)) + 1;
			//print("ticks for rocket to explode when shooting at end position: %d.\n", ticksAtGround);
			//	Ray_t myRay;
			//	CTraceFilter filter;
			//	Vector start = pPlayer->GetAbsOrigin();

			//	Vector m_vecVelocity = g_pMoveData->m_vecVelocity;
			//	Vector end = start;
			//	
			//	Positions2.push_back(end);
			//	DWORD address = (DWORD)(pPlayer)+0x107C;
			//	//DWORD value = (DWORD)(pPlayer + (0x122C / 4));
			//	DWORD value = (DWORD)&noNullPtr;
			//	writeMemoryWithProtection<DWORD>(address, value);
			//	gpGlobals->frametime = 0.015;
			//	float lastzVel;
			//	int iterations = 0;
			//	do {
			//		lastzVel = myGameMovement->mv->m_vecVelocity.z;
			//		velPrediction.push_back(myGameMovement->mv->m_vecVelocity);
			//		Positions2.push_back(myGameMovement->mv->m_vecAbsOrigin);
			//		try {

			//			//myGameMovement->mv->m_flForwardMove = -450.f;

			//			//Positions2.push_back(myGameMovement->mv->m_vecAbsOrigin);
			//			ProcessMovement(myGameMovement, pPlayer, myGameMovement->mv);
			//		}
			//		catch (...) {
			//			print("error");
			//		}
			//	}
			//		 while ((myGameMovement->mv->m_vecVelocity.LengthSqr()) != 0 && iterations++ < 500);


			//start:
			//	Positions.push_back(end);
			//	zVels.push_back(m_vecVelocity.z);
			//	//FullWalkMove(myGameMovement);
			//	
			//	//``AirMove(m_vecVelocity, fmove)...
			//	end += m_vecVelocity * 0.015f;//gpGlobals->frametime;
			//	//myRay.Init(start, end, GetPlayerMins(), GetPlayerMaxs());
			//	myRay.Init(start, end);
			//	gInts::EngineTrace->TraceRay(myRay, 0x46004003, &filter, &myTrace);
			//	if (myTrace.fraction == 1) {
			//		++numOfPositions;
			//		start = end;  m_vecVelocity.z -= 12.f;
			//		goto start;
			//	}

			//	/*myGameMovement->mv->m_flForwardMove = 0;
			//	myGameMovement->mv->m_flSideMove = 0;*/
			//	
			//	++numOfPositions;
			//	
			//	Positions.push_back(myTrace.endpos);
			//	
			//	zVels.push_back(m_vecVelocity.z);
			
			
			

			//else
			//{
			//	//print("Predicted EndPos: ");

			//}
			// THE ACTUAL EXPLOSION IS AT THE TRACE ENDPOS WITH 1.0 ADDED TO THE Z POS!!!!!!
			// DOUBLE CHECK AT the_real_explode_0 + 0xE3
			//myTrace.endpos.z += 1.0;
			//printVector(predictedLandingPos);
			Vector explodePos = predictedLandingPos;
			explodePos.z += 1.f;
			//print("Predicted explode pos: ");
			//printVector(explodePos);
			
			for ( auto position = Positions.begin(); position != Positions.end();  position++) {
				Vector angles;
				Vector vecSrc = rocketSource2(&angles, NULL, &(*position));
				endPosDists.push_back(dist(vecSrc, desiredRocketEndPos));
			}
			//myTrace.endpos;//g_pMoveData->m_vecVelocity;
			syncing = true;
			latestTick = 0;
			
			while ((latestTick < Positions.size()) && endPosDists[latestTick] > latestDistance)
				++latestTick;

			//latestTick -= 2;it doesn't work as well without this. This could account for the trace endpos being different from the actually explosion pos
			//unlikely, more likely the movement prediction is off
			//while ((latestTick <= Positions.size()) && timeToLandFn() > firedelay)
			//	++latestTick;
			//waitingForGroundRocket = false;
			//print("%d\n", latestTick);

			//checkForLandTick = true;
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
				//if (gpGlobals->tickcount - syncStartTick <= latestTick) {
					/* This will cause you to to go to the else clause too early->&& gpGlobals->tickcount <= (syncStartTick + latestTick))*//*!closeToEndPos(pPlayer->GetAbsOrigin(), myTrace.endpos, 5.f)*/ // what if you're syncing on a wall, you would still be in the air...
					/*gInts::cvar->ConsolePrintf("Predicted z vel%f\nActual z vel%f\nActual Position: ", zVels[positionCounter], g_pMoveData->m_vecVelocity.z);
					printVector(pPlayer->GetAbsOrigin());*/


					/*print("Ticks so far: %d\nPredicted Position: ", gpGlobals->tickcount - syncStartTick);
					if(positionCounter<Positions2.size())
					printVector(Positions2[positionCounter++]);
					print("Actual Position: ");
					printVector(pPlayer->GetAbsOrigin());
					print("predicted velocity");
					if(positionCounter<velPrediction.size())
					printVector(velPrediction[positionCounter++]);
					print("Actual Velocity??");
					printVector(g_pMoveData->m_vecVelocity);

					print("Tick %d\nPredicted Time to Land == %f\nActual Time to Land == %f\n", gpGlobals->tickcount - syncStartTick, timeToLandFn(zVels[gpGlobals->tickcount - syncStartTick], Positions[gpGlobals->tickcount - syncStartTick].z, myTrace.endpos.z), timeToLandFn(g_pMoveData->m_vecVelocity.z, pPlayer->GetAbsOrigin().z, myTrace.endpos.z));*/
					constexpr float thresholdDiff = 1100.f * 0.015f;// being in this range doesn't necessarily mean the rocket will explode at the same tick.
					//Vector playerPos = Positions[gpGlobals->tickcount - syncStartTick+1];//pPlayer->GetAbsOrigin();
					
					
					//print("Tick %d: %f\n", gpGlobals->tickcount - syncStartTick, dist(myRocketSource, predictedLandingPos));
					static float futureRocketDist;
					
					//static Vector futureRocketPos;
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
						if (rocketShot) { // Threshold ticks might be pointless. Not sure if it's possible to angle Quad AND land exactly where your rockets
							// are heading... This may be useful for preFire syncs...
							/*if ((int)(rocketShotDist - (gpGlobals->tickcount - rocketShotTick) * 1100.f * 0.015f) == (int)dist(myRocketSource, predictedLandingPos)) {
								pCommand->buttons |= 1;
								waitingForGroundRocket = true;
							}*/
							if ( gpGlobals->tickcount == tickAtClosestDiff + syncStartTick ||  gpGlobals->tickcount == shot1)/* with or without the -1 the last rocket is still always one tick too late*/ { // should it be equals within a range?
								//tickAtClosestDiff = 0;
								//rocketSource2(&(pCommand->viewangles), NULL, NULL);
								
								pCommand->buttons |= 1;//IN_ATTACK;
								shotTick = gpGlobals->tickcount;
								print("endPosDists[tickAtClosestDiff] == %f\n", endPosDists[tickAtClosestDiff]);
								print("futureRocketDist: %f\nfutureRocketDist + (1100*0.015f): %f\n", futureRocketDist, futureRocketDist + (1100 * 0.015f));
								//print("dist(myRocketSource, predictedLandingPos) %f\n", dist(myRocketSource, predictedLandingPos));
								//print("predicted rocket origin: ");
								Vector angles;
								//printVector(myRocketSource);
								
								/*print("predicted player pos: ");
								printVector(playerPos);*/
								//rocketSource2(&(pCommand->viewangles), NULL, &Positions[gpGlobals->tickcount - syncStartTick]);
								//tickAtClosestDiff = 0;
								if(!quinting)
									waitingForGroundRocket = true;
								//thresholdTicks.erase(thresholdTicks.begin());
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
							
							//static int landTick = gpGlobals->tickcount;
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
								
								
								/*
								* myGameMovement->mv->m_vecViewAngles[YAW] = angle;
								myGameMovement->mv->m_flForwardMove = 450.f;
								 writeMemoryWithProtection<float>((DWORD) & (g_pMoveData->m_flForwardMove), 450.f);
								ProcessMovement(myGameMovement, pPlayer, myGameMovement->mv);
								myGameMovement->mv->m_vecAbsOrigin = pPlayer->GetAbsOrigin();
								
								if (GetGroundEntity(pPlayer)) {
									--firstBundleTicks;
									pCommand->forwardmove = 450.f;
									return;
								}*/
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
									//--firstBundleTicks;
								    //--ticksToNextShot;
									// You need to check for collisions with any wall, it is possible to get stuck in an infinte loop here.
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
							//--firstBundleTicks;
							//--ticksToNextShot;
							/*if (firstBundleTicks <= 0) {
								quint2ndBundle = false;
								initialized = false;
								ticksToNextShot = firedelayticks;
							}*/
							// if quint2ndBundle is false or playMoves is true... don't move....
							
						}
						

						else if (!rocketShot){
							
							gInts::Engine->SetViewAngles(pCommand->viewangles);
							float closestDifference = FLT_MAX;
							//float timeToLand = ((-(g_pMoveData->m_vecVelocity.z) - sqrt(g_pMoveData->m_vecVelocity.z * g_pMoveData->m_vecVelocity.z - (2 * gravity * (playerPos.z - myTrace.endpos.z)))) / gravity);
							//float timeToExplode = (dist(myRocketSource, myTrace.endpos) / 1100.f);
							//if (timeToExplode < timeToLand) {
								/*playerPos.x += g_pMoveData->m_vecVelocity.x * firedelay;
								playerPos.y += g_pMoveData->m_vecVelocity.y * firedelay;
								playerPos.z += 0.5f * 800.f * firedelay * firedelay + g_pMoveData->m_vecVelocity.z * firedelay;*/
								/*Vector rocketAngle;
								VectorAngles(myRocketSource - myTrace.endpos, rocketAngle);
								ClampAngle(rocketAngle);
								float zvel = sin(rocketAngle[PITCH]) * 1100.f;
								float horizontalVel = cos(rocketAngle[PITCH]) * 1100.f;
								float yvel = cos(rocketAngle[YAW]) * horizontalVel;
								float xvel = sin(rocketAngle[YAW]) * horizontalVel;
								rocketVel = Vector(xvel, yvel, zvel);
								Vector futureRocketPos = myRocketSource + rocketVel * firedelay;*/
								//Vector rocketVel = desiredRocketEndPos /*myTrace.endpos*/ - myRocketSource;
								//VectorNormalize(rocketVel);
								//rocketVel *= 1100.f;

								// futureRocketPos = myRocketSource + rocketVel * t
								// playerPos.x += g_pMoveData->m_vecVelocity.x * t;
								// playerPos.y += g_pMoveData->m_vecVelocity.y * t;
								// playerPos.z += 0.5f * 800.f * t^2 + g_pMoveData->m_vecVelocity.z * t;
								// why don't you check the distances where you can land slower than the firerate
								//


								//float lastDifference = FLT_MAX;



								//gInts::cvar->ConsolePrintf("Actual dist%f\nPredicted dist%f\n", dist(rocketSource(), myTrace.endpos), endPosDists[(gpGlobals->tickcount - syncStartTick) + (firedelay / 0.015f)])
								//rocketShotDist = dist(myRocketSource, predictedLandingPos);

								/*if (rocketShotDist - (latestTick2 - gpGlobals->tickcount) * 1100.f * 0.015f > latestDistance) {
									pCommand->buttons |= 1;
									gInts::Engine->SetViewAngles(pCommand->viewangles);
									rocketShot = true;
									explodeTick = (int)(rocketShotDist / (1100.f * 0.015f));
									rocketShotTick = gpGlobals->tickcount+1;
								}*/
								int t = (gpGlobals->tickcount - syncStartTick) +/*1*/(int)(firedelay / 0.015f) + 1;
								if (t > latestTick) {
									syncing = false;
									Positions.clear();
									endPosDists.clear();
									return;
								}

							for (; t < Positions.size()  && t < latestTick; ++t) {
								//for (int t = gpGlobals->tickcount + (int)(firedelay / 0.015f) + 1; t < latestTick2; ++t){
									/*playerPos.x += g_pMoveData->m_vecVelocity.x * gpGlobals->interval_per_tick * t;
									playerPos.y += g_pMoveData->m_vecVelocity.y * gpGlobals->interval_per_tick * t;
									playerPos.z += 0.5f * 800.f * 0.015f * t* t + g_pMoveData->m_vecVelocity.z*0.015f * t;*/
								//Vector futureRocketPos = myRocketSource + rocketVel * 0.015f * (t - (gpGlobals->tickcount - syncStartTick+1));
								//futureRocketDist = dist(myRocketSource, predictedLandingPos) - (1100.f * 0.015f * (t - (gpGlobals->tickcount - syncStartTick)-1));
								//futureRocketDist = dist(myRocketSource, predictedLandingPos) - (1100.f * 0.015f * (t - gpGlobals->tickcount + (int)(firedelay / 0.015f) + 1));
								// in one ticks your rocket will only be at it's starting pos. It doesn't come out the tick you shoot...
								// 
								//if (futureRocketPos.z < myTrace.endpos.z)// if it goes past the endPos... how would this work for horizontal syncs?
								//	break;
								//futureRocketDist = dist(futureRocketPos, desiredRocketEndPos);

								futureRocketDist = dist(myRocketSource, desiredRocketEndPos) - 1100.f * 0.015f * (t - (gpGlobals->tickcount - syncStartTick + 1));
								//print("futureRocketDist - endPosDists[t] == %f\n", rocketDist - endPosDists[t]);
								firstBundleTicks = ((int)(futureRocketDist / (1100.f * 0.015f))) + 1;
								// if you shoot at tick t it will explode in endPosDists[t] / (1100.f * 0.015f))+1 plus the one tick it take for actually shooting the rocket...
								//if(t - gpGlobals->tickcount < endPosDists.size())
								if (firstBundleTicks == ((int)(endPosDists[t] / (1100.f * 0.015f)))+2 /* + 1 commenting this off cuz the rocket seems to consistently be one tick off...*/) {
									// if you're quinting, check that ticks to explode for rocket to shoot == ticks to explode for previous bundle...
									//print("ticksToExplode %d, (int)(endPosDists[t] / (1100.f * 0.015f))+1 %d\n", ticksToExplode, (int)(endPosDists[t] / (1100.f * 0.015f)) + 1);
									tickAtClosestDiff = t;
									pCommand->buttons |= 1;//IN_ATTACK;
									shotTick = gpGlobals->tickcount;
									/*print("predicted rocket origin: ");
									printVector(myRocketSource);
									print("predicted player pos: ");
									printVector(Positions[gpGlobals->tickcount - syncStartTick]);*/
									//pCommand->viewangles = correctAngles;
									//gInts::Engine->SetViewAngles(pCommand->viewangles);
									if (quinting) 
										quint2ndBundle = true;
									rocketShot = true;
									// ticksToExplode is off by one anyways...
									explodeTick = 1+t + firstBundleTicks + syncStartTick; // explode tick or disappearance tick? Is there a difference? + 1 tick to shoot the rocket??
									break;

								}
								//float difference = abs(rocketDist - endPosDists[t]);
								// the difference between the rocket start pos and the future rocketPos should keep getting smaller until they are equal...and then get larger again
								// except that initially the distance will get larger before it gets smaller...
								/*if (difference > lastDifference) {
									break;
								}
								else
									lastDifference = difference;*/
									/*if (difference < thresholdDiff) {

										thresholdTicks.push_back(t);
									}*/
									/*if (difference < closestDifference) {
										closestDifference = difference;
										tickAtClosestDiff = t;
									}*/
							}
							//closestOfClosestDifferences = min(closestOfClosestDifferences, closestDifference);
							//int ticksToExplode = (int)(dist(myRocketSource, predictedLandingPos) / (1100.f * 0.015f));
							//if ((int)(closestDifference / (1100.f * 0.015f)) == ticksToExplode) {
							//	//print("Amount of thresholdTicks: %d\n", thresholdTicks.size());
							//	//doubleSync = false;
							//	//rocketShotTime = gpGlobals->curtime;
							//	//rocketShotLocation = myRocketSource;
							//	explodeTick =  gpGlobals->tickcount + ticksToExplode +1; // compare all the explode times for rockets shot
							//	//explodeTick = gpGlobals->tickcount +(int)(timeToExplode/0.015f) + 1 /*+1?*/;
							//	// at every tick???
							//	// this would be shooting one tick too late
							//	pCommand->buttons |= 1;//IN_ATTACK;
							//	//pCommand->viewangles = correctAngles;
							//	gInts::Engine->SetViewAngles(pCommand->viewangles);
							//	rocketShot = true;
							//}

							//}
						}
					}
					else
						if (!rocketShot) {
							Vector myRocketSource = rocketSource2(&(pCommand->viewangles), NULL, NULL);
							float timeToLand = ((-(g_pMoveData->m_vecVelocity.z) - sqrt(g_pMoveData->m_vecVelocity.z * g_pMoveData->m_vecVelocity.z - (2 * gravity * (/*playerPos.z*/pPlayer->GetAbsOrigin().z - predictedLandingPos.z)))) / gravity);
							float timeToExplode = (dist(myRocketSource, predictedLandingPos) / 1100.f);

							// why use g_pMoveData's velocity instead of the player's velocity...
							// zpos - endPos.z or endPos.z - zpos which is correct??

							/*gInts::cvar->ConsolePrintf("\nPredicted Position\n");
							printVector(Positions[positionCounter]);

							gInts::cvar->ConsolePrintf("Actual Position\n");

							printVector(playerPos);
							if (positionCounter > 0) {
								print("Predicted Difference\n");
								printVector(Positions[positionCounter] - Positions[positionCounter - 1]);
								print("Actual Difference\n");
								printVector(playerPos - lastPos);
							}*/
							//lastPos = playerPos;
							//++positionCounter;
							//constexpr float delay1 = 0.015;
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


					/*else if (GetGroundEntity(pPlayer) != NULL) {
						gInts::cvar->ConsolePrintf("No Rockets Shot\n");
					}*/

				//}
				else {
					print("Error, failed to sync")
					syncing = false;
					Positions.clear();
					zVels.clear();
					endPosDists.clear();
					/*positionCounter = 0;
					numOfPositions = 0;
					thresholdTicks.clear();
					Positions2.clear();
					velPrediction.clear();
					print("%f\n", closestOfClosestDifferences);
					closestOfClosestDifferences = FLT_MAX;*/

				}
			}
			else {
				if (rocketShot) {
					//if(GetAbsVelocity().Lenth() == 0)// if you're stopped check how many ticks it would take to shoot a rocket where you want...
					/*if (positionCounter < Positions.size()) {
						gInts::cvar->ConsolePrintf("\nPredicted Position\n");
						printVector(Positions[positionCounter]);
						++positionCounter;
					}
					else
						gInts::cvar->ConsolePrintf("\nNo more Predicted Positions\n");
					gInts::cvar->ConsolePrintf("Actual Position\n");
					printVector(pPlayer->GetAbsOrigin());*/
					
					constexpr float groundDelay = 0.015;
					// IDK WHERE TO WRITE THIS but this method that uses ProcessMovement would actually predict the final landing pos probably more
					// accurately than the old method, however due to the fact that the old method over estimated in that regard in accounted for friction.
					// You could simply change GetEndPos() to stop whenver your movement speed in all directions is 0. Or you could continously cancel friction on the ground...
					// What if it isn't even possible to cancel out friction in one tick...???

					// Regardless of whether you subtract one or not, the ground rocket almost always still seems to hit the ground at the same time...
					if (gpGlobals->tickcount == explodeTick-ticksAtGround/* - 1 for 1 tick to shoot ? And one tick to explode ? ? ? */ /* - groundDelay*/) { // assume it takes two ticks for a rocket to explode after shooting at the ground...
						// OBSERVATION: changing the code can result in tick landing differences that are exactly the same yet still have dramatic differences
						// in the success of the sync itself.
						//rocketSource();
						//pCommand->viewangles = correctAngles;
						shotTick = gpGlobals->tickcount;
						pCommand->viewangles.x = 89.f;
						moveCounter = -1;
						fmoves.clear();
						// if the rocket doesn't explode instatnly, why not jump 3 ticks later?
						pCommand->buttons = (pCommand->buttons | 1 | 2 | 4); // 1 to shoot, 2 to jump and 4 to crouch (0111 = 7)
						// if the rocket comes out of the position you will be in next, tick then crouching and jumping are going to influence that position.
						// The calculation for ticksAtGround is likely wrong. 
						//gInts::cvar->ConsolePrintf("LastRocketShot\nEndPos: ");
						//printVector(pPlayer->GetAbsOrigin());
						rocketShot = false;
						waitingForGroundRocket = false;
						// Why copy all of this????
						syncing = false;
						Positions.clear();
						zVels.clear();
						endPosDists.clear();
						/*positionCounter = 0;
						numOfPositions = 0;
						thresholdTicks.clear();
						Positions2.clear();
						velPrediction.clear();
						print("%f\n", closestOfClosestDifferences);
						closestOfClosestDifferences = FLT_MAX;*/
						//doubleSync = true;
					}
				}
				else {
					print("Error, failed to sync\n");
					syncing = false;
					Positions.clear();
					zVels.clear();
					endPosDists.clear();
					/*positionCounter = 0;
					numOfPositions = 0;
					thresholdTicks.clear();
					Positions2.clear();
					velPrediction.clear();
					print("%f\n", closestOfClosestDifferences);
					closestOfClosestDifferences = FLT_MAX;*/

				}
			}
			
		}
		//if (quinting && waitingOnLedge) {
		//	std::vector<std::pair<float, Vector>>Fmoves;
		//	float nextFmove;
		//	while (getNextFmove(pCommand->viewangles, nextFmove))
		//		Fmoves.push_back(std::make_pair(nextFmove, pCommand->viewangles));
		//	float ticksToThirdRocketShot, ticksToFourthRocketShot;
		//	for (auto& Position : Positions)
		//		;
		//	if (ticksForOriginalRocketsToExplode == ticksForThirdRocketToExplode + ticksToThirdRocketShot)
		//		pCommand->buttons |= 1;
		//	else if (ticksForOriginalRocketsToExplode == ticksForFourthRocketToExplode + ticksToFourthRocketShot)
		//		pCommand->buttons |= 1;
		//	//dist(correctRocketPos, v_customEndPos) - ticksToFirstRocketShot * 0.015f * 1100.f == distOfFirstRocketShot;
		//	//dist(correctRocketPos + ticksToFirstRocketShot * 0.015f * rocketVel, v_customEndPos) == distOfFirstRocketShot;
		//	// find the right time to walk off the edge ( so that your previous rockets will be in sync with the rocket shot at firstRocketShotTick;
		//	// find out where or when you would normally triple even if you weren't quinting.
		//	// 
		//	// 
		//	//Find how much time it would take to walk off the edge. 

		//}
}
