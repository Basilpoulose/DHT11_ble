#include <stdio.h>
#include <unistd.h>
#include "driver/gpio.h"

#define DHT_OK 0
#define DHT_ERROR_TIMEOUT -1
#define DHT_ERROR_CHECKSUM -2
#define DHT_CHECK_PERIOD_US 2
#define DHT_CHECK_TIMEOUT_US 1000

#define pin GPIO_NUM_4

int wait_gpio_level(gpio_num_t pin, int level, unsigned int timeout)
{
    unsigned int response_time = timeout;
    while (gpio_get_level(pin) == level)
    {
        if (response_time == 0)
        {
            printf("Error: Timeout\n");
            return DHT_ERROR_TIMEOUT;
        }
        usleep(DHT_CHECK_PERIOD_US);
        response_time--;
    }
    return DHT_OK;
}

int dht_read(gpio_num_t pin, int dht_type, float *humidity, float *temperature)
{
    unsigned int data[5] = {0};
    unsigned int cnt = 7;
    unsigned int idx = 0;

    // Start signal
    gpio_set_direction(pin, GPIO_MODE_OUTPUT);
    gpio_set_level(pin, 0);
    usleep(2000);  // 20ms
    gpio_set_level(pin, 1);
    usleep(40);     // 40us
    gpio_set_direction(pin, GPIO_MODE_INPUT);

    if (wait_gpio_level(pin, 0, DHT_CHECK_TIMEOUT_US) != DHT_OK)
    {
        return DHT_ERROR_TIMEOUT;
    }

    if (wait_gpio_level(pin, 1, DHT_CHECK_TIMEOUT_US) != DHT_OK)
    {
        return DHT_ERROR_TIMEOUT;
    }

    // Read data
    for (int i = 0; i < 40; i++)
    {
        unsigned int response_time = 1000;
        while (gpio_get_level(pin) == 0)
        {
            if (response_time == 0)
            {
                printf("Error: Timeout\n");
                return DHT_ERROR_TIMEOUT;
            }
            usleep(DHT_CHECK_PERIOD_US);
            response_time--;
        }
        response_time = 1000;
        while (gpio_get_level(pin) == 1)
        {
            if (response_time == 0)
            {
                printf("Error: Timeout\n");
                return DHT_ERROR_TIMEOUT;
            }
            usleep(DHT_CHECK_PERIOD_US);
            response_time--;
        }

        if (response_time > 40)
        {
            data[idx] |= (1 << cnt);
        }

        if (cnt == 0)
        {
            cnt = 7;
            idx++;
        }
        else
        {
            cnt--;
        }
    }

    if (data[0] + data[1] + data[2] + data[3] != data[4])
    {
        return DHT_ERROR_CHECKSUM;
    }

    if (dht_type == 11)
    {
        *humidity = (float)data[0];
        *temperature = (float)data[2];
    }
    else if (dht_type == 22)
    {
        *humidity = ((float)(data[0] << 8) + data[1]) / 10.0f;
        *temperature = ((float)((data[2] & 0x7F) << 8) + data[3]) / 10.0f;
        if (data[2] & 0x80)
        {
            *temperature *= -1.0f;
        }
    }
    else
    {
        return DHT_ERROR_CHECKSUM;
    }
    
    return DHT_OK;
}

void app_main()
{
    printf("DHT11/DHT22 reading\n");
    float humidity = 0.0f;
    float temperature = 0.0f;
    int result = dht_read(pin, 22, &humidity, &temperature);  // Change the second parameter to 11 for DHT11
    if (result == DHT_OK)
    {
        printf("Temperature: %.1f°C, Humidity: %.1f%%\n", temperature, humidity);
    }
    else if (result == DHT_ERROR_TIMEOUT)
    {
        printf("Error: Timeout\n");
    }
    else if (result == DHT_ERROR_CHECKSUM)
    {
        printf("Error: Checksum mismatch\n");
    }
    else
    {
        printf("Error: Unknown error\n");
    }
}
