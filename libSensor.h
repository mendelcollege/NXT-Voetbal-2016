#include "HTSMUX-driver.h"

#define IRSEEKERPORT S1
#define COMPASSSENSORPORT S2
#define LIGHTSENSORPORT S3
#define MULTIPLEXORPORT S4

#define COMPASSVAL SensorHTCompass(S2)
#define LIGHTVAL SENSOR_3
#define USVAL1 smuxSensorLegoUS(msensor_S4_1)
#define USVAL2 smuxSensorLegoUS(msensor_S4_2)
#define USVAL3 smuxSensorLegoUS(msensor_S4_3)
#define USVAL4 smuxSensorLegoUS(msensor_S4_4)

#define ANGLESENSORUS1 0
#define ANGLESENSORUS2 90
#define ANGLESENSORUS3 180
#define ANGLESENSORUS4 270

void HTEnhancedIRSeekerV2(const byte  port, int &dir, int &strength)
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

short compassbeginval;                                                          //compassbeginval =  COMPASSVAL;

inline short CompassVal()
{
    return COMPASSVAL - compassbeginval;
}

short RelCompassVal()
{
    short tmp = COMPASSVAL - compassbeginval;
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


