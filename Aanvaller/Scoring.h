void BoemBoemBatsiBa()
{
    GoNowhere();
    while(true)
    {
        UpdateIRValues();
        TextOut(0, LCD_LINE7, "Richten...", DRAW_OPT_CLEAR_EOL);
        if(!BALLPOSSESSION) break;
        if(RELCOMPASSVAL > 5) TurnLeft(75);
        if(RELCOMPASSVAL < -5) TurnRight(75);
        if(abs(RELCOMPASSVAL) < 5)
        {
            Off(MOTORALL);
            if((CurrentTick() - tlastkick) >= RECHARGINGTIME)
            {
                TextOut(0, LCD_LINE7, "Kick...", DRAW_OPT_CLEAR_EOL);
                GoForward(100);
                Wait(500);
                Kick();
                Wait(500);
                Off(MOTORALL);
                break;
            }
            else
            {
                TextOut(0, LCD_LINE7, "Charging...", DRAW_OPT_CLEAR_EOL);
            }
        }
    }
}

void BoemBoemBatsiBaTheSequel()
{
    int improveddistance[4];
    int aim;
    bool blocked;
    float temp;

    //Turn maybe
    GoNowhere();
    UpdateIRValues();
    if(!BALLPOSSESSION) return;
    for(int i = 0; i < 4; i++)
    {
        if (distance[i] < 10) blocked = true;
    }
    PlayTone(TONE_G3, 500);
    if(blocked) TurnTo(RELCOMPASSVAL + 45, 75);
    Wait(1000);
    UpdateIRValues();
    if(!BALLPOSSESSION) return;
    
    //Check Readings
    if(aproxequal(distance[LEFT] + distance[RIGHT], STADIUMWIDTH, 10));
    {
        PlayTone(TONE_A4, 500);
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
    aim = atan(temp) * 57.296;

    //Aim
    TurnTo(aim, 75);
    UpdateIRValues();
    if(!BALLPOSSESSION) return;

    //Kick
    while(CurrentTick() - tlastkick < RECHARGINGTIME - 500)
    {
        TextOut(0, LCD_LINE7, "Charging...", DRAW_OPT_CLEAR_EOL);
        PlayTone(TONE_A7, 50);
    }
    UpdateIRValues();
    if(!BALLPOSSESSION) return;
    TextOut(0, LCD_LINE7, "Kick...", DRAW_OPT_CLEAR_EOL);
    GoForward(100);
    Wait(500);
    Kick();
}

void BasicBitch()
{
    int improveddistance [4];
    bool blocked;
    
    //Turn maybe
    GoNowhere();
    UpdateIRValues();
    if(!BALLPOSSESSION) return;
    for(int i = 0; i < 4; i++)
    {
        if (distance[i] < 10) blocked = true;
    }
    PlayTone(TONE_G3, 500);
    if(blocked) TurnTo(RELCOMPASSVAL + 45, 75);
    Wait(1000);
    UpdateIRValues();
    if(!BALLPOSSESSION) return;

    //Check Readings
    if(aproxequal(distance[LEFT] + distance[RIGHT], STADIUMWIDTH, 10));
    {
        PlayTone(TONE_A4, 500);
    }

    //Calc pos from goal
    if(distance[LEFT] > distance[RIGHT])
    {
        improveddistance[LEFT] = distance[LEFT];
        improveddistance[RIGHT] = STADIUMWIDTH - distance[LEFT];
    }
    else
    {
        improveddistance[RIGHT] = distance[RIGHT];
        improveddistance[LEFT] = STADIUMWIDTH - distance[RIGHT];
    }
    if(improveddistance[LEFT] < improveddistance[RIGHT])
    {
        TurnTo(-60,75);
        UpdateIRValues();
        if(!BALLPOSSESSION) return;
        GoRB(100);
        while(STADIUMWIDTH / 2 < distance[RIGHT]);
        TurnTo(0,75);
        UpdateIRValues();
        if(!BALLPOSSESSION) return;
    }
    else
    {
        TurnTo(60,75);
        UpdateIRValues();
        if(!BALLPOSSESSION) return;
        GoLB(100);
        while(STADIUMWIDTH / 2 < distance[LEFT]);
        TurnTo(0,75);
        UpdateIRValues();
        if(!BALLPOSSESSION) return;
    }
    if(distance[FORWARD] < 20)
    {
        while(CurrentTick() - tlastkick < RECHARGINGTIME - 1300)
        {
            TextOut(0, LCD_LINE7, "Charging...", DRAW_OPT_CLEAR_EOL);
            PlayTone(TONE_A7, 50);
        }
        UpdateIRValues();
        if(!BALLPOSSESSION) return;
        TextOut(0, LCD_LINE7, "Kick...", DRAW_OPT_CLEAR_EOL);
        if(Random(2) == 1)
        {
            GoLB(100);
            Wait(1000);
            GoForward(100);
            Wait(300);
            Kick();
        }
        else
        {
            GoRB(100);
            Wait(1000);
            GoForward(100);
            Wait(300);
            Kick();
        }
    }
    else
    {
        GoForward(100);
        while(distance[FORWARD] > 20);
        GoNowhere();
        while(CurrentTick() - tlastkick < RECHARGINGTIME - 1300)
        {
            TextOut(0, LCD_LINE7, "Charging...", DRAW_OPT_CLEAR_EOL);
            PlayTone(TONE_A7, 50);
        }
        UpdateIRValues();
        if(!BALLPOSSESSION) return;
        TextOut(0, LCD_LINE7, "Kick...", DRAW_OPT_CLEAR_EOL);
        if(Random(2) == 1)
        {
            GoLB(100);
            Wait(1000);
            GoForward(100);
            Wait(300);
            Kick();
        }
        else
        {
            GoRB(100);
            Wait(1000);
            GoForward(100);
            Wait(300);
            Kick();
        }
    }
    
}
