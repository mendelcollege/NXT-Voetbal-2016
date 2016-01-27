void BoemBoemBatsiBa()
{
    GoNowhere();
    while(true)
    {
        UpdateIRValues();
        TextOut(0, LCD_LINE7, "Richten... ");
        if(!BALLPOSSESSION) break;
        if(RELCOMPASSVAL > 5) TurnLeft(75);
        if(RELCOMPASSVAL < -5) TurnRight(75);
        if(abs(RELCOMPASSVAL) < 5)
        {
            Off(MOTORALL);
            if((CurrentTick() - tlastkick) >= RECHARGINGTIME)
            {
                TextOut(0, LCD_LINE7, "Kick...    ");
                GoForward(100);
                Wait(500);
                Kick();
                Wait(500);
                Off(MOTORALL);
                break;
            }
            else
            {
                TextOut(0, LCD_LINE7, "Charging...");
            }
        }
    }
}

void BoemBoemBatsiBaTheSequel()
{
    int improveddistance[4];
    int aim;
    float temp;

    //Turn Forward
    GoNowhere();
    if(!BALLPOSSESSION) return;
    TurnTo(0, 75);
    if(!BALLPOSSESSION) return;

    //Check Readings
    if(aproxequal(distance[LEFT] + distance[RIGHT], STADIUMWIDTH, 10));
    {
        PlayTone(TONE_G3, 500);
    }

    //Calc pos from goal
    if(distance[LEFT] > distance[RIGHT])
    {
        improveddistance[LEFT] = distance[LEFT];
        improveddistance[RIGHT] = STADIUMWIDTH - distance[LEFT];
        temp = (STADIUMWIDTH / 2) - improveddistance[RIGHT];
    }
    else
    {
        improveddistance[RIGHT] = distance[RIGHT];
        improveddistance[LEFT] = STADIUMWIDTH - distance[RIGHT];
        temp = (STADIUMWIDTH / 2) - improveddistance[LEFT];
    }
    if(improveddistance[LEFT] < GOALWALLDIST || improveddistance[RIGHT] < GOALWALLDIST)
        improveddistance[FORWARD] = distance[FORWARD] - 25;

    //Calc tan -> angle
    temp = improveddistance[FORWARD] / temp;
    aim = (atan(temp) * 57.296);

    //Aim
    TurnTo(aim, 75);
    if(!BALLPOSSESSION) return;

    //Kick
    while(CurrentTick() - tlastkick < RECHARGINGTIME - 500)
    {
        TextOut(0, LCD_LINE7, "Charging...");
        PlayTone(TONE_A7, 50);
    }
    if(!BALLPOSSESSION) return;
    TextOut(0, LCD_LINE7, "Kick...    ");
    GoForward(100);
    Wait(500);
    Kick();
}
