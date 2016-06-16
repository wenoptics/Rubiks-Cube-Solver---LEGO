#pragma config(Sensor, S2,     sensorLightLSRotate, sensorLightActive)
#pragma config(Sensor, S3,     sensorLightRTRotate, sensorLightActive)
#pragma config(Sensor, S4,     sensorCubeAttatch, sensorTouch)
#pragma config(Motor,  motorA,          motorRotate,   tmotorNXT, PIDControl, encoder)
#pragma config(Motor,  motorB,          motorPlatform, tmotorNXT, PIDControl, encoder)
#pragma config(Motor,  motorC,          motorLS,       tmotorNXT, PIDControl, encoder)
//*!!Code automatically generated by 'ROBOTC' configuration wizard               !!*//

const int sensorLightLSRotate = S2;
const int sensorLightRTRotate = S3;
const int sensorCubeAttatch 	= S4;
const int motorRotate 	= motorA;
const int motorPlatform = motorB;
const int motorLS 			= motorC;

///////////////////////////////////////////////////////////

// a 67x67mm size Rubik's Revenge
#define RRCubeSize_67
#undef RRCubeSize_67
// a 64x64mm size Rubik's Revenge
#define RRCubeSize_64

const float tRatioRT = (float)56 / 23;
const float tRatioLS = (float)56 / 8;

const int rotateLimitLightThreshold = 76;
const int LSRotateLightThreshold = 65;
const int LSRotateLightCandidateThreshold = 60;

const int LSRotateExtraGap = 160;

//////////// for a larger 4x4x4 cube ////////////
#ifdef RRCubeSize_67
const int ENCODERvAL_LS_BASE = -1750;
const int ENCODERvAL_LS_ARRAY[] = {
    ENCODERvAL_LS_BASE,
    // Below is the increasement value of LS
    0,          // LS1
    -380,       // LS2
    -730,       // LS3
    -1110       // LS4
};
#endif
//////////////////////////////////////////////////
#ifdef RRCubeSize_64
const int ENCODERvAL_LS_BASE = -1800;
const int ENCODERvAL_LS_ARRAY[] = {
    ENCODERvAL_LS_BASE,
    // Below is the increasement value of LS
    0,          // LS1
    -350,       // LS2
    -700,       // LS3
    -1050       // LS4
};
#endif

const int PT_findCube_maxEncoderVal = -800;
const int PT_moveDown_afterDettach_EncoderVal = 300;

const int msForMultiTasking = 50;


//////////////////////////////////////////////////////////
const int BTN_Gray_Rectangle    = 0;
const int BTN_Right_Arrow       = 1;
const int BTN_Left_Arrow        = 2;
const int BTN_Orange_Square     = 3;

const int STATE_ROTATE_AT_INIT      = 0;
const int STATE_ROTATE_AT_BEHIND    = 1;
const int STATE_ROTATE_AT_LEAN      = 2;
const int STATE_ROTATE_AT_ELSE      = 3;

const char STATE_PLATFORM_AT_BOTTOM_LIMIT = 0;
const char STATE_PLATFORM_AT_LS1          = 101;
const char STATE_PLATFORM_AT_LS2          = 102;
const char STATE_PLATFORM_AT_LS3          = 103;
const char STATE_PLATFORM_AT_LS4          = 104;
