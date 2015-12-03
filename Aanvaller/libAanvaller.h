#include "NXTMMX-lib.h"

//Motorstuffs (to-do)
#define MOTORLEFT OUT_A
#define MOTORBACK OUT_B
#define MOTORRIGHT OUT_C
#define MOTORALL OUT_ABC

#define TurnRight(speed)    OnFwd(MOTORLEFT, (-speed));\
                            OnFwd(MOTORRIGHT, (-speed));\
                            OnFwd(MOTORBACK, (-speed))
                            
#define TurnLeft(speed)     OnFwd(MOTORLEFT, speed);\
                            OnFwd(MOTORRIGHT, speed);\
                            OnFwd(MOTORBACK, speed)

#define GoForward(speed)    OnFwd(MOTORLEFT, (-speed));\
                            OnFwd(MOTORRIGHT, speed);\
                            Off(MOTORBACK)
                            
#define GoBackward(speed)   OnFwd(MOTORLEFT, speed);\
                            OnFwd(MOTORRIGHT, (-speed));\
                            Off(MOTORBACK)
                            
#define GoNowhere() Off(MOTORALL)

//Port aliases
#define IRSEEKERPORT S1
#define COMPASSSENSORPORT S2
#define LIGHTSENSORPORT S3
#define USSENSORPORT S4
#define MMXPORT S4

//Kicker
#define RECHARGINGTIME 3000

void Kick()
{
    MMX_Run_Unlimited(MMXPORT, 0x06, MMX_Motor_2, MMX_Direction_Forward, 75);
    Wait(500);
    MMX_Stop(MMXPORT, 0x06, MMX_Motor_2, MMX_Next_Action_Float);
}

//Sensor aliases
#define UpdateIRValues() HTEnhancedIRSeekerV2(IRSEEKERPORT, dir, dist)
#define RAWCOMPASSVAL SensorHTCompass(S2)
#define COMPASSVAL CompassVal()
#define RELCOMPASSVAL RelCompassVal()
#define LIGHTVAL SENSOR_3
#define WHITE 55 //?
#define BLACK 40 //?
#define USVAL SensorUS(S4)

//IRBall
#define BALLDIRLEFT (dir > 5)
#define BALLDIRRIGHT (dir < 5 && dir != 0)
#define BALLDIRSTRAIGHT (dir == 5)
#define BALLDIRUNKNOWN (dir == 0)
#define POSSESSIONTHRESHOLD 180
#define BALLCLOSE 140
#define BALLPOSSESSION (dir == 5 && dist > POSSESSIONTHRESHOLD)

int dir;
int dist;
char lastballstate;

void HTEnhancedIRSeekerV2(const byte  port, int &dir = dir, int &strength = dist)
{
  int cResp;
  byte cmdBuf[] = {0x10, 0x43};
  byte respBuf[];
  bool fSuccess;
  int i, iMax;
  long dcSigSum, dcStr;
  dir = 0;
  strength = 0;
  cResp=6;
  fSuccess = I2CBytes(port, cmdBuf, cResp, respBuf);
  if (fSuccess) {
    iMax = 0;
    for (i=1; i<5; i++) if (respBuf[i] > respBuf[iMax]) iMax = i;
    dir = iMax*2+1;
    dcSigSum = respBuf[iMax] + respBuf[5];
    if ((iMax > 0) && (respBuf[iMax-1] > respBuf[iMax]/2)) {
        dir--;
        dcSigSum += respBuf[iMax-1];
    }
    if ((iMax < 4) && (respBuf[iMax+1] > respBuf[iMax]/2)) {
        dir++;
        dcSigSum += respBuf[iMax+1];
    }
    dcSigSum *= 500; dcStr = 1;
    repeat(10) dcStr = (dcSigSum/dcStr + dcStr) / 2;  // sqrt
    strength = dcStr;
    if (strength <= 200) {
      dir = 0; strength = 0;
      cmdBuf[1] = 0x49;
      cResp=6;
      fSuccess = I2CBytes(port, cmdBuf, cResp, respBuf);
      if (fSuccess) {
        dir = respBuf[0];
        if (dir > 0) for (i=1; i<=5; i++) strength += respBuf[i];
      }
    }
  }
}

//Compass
short compassbeginval;                                                          //compassbeginval =  COMPASSVAL;

safecall short CompassVal()
{
    if(RAWCOMPASSVAL < 0)
    {
        return RAWCOMPASSVAL - compassbeginval + 360;
    }
    else
    {
        return RAWCOMPASSVAL - compassbeginval;
    }
}

safecall short RelCompassVal()
{
    short tmp = RAWCOMPASSVAL - compassbeginval;
    if(tmp <= 180 && tmp >= -179)
    {
        return tmp;
    }
    if(tmp > 180)
    {
        return tmp - 360;
    }
    if(tmp <= -180)
    {
        return tmp + 360;
    }
}

//Ultrasone / Positioning
long usdir;

#define FORWARD 0
#define BACK 180
#define RIGHT 90
#define LEFT 270

byte GetDist(long direction)
{
    usdir = direction;
    Wait(100);
    MMX_WaitUntilTachoDone(MMXPORT, 0x06, MMX_Motor_1);
    return USVAL;
}

task USCorrector()
{
    long tachopos;
    while(true)
    {
        tachopos  = usdir - RELCOMPASSVAL;
        MMX_Run_Tachometer(MMXPORT,
                           0x06,
                           MMX_Motor_1,
                           MMX_Direction_Forward,
                           100,
                           tachopos,
                           false,  //Relative
                           true,   //Wait for completion.
                           MMX_Next_Action_BrakeHold);
        Wait(100);
    }
}

//Display
void DrawSensorLabels()
{
    TextOut(0,  LCD_LINE1, "IRdir:");
    TextOut(0,  LCD_LINE2, "IRdist:");
    TextOut(0,  LCD_LINE3, "Compass:");
    TextOut(0,  LCD_LINE4, "Light:");
    TextOut(0,  LCD_LINE5, "US:");
}

void DrawSensorValues()
{
    NumOut(50,  LCD_LINE1, dir);
    TextOut(50, LCD_LINE2, "    ");
    NumOut(50,  LCD_LINE2, dist);
    TextOut(50, LCD_LINE3, "    ");
    NumOut(50,  LCD_LINE3, RELCOMPASSVAL);
    TextOut(50, LCD_LINE4, "   ");
    NumOut(50,  LCD_LINE4, LIGHTVAL);
    TextOut(50, LCD_LINE5, "   ");
    NumOut(50,  LCD_LINE5, USVAL);
}

//Initialisation
void Init()
{
    SetSensorLowspeed(IRSEEKERPORT);
    SetSensorLowspeed(COMPASSSENSORPORT);
    SetSensorLight(LIGHTSENSORPORT);
    SetSensorLowspeed(MMXPORT);
    MMX_Init(MMXPORT, 0x06, MMX_Profile_NXT_Motors);
    compassbeginval = RAWCOMPASSVAL;
    lastballstate = 1;
    usdir = 0;
    DrawSensorLabels();
    start USCorrector;
}
