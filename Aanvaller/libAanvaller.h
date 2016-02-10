#include "NXTMMX-lib.h"

//General
#define aproxequal(n1, n2, maxdif) (abs(n1-n2) < maxdif)

//Motorstuffs (to-do)
#define MOTORLEFT OUT_A
#define MOTORBACK OUT_B
#define MOTORRIGHT OUT_C
#define MOTORALL OUT_ABC

enum drivedir
{
    FORWARD,
    BACKWARD,
    LF,
    RF,
    LB,
    RB
} currentdrivedir;

inline void TurnRight(char speed)
{
    OnRev(MOTORLEFT,  speed);
    OnRev(MOTORRIGHT, speed);
    OnRev(MOTORBACK,  speed);
}

inline void TurnLeft(char speed)
{
    OnFwd(MOTORLEFT , speed);
    OnFwd(MOTORRIGHT, speed);
    OnFwd(MOTORBACK , speed);
}
                            
inline void GoForward(char speed)
{
    OnRev(MOTORLEFT , speed);
    OnFwd(MOTORRIGHT, speed);
    Off(MOTORBACK);
    currentdrivedir = FORWARD;
}

inline void GoBackward(char speed)
{
    OnFwd(MOTORLEFT , speed);
    OnRev(MOTORRIGHT, speed);
    Off(MOTORBACK);
    currentdrivedir = BACKWARD;
}
                            
inline void GoLB(char speed)
{
    OnFwd(MOTORLEFT , speed);
    Off(MOTORRIGHT);
    OnRev(MOTORBACK , speed);
    currentdrivedir = LB;
}

inline void GoRB(char speed)
{
    Off(MOTORLEFT);
    OnRev(MOTORRIGHT, speed);
    OnFwd(MOTORBACK , speed);
    currentdrivedir = RB;
}

inline void GoLF(char speed)
{
    Off(MOTORLEFT);
    OnFwd(MOTORRIGHT, speed);
    OnRev(MOTORBACK , speed);
    currentdrivedir = LF;
}

inline void GoRF(char speed)
{
    OnRev(MOTORLEFT , speed);
    Off(MOTORRIGHT);
    OnFwd(MOTORBACK , speed);
    currentdrivedir = RF;
}

inline void GoNowhere()
{
    Off(MOTORALL);
}

void GoOpposite()
{
    switch(currentdrivedir)
    {
        case FORWARD:
            GoBackward(100);
            break;
        case BACKWARD:
            GoForward(100);
            break;
        case LF:
            GoRB(100);
            break;
        case RF:
            GoLB(100);
            break;
        case LB:
            GoRF(100);
            break;
        case RB:
            GoLF(100);
            break;
    }
}

//Port aliases
#define IRSEEKERPORT S1
#define COMPASSSENSORPORT S2
#define LIGHTSENSORPORT S3
#define USSENSORPORT S4
#define MMXPORT S4

//Kicker
#define RECHARGINGTIME 10000
#define KICKTIME 30
unsigned long tlastkick = 0;

void Kick()
{
    MMX_Run_Unlimited(MMXPORT, 0x06, MMX_Motor_2, MMX_Direction_Forward, 100);
    Wait(KICKTIME);   //Hou zo laag mogelijk. => Save energy! Save the planet!
    MMX_Stop(MMXPORT, 0x06, MMX_Motor_2, MMX_Next_Action_Float);
    tlastkick = CurrentTick();
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
#define POSSESSIONTHRESHOLD 255
#define BALLCLOSE 130
#define BALLPOSSESSION (dir == 5 && dist > POSSESSIONTHRESHOLD)

int dir;
int dist;
char lastballstate = 1;

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
int compassbeginval;

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
    int tmp = RAWCOMPASSVAL - compassbeginval;
    while(tmp <= -180) tmp += 360;
    while(tmp >   180) tmp -= 360;
}

void TurnTo(int turn, char speed)
{
    int compass = RELCOMPASSVAL;
    while(turn <= compass - 180) turn += 360;
    while(turn >  compass + 180) turn -= 360;
    TurnLeft(speed);
    while(RELCOMPASSVAL > turn + 5);
    TurnRight(speed);
    while(RELCOMPASSVAL < turn - 5);
    GoNowhere();
}

//Field dimensions (in cm)
#define STADIUMWIDTH 182
#define STADIUMLENGTH 244

#define FIELDWIDTH 122
#define FIELDLENGTH 184

#define GOALWIDTH 60
#define GOALWALLDIST 61
#define GOALDEPTH 10

//Ultrasone / Positioning
#define FORWARD 0
#define RIGHT 1
#define BACK 2
#define LEFT 3

byte distance[4];
const int dirdeg[4] = {0, 90, 180, 270};
//mutex distchecker;
bool usrotate = true;
char usrotation = 0;

void SetUS(int direction)
{
    MMX_Run_Tachometer(MMXPORT,
                           0x06,
                           MMX_Motor_1,
                           MMX_Direction_Forward,
                           100,
                           direction,
                           false,  //Relative
                           true,   //Wait for completion.
                           MMX_Next_Action_Brake);
}

//Kijk uit voor ruzie met DistChecker
//Laat maar, gebruik nu toch ander systeem
//Nu maar uitkijken dat je even wacht met metingen, motor heeft tijd nodig

inline void PointUS(char rotation)
{
    usrotate = false;
    usrotation = rotation;
    //Wacht voor een seconde voor motor, moet bij function call
}

inline void ResetUS()
{
    usrotate = true;
}

task DistChecker()
{
    int abspos;
    while(true)
    {
        //Acquire(distchecker);
        abspos  = dirdeg[usrotation] - RELCOMPASSVAL;
        while(abspos <= -180) abspos += 360;
        while(abspos >   180) abspos -= 360;
        MMX_Run_Tachometer(MMXPORT,
                           0x06,
                           MMX_Motor_1,
                           MMX_Direction_Forward,
                           100,
                           abspos,
                           false,  //Relative
                           true,   //Wait for completion.
                           MMX_Next_Action_Brake);
        //MMX_WaitUntilTachoDone(MMXPORT, 0x06, MMX_Motor_1);
        distance[usrotation] = USVAL;
        if(usrotate)
        {
            usrotation++;
            if(usrotation == 4) usrotation = 0;
        }
        //Release(distchecker);
        Yield();
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
    NumOut(50,  LCD_LINE1, dir, DRAW_OPT_CLEAR_EOL);
    NumOut(50,  LCD_LINE2, dist, DRAW_OPT_CLEAR_EOL);
    NumOut(50,  LCD_LINE3, RELCOMPASSVAL, DRAW_OPT_CLEAR_EOL);
    NumOut(50,  LCD_LINE4, LIGHTVAL, DRAW_OPT_CLEAR_EOL);
    NumOut(50,  LCD_LINE5, USVAL, DRAW_OPT_CLEAR_EOL);
}

//Initialisation
void Init()
{
    SetSensorLowspeed(IRSEEKERPORT);
    SetSensorLowspeed(COMPASSSENSORPORT);
    SetSensorType(LIGHTSENSORPORT, SENSOR_TYPE_LIGHT);
    SetSensorMode(LIGHTSENSORPORT, SENSOR_MODE_PERCENT);
    SetSensorLowspeed(MMXPORT);
    MMX_Init(MMXPORT, 0x06, MMX_Profile_NXT_Motors);
    compassbeginval = RAWCOMPASSVAL;
    DrawSensorLabels();
    start DistChecker;
}
