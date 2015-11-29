// Do not remove the include below
#include "arduino_countdown_watch.h"

#include "Arduino.h"
#include "Blanking.h"
#include "Timer.h"

#include "Watch.h"
#include <TM1638.h>

#include <Cmd.h>

// Define a TM1638 module on data pin 3 (DIO, yellow), clock pin 2 (CLK, brown), strobe pin 4 (STB0, green)
TM1638* tm1638 = 0;

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
 * TM1638 LEDs: (obsolete)
 * - L8: Seconds selected
 * - L4 & L5: Minutes selected
 * - L1: Hours selected
 */

word dots = 0;
char sign = ' ';
char text[17];

Blanking* dispPartBlanking = 0;

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
Timer* dbgPrintTimer = 0;


unsigned int mmiInterval = 200;
class MmiTimerAdapter : public TimerAdapter
{
public:
  void timeExpired()
  {
    handleMMI();
  }
};
Timer* mmiTimer = 0;


Watch* watch = 0;
wTime* t;
byte buttons = 0;


void hello(int arg_cnt, char **args)
{
  Serial.println("Hello world.");
}

void debug(int arg_cnt, char **args)
{
  if ((0 != dbgPrintTimer) && (arg_cnt > 1))
  {
    if (strcmp(args[1], "on") == 0)
    {
      dbgPrintTimer->startTimer(oneSecMillis);
    }
    else
    {
      dbgPrintTimer->cancelTimer();
    }
  }
}

void watchCtrl(int arg_cnt, char **args)
{
  if ((0 != watch) && (arg_cnt > 1))
  {
    if (strcmp(args[1], "run") == 0)
    {
      watch->run();
    }
    else if (strcmp(args[1], "stop") == 0)
    {
      watch->stop();
    }
    else if (strcmp(args[1], "load") == 0)
    {
      watch->load();
    }
    else if (strcmp(args[1], "store") == 0)
    {
      watch->store();
    }
    else if (strcmp(args[1], "sel") == 0)
    {
      if (arg_cnt > 2)
      {
        if (strcmp(args[2], "up") == 0)
        {
          watch->partSelUp();
        }
        else if (strcmp(args[2], "down") == 0)
        {
          watch->partSelDn();
        }
      }
    }
    else if (strcmp(args[1], "incr") == 0)
    {
      watch->incrSet();
    }
    else if (strcmp(args[1], "decr") == 0)
    {
      watch->decrSet();
    }

  }
}


//The setup function is called once at startup of the sketch
void setup()
{
  tm1638 = new TM1638(3, 2, 4);

  cmdAdd("hello", hello);
  cmdAdd("debug", debug);
  cmdAdd("watch", watchCtrl);

  // Initialize ArduinoCmd lib and open the serial port at 115200 bps:
  cmdInit(115200);

  watch = new Watch();
  watch->setSig(1);
  watch->setHrs(0);
  watch->setMin(0);
  watch->setSec(10);
  watch->store();

  dispPartBlanking = new Blanking();
  mmiTimer = new Timer(new MmiTimerAdapter(), Timer::IS_RECURRING, mmiInterval);
  dbgPrintTimer = new Timer(new DbgPrintTimerAdapter(), Timer::IS_RECURRING, 0);

  pinMode(LED_BUILTIN, OUTPUT);
}

void handleMMI()
{
  handleButtons();
  handleDisplay();
  digitalWrite(LED_BUILTIN, watch->isRunning());
}

void handleButtons()
{
  if (0 != tm1638 && 0 != watch)
  {
    byte newButtons = tm1638->getButtons();
    if (newButtons != buttons)
    {
      buttons = newButtons;
      if (buttons & 0x40)
      {
        watch->stop();
      }
      else if (buttons & 0x80)
      {
        watch->run();
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
  }
}

void handleDisplay()
{
  dots = 0x14;

  if ((0 != tm1638) && (0 != watch) && (0 != dispPartBlanking))
  {
    t = watch->getTime();
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

/*
 SerialEvent occurs whenever a new data comes in the
 hardware serial RX.  This routine is run between each
 time loop() runs, so using delay inside loop can delay
 response.  Multiple bytes of data may be available.
 */
void serialEvent()
{
  cmdPoll();
}

