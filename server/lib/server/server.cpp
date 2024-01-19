#include "server.h"
#include <Arduino.h>
#include "protocol.h"
#include <string.h>

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

    if (length == 0)
    {
        sprintf(buffer, "%d", session_established);
    }
    else if (length == 1)
    {
        press_command(command, buffer);
    }
    else
    {
        ;
    }

    protocol_send((uint8_t *)buffer, strlen(buffer));
    memset(buffer, 0, sizeof(buffer));  

    // size_t length = protocol_receive((uint8_t *)command, sizeof(command) -1);

    // if (length == 0)
    // {
    //     sprintf(buffer, "%d", session_established);
    // }
    // else if (length == 1)
    // {
    //     press_command(command, buffer);
    // }
    // else
    // {
    //     ;
    // }

    //    protocol_send((uint8_t *)buffer, strlen(buffer));
}