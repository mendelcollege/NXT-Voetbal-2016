//Motors
/*
 * OUT_A:Fwd=rechts Rev=links
 * OUT_B:Fwd=achteruit Rev=vooruit
 * OUT_C:Fwd=rechtsom Rev=linksom
 * naar rechts: OnFwd(OUT_A,100); OnRev(OUT_C,55);
 * naar links: OnRev(OUT_A,100); OnFwd(OUT_C,58);
 *
 *achteruit: OnFwd(OUT_B,100); OnRev(OUT_C,28);
*/
#define MOTORFORWARD OUT_B
#define MOTORSIDE OUT_A
#define COMPENSATOR OUT_C

task ForwardDriver()
{

}

task BackwardDriver()
{

}

task LeftDriver()
{

}

task RightDriver()
{

}

//Port aliases
#define IRSEEKERPORT S1
#define LIGHTSENSORPORT S2
#define USSENSORLEFTPORT S3
#define USSENSORBACKPORT S4

//Sensor value aliases
#define UpdateIRValues() HTEnhancedIRSeekerV2(IRSEEKERPORT, richting, afstand)
#define LIGHTVAL SENSOR_2
#define USLEFTVAL SensorUS(USSENSORLEFTPORT)
#define USBACKVAL SensorUS(USSENSORBACKPORT)

//IRBall
#define BALLDIRLEFT (richting > 5)
#define BALLDIRRIGHT (richting < 5)
#define BALLDIRSTRAIGHT (richting == 5)
#define BALLDIRUNKNOWN (richting == 0)
#define BALLDISTANCETHRESHOLD 300

int richting;
int afstand;
char lastballstate;

//Position
#define XPOS (USLEFTVAL - x0)
#define YPOS (USBACKVAL - y0)

unsigned char x0;
unsigned char y0;

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

void DrawSensorLabels()
{
    TextOut(0,  LCD_LINE1, "IRdir:");
    TextOut(0,  LCD_LINE2, "IRdist:");
    TextOut(0,  LCD_LINE3, "Light:");
    TextOut(0,  LCD_LINE4, "US1:");
    TextOut(50, LCD_LINE4, "US2:");
}

void DrawSensorValues()
{
    NumOut(50,  LCD_LINE1, richting);
    TextOut(50, LCD_LINE2, "    ");
    NumOut(50,  LCD_LINE2, afstand);
    TextOut(50, LCD_LINE3, "   ");
    NumOut(50,  LCD_LINE3, LIGHTVAL);
    TextOut(25, LCD_LINE4, "   ");
    NumOut(25,  LCD_LINE4, USLEFTVAL);
    TextOut(75, LCD_LINE4, "   ");
    NumOut(75,  LCD_LINE4, USBACKVAL);
}
