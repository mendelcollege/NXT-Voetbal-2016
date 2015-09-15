#include "HTSMUX-driver.h"

//Motors
#define MOTORVOORUIT OUT_B
#define MOTORSTUUR OUT_A
#define GoForward(speed) Off(MOTORSTUUR); OnFwd(MOTORVOORUIT, (-speed))
#define GoBackward(speed) Off(MOTORSTUUR); OnFwd(MOTORVOORUIT, speed)
#define TurnLeft(speed) OnFwd(MOTORSTUUR, speed); OnFwd(MOTORVOORUIT, -50)
#define TurnRight(speed) OnFwd(MOTORSTUUR, speed); OnFwd(MOTORVOORUIT, -50) 

//Port aliases
#define IRSEEKERPORT S1
#define COMPASSSENSORPORT S2
#define LIGHTSENSORPORT S3
#define MULTIPLEXORPORT S4

//Sensors aliases
#define UpdateIRValues() HTEnhancedIRSeekerV2(IRSEEKERPORT, richting, afstand)
#define RAWCOMPASSVAL SensorHTCompass(S2)
#define COMPASSVAL CompassVal()
#define RELCOMPASSVAL RelCompassVal()
#define LIGHTVAL SENSOR_3
#define USVAL1 smuxSensorLegoUS(msensor_S4_1)
#define USVAL2 smuxSensorLegoUS(msensor_S4_2)
#define USVAL3 smuxSensorLegoUS(msensor_S4_3)
#define USVAL4 smuxSensorLegoUS(msensor_S4_4)

//Sensor placement
#define ANGLESENSORUS1 0
#define ANGLESENSORUS2 90
#define ANGLESENSORUS3 180
#define ANGLESENSORUS4 270

//IRBall
#define BALLDIRLEFT (richting > 5)
#define BALLDIRRIGHT (richting < 5 && richting != 0)
#define BALLDIRSTRAIGHT (richting == 5)
#define BALLDIRUNKNOWN (richting == 0)
#define POSSESSIONTHRESHOLD 300
#define BALLPOSSESSION (richting == 5 && afstand > POSSESSIONTHRESHOLD)

enum ballstate
{
    left,
    right,
    straight,
    unknown
};

int richting;
int afstand;
short compassbeginval;                                                          //compassbeginval =  COMPASSVAL;
ballstate lastballstate;

void HTEnhancedIRSeekerV2(const byte  port, int &dir = richting, int &strength = afstand)
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
    if(tmp <= 180 && tmp >= -181)
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

void DrawSensorLabels()
{
    TextOut(0,  LCD_LINE1, "IRdir:");
    TextOut(0,  LCD_LINE2, "IRdist:");
    TextOut(0,  LCD_LINE3, "Compass:");
    TextOut(0,  LCD_LINE4, "Light:");
    TextOut(0,  LCD_LINE5, "US1:");
    TextOut(50, LCD_LINE5, "US2:");
    TextOut(0,  LCD_LINE6, "US3:");
    TextOut(50, LCD_LINE6, "US4:");
}


void DrawSensorValues()
{
    NumOut(50,  LCD_LINE1, richting);
    TextOut(50, LCD_LINE2, "    ");
    NumOut(50,  LCD_LINE2, afstand);
    TextOut(50, LCD_LINE3, "    ");
    NumOut(50,  LCD_LINE3, RELCOMPASSVAL);
    TextOut(50, LCD_LINE4, "   ");
    NumOut(50,  LCD_LINE4, LIGHTVAL);
/*
    TextOut(25, LCD_LINE5, "   ");
    NumOut(25,  LCD_LINE5, USVAL1);
    TextOut(75, LCD_LINE5, "   ");
    NumOut(75,  LCD_LINE5, USVAL2);
    TextOut(25, LCD_LINE6, "   ");
    NumOut(25,  LCD_LINE6, USVAL3);
    TextOut(75, LCD_LINE6, "   ");
    NumOut(75,  LCD_LINE6, USVAL4);
*/
}

void KickSmit()
{
    OnFwd(OUT_C, 100);
    Wait(100);
    Float(OUT_C);
}
