/* mipslabwork.c

  Written by Ludvig Edvinson, 2021

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

static directions Direction; // Decides which direction snake is moving

int timeoutcounter = 0;   // counts up until move command is issued
uint8_t snekboard[512];   // contains gameplay board
int snake[255]; // index 1-4: row   index 5-8: y   index 9-16: x // lowest index = head, highest index = tail
int sneklength = 5;       // length of current snake
int food = 0;             // coordinates for food on field
int ready = 0;            // goes high when gameplay starts, inhibits move command when low
int randomer = 0;         // counts up to get random seed
int restart = 0;          // goes high when game is lost
int move = 0;             // if high, move command is issued
int scorecheck = 1;       // pauses game when score is shown
int speed = 20;           // sets the gameplay speed
int speedup = 0;          // decides if gameplay has speedup or not
int foodblink = 0;        // used for food blinking when getting added
int blinkc = 0;           // same as above
char name[] = "NAME    "; // current user name
int nameorgame = 2;       // used to change graphic setup
const int maxspeed = 10;  // highest possible gamespeed
const int minspeed = 20;  // lowest possible gamespeed
const int diffchoice = 2; // for main menu
const int gamechoice = 0; // for main menu
const int namechoice = 1; // for main menu
const int scorechoice = 3;// for main menu
int highscore = 0;        // highest score for current session
int diff = 1;             // difficulty setting 1-3
int hardfade = 0;         // counts up to fade food on hard difficulty

static void num32ascf( char * s, int n ) 
{
  int i;
  for( i = 28; i >= 0; i -= 4 )
    *s++ = "0123456789ABCDEF"[ (n >> i) & 15 ];
}

static void num32ascdec( char * ca, int t ) // int -> string
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

void addcube(int cord) // adds 2x2 pixel cube at parsed coordinate, bottom left pixel
{
  snekboard[((cord & 0xf) * 128) + ((cord >> 8) & 0xff)] |= (1 << ((cord >> 4) & 0xf)) + (1 << (((cord >> 4) & 0xf) - 1));
  snekboard[(((cord & 0xf) * 128) + ((cord >> 8) & 0xff)) + 1] |= (1 << ((cord >> 4) & 0xf)) + (1 << (((cord >> 4) & 0xf) - 1));
}

void removecube(int cord) // removes 2x2 pixel cube at parsed coordinate, bottom left pixel
{
  int b = 255 ^ ((1 << ((cord >> 4) & 0xf)) + (1 << (((cord >> 4) & 0xf) - 1)));
  int a1 = snekboard[((cord & 0xf) * 128) + ((cord >> 8) & 0xff)];
  int a2 = snekboard[(((cord & 0xf) * 128) + ((cord >> 8) & 0xff)) + 1];
  a1 &= b;
  a2 &= b;
  snekboard[((cord & 0xf) * 128) + ((cord >> 8) & 0xff)] = a1;
  snekboard[(((cord & 0xf) * 128) + ((cord >> 8) & 0xff)) + 1] = a2;
}

void addfood(void) // adds new food on random location
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
    while(i < sneklength){ if((snake[i++]&0xffff) == (food&0xffff)) boo = 1; }
  }
  addcube(food);
  foodblink = 1;
}

int moveleft(void) // snek moves left
{
  int wall = 0;
  int mask = (snake[0] & 0xff);
  int tmp = sneklength - 1;
  int last = snake[sneklength - 1];
  int next = snake[tmp + 1];
  while (tmp > 0)
  {
    snake[tmp] = snake[tmp - 1];
    tmp--;
  }
  if (snake[0] <= 0x2000 + mask)
  {
    snake[0] = 0x8000 + mask;
    wall = 1;
  }
  int new = snake[0] - ((2) << 8);
  tmp = 3;
  while(tmp < sneklength){
    if((new&0x7fff) == (snake[tmp++]&0x7fff)){
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
    snake[sneklength - 1] = last;
    addfood();
    addcube(new);
  }else{
    removecube(last);
    addcube(new);
  }
  snake[0] = new;
  return wall;
}

int moveright(void) // snek moves right
{
  int wall = 0;
  int mask = (snake[0] & 0xff);
  int tmp = sneklength - 1;
  int last = snake[sneklength - 1];
  while (tmp > 0)
  {
    snake[tmp] = snake[tmp - 1];
    tmp--;
  }
  if (snake[0] >= 0x5e00 + mask)
  {
    snake[0] = mask;
    wall = 1;
  }
  int new = snake[0] + ((2) << 8);
  tmp = 3;
  while(tmp < sneklength){
    if((new&0x7fff) == (snake[tmp++]&0x7fff)){
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
    snake[sneklength - 1] = last;
    addfood();
    addcube(new);
  }else{
    removecube(last);
    addcube(new);
  }
  snake[0] = new;
  return wall;
}

int moveup(void) // snek moves up
{
  int wall = 0;
  int r = snake[0] & 0xf;
  int ya = (snake[0] >> 4) & 0xf;
  int tmp = sneklength - 1;
  int last = snake[sneklength - 1];
  while (tmp > 0)
  {
    snake[tmp] = snake[tmp - 1];
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
  snake[0] &= 0xff00;
  snake[0] += (ya << 4);
  snake[0] += r;
  tmp = 3;
  while(tmp < sneklength){
    if((snake[0]&0x7fff) == (snake[tmp++]&0x7fff)){
      wall = 1;
    }
  }
  if((snake[0]&0x7fff) == (food&0x7fff)){
    sneklength++;
    hardfade = 0;
    if(speedup){
      if((sneklength%2) == 0){
        speed--;
        if(speed < maxspeed)  speed = maxspeed;
      }
    }
    snake[sneklength - 1] = last;
    addfood();
    addcube(snake[0]);
  }else{
    removecube(last);
    addcube(snake[0]);
  }
  return wall;
}

int movedown(void) // snek moves down
{
  int wall = 0;
  int r = snake[0] & 0xf;
  int ya = (snake[0] >> 4) & 0xf;
  int tmp = sneklength - 1;
  int last = snake[sneklength - 1];
  while (tmp > 0)
  {
    snake[tmp] = snake[tmp - 1];
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
  snake[0] &= 0xff00;
  snake[0] += (ya << 4);
  snake[0] += r;
  tmp = 3;
  while(tmp < sneklength){
    if((snake[0]&0x7fff) == (snake[tmp++]&0x7fff)){
      wall = 1;
    }
  }
  if((snake[0]&0x7fff) == (food&0x7fff)){
    sneklength++;
    hardfade = 0;
    if(speedup){
      if((sneklength%2) == 0){
        speed--;
        if(speed < maxspeed)  speed = maxspeed;
      }
    }
    snake[sneklength - 1] = last;
    addfood();
    addcube(snake[0]);
  }else{
    removecube(last);
    addcube(snake[0]);
  }
  return wall;
}

void disappear_food(void) // changes food coordinates
{
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
  if(nameorgame == 0 && timeoutcounter == 1){
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
  
  if(diff == 3 && ready == 1 && timeoutcounter != speed){
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

void testest(void) // tests
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

void display_score(void) // displays score for ~2 seconds
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

void board_init(void) // initializes gameplay board
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

void snek_init(void) // puts snake on board
{
  int a = 0, c = 0x4072;
  int buttons = getbtns();
  display_snek(snekboard);

  board_init();

  while (a < sneklength) // snake init
  {
    snake[a++] = c;
    c += ((2) << 8);
  }
  a--;
  while (a >= 0) // add snake
  {
    addcube(snake[a--]);
  }
  display_snek(snekboard);
}

int resetmenu(void) // menu shown on reset
{
  nameorgame = 2;
  display_clear();
  char r[] = "    Retry";
  char m[] = "    MainMenu";
  int choice = 1;
  display_string(1, r);
  display_string(2, m);
  display_update2(choice);
  display_image2(0, font);
  display_image2(96, font);
  while(1){
    int buttons = getbtns();
    while((buttons&0xf) == 0) buttons = getbtns();
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
    display_image2(0, font);
    display_image2(96, font);

    while ((*((volatile int *)0xbf8860d0) >> 5) & 7)
      ;
    while ((0x1 & (PORTF >> 1)) == 1)
      ;
  }
}

int reset_routine(void) // reset routine
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

void snek(void) // controls and gameplay control of game
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
  hardfade = 0;
  ready = 1;
  quicksleep(100);
  addfood();
  hardfade = 0;

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
      int switches = getsw();
      if((switches&0xf) == 2){
        ready = 0;
        nameorgame = 2;
        display_image2(32, pause1);
        display_image2(64, pause2);
        while((switches&0xf) == 2){
        switches = getsw();
      } 
      }
      nameorgame = 0;
      ready = 1;
      int re = 0;
      if((snake[sneklength-1]&0xffff) == (food&0xffff)){
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
      int resetcheck = reset_routine();
      restart = 0;
      if(resetcheck) return;
    }

    // while ((*((volatile int *)0xbf8860d0) >> 5) & 7)
    //   ;
    // while ((0x1 & (PORTF >> 1)) == 1)
    //   ;
  }
}

void instructions(void) // instructions for controls in program
{
  int instruct = 0;
  display_string(0, " <INSTRUCTIONS>0");
  display_string(1, "BTN1: go to");
  display_string(2, "next instruction");
  display_string(3, "BTN2: previous");
  display_update();

  int switches = getsw();
  while(switches&0x1){
    switches = getsw();
    int buttons = getbtns();
    while((buttons&0xf) == 0 && (switches&0x1)) {
      buttons = getbtns();
      switches = getsw();
    }
    if(buttons == 8){
      instruct++;
    }
    if(buttons == 1){
      instruct--;
    }
    if(instruct < 0 || instruct > 5){
      instruct = 0;
    }
    if(instruct == 0){
      display_string(0, " <INSTRUCTIONS>0");
      display_string(1, "BTN1: go to");
      display_string(2, "next instruction");
      display_string(3, "BTN2: previous");
    }
    if(instruct == 1){
      display_string(0, "     <MENU>    1");
      display_string(1, "BTN3: move up");
      display_string(2, "BTN4: move down");
      display_string(3, "BTN1: select");
    }
    if(instruct == 2){
      display_string(0, "     <GAME>    2");
      display_string(1, "BTN1: move right");
      display_string(2, "BTN2: move left");
      display_string(3, "BTN3: move up");
    }
    if(instruct == 3){
      display_string(0, "     <GAME>    3");
      display_string(1, "BTN4: move down");
      display_string(2, "SW2: pause game");
      display_string(3, " ");
    }
    if(instruct == 4){
      display_string(0, "     <NAME>    4");
      display_string(1, "BTN1: del char");
      display_string(2, "BTN3: sel char");
      display_string(3, " ");
    }
    if(instruct == 5){
      display_string(0, "     <NAME>    5");
      display_string(1, "BTN2: go right");
      display_string(2, "BTN4: go left");
      display_string(3, "SW4: finish");
    }
    display_update();
    while ((*((volatile int *)0xbf8860d0) >> 5) & 7);
    while ((0x1 & (PORTF >> 1)) == 1);
  }
  quicksleep(1000);
}

int mainmenu(void) // main menu
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
    int switches = getsw();
    while((buttons&0xf) == 0) {
      buttons = getbtns();
      switches = getsw();
      if(switches&0x1) {
        instructions();
        display_string(namechoice, n);
        display_string(gamechoice, g);
        display_string(diffchoice, d);
        display_string(scorechoice, s);
        display_update2(choice);
        display_image2(0, snekfont);
        display_image2(96, snekfont);
      }
    }
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

void set_difficulty(void) // difficulty settings
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
      quicksleep(100);
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

void show_scorelist(void) // shows stored scores
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
