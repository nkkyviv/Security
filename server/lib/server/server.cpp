#include "server.h"
#include <Arduino.h>
#include "protocol.h"
#include <string.h>

#define LED 21

bool session_established = false;

#ifdef __cplusplus
extern "C"
{
#endif
    uint8_t temprature_sens_read();
#ifdef __cplusplus
}
#endif

float getTemperature()
{
    uint8_t raw = temprature_sens_read();
    float temperature = (raw - 32) / 1.8;

    return temperature;
}

bool establish_session(void)
{
    if (!session_established)
    {
        session_established = true;
    }

    return true;
}

bool toggle_led(void)
{
    pinMode(LED, OUTPUT);
    static bool led_state = false;
    if (!led_state)
    {
        digitalWrite(LED, HIGH);
        led_state = true;
    }
    else if (led_state)
    {
        digitalWrite(LED, LOW);
        led_state = false;
    }

    return led_state;
}

bool close_session(void)
{
    if (session_established)
    {
        session_established = false;
    }

    return true;
}

void press_command(char *command, char *buffer)
{
    if (strcmp(command, "1") == 0)
    {
        sprintf(buffer, "%d", establish_session());
    }
    else if (strcmp(command, "2") == 0)
    {
        sprintf(buffer, "%.1f", getTemperature());
    }
    else if (strcmp(command, "3") == 0)
    {
        sprintf(buffer, "%d", toggle_led());
    }
    else if (strcmp(command, "4") == 0)
    {
        sprintf(buffer, "%d", close_session());
    }
    else
    {
        ;
    }
}

void server_init(uint32_t speed)
{
    protocol_init(speed);
    delay(1000);
}

void server_run()
{
    char buffer[8] = {0};
    char command[4] = {0};

    size_t length = protocol_receive((char *)command, sizeof(command) - 1);

    if (length == 1)
    {
        press_command(command, buffer);
    }
    else
    {
        protocol_receive((char *)buffer, sizeof(buffer) - 1);
    }

    size_t size = protocol_send((uint8_t *)buffer, strlen(buffer));

    if (size != strlen(buffer))
    {
        protocol_send((uint8_t *)buffer, strlen(buffer));
    }
    else
    {
        ;
    }
}