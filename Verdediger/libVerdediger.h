//Port aliases
#define IRSEEKERPORT S1
#define COMPASSSENSORPORT S2
#define USSENSORLEFTPORT S3
#define USSENSORBACKPORT S4

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
    TextOut(50, LCD_LINE2, "    ");
    NumOut(50,  LCD_LINE2, dist);
    TextOut(50, LCD_LINE3, "    ");
    NumOut(50,  LCD_LINE3, RELCOMPASSVAL);
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
 *OUT_A:Fwd=rechts Rev=links
 *OUT_B:Fwd=achteruit Rev=vooruit
 *OUT_C:Fwd=rechtsom Rev=linksom
 *naar rechts: OnFwd(OUT_A, 100); OnFwd(OUT_C, -55);
 *naar links: OnFwd(OUT_A, -100); OnFwd(OUT_C, 58);
 *
 *achteruit: OnFwd(OUT_B, 100); OnFwd(OUT_C, -28);
*/

char stdcorrectingspeed;

#define MOTOR_Y OUT_B
#define MOTOR_X OUT_A
#define COMPENSATOR OUT_C
#define MOTOR_ALL OUT_ABC

#define CORSPEEDLEFT 30
#define CORSPEEDRIGHT -30
#define CORSPEEDFORWARD 40
#define CORSPEEDBACKWARD -40

#define TurnRight(speed) OnFwd(COMPENSATOR, speed)
#define TurnLeft(speed) OnFwd(COMPENSATOR, (-speed))

inline void GoLeft()
{
    OnFwd(MOTOR_X, 100);
    Off(MOTOR_Y);
    //if(!correctorenabled) OnFwd(COMPENSATOR, CORSPEEDLEFT);
    stdcorrectingspeed = CORSPEEDLEFT;
}

inline void GoRight()
{
    OnFwd(MOTOR_X, -100);
    Off(MOTOR_Y);
    //if(!correctorenabled) OnFwd(COMPENSATOR, CORSPEEDRIGHT);
    stdcorrectingspeed = CORSPEEDRIGHT;
}

inline void GoForward()
{
    Off(MOTOR_X);
    OnFwd(MOTOR_Y, -100);
    //if(!correctorenabled) OnFwd(COMPENSATOR, CORSPEEDFORWARD);
    stdcorrectingspeed = CORSPEEDFORWARD;
}

inline void GoBackward()
{
    Off(MOTOR_X);
    OnFwd(MOTOR_Y, 100);
    //if(!correctorenabled) OnFwd(COMPENSATOR, CORSPEEDBACKWARD);
    stdcorrectingspeed = CORSPEEDBACKWARD;
}

inline void GoNowhere()
{
    Off(MOTOR_X);
    Off(MOTOR_Y);
    Off(COMPENSATOR);
    stdcorrectingspeed = 0;
}

void Go(char speedx, char speedy)
{
    int correctingspeedx, correctingspeedy;
    speedy = -speedy;
    if(speedx >  100)   speedx           = -100;
    if(speedx < -100)   speedx           =  100;
    if(speedy >  100)   speedy           =  100;
    if(speedy < -100)   speedy           = -100;
    if(speedx >  0)     correctingspeedx =  CORSPEEDRIGHT;
    if(speedx <  0)     correctingspeedx =  CORSPEEDLEFT;
    if(speedx == 0)     correctingspeedx =  0;
    if(speedy >  0)     correctingspeedy =  CORSPEEDBACKWARD;
    if(speedy <  0)     correctingspeedy =  CORSPEEDFORWARD;
    if(speedy == 0)     correctingspeedy =  0;
    
    correctingspeedx = correctingspeedx * speedx / 100;
    correctingspeedy = correctingspeedy * speedy / 100;
    
    OnFwd(MOTOR_X, speedx);
    OnFwd(MOTOR_Y, speedy);
    //if(!correctorenabled) OnFwd(COMPENSATOR, correctingspeedx + correctingspeedy);
    stdcorrectingspeed = correctingspeedx + correctingspeedy;
}

bool correctorenabled;

task Corrector()
{
    correctorenabled = true;
    int correctingspeed = 0;
    while(true)
    {
        correctingspeed = stdcorrectingspeed - RELCOMPASSVAL * 2;
        if(correctingspeed > 100) correctingspeed = 100;
        if(correctingspeed < -100) correctingspeed = -100;
        if(correctorenabled) OnFwd(COMPENSATOR, correctingspeed);
        Wait(10);
    }
}

//Initialisation
void Init()
{
    SetSensorLowspeed(IRSEEKERPORT);
    SetSensorLowspeed(COMPASSSENSORPORT);
    SetSensorLowspeed(USSENSORLEFTPORT);
    SetSensorLowspeed(USSENSORBACKPORT);
    compassbeginval = SensorHTCompass(COMPASSSENSORPORT);
    start Corrector;
    y0 = USBACKVAL;
    x0 = USLEFTVAL;
    DrawSensorLabels();
}
