/* mipslabwork.c

   This file written 2015 by F Lundevall
   Updated 2017-04-21 by F Lundevall

   This file should be changed by YOU! So you must
   add comment(s) here with your name(s) and date(s):

   This file modified 2017-04-31 by Ture Teknolog 

   For copyright and licensing, see file COPYING */

#include <stdint.h>  /* Declarations of uint_32 and the like */
#include <pic32mx.h> /* Declarations of system-specific addresses etc */
#include "mipslab.h" /* Declatations for these labs */

typedef enum
{
  Up,
  Down,
  Left,
  Right
} directions;

static directions Direction;

int timeoutcounter = 0;
uint8_t snekboard[512];
int snekk[128]; // index 1-4: row   index 5-8: y   index 9-16: x // lowest index = head, highest index = tail
int sneklength = 5;
int food = 0;
int ready = 0;
int randomer = 0;
int restart = 0;
int move = 0;
int scorecheck = 1;
int speed = 20;
int speedup = 0;
int foodblink = 0;
int blinkc = 0;
char name[] = "        ";
int nameorgame = 2;
const int maxspeed = 10;
const int minspeed = 20;
const int diffchoice = 2;
const int gamechoice = 0;
const int namechoice = 1;
const int scorechoice = 3;
int highscore = 0;
int diff = 1;
int hardfade = 0;

static void num32ascf( char * s, int n ) 
{
  int i;
  for( i = 28; i >= 0; i -= 4 )
    *s++ = "0123456789ABCDEF"[ (n >> i) & 15 ];
}

static void num32ascdec( char * ca, int t ) 
{
  if(t > 99999999){
    t = 99999999;
  }
  int arr[8] = {0,0,0,0,0,0,0,0}; 
  while(t > 9999999){
    arr[0]++;
    t -= 10000000;
  }
  while(t > 999999){
    arr[1]++;
    t -= 1000000;
  }
  while(t > 99999){
    arr[2]++;
    t -= 100000;
  }
  while(t > 9999){
    arr[3]++;
    t -= 10000;
  }
  while(t > 999){
    arr[4]++;
    t -= 1000;
  }
  while(t > 99){
    arr[5]++;
    t -= 100;
  }
  while(t > 9){
    arr[6]++;
    t -= 10;
  }
  while(t > 0){
    arr[7]++;
    t--;
  }

  int tmp = 0;
  while(tmp < 8){
    *ca++ = "0123456789"[ arr[tmp++] ];
  }
}

void addcube(int cord)
{
  snekboard[((cord & 0xf) * 128) + ((cord >> 8) & 0xff)] |= (1 << ((cord >> 4) & 0xf)) + (1 << (((cord >> 4) & 0xf) - 1));
  snekboard[(((cord & 0xf) * 128) + ((cord >> 8) & 0xff)) + 1] |= (1 << ((cord >> 4) & 0xf)) + (1 << (((cord >> 4) & 0xf) - 1));
}

void removecube(int cord)
{
  int b = 255 ^ ((1 << ((cord >> 4) & 0xf)) + (1 << (((cord >> 4) & 0xf) - 1)));
  int a1 = snekboard[((cord & 0xf) * 128) + ((cord >> 8) & 0xff)];
  int a2 = snekboard[(((cord & 0xf) * 128) + ((cord >> 8) & 0xff)) + 1];
  a1 &= b;
  a2 &= b;
  snekboard[((cord & 0xf) * 128) + ((cord >> 8) & 0xff)] = a1;
  snekboard[(((cord & 0xf) * 128) + ((cord >> 8) & 0xff)) + 1] = a2;
}

void addfood(void)
{
  int boo = 1;
  int mask;
  while (boo)
  {
    int seed = (randomer * randomer) + 1;
    int i = 0;
    boo = 0;
    if((seed&0x7e) > 95 || (seed&0x7e) < 31)  mask = 0x20;
    else                  mask = 0x00;
    food = (((seed % 4)&0xf) + (((((seed % 4)*2)+1)&0xf) << 4) + ((((seed)^mask)&0x7e) << 8)) &0x7fff;
    while(i < sneklength){ if((snekk[i++]&0xffff) == (food&0xffff)) boo = 1; }
  }
  addcube(food);
  foodblink = 1;
}

int moveleft(void)
{
  int wall = 0;
  int mask = (snekk[0] & 0xff);
  int tmp = sneklength - 1;
  int last = snekk[sneklength - 1];
  int next = snekk[tmp + 1];
  while (tmp > 0)
  {
    snekk[tmp] = snekk[tmp - 1];
    tmp--;
  }
  if (snekk[0] <= 0x2000 + mask)
  {
    snekk[0] = 0x8000 + mask;
    wall = 1;
  }
  int new = snekk[0] - ((2) << 8);
  tmp = 3;
  while(tmp < sneklength){
    if((new&0x7fff) == (snekk[tmp++]&0x7fff)){
      wall = 1;
    }
  }
  if((new&0x7fff) == (food&0x7fff)){
    sneklength++;
    hardfade = 0;
    if(speedup){
      if((sneklength%2) == 0){
        speed--;
        if(speed < maxspeed)  speed = maxspeed;
      }
    }
    snekk[sneklength - 1] = last;
    addfood();
    addcube(new);
  }else{
    removecube(last);
    addcube(new);
  }
  snekk[0] = new;
  return wall;
}

int moveright(void)
{
  int wall = 0;
  int mask = (snekk[0] & 0xff);
  int tmp = sneklength - 1;
  int last = snekk[sneklength - 1];
  while (tmp > 0)
  {
    snekk[tmp] = snekk[tmp - 1];
    tmp--;
  }
  if (snekk[0] >= 0x5e00 + mask)
  {
    snekk[0] = mask;
    wall = 1;
  }
  int new = snekk[0] + ((2) << 8);
  tmp = 3;
  while(tmp < sneklength){
    if((new&0x7fff) == (snekk[tmp++]&0x7fff)){
      wall = 1;
    }
  }
  if((new&0x7fff) == (food&0x7fff)){
    sneklength++;
    hardfade = 0;
    if(speedup){
      if((sneklength%2) == 0){
        speed--;
        if(speed < maxspeed)  speed = maxspeed;
      }
    }
    snekk[sneklength - 1] = last;
    addfood();
    addcube(new);
  }else{
    removecube(last);
    addcube(new);
  }
  snekk[0] = new;
  return wall;
}

int moveup(void)
{
  int wall = 0;
  int r = snekk[0] & 0xf;
  int ya = (snekk[0] >> 4) & 0xf;
  int tmp = sneklength - 1;
  int last = snekk[sneklength - 1];
  while (tmp > 0)
  {
    snekk[tmp] = snekk[tmp - 1];
    tmp--;
  }
  ya -= 2;
  if (ya < 1)
  {
    ya = 7;
    r--;
    if (r < 0)
    {
      r = 3;
      wall = 1;
    }
  }
  snekk[0] &= 0xff00;
  snekk[0] += (ya << 4);
  snekk[0] += r;
  tmp = 3;
  while(tmp < sneklength){
    if((snekk[0]&0x7fff) == (snekk[tmp++]&0x7fff)){
      wall = 1;
    }
  }
  if((snekk[0]&0x7fff) == (food&0x7fff)){
    sneklength++;
    hardfade = 0;
    if(speedup){
      if((sneklength%2) == 0){
        speed--;
        if(speed < maxspeed)  speed = maxspeed;
      }
    }
    snekk[sneklength - 1] = last;
    addfood();
    addcube(snekk[0]);
  }else{
    removecube(last);
    addcube(snekk[0]);
  }
  return wall;
}

int movedown(void)
{
  int wall = 0;
  int r = snekk[0] & 0xf;
  int ya = (snekk[0] >> 4) & 0xf;
  int tmp = sneklength - 1;
  int last = snekk[sneklength - 1];
  while (tmp > 0)
  {
    snekk[tmp] = snekk[tmp - 1];
    tmp--;
  }
  ya += 2;
  if (ya > 7)
  {
    ya = 1;
    r++;
    if (r > 3)
    {
      r = 0;
      wall = 1;
    }
  }
  snekk[0] &= 0xff00;
  snekk[0] += (ya << 4);
  snekk[0] += r;
  tmp = 3;
  while(tmp < sneklength){
    if((snekk[0]&0x7fff) == (snekk[tmp++]&0x7fff)){
      wall = 1;
    }
  }
  if((snekk[0]&0x7fff) == (food&0x7fff)){
    sneklength++;
    hardfade = 0;
    if(speedup){
      if((sneklength%2) == 0){
        speed--;
        if(speed < maxspeed)  speed = maxspeed;
      }
    }
    snekk[sneklength - 1] = last;
    addfood();
    addcube(snekk[0]);
  }else{
    removecube(last);
    addcube(snekk[0]);
  }
  return wall;
}

int nextmoveisfood(directions dir){
  if(dir == Left){
    if(food&0x7fff == ((snekk[sneklength - 1]  - ((2) << 8))&0x7fff)) return 1;
    else return 0;
  }

  else if(dir == Right){
    if(food&0x7fff == ((snekk[sneklength - 1]  + ((2) << 8))&0x7fff)) return 1;
    else return 0;
  }

  else if(dir == Up){
    int tmp;
    if((snekk[sneklength - 1] << 4)&0xf == 1){
      tmp = (snekk[sneklength - 1] & 0xff0f) - 1;
      tmp += 7 << 4;
      if(tmp&0x7fff == food&0x7fff) return 1;
    }
    else {
      if(food&0x7fff == (snekk[sneklength - 1] - (2 << 4)&0x7fff)) return 1;
    }
    return 0;
  } 

  else if(dir == Down){
    int tmp;
    if((snekk[sneklength - 1] << 4)&0xf == 7){
      tmp = (snekk[sneklength - 1] & 0xff0f) + 1;
      tmp += 1 << 4;
      if(tmp&0x7fff == food&0x7fff) return 1;
    }
    else {
      if(food&0x7fff == (snekk[sneklength - 1] + (2 << 4)&0x7fff)) return 1;
    }
    return 0;
  } 
}

void disappear_food(){
  removecube(food);
  addfood();
}
/* Interrupt Service Routine */
void user_isr(void)
{
  if (IFS(0) & 0x0100)
  {
    timeoutcounter++;
    randomer++;
    hardfade++;
    IFS(0) &= ~0x0100;
  }
  
  if(nameorgame == 2){
      // display_image2(0, snekfont);
      // display_image2(96, snekfont);
  }
  if(nameorgame == 1){
    display_update();
  }
  if(nameorgame == 0){
    if(scorecheck){
    display_snek(snekboard);
  }
  }


  if(foodblink){
    blinkc++;
    if(blinkc == 10 || blinkc == 30)  removecube(food);
    if(blinkc == 20)                  addcube(food);
    if(blinkc == 40){
      addcube(food);
      blinkc = 0;
      foodblink = 0;
    }
  }
  
  if(diff == 3 && ready == 1){
    if(hardfade >= 299){
      disappear_food();
      hardfade = 0;
    }
  }

  if (randomer == 150) randomer = 0;

  if (timeoutcounter == speed)
  {
    if (ready)
    {
      move = 1;
    }
    timeoutcounter = 0;
  }

  return;
}

/* Lab-specific initialization goes here */
void labinit(void)
{
  volatile int *trise = 0xbf886100;
  (*trise) &= ~0xFF;
  TRISD |= 0xFE0;
  TRISF |= 0x2;

  T2CON = 0x0;
  T2CONSET = 0x70;
  TMR2 = 0x0;
  PR2 = 80000000 / 256 / 60;
  T2CONSET = 0x8000;

  IECSET(0) = 0x900;
  IPCSET(2) = 0x1F;
  enable_interrupt();
  return;
}

void testest()
{
  int test = 0;
  while (test < 512)
  {
    snekboard[test++] = 255;
  }
  display_snek(snekboard);
  while (1)
  {
    int buttons = getbtns();
    if (buttons == 8)
    {
      removecube(0x4072);
      display_snek(snekboard);
    }
    if (buttons == 1)
    {
      removecube(0x3851);
      display_snek(snekboard);
    }
    if (buttons == 2)
    {
      removecube(0x3230);
      display_snek(snekboard);
    }
    if (buttons == 4)
    {
      removecube(0x2212);
      display_snek(snekboard);
    }
    while ((0x1 & (PORTF >> 1)) == 1)
      ;
    while ((*((volatile int *)0xbf8860d0) >> 5) & 7)
      ;
  }
}

void display_score()
{
  display_clear();
  char score[] = "        ";
  char txt[] = "Score:";
  num32ascdec( score, sneklength - 5);
  //display_update();
  scorecheck = 0;
  display_string(2, score);
  display_string(1, txt);
  display_string(3, name);
  display_update();
  int timer = 0;
  while(timer++ < 20){
    quicksleep(500000);
  }
  display_clear();
  scorecheck = 1;
}

void board_init()
{
  int btow = 32;
  int borw = 1;
  int checker = 0;
  int b = 0, d = 0;
  while(b < 512){
    btow++;

    if(checker == 170){
      checker = 85;
    }
    else{
      checker = 170;
    }

    if(borw){
      snekboard[b++] = checker;
    }
    else{
      snekboard[b++] = 0;
    }

    if(btow == 64){
      btow = 0;
      if(borw == 1){
        borw = 0;
      }
      else{
        borw = 1;
      }
    }
  }

  b = 128;
  while(b < 160){
    snekboard[b++] = sn[d++];
  }
  b = 256;
  while(b < 288){
    snekboard[b++] = sn[d++];
  }

  d = 0;
  b = 224;
  while(b < 256){
    snekboard[b++] = ek[d++];
  }
  b = 352;
  while(b < 384){
    snekboard[b++] = ek[d++];
  }
}

void snek_init()
{
  int a = 0, c = 0x4072;
  int buttons = getbtns();
  display_snek(snekboard);

  board_init();

  while (a < sneklength) // snake init
  {
    snekk[a++] = c;
    c += ((2) << 8);
  }
  a--;
  while (a >= 0) // add snake
  {
    addcube(snekk[a--]);
  }
  display_snek(snekboard);
}

int resetmenu()
{
  nameorgame = 2;
  display_clear();
  char r[] = "    Retry";
  char m[] = "    MainMenu";
  int choice = 1;
  display_string(1, r);
  display_string(2, m);
  display_update2(choice);
  while(1){
    int buttons = getbtns();
    while(buttons == 0) buttons = getbtns();
    if(buttons == 2){
      choice++;
    }
    if(buttons == 4){
      choice--;
    }
    if(buttons == 8){
      choice--;
      while ((0x1 & (PORTF >> 1)) == 1);
      return choice; ////////
    }
    if (choice < 1) choice = 1;
    if (choice > 2) choice = 2;

    display_update2(choice);

    while ((*((volatile int *)0xbf8860d0) >> 5) & 7)
      ;
    while ((0x1 & (PORTF >> 1)) == 1)
      ;
  }
}

int reset()
{
  int tmp = 1;
  while(tmp < 9){
    Show_Led(tmp++);
    quicksleep(200000);
  }
  while(tmp >= 0){
    Show_Led(--tmp);
    quicksleep(200000);
  }

  if(highscore < (sneklength - 5)) highscore = sneklength - 5;
  display_score();
  sneklength = 5;
  if(diff == 2) speed = minspeed; 

  int resetchoice = resetmenu();
  if(resetchoice){
    return 1;
  } 

  nameorgame = 0;
  snek_init();
  int buttons = getbtns();
  while (1) // wait
  {
    buttons = getbtns();
    if (buttons == 1)
    {
      Direction = Left;
      while (buttons == 1)
      {
        buttons = getbtns();
      }
      break;
    }
    if (buttons == 2)
    {
      Direction = Up;
      while (buttons == 2)
      {
        buttons = getbtns();
      }
      break;
    }
    if (buttons == 4)
    {
      Direction = Down;
      while (buttons == 4)
      {
        buttons = getbtns();
      }
      break;
    }
  }
  ready = 1;
  quicksleep(100);
  addfood();
  return 0;
}

void snek(void)
{
  int buttons = getbtns();
  sneklength = 5;
  snek_init();

  while (1) // wait
  {
    buttons = getbtns();
    if (buttons == 1)
    {
      Direction = Left;
      while (buttons == 1)
      {
        buttons = getbtns();
      }
      break;
    }
    if (buttons == 2)
    {
      Direction = Up;
      while (buttons == 2)
      {
        buttons = getbtns();
      }
      break;
    }
    if (buttons == 4)
    {
      Direction = Down;
      while (buttons == 4)
      {
        buttons = getbtns();
      }
      break;
    }
  }
  ready = 1;
  quicksleep(100);
  addfood();

  while (1) // controls
  {
    buttons = getbtns();
    if (buttons == 8 && Direction != Left){ Direction = Right; }
    buttons = getbtns();
    if (buttons == 1 && Direction != Right){ Direction = Left; }
    buttons = getbtns();
    if (buttons == 2 && Direction != Down){ Direction = Up; }
    buttons = getbtns();
    if (buttons == 4 && Direction != Up){ Direction = Down; }

    if (move)
    {
      int re = 0;
      if((snekk[sneklength-1]&0xffff) == (food&0xffff)){
        addfood();
      }
      if (Direction == Left)
        re = moveleft();
      if (Direction == Right)
        re = moveright();
      if (Direction == Down)
        re = movedown();
      if (Direction == Up)
        re = moveup();
      restart = re;
      move = 0;
    }

    if(restart == 1){
      ready = 0;
      int resetcheck = reset();
      restart = 0;
      if(resetcheck) return;
    }

    // while ((*((volatile int *)0xbf8860d0) >> 5) & 7)
    //   ;
    // while ((0x1 & (PORTF >> 1)) == 1)
    //   ;
  }
}

int mainmenu()
{
  char n[] = "    NAMECONF";
  char g[] = "    PLAY";
  char d[] = "    DIFFCULT";
  char s[] = "    SCORES";
  int choice = 0;
  display_string(namechoice, n);
  display_string(gamechoice, g);
  display_string(diffchoice, d);
  display_string(scorechoice, s);
  display_update2(choice);
  display_image2(0, snekfont);
  display_image2(96, snekfont);
  while(1){
    int buttons = getbtns();
    while(buttons == 0) buttons = getbtns();
    if(buttons == 2){
      choice++;
    }
    if(buttons == 4){
      choice--;
    }
    if(buttons == 8){
      return choice; ////////
    }
    if (choice < 0) choice = 0;
    if (choice > 3) choice = 3;

    display_update2(choice);
    display_image2(0, snekfont);
    display_image2(96, snekfont);
    while ((*((volatile int *)0xbf8860d0) >> 5) & 7)
      ;
    while ((0x1 & (PORTF >> 1)) == 1)
      ;
  }
}

void set_difficulty()
{
  display_clear();
  char e[] = "      EASY";
  char m[] = "     MEDIUM";
  char h[] = "      HARD";
  int choice = diff;
  display_string(0, "  <DIFFICULTY>");
  display_string(1, e);
  display_string(2, m);
  display_string(3, h);
  display_update2(choice);
  while ((0x1 & (PORTF >> 1)) == 1);
  while(1){
    int buttons = getbtns();
    while(buttons == 0) buttons = getbtns();
    if(buttons == 2){
      choice++;
    }
    if(buttons == 4){
      choice--;
    }
    if(buttons == 8){
      diff = choice;
      if(choice == 1){
        speed = minspeed;
        speedup = 0;
      }
      if(choice == 2){
        speed = minspeed;
        speedup = 1;
      }
      if(choice == 3){
        speed = maxspeed;
        speedup = 0;
      }
      diff = choice;
      while ((0x1 & (PORTF >> 1)) == 1);
      return; ////////
    }
    if (choice < 1) choice = 1;
    if (choice > 3) choice = 3;

    display_update2(choice);

    while ((*((volatile int *)0xbf8860d0) >> 5) & 7)
      ;
    while ((0x1 & (PORTF >> 1)) == 1)
      ;
  }
}

void show_scorelist()
{
  display_clear();
  char score[] = "        ";
  num32ascdec( score, highscore);
  display_string(2, score);
  display_string(1, name);
  display_update();
  while ((0x1 & (PORTF >> 1)) == 1);
  int buttons = getbtns();
  while(1){
    buttons = getbtns();
    if(buttons == 8) return;
  }
  while ((0x1 & (PORTF >> 1)) == 1);
}

/* This function is called repetitively from the main program */
void labwork(void)
{
  display_clear();
  display_update();
  nameorgame = 2;
  int menuchoice = mainmenu();
  if(menuchoice == namechoice) { display_clear(); nameorgame = 1; Name_change(name); }
  if(menuchoice == gamechoice) { nameorgame = 0; snek(); }
  if(menuchoice == diffchoice) set_difficulty();
  if(menuchoice == scorechoice) show_scorelist();
  //testest();
}
