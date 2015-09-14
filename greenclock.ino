
/* To set time: (from TimeSerial.pde)
 * Messages consist of the letter T followed by ten digit time (as seconds since Jan 1 1970)
 * you can send the text on the next line using Serial Monitor to set the clock to noon Jan 1 2013
 T1357041600  
 *
 * A Processing example sketch to automatically send the messages is inclided in the download
 * On Linux, you can use "date +T%s\n > /dev/ttyACM0" (UTC time zone)
*/ 

#include <Bounce2.h>        // Buttons to set the time locally
#include <Time.h>  
//#include <TeensyRTC.h>
#define TIME_HEADER  "T"   // Header tag for serial time sync message
#define TIME_REQUEST  7    // ASCII bell character requests a time sync message 

int number = 0;
int led = 13;
int h = 1;
int m = 1;
int h1=1, h2=1, m1=1, m2=1;
int t[ 4 ] = { 1, 2, 3, 4 };

boolean colon = false;
byte segmentClock = 17;
byte segmentLatch = 19;
byte segmentData = 23;
byte segmentDataBack = 22;
byte segmentOE = 16;

Bounce btnH = Bounce();  // if a button is too "sensitive"
Bounce btnM = Bounce();  // to rapid touch, you can increase this time.

void setup()
{
  Serial.begin(9600);
  Serial.println("Large Digit Driver");

  pinMode(segmentClock, OUTPUT);
  pinMode(segmentData, OUTPUT);
  pinMode(segmentLatch, OUTPUT);
  pinMode(segmentOE, OUTPUT);
  pinMode(segmentDataBack, INPUT);
  pinMode(led, OUTPUT);
  
  pinMode(2, INPUT_PULLUP);
  pinMode(3, INPUT_PULLUP);
  btnH.attach(2);
  btnH.interval(10);
  btnM.attach(3);
  btnM.interval(10);
  
  digitalWrite(segmentClock, LOW);
  digitalWrite(segmentData, LOW);
  digitalWrite(segmentLatch, LOW);
  digitalWrite(segmentOE, LOW);
  
  setSyncProvider(requestSync);  //set function to call when sync required
  setSyncInterval(60);
  //setTime(1442119490);
  setTime(requestSync()); 
  //Teensy3Clock.set(1442119490);
}

void loop()
{ 
  if (Serial.available()) {
    time_t clk = processSyncMessage();
    if (clk != 0) {
      Teensy3Clock.set(clk); // set the RTC
      setTime(clk); // set the sys clock
    }
  }
  
  btnH.update();
  btnM.update();
  
  //if (timeStatus() == timeSet) 
    h = hour();
    m = minute();
    t[0] = h/10;
    t[1] = h%10;
    t[2] = m/10;
    t[3] = m%10;
  
  //if (m%10 == 0) setTime(requestSync());  //sync with the RTC every 10 mins
  showNumber(t, colon);
  
  //buttons to set the clock, m and h
   /* if (btnH.read() == LOW) {
      setTime(now()+3600);
      Teensy3Clock.set(now());
    }

    if (btnM.read() == LOW) {
      setTime(now()+60);
      Teensy3Clock.set(now());
    }*/
    if (btnH.read() == LOW) {
      Teensy3Clock.set(now()+3600);
      setTime(Teensy3Clock.get());
    }

    if (btnM.read() == LOW) {
      Teensy3Clock.set(now()+60);
      setTime(Teensy3Clock.get());
    }
    
  colon = second()%2;
  digitalWrite(led, colon);
  delay(200);
}

//Takes array of 4 clock numbers and displays
void showNumber(int value[], boolean colon)
{
  for (byte x = 0 ; x < 4 ; x++)
  { 
    int v = value[x];
    postNumber(v, colon);
    Serial.println(v);
  }

  //Latch the current segment data
  digitalWrite(segmentLatch, LOW);
  digitalWrite(segmentLatch, HIGH); //Register moves storage register on the rising edge of RCK
}

//Given a number, or '-', shifts it out to the display
void postNumber(byte number, boolean decimal)
{
  //    -  A
  //   / / F/B
  //    -  G
  //   / / E/C
  //    -. D/DP

#define a  1<<0
#define b  1<<1
#define c  1<<2
#define d  1<<3
#define e  1<<4
#define f  1<<5
#define g  1<<6
#define dp 1<<7

  byte segments;

  switch (number)
  {
    case 1: segments = b | c; break;
    case 2: segments = a | b | d | e | g; break;
    case 3: segments = a | b | c | d | g; break;
    case 4: segments = f | g | b | c; break;
    case 5: segments = a | f | g | c | d; break;
    case 6: segments = a | f | g | e | c | d; break;
    case 7: segments = a | b | c; break;
    case 8: segments = a | b | c | d | e | f | g; break;
    case 9: segments = a | b | c | d | f | g; break;
    case 0: segments = a | b | c | d | e | f; break;
    case ' ': segments = 0; break;
    case 'c': segments = g | e | d; break;
    case '-': segments = g; break;
  }

  if (decimal) segments |= dp;

  //Clock these bits out to the drivers
  for (byte x = 0 ; x < 8 ; x++)
  {
    digitalWrite(segmentClock, LOW);
    digitalWrite(segmentData, segments & 1 << (7 - x));
    digitalWrite(segmentClock, HIGH); //Data transfers to the register on the rising edge of SRCK
  }
}

unsigned long processSyncMessage() {
  unsigned long pctime = 0L;
  const unsigned long DEFAULT_TIME = 1357041600; // Jan 1 2013

  if(Serial.find(TIME_HEADER)) {
     pctime = Serial.parseInt();
     if( pctime <= DEFAULT_TIME) { // check the integer is a valid time (greater than Jan 1 2013)
       pctime = 0L;
     }
  }
  return (pctime);
}

time_t requestSync()
{
  return Teensy3Clock.get();
}

