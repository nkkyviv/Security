#include "server.h"
#include <Arduino.h>
#include "protocol.h"

/*arduino esp32 core temperature*/
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
        // Session successfully established
    }

    return true; // Session already established
}

bool close_session(void)
{
    if (session_established)
    {
        session_established = false;
    }

    return true; // No session to close
}


void server_init(uint32_t speed)
{
    protocol_init(speed);
}

void server_run()
{
    char command[30];

    protocol_receive((uint8_t *)command, sizeof(command) - 1); // -1 to leave space for the null terminator

    command[sizeof(command) - 1] = '\0';

    Serial.println("Received command: ");

    if (strcmp(command, "establish_session") == 0)
    {

        bool status = establish_session();

        protocol_send((uint8_t *)&status, sizeof(status));

        Serial.flush();
    }

    else if (strcmp(command, "get_temp") == 0)
    {

        float temperature = getTemperature();

        protocol_send(reinterpret_cast<uint8_t *>(&temperature), sizeof(temperature)); // send temp to client using

        Serial.flush();
    }

    else if (strcmp(command, "close_session") == 0)
    {
        bool status = close_session();
        protocol_send((uint8_t *)&status, sizeof(status));

        Serial.flush();
    }

    else
    {
        ;
    }
}




