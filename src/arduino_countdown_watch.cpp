// Do not remove the include below
#include "arduino_countdown_watch.h"

#include "Blanking.h"
#include "Timer.h"

#include "Watch.h"
#include <TM1638.h>

// Define a TM1638 module on data pin 3 (DIO, yellow), clock pin 2 (CLK, brown), strobe pin 4 (STB0, green)
TM1638* tm1638 = new TM1638(3, 2, 4);

/**
 * TM1638 Buttons:
 * - B8: START
 * - B7: STOP
 * - B6: STORE
 * - B5: LOAD
 * - B4: PART_SEL_DN
 * - B3: PART_SEL_UP
 * - B2: DECR_SET
 * - B1: INCR_SET
 */

/**
 * TM1638 LEDs:
 * - L8: Seconds selected
 * - L4 & L5: Minutes selected
 * - L1: Hours selected
 */

word dots = 0;
char sign = ' ';
char text[17];

Blanking* dispPartBlanking   = new Blanking();

const unsigned int DEBOUNCE_TIME_MILLIS = 200;

void dbgPrinter();
void handleMMI();
void handleButtons();
void handleDisplay();

unsigned int oneSecMillis = 1000;
class DbgPrintTimerAdapter : public TimerAdapter
{
public:
  void timeExpired()
  {
    dbgPrinter();
  }
};
Timer* dbgPrintTimer = new Timer(new DbgPrintTimerAdapter(), Timer::IS_RECURRING, oneSecMillis);

unsigned int mmiInterval = 200;
class MmiTimerAdapter : public TimerAdapter
{
public:
  void timeExpired()
  {
    handleMMI();
  }
};
Timer* mmiTimer = new Timer(new MmiTimerAdapter(), Timer::IS_RECURRING, mmiInterval);


Watch* watch = new Watch();
wTime* t;
byte buttons = 0;

//The setup function is called once at startup of the sketch
void setup()
{
  watch->setSig(1);
  watch->setHrs(0);
  watch->setMin(0);
  watch->setSec(10);
  watch->store();

  // open the serial port at 115200 bps:
  Serial.begin(115200);
}

void handleMMI()
{
  handleButtons();
  t = watch->getTime();
  handleDisplay();
}

void handleButtons()
{
  buttons = tm1638->getButtons();
  if (buttons & 0x80)
  {
    watch->run();
  }
  else if (buttons & 0x40)
  {
    watch->stop();
  }
  else if (buttons & 0x20)
  {
    watch->store();
  }
  else if (buttons & 0x10)
  {
    watch->load();
  }
  else if (buttons & 0x04)
  {
    watch->partSelUp();
  }
  else if (buttons & 0x08)
  {
    watch->partSelDn();
  }
  else if (buttons & 0x01)
  {
    watch->incrSet();
  }
  else if (buttons & 0x02)
  {
    watch->decrSet();
  }
}

void handleDisplay()
{
  dots = 0x14;
  sign = t->sign ? '-' : ' ';

  if (watch->isHrsSelected())
  {
    if (dispPartBlanking->isSignalBlanked())
    {
      dots = 0x04;
    }
    else
    {
      dots = 0x34;
    }
  }
  else if (watch->isMinSelected())
  {
    if (dispPartBlanking->isSignalBlanked())
    {
      dots = 0x10;
    }
    else
    {
      dots = 0x1C;
    }
  }
  else if (watch->isSecSelected())
  {
    if (dispPartBlanking->isSignalBlanked())
    {
      dots = 0x14;
    }
    else
    {
      dots = 0x17;
    }
  }
  else if (watch->isSigSelected())
  {
    if (dispPartBlanking->isSignalBlanked())
    {
      sign = t->sign ? '-' : ' ';
    }
    else
    {
      sign = t->sign ? '=' : '_';
    }
  }

  sprintf(text, " %c%02u%02u%02u", sign, t->h, t->m, t->s);
  tm1638->setDisplayToString(text, dots);
}

void dbgPrinter()
{
  sprintf(text, "%c%02u:%02u:%02u", t->sign ? '-' : ' ' , t->h, t->m, t->s);
  Serial.println(text);
  Serial.println(watch->currentStateString());

  Serial.print(watch->isSigSelected() ? "-/+." : "---.");
  Serial.print(watch->isHrsSelected() ? "HRS." : "---.");
  Serial.print(watch->isMinSelected() ? "MIN." : "---.");
  Serial.println(watch->isSecSelected() ? "SEC" : "---");
}

// The loop function is called in an endless loop
void loop()
{
  yield();
}

