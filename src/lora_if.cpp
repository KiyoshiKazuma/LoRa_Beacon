#include "lora_if.h"
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

LoRa::LoRa(
        uint8_t uartNum,
        uint16_t rxPin,
        uint16_t txPin,
        uint16_t m0Pin,
        uint16_t m1Pin)
    :
      loraSerial(uartNum),
      u2_pin_rx(rxPin),
      u2_pin_tx(txPin),
      u2_pin_m0(m0Pin),
      u2_pin_m1(m1Pin),
      ioMutex(nullptr),
      isConfigMode(false),
      initialized(false)
{
}

LoRa::~LoRa()
{
    if (initialized)
    {
        loraSerial.end();
    }

    if (ioMutex != nullptr)
    {
        vSemaphoreDelete(ioMutex);
    }
}

bool LoRa::begin(uint32_t baudRate, uint32_t serialConfig)
{
    if (initialized)
    {
        return true;
    }

    ioMutex = xSemaphoreCreateMutex();
    if (ioMutex == nullptr)
    {
        return false;
    }

    pinMode(u2_pin_m0, OUTPUT);
    pinMode(u2_pin_m1, OUTPUT);
    digitalWrite(u2_pin_m0, HIGH);
    digitalWrite(u2_pin_m1, HIGH);
    vTaskDelay(pdMS_TO_TICKS(100));
    isConfigMode = true;

    loraSerial.begin(baudRate, serialConfig, u2_pin_rx, u2_pin_tx);
    initialized = true;
    return true;
}

bool LoRa::writeConfig(const uint8_t *config, size_t length)
{
    size_t bytesWritten = 0;

    if (!initialized || config == nullptr || length == 0)
    {
        return false;
    }

    xSemaphoreTake(ioMutex, portMAX_DELAY);

    if (!isConfigMode)
    {
        set_configumode();
    }

    bytesWritten = loraSerial.write(config, length);
    loraSerial.flush();

    xSemaphoreGive(ioMutex);

    return bytesWritten == length;
}

bool LoRa::sendMessage(const uint8_t *message, size_t length)
{
    size_t bytesWritten = 0;

    if (!initialized || message == nullptr || length == 0)
    {
        return false;
    }

    xSemaphoreTake(ioMutex, portMAX_DELAY);

    if (isConfigMode)
    {
        set_transmitmode();
    }


    bytesWritten = loraSerial.write(message, length);
    loraSerial.flush();

    xSemaphoreGive(ioMutex);

    return bytesWritten == length;
}

size_t LoRa::receiveConfigResponse(uint8_t *buffer, size_t length)
{
    size_t bytesRead = 0;

    if (!initialized || buffer == nullptr || length == 0)
    {
        return 0;
    }

    xSemaphoreTake(ioMutex, portMAX_DELAY);

    if (!isConfigMode)
    {
        set_configumode();
    }

    bytesRead = loraSerial.readBytes(buffer, length);
    xSemaphoreGive(ioMutex);

    return bytesRead;
}

size_t LoRa::receiveMessage(uint8_t *buffer, size_t length)
{
    size_t bytesRead = 0;

    if (!initialized || buffer == nullptr || length == 0)
    {
        return 0;
    }

    xSemaphoreTake(ioMutex, portMAX_DELAY);
    
    if (isConfigMode)
    {
        set_transmitmode();
    }

    bytesRead = loraSerial.readBytes(buffer, length);
    xSemaphoreGive(ioMutex);

    return bytesRead;
}

size_t LoRa::receiveAvailable(uint8_t *buffer, size_t length)
{
    int availableBytes = 0;
    size_t bytesToRead = 0;
    size_t bytesRead = 0;

    if (!initialized || buffer == nullptr || length == 0)
    {
        return 0;
    }

    xSemaphoreTake(ioMutex, portMAX_DELAY);

    if (isConfigMode)
    {
        set_transmitmode();
    }

    availableBytes = loraSerial.available();
    if (availableBytes > 0)
    {
        bytesToRead = static_cast<size_t>(availableBytes) < length
                          ? static_cast<size_t>(availableBytes)
                          : length;
        bytesRead = loraSerial.readBytes(buffer, bytesToRead);
    }

    xSemaphoreGive(ioMutex);

    return bytesRead;
}

void LoRa::set_configumode(void)
{
    digitalWrite(u2_pin_m0, HIGH);
    digitalWrite(u2_pin_m1, HIGH);
    vTaskDelay(pdMS_TO_TICKS(100));
    isConfigMode = true;
}

void LoRa::set_transmitmode(void)
{
    digitalWrite(u2_pin_m0, LOW);
    digitalWrite(u2_pin_m1, LOW);
    vTaskDelay(pdMS_TO_TICKS(100));
    isConfigMode = false;
}