#include "electricBase.c"
#include "consts.c"
#include "globalVariables.h"

//////////////////////////////////////////  RT  //////////////////////////////////////////

/// is RT at the limit position
inline bool isRTAtPositon () {
    return (SensorValue[sensorLightRTRotate] >= rotateLimitLightThreshold);
}

/// rotate RT to the limit position
void RTrotateToPosition(int motorName, int direction, int speed=20) {
    direction = direction>0 ? 1 : -1;
    speed = abs(speed);

    motor[motorName] = direction * speed;

    while ( ! isRTAtPositon() ) {
        wait1Msec(msForMultiTasking);
    }
    motor[motorName] = 0;

}

void action_RTrotateToLean() {

    switch(state_currentRotatePosition) {
        case STATE_ROTATE_AT_INIT:
            setAngleRelative(motorRotate, -58*tRatioRT, -30);
            break;
        case STATE_ROTATE_AT_BEHIND:
            setAngleRelative(motorRotate,  58*tRatioRT,  30);
            break;
        case STATE_ROTATE_AT_LEAN:
            return;
        default:
            return;
    }

    state_currentRotatePosition = STATE_ROTATE_AT_LEAN;

}

void action_RTrotateToBehind() {
    switch(state_currentRotatePosition) {
        case STATE_ROTATE_AT_INIT:
            setAngleRelative(motorRotate, -70*tRatioRT, -30);
            RTrotateToPosition(motorRotate, -1);
            break;
        case STATE_ROTATE_AT_LEAN:
            RTrotateToPosition(motorRotate, -1);
            break;
        case STATE_ROTATE_AT_BEHIND:
            return;
        default:
            return;
    }

    state_currentRotatePosition = STATE_ROTATE_AT_BEHIND;
}

void action_RTrotateToFront() {
    switch (state_currentRotatePosition) {
        case STATE_ROTATE_AT_BEHIND:
            setAngleRelative(motorRotate, 70*tRatioRT, 30);
            RTrotateToPosition(motorRotate, 1);
            break;
        case STATE_ROTATE_AT_LEAN:
            RTrotateToPosition(motorRotate, 1);
            break;
        case STATE_ROTATE_AT_INIT:
            return;
        default:
            return;
    }

    state_currentRotatePosition = STATE_ROTATE_AT_INIT;
}


//////////////////////////////////////////  PT  //////////////////////////////////////////

/// get the layer value of which PT is currently at
///     i.e. LS1
///     return the "n" of LS
inline int getCurrentPT_LS() {
    return (state_currentPlatformPosition - STATE_PLATFORM_AT_LS1 + 1);
}

inline int currentPT_to_LS1_Val() {
    int index = getCurrentPT_LS();
    return ENCODERvAL_LS_ARRAY[index];
}

/// Whether the cube is attached to the platform
inline bool isCubeAttachedPT() {
    return (SensorValue[sensorCubeAttatch] == 1);
}

void waitUntilCubeDettachedPT_filtered() {

    while(isCubeAttachedPT()) {
        while (isCubeAttachedPT()) {
            wait1Msec(msForMultiTasking);
        }
        wait1Msec(180);
    }
}

void action_PT_goUp() {
    motor[motorPlatform] = -100;
}

void action_PT_goDown() {
    motor[motorPlatform] = 100;
}

void action_PT_stop() {
    motor[motorPlatform] = 0;
}


bool action_PT_up_fromBottom_toLSn(int n) {

    if (state_currentPlatformPosition != STATE_PLATFORM_AT_BOTTOM_LIMIT)
        return false;

    int destVal, finalStateVal;

    destVal = ENCODERvAL_LS_BASE + ENCODERvAL_LS_ARRAY[n];
    switch(n) {
        case 1:
            finalStateVal = STATE_PLATFORM_AT_LS1;
            break;
        case 2:
            finalStateVal = STATE_PLATFORM_AT_LS2;
            break;
        case 3:
            finalStateVal = STATE_PLATFORM_AT_LS3;
            break;
        case 4:
            finalStateVal = STATE_PLATFORM_AT_LS4;
            break;
        defaut: return false;
    }

    if ( ! isCubeAttachedPT() )
    {
        // we assume the platform is at the below

        nMotorEncoder[motorPlatform] = 0;

        action_PT_goUp();
        int encoderVal;
        while ( ! isCubeAttachedPT() ) {
            encoderVal = nMotorEncoder[motorPlatform];

            if (encoderVal < PT_findCube_maxEncoderVal) {
                // move up a distance still
                //      no cube? bad situation.
                break;
            }

            wait1Msec(msForMultiTasking);

        }
        action_PT_stop();
    }
    if ( ! isCubeAttachedPT() )
    {
        // nothing hit? I assume there's no cube on the platform!
        PlaySound(soundException);
        wait1Msec(600);

        // move the platform to the origin position
        setAngleRelative(motorPlatform, -nMotorEncoder[motorPlatform], 95);
        return false;
    }
    else
    {
        // the platform now just hit the cube
        setAngleRelative(motorPlatform, destVal, 95);

        /*
        while(1) {
            eraseDisplay();
            nxtDisplayString(2, "encoder=%d", nMotorEncoder[motorPlatform]);
            wait1Msec(20);
        }
        */

    }

    state_currentPlatformPosition = finalStateVal;
    return true;
}


void action_PT_down_toBottomLimit(bool ignoreCurrentPos=false) {

    if ( !ignoreCurrentPos )
        if (state_currentPlatformPosition < STATE_PLATFORM_AT_LS1)
            return;

    //if(! isCubeAttached())
    //  return;

    const int confidentialMovedDownVal = 800;

    int fromEncoderVal = nMotorEncoder[motorPlatform];

    do {
        action_PT_goDown();

        // this is a filtered waiting,
        //      in case of vibration making the wrong state.
        waitUntilCubeDettachedPT_filtered();

        action_PT_stop();

        // too early for the platform to stop move down,
        //      assuming that the cube dettached accidentally
        if ( nMotorEncoder[motorPlatform] - fromEncoderVal
                        < confidentialMovedDownVal   )
        {

            setAngleRelative(motorPlatform, 360, 80);

            bool isFirst = true;
            while (! isCubeAttachedPT())
            {
                PlaySound(soundException);
                if (isFirst) {
                    isFirst = false;
                    for(int i=0; i< 5; i++) {
                        setAngleRelative(motorLS, 10*tRatioLS, 50);
                        if ( isCubeAttachedPT()) break;
                        setAngleRelative(motorLS, -10*tRatioLS, 50);
                        if ( isCubeAttachedPT()) break;

                        wait1Msec(msForMultiTasking);
                    }
                }
                wait1Msec(500);
            }
        }else {
            break;
        }
    } while(1);

    // the cube is landed on the RT shelf,
    //      hide(move down) the platform;
    setAngleRelative(motorPlatform, PT_moveDown_afterDettach_EncoderVal, 80);

    state_currentPlatformPosition = STATE_PLATFORM_AT_BOTTOM_LIMIT;

}


void action_PT_fromLS_toLS(int LSfrom, int LSto) {

    if (state_currentPlatformPosition < STATE_PLATFORM_AT_LS1)
        return;

    //if(! isCubeAttached())
    //  return;

        if (LSfrom == LSto)
            return;

    int encoderVal = ENCODERvAL_LS_ARRAY[LSto] - ENCODERvAL_LS_ARRAY[LSfrom];

    setAngleRelative(motorPlatform, encoderVal, 70);

    bool isFirst = true;
    while (! isCubeAttachedPT())
    {
        PlaySound(soundException);
        if (isFirst) {
            isFirst = false;
            for(int i=0; i< 5; i++) {
                setAngleRelative(motorLS, 10*tRatioLS, 50);
                if ( isCubeAttachedPT()) break;
                setAngleRelative(motorLS, -10*tRatioLS, 50);
                if ( isCubeAttachedPT()) break;

                wait1Msec(msForMultiTasking);
            }
        }
        wait1Msec(500);
    }

    switch(LSto) {
        case 1:
            state_currentPlatformPosition = STATE_PLATFORM_AT_LS1;
            break;
        case 2:
            state_currentPlatformPosition = STATE_PLATFORM_AT_LS2;
            break;
        case 3:
            state_currentPlatformPosition = STATE_PLATFORM_AT_LS3;
            break;
        case 4:
            state_currentPlatformPosition = STATE_PLATFORM_AT_LS4;
            break;
        defaut: return;
    }

}




//////////////////////////////////////////  LS  //////////////////////////////////////////


void swingTheLS() {

    for(int i=0; i< 5; i++) {
            setAngleRelative(motorLS, 10*tRatioLS, 50);
            setAngleRelative(motorLS, -10*tRatioLS, 50);
    }

}

int LS_DirectionBeforeAligned = 0;


void LSFindPosition_currentAsCentre(int startDirection, int maxFindAngle, int speed, int lowRequirement=0) {

    const int minSearchSpeed = 15;

    if (startDirection == 0 ) return;
    int direction = startDirection>0 ? 1 : -1;

    LS_DirectionBeforeAligned = 0;

    speed = abs(speed);
    int _maxAngle, maxAngle = abs(maxFindAngle);

    bool isFoundPosition = false;

    int maxLastTurnMax;
    int nowVal;


    if (speed <= minSearchSpeed) {
        // still cannot find it. dont speed too much time on this.
        writeDebugStreamLine("still cannot find it, exit");
        return;
    }

    int _turn = 0;
    while ( 1 ) {

        _turn ++;
        maxLastTurnMax = -1;

        // reset the encoderVal to 0
        nMotorEncoder[motorLS] = 0;


        // the first turn, we make the turn half of the maxFindAngle.
        if (_turn==1) {
            _maxAngle = maxAngle / 2;
        }else if (_turn==2){
            _maxAngle = maxAngle;
        }else{
            // the larger the turn, the wider we rotate to find
            _maxAngle = (float)(1.3 * _maxAngle);
        }

        if (_turn>=9) {
            writeDebugStreamLine("we speed too much turn, exit");
            motor[motorLS] = 0;
            break;
        }

        motor[motorLS] = direction * speed;

        #define nowVal SensorValue[sensorLightLSRotate]

        while ( nowVal < LSRotateLightThreshold ) {

            // we stop at position where produced the max value, ...or
                //     just stop at the minCandidate value.
            if ( nowVal > LSRotateLightCandidateThreshold && nowVal > lowRequirement ) {

                motor[motorLS] = 0;
                wait1Msec(200);

                // the deeper we located, the narrower we rotate to find
                maxFindAngle = (float)(0.8*maxFindAngle);

                writeDebugStreamLine("get in with Val==%d nextMaxFindAngle=%d ,at turn%d", 
                    nowVal, maxFindAngle, _turn );

                LSFindPosition_currentAsCentre(direction, maxFindAngle, speed, nowVal);

                return;
            }

            if(nowVal > maxLastTurnMax)
                maxLastTurnMax = nowVal;

            wait1Msec(msForMultiTasking);

            if (abs(nMotorEncoder[motorLS]) > _maxAngle) {
                // rotate too much?
                break;
            }

        }
        motor[motorLS] = 0;

        wait1Msec(200);
        if (SensorValue[sensorLightLSRotate] < LSRotateLightThreshold) {
            // I think the rotation is just too much... rotate back
            direction *= -1;

        }else{
            // we found the position!
            writeDebugStreamLine("found it!exit");
            LS_DirectionBeforeAligned = direction;
            break;
        }

        writeDebugStreamLine("end of turn%d, with max=%d, speed=%d, maxAngle=%d", 
            _turn, maxLastTurnMax, speed, _maxAngle);

        // gradually decrease the speed.
        speed = speed <= minSearchSpeed ? minSearchSpeed : speed-5;
    }

    #undef nowVal
}

void LSFindPosition_currentAsCentre_v2(int startDirection, int maxFindAngle, int speed) {

    const int minSearchSpeed = 15;

    if (startDirection == 0 ) return;
    int direction = startDirection>0 ? 1 : -1;

    LS_DirectionBeforeAligned = 0;

    speed = abs(speed);
    int _maxAngle, maxAngle = abs(maxFindAngle);

    bool isFoundPosition = false;

    int maxFoundVal = -1;
    int maxLastTurnMax;
    int nowVal;


    if (speed <= minSearchSpeed) {
        // still cannot find it. dont speed too much time on this.
        writeDebugStreamLine("still cannot find it, exit");
        return;
    }

    int _turn = 0;
    while ( 1 ) {

        _turn ++;
        maxLastTurnMax = -1;

        // reset the encoderVal to 0
        nMotorEncoder[motorLS] = 0;


        // the first turn, we make the turn half of the maxFindAngle.
        if (_turn==1) {
            _maxAngle = maxAngle / 2;
        }else{
            _maxAngle = maxAngle;
        }

        if (_turn>=9) {
            writeDebugStreamLine("we speed too much turn, exit");
            motor[motorLS] = 0;
            break;
        }

        motor[motorLS] = direction * speed;

        #define nowVal SensorValue[sensorLightLSRotate]

        while ( nowVal < LSRotateLightThreshold ) {

            if ( _turn<=2 ) {
            // if this is the record turn
                // record the max value
                if (nowVal > maxFoundVal) {
                    maxFoundVal = nowVal;
                    writeDebugStreamLine("...updated maxVal==%d at turn%d", maxFoundVal, _turn);
                }
            } else {
            // this is the finding turn
                // then we stop at position where produced the max value, ...or
            		//     just stop at the minCandidate value.
                if (nowVal > maxFoundVal || nowVal > LSRotateLightCandidateThreshold ) {
                    motor[motorLS] = 0;

                    writeDebugStreamLine("get in with maxVal==%d at turn%d", maxFoundVal, _turn );

                    LSFindPosition_currentAsCentre(direction, maxFindAngle, speed);

                    return;
                }
            }

            if(nowVal > maxLastTurnMax)
                maxLastTurnMax = nowVal;

            wait1Msec(msForMultiTasking);

            if (abs(nMotorEncoder[motorLS]) > _maxAngle) {
                // rotate too much?
                break;
            }

        }
        motor[motorLS] = 0;

        wait1Msec(200);
        if (SensorValue[sensorLightLSRotate] < LSRotateLightThreshold) {
            // I think the rotation is just too much... rotate back
            direction *= -1;

        }else{
            // we found the position!
            writeDebugStreamLine("found it!exit");
            LS_DirectionBeforeAligned = direction;
            break;
        }

        writeDebugStreamLine("end of turn%d, with max=%d, speed=%d", _turn, maxLastTurnMax, speed);

        // gradually decrease the speed.
        speed = speed <= minSearchSpeed ? minSearchSpeed : speed-5;
    }

    #undef nowVal

}


void LSRotateToPosition(int motorName, int direction, int maxFindAngle=40*tRatioLS) {

    if (direction == 0 ) return;
    direction = direction>0 ? 1 : -1;

    const int minSearchSpeed = 25;
    int speed = 55;
    int _maxAngle, maxAngle = abs(maxFindAngle);

    bool isFirstTime=true;
        int nTurn = 0;

    // this is for the test proprose.
    LSFindPosition_currentAsCentre(direction, maxFindAngle, 55);
    return;

    while (1) {

        // reset the encoderVal to 0
        nMotorEncoder[motorLS] = 0;

        writeDebugStreamLine("\n + at turn %d (%d)", nTurn, nTurn%2==0);
        nTurn++;

        motor[motorName] = direction * speed;
        while ( SensorValue[sensorLightLSRotate] < LSRotateLightThreshold) {

            // the first turn, we make the turn half of the maxFindAngle.
            if (isFirstTime) {
                _maxAngle = maxAngle / 2;
                isFirstTime = false;
            }else{
                _maxAngle = maxAngle;
            }

            // TODO: in every turn, we record the maxim value,
            //      in next turn, we should at least turn to that value
            //      (reliability promise)
            writeDebugStream("%d, ", SensorValue[sensorLightLSRotate]);

            wait1Msec(msForMultiTasking);

            if (abs(nMotorEncoder[motorLS]) > _maxAngle) {
                // rotate too much?
                break;
            }

        }
        motor[motorName] = 0;

        wait1Msec(200);
        if (SensorValue[sensorLightLSRotate] < LSRotateLightThreshold) {
            // I think the rotation is just too much... rotate back
            direction *= -1;

            if (speed <= minSearchSpeed) {
                // still cannot find it. dont speed too much time on this.
                    writeDebugStreamLine("cannot found it");
                break;
            }

            // gradually decrease the speed.
            speed = speed <= minSearchSpeed ? minSearchSpeed : speed-7;

        }else{
                writeDebugStreamLine("found it");
            break;
        }
    }

    // TODO: continue to turn(small), find the largest val


}

void action_LS_turn_90i() {
    bool hasLoad =
        (state_currentPlatformPosition >= STATE_PLATFORM_AT_LS1);

    int speed=100;

    int currentVal, destVal = (90+10)*tRatioLS;
    setAngleRelative(motorLS, destVal, speed);

    // wait a short time for the twist force back
    wait1Msec(300);

    currentVal = nMotorEncoder[motorLS];
    LSRotateToPosition(motorLS, -1, 20*tRatioLS );

    /*
    if (LS_DirectionBeforeAligned != 0) {
        // indicating that we found the position
        // noted, 之所以要使用上一次对中时候，最后的转动方向，是因为我希望避免回程差。

        // turn an extra angle because of gap
        wait1Msec(200);
        setAngleRelative(motorLS, LS_DirectionBeforeAligned * LSRotateExtraGap, 70);
        wait1Msec(200);
        setAngleRelative(motorLS, -LS_DirectionBeforeAligned * 0.5 * LSRotateExtraGap, 70);
        //setAngleRelative(motorLS, -10*tRatioLS, speed);
        wait1Msec(200);
        LSRotateToPosition(motorLS, -LS_DirectionBeforeAligned, LSRotateExtraGap);
    } */

    if (LS_DirectionBeforeAligned != 0) {
        // indicating that we found the position

        // turn an extra angle because of gap
        wait1Msec(200);
        setAngleRelative(motorLS, +1 * LSRotateExtraGap, 70);
        wait1Msec(200);
        setAngleRelative(motorLS, (float)(-1 * 0.5 * LSRotateExtraGap), 70);
        //setAngleRelative(motorLS, -10*tRatioLS, speed);
        wait1Msec(200);
        LSRotateToPosition(motorLS, +1, LSRotateExtraGap);
    }

}

void action_LS_turn_90() {
    bool hasLoad =
        (state_currentPlatformPosition >= STATE_PLATFORM_AT_LS1);

    int speed=100;

    int currentVal, destVal = -(90)*tRatioLS;
    setAngleRelative(motorLS, destVal, speed);

    // wait a short time for the twist force back
    wait1Msec(300);

    currentVal = nMotorEncoder[motorLS];
    LSRotateToPosition(motorLS, 1, 20*tRatioLS );

    /*
    if (LS_DirectionBeforeAligned != 0) {
        // indicating that we found the position
        // noted, 之所以要使用上一次对中时候，最后的转动方向，是因为我希望避免回程差。

        // turn an extra angle because of gap
        wait1Msec(200);
        setAngleRelative(motorLS, LS_DirectionBeforeAligned * LSRotateExtraGap, 70);
        wait1Msec(200);
        setAngleRelative(motorLS, -LS_DirectionBeforeAligned * 0.5 * LSRotateExtraGap, 70);
        //setAngleRelative(motorLS, -10*tRatioLS, speed);
        wait1Msec(200);
        LSRotateToPosition(motorLS, -LS_DirectionBeforeAligned, LSRotateExtraGap);
    } */

    if (LS_DirectionBeforeAligned != 0) {
        // indicating that we found the position

        // turn an extra angle because of gap
        // and, seems it's has a different effect between inverse rotate and not, with the same value,
        //      so, I make it different (from LSRotateExtraGap).
        wait1Msec(200);
        setAngleRelative(motorLS, (float)(-1 * 0.6 * LSRotateExtraGap), 70);
        wait1Msec(200);
        setAngleRelative(motorLS, (float)(+1 * 0.8 * LSRotateExtraGap), 70);
        //setAngleRelative(motorLS, -10*tRatioLS, speed);
        wait1Msec(200);
        LSRotateToPosition(motorLS, -1, LSRotateExtraGap);
    }

}
