void BoemBoemBatsiBaTheSequel()
{
    int improveddistance[4];
    int aim;
    float temp;

    //Turn maybe
    GoNowhere();
    BallCheckReturn();
    Deblock();
    //Check Readings
    if(!aproxequal(distance[LEFT] + distance[RIGHT], STADIUMWIDTH, 10))
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
    BallCheckReturn();

    //Kick
    while(CurrentTick() - tlastkick < RECHARGINGTIME - 500)
    {
        TextOut(0, LCD_LINE7, "Charging...", DRAW_OPT_CLEAR_EOL);
        PlayTone(TONE_A7, 50);
    }
    BallCheckReturn();
    TextOut(0, LCD_LINE7, "Kick...", DRAW_OPT_CLEAR_EOL);
    GoForward(100);
    Wait(500);
    Kick();
    GoNowhere();
}

void BasicBitch()
{
    int improveddistance[4];
    
    //Turn maybe
    GoNowhere();
    BallCheckReturn();
    Deblock();
    
    //Check Readings
    if(!aproxequal(distance[LEFT] + distance[RIGHT], STADIUMWIDTH, 10))
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
    //Go to middle
    if(improveddistance[LEFT] < improveddistance[RIGHT])
    {
        TurnTo(-60,75);
        BallCheckReturn();
        PointUS(RIGHT);
        GoRB(100);
        while(STADIUMWIDTH / 2 < distance[RIGHT]);
    }
    else
    {
        TurnTo(60,75);
        BallCheckReturn();
        PointUS(LEFT);
        GoLB(100);
        while(STADIUMWIDTH / 2 < distance[LEFT]);
    }
    //Turn Forward
    TurnTo(0,75);
    BallCheckReturn();
    PointUS(FORWARD);
    Wait(500); //?
    //If close to goal, go backward and kick
    if(distance[FORWARD] < 20)
    {
        while(CurrentTick() - tlastkick < RECHARGINGTIME - 1300)
        {
            TextOut(0, LCD_LINE7, "Charging...", DRAW_OPT_CLEAR_EOL);
            PlayTone(TONE_A7, 50);
        }
        BallCheckReturn();
        TextOut(0, LCD_LINE7, "Kick...", DRAW_OPT_CLEAR_EOL);
        if(Random(2) == 1)
        {
            GoLB(100);
            Wait(1000);
            GoForward(100);
            Wait(300);
            Kick();
            GoNowhere();
        }
        else
        {
            GoRB(100);
            Wait(1000);
            GoForward(100);
            Wait(300);
            Kick();
            GoNowhere();
        }
    }
    //If far from goal, go forward up until goal then kick
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
        BallCheckReturn();
        TextOut(0, LCD_LINE7, "Kick...", DRAW_OPT_CLEAR_EOL);
        if(Random(2) == 1)
        {
            GoLB(100);
            Wait(1000);
            GoForward(100);
            Wait(300);
            Kick();
            GoNowhere();
        }
        else
        {
            GoRB(100);
            Wait(1000);
            GoForward(100);
            Wait(300);
            Kick();
            GoNowhere();
        }
    }
}

void SneakyBeaky()
{
    char route;
    Deblock();
    //Decide if right or left
    //No robot in the way
    if(aproxequal(distance[LEFT] + distance[RIGHT], STADIUMWIDTH, 10))
    {
        if(distance[LEFT] > distance[RIGHT])
        {
            route = RIGHT;
        }
        else route = LEFT;
    }
    //Robot in the way!!!!!
    else
    {
        if(distance[LEFT] > distance[RIGHT])
        {
            route = LEFT;
        }
        else route = RIGHT;
    }
    if(route == LEFT)
    {
    }
    else
    {
    }
}
