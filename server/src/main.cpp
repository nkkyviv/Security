#include <Arduino.h>
#include "server.h"

void setup()
{
  server_init(115200);
}

void loop()
{
  server_run();
}