#include <Arduino.h>
#include "session.h"

void setup()
{
  session_init();
  pinMode(GPIO_NUM_32, OUTPUT);
  pinMode(GPIO_NUM_21, OUTPUT);
}

void loop()
{
  static uint8_t state = LOW;
  uint8_t response[7] = {SESSION_OKAY};

  int req = session_request();

  digitalWrite(GPIO_NUM_32, LOW);

  switch (req)
  {
  case SESSION_TEMPERATURE:
    sprintf((char *)&response[1], "%2.2f", temperatureRead());
    if (!session_response(response, sizeof(response)))
    {
      req = SESSION_ERROR;
    }
    break;

  case SESSION_TOGGLE_LED:
    state = (state == LOW) ? HIGH : LOW;
    digitalWrite(GPIO_NUM_21, state);
    response[1] = digitalRead(GPIO_NUM_21);
    if (state != response[1])
    {
      response[0] = SESSION_ERROR;
    }
    if (!session_response(response, sizeof(response)))
    {
      req = SESSION_ERROR;
    }
    break;
  default:
    break;
  }

  if (req == SESSION_ERROR)
  {
    digitalWrite(GPIO_NUM_32, HIGH);
  }
}