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

#define MOTORFORWARD OUT_B
#define MOTORSIDE OUT_A
#define COMPENSATOR OUT_C

#define GoLeft() start LeftDriver
#define GoRight() start RightDriver
#define GoForward() start ForwardDriver
#define GoBackward() start BackwardDriver

task ForwardDriver()
{
    char correctingspeed;
    stop LeftDriver;
    stop RightDriver;
    stop BackwardDriver;
    Off(OUT_A);
    OnFwd(OUT_B, -100);
    OnFwd(OUT_C, 28);                                                           //Still needs to be calibrated
    while(true)
    {
        correctingspeed = 28 - RELCOMPASSVAL / 4;                               //Dit dan ook nog goed instellen
        OnFwd(OUT_C, correctingspeed);
    }
}

task BackwardDriver()
{
    char correctingspeed;
    stop LeftDriver;
    stop RightDriver;
    stop ForwardDriver;
    Off(OUT_A);
    OnFwd(OUT_B, 100);
    OnFwd(OUT_C, -28);
    while(true)
    {
        correctingspeed = -28 - RELCOMPASSVAL / 4;
        OnFwd(OUT_C, correctingspeed);
    }
}

task LeftDriver()
{
    char correctingspeed;
    stop RightDriver;
    stop ForwardDriver;
    stop BackwardDriver;
    Off(OUT_B);
    OnFwd(OUT_A, -100);
    OnFwd(OUT_C, 58);
    while(true)
    {
        correctingspeed = 58 - RELCOMPASSVAL / 4;
        OnFwd(OUT_C, correctingspeed);
    }
}

task RightDriver()
{
    char correctingspeed;
    stop LeftDriver;
    stop ForwardDriver;
    stop BackwardDriver;
    Off(OUT_B);
    OnFwd(OUT_A, 100);
    OnFwd(OUT_C, -55);
    while(true)
    {
        correctingspeed = -55 - RELCOMPASSVAL / 4;
        OnFwd(OUT_C, correctingspeed);
    }
}

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

//Position
#define XPOS (USLEFTVAL - x0)
#define YPOS (USBACKVAL - y0)

unsigned char x0;
unsigned char y0;

//IRBall
#define BALLDIRLEFT (dir > 5)
#define BALLDIRRIGHT (dir < 5)
#define BALLDIRSTRAIGHT (dir == 5)
#define BALLDIRUNKNOWN (dir == 0)
#define BALLDISTANCETHRESHOLD 300

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

short CompassVal()
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

short RelCompassVal()
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
}

void DrawSensorValues()
{
    NumOut(50,  LCD_LINE1, dir);
    TextOut(50, LCD_LINE2, "    ");
    NumOut(50,  LCD_LINE2, dist);
    TextOut(50, LCD_LINE3, "   ");
    NumOut(50,  LCD_LINE3, RELCOMPASSVAL);
    TextOut(25, LCD_LINE4, "   ");
    NumOut(25,  LCD_LINE4, USLEFTVAL);
    TextOut(75, LCD_LINE4, "   ");
    NumOut(75,  LCD_LINE4, USBACKVAL);
}
