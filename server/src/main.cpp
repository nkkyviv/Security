#include <Arduino.h>
#include "session.h"
// #include "ser.h"

#define LED_PIN 21

// Variable to keep track of LED state
int ledState = LOW;

static void toggle_led(void)
{
  ledState = !ledState;
  digitalWrite(LED_PIN, ledState);
}

void setup()
{
  // ser_init();
  pinMode(LED_PIN, OUTPUT); 
  pinMode(GPIO_NUM_21, OUTPUT);
  delay(100);
  session_init();
}

void loop()
{
  // ser_run();
  response_t resp = {0};

  int req = session_request();

  digitalWrite(GPIO_NUM_32, LOW);

  if (req == SESSION_TEMPERATURE)
  {
    resp.data[0] = SESSION_OKAY;
    sprintf((char *)&resp.data[1], "%2.2f", temperatureRead());

    req = session_response(&resp);
  }
  else if (req == SESSION_TOGGLE_LED)
  {
    resp.data[0] = SESSION_OKAY;
    toggle_led();

    sprintf((char *)&resp.data[1], "%d", ledState);
    req = session_response(&resp);
  }

  if (req == SESSION_ERROR)
  {
    digitalWrite(GPIO_NUM_32, HIGH);
  }
}
