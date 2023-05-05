//void SpeedShot() {
//	if (!speedShoting && GetAsyncKeyState(SpeedShotKey[0]) & 1)
//	{
//		moveForward2 = true;
//		checkForLandTick = true;
//		pCommand->buttons |= 4;
//		speedShotStartTick = gpGlobals->tickcount + 1;
//	}
//	if (speedShotStartTick == gpGlobals->tickcount && GetGroundEntity(pPlayer) == NULL) {
//		GetEndPos(false, true);
//		speedShoting = true;
//		moveForward2 = false;
//		ticksToLand = Positions.size();
//	}
//	if (speedShoting) {
//
//		pCommand->forwardmove = 0; // Cancel all movement...
//		pCommand->sidemove = 0;
//		pCommand->buttons |= 4;
//		int currentTick = gpGlobals->tickcount - speedShotStartTick;// *(int*)((DWORD)pPlayer+4428)
//		if (currentTick + 1 < Positions.size()) {
//
//			Vector myRocketSource;
//			Vector endPosLocal;
//			//myRocketSource = rocketSource2(&(pCommand->viewangles), &endPosLocal, NULL);
//			//myRocketSource = rocketSource2(&(pCommand->viewangles), NULL, NULL);
//			myRocketSource = rocketSource2(&(pCommand->viewangles), NULL, &Positions[currentTick + 1]);
//			//int ticksToExplode = (int)(dist(myRocketSource, endPosLocal) / (1100.f*0.015f))+1;
//			int ticksToExplode = (int)(dist(myRocketSource, desiredRocketEndPos) / (1100.f * 0.015f)) + 1;
//
//			// land one tick before rocket reaches endPos for explosion???
//			// +1 tick to shoot, + 1 tick to explode?
//			if (ticksToLand == ticksToExplode + 1/*(timeToLand - (timeToExplode)) <0.01f*/) {//Equal exactly????? Im guessing they don't have to be equal exactly. I think the rocket should explode once your
//
//
//				pCommand->buttons |= 1;
//				shotTick = gpGlobals->tickcount;
//
//
//				speedShoting = false;
//				zVels.clear();
//				Positions.clear();
//				Positions2.clear();
//				zVels2.clear();
//				mySpecialIndex = 0;
//			}
//			--ticksToLand;
//
//		}
//		else {
//			print("SpeedShot failed\n");
//			speedShoting = false;
//			zVels.clear();
//			Positions.clear();
//			Positions2.clear();
//			zVels2.clear();
//			mySpecialIndex = 0;
//		}
//		/*z vel is 0 and your horizontal velocity isn't fully slowed down by friction yet. Although maybe you don't get nearly as much speed as when the rocket
//	explodes on the tick you land. (Failed speedshots with a big red damage bar...)*/
//	}
//}