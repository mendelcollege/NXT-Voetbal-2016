#include "NXTMMX-lib.h"

//Port aliases
#define IRSEEKERPORT S1
#define COMPASSSENSORPORT S2
#define USSENSORLEFTPORT S3
#define USSENSORBACKPORT S4
#define MMXPORT S4

//Sensor value aliases
#define UpdateIRValues() HTEnhancedIRSeekerV2(IRSEEKERPORT, dir, dist)
#define RAWCOMPASSVAL SensorHTCompass(S2)
#define COMPASSVAL CompassVal()
#define RELCOMPASSVAL RelCompassVal()
#define USLEFTVAL SensorUS(USSENSORLEFTPORT)
#define USBACKVAL SensorUS(USSENSORBACKPORT)

int x0;
int y0;

//Position
inline int XPos()
{   
    return USLEFTVAL - x0;
}

inline int YPos()
{
    return USBACKVAL - y0;
}

//IRBall
#define BALLDIRLEFT (dir > 5)
#define BALLDIRRIGHT (dir < 5)
#define BALLDIRSTRAIGHT (dir == 5)
#define BALLDIRUNKNOWN (dir == 0)
#define BALLDISTANCETHRESHOLD 200

int dir;
int dist;

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
int compassbeginval;                                                            //compassbeginval =  COMPASSVAL;

safecall int CompassVal()
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

safecall int RelCompassVal()
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

//Display
void DrawSensorLabels()
{
    TextOut(0,  LCD_LINE1, "IRdir:");
    TextOut(0,  LCD_LINE2, "IRdist:");
    TextOut(0,  LCD_LINE3, "Compass:");
    TextOut(0,  LCD_LINE4, "USL:");
    TextOut(50, LCD_LINE4, "USB:");
    TextOut(0,  LCD_LINE5, "X:");
    TextOut(50, LCD_LINE5, "Y:");
}

void DrawSensorValues()
{
    NumOut(50,  LCD_LINE1, dir);
    NumOut(50,  LCD_LINE2, dist, DRAW_OPT_CLEAR_EOL);
    NumOut(50,  LCD_LINE3, RELCOMPASSVAL, DRAW_OPT_CLEAR_EOL);
    TextOut(25, LCD_LINE4, "   ");
    NumOut(25,  LCD_LINE4, USLEFTVAL);
    TextOut(75, LCD_LINE4, "   ");
    NumOut(75,  LCD_LINE4, USBACKVAL);
    TextOut(25, LCD_LINE5, "   ");
    NumOut(25,  LCD_LINE5, XPos());
    TextOut(75, LCD_LINE5, "   ");
    NumOut(75,  LCD_LINE5, YPos());
}

//Motors
/*
 *OUT_A:Fwd=links Rev=rechts (Door onbekende redenen anders)
 *OUT_B:Fwd=achteruit Rev=vooruit
 *OUT_C:Fwd=rechtsom Rev=linksom
 *naar rechts: OnFwd(OUT_A, 100); OnFwd(OUT_C, -55);
 *naar links: OnFwd(OUT_A, -100); OnFwd(OUT_C, 58);
 *
 *achteruit: OnFwd(OUT_B, 100); OnFwd(OUT_C, -28);
*/

char stdcorrectingspeed = 0;

#define MOTOR_Y OUT_B
#define MOTOR_X OUT_A
#define COMPENSATOR OUT_C
#define MOTOR_ALL OUT_ABC

#define CORSPEEDLEFT 40
#define CORSPEEDRIGHT -35
#define CORSPEEDFORWARD 20
#define CORSPEEDBACKWARD -20

#define TurnRight(speed) OnFwd(COMPENSATOR, speed)
#define TurnLeft(speed) OnFwd(COMPENSATOR, (-speed))

inline void GoLeft()
{
    OnFwd(MOTOR_X, 100);
    MMX_Run_Unlimited(MMXPORT, 0x06, MMX_Motor_Both, MMX_Direction_Forward, 100);
    Off(MOTOR_Y);
    stdcorrectingspeed = CORSPEEDLEFT;
}

inline void GoRight()
{
    OnFwd(MOTOR_X, -100);
    MMX_Run_Unlimited(MMXPORT, 0x06, MMX_Motor_Both, MMX_Direction_Reverse, 100);
    Off(MOTOR_Y);
    stdcorrectingspeed = CORSPEEDRIGHT;
}

inline void GoForward()
{
    Off(MOTOR_X);
    MMX_Stop(MMXPORT, 0x06, MMXPORT, MMX_Next_Action_Brake);
    OnFwd(MOTOR_Y, -100);
    stdcorrectingspeed = CORSPEEDFORWARD;
}

inline void GoBackward()
{
    Off(MOTOR_X);
    MMX_Stop(MMXPORT, 0x06, MMXPORT, MMX_Next_Action_Brake);
    OnFwd(MOTOR_Y, 100);
    stdcorrectingspeed = CORSPEEDBACKWARD;
}

inline void GoNowhere()
{
    Off(MOTOR_X);
    MMX_Stop(MMXPORT, 0x06, MMXPORT, MMX_Next_Action_Brake);
    Off(MOTOR_Y);
    //Off(COMPENSATOR);
    stdcorrectingspeed = 0;
}

void Go(char speedx, char speedy) //Positive = Right Forward Negative = Left, Rev
{
    int correctingspeedx = 0, correctingspeedy = 0;
    
    //Take the right correctingspeed
    if(speedx >  0)     correctingspeedx =  CORSPEEDRIGHT;
    if(speedx <  0)     correctingspeedx =  CORSPEEDLEFT;
    if(speedy >  0)     correctingspeedy =  CORSPEEDFORWARD;
    if(speedy <  0)     correctingspeedy =  CORSPEEDBACKWARD;
    
    //Invert for motor polarity
    speedy = -speedy;
    speedx = -speedx;
    
    //Calculate vectorial correctingspeed
    correctingspeedx = correctingspeedx * abs(speedx) / 100;
    correctingspeedy = correctingspeedy * abs(speedy) / 100;
    
    //Calc total correctingspeed and turn on motors
    OnFwd(MOTOR_X, speedx);
    MMX_Run_Unlimited(MMXPORT, 0x06, MMX_Motor_Both, MMX_Direction_Forward, speedx);
    OnFwd(MOTOR_Y, speedy);
    stdcorrectingspeed = correctingspeedx + correctingspeedy;
}

#define CORRECTORISTASK

mutex corrector;
int aim = 0;

task Corrector()
{
    int correctingspeed;
    while(true)
    {
        //Acquire(corrector);
        //correctingspeed = stdcorrectingspeed - pow((RELCOMPASSVAL - aim), 3) / 5;
        correctingspeed = stdcorrectingspeed - (RELCOMPASSVAL - aim) * 3;
        if(correctingspeed > 100) correctingspeed = 100;
        if(correctingspeed < -100) correctingspeed = -100;
        OnFwd(COMPENSATOR, correctingspeed);
        //Release(corrector);
        //Yield();
    }
}

void Correct()
{
    static int correctingspeed;
    correctingspeed = stdcorrectingspeed - RELCOMPASSVAL * 2;
    if(correctingspeed > 100) correctingspeed = 100;
    if(correctingspeed < -100) correctingspeed = -100;
    OnFwd(COMPENSATOR, correctingspeed);
}

//Behaviour
#define RETURNTHRESHOLD 2000
#define DEFLECTTHRESHOLD 1000

//Initialisation
void Init()
{
    SetSensorLowspeed(IRSEEKERPORT);
    SetSensorLowspeed(COMPASSSENSORPORT);
    SetSensorLowspeed(USSENSORLEFTPORT);
    SetSensorLowspeed(USSENSORBACKPORT);
    MMX_Init(MMXPORT, 0x06, MMX_Profile_RCX_Motors);
    compassbeginval = SensorHTCompass(COMPASSSENSORPORT);
    start Corrector;
    y0 = USBACKVAL;
    x0 = USLEFTVAL;
    DrawSensorLabels();
}
