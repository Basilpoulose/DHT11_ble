#include <stdio.h>
#include <unistd.h>
#include "driver/gpio.h"

#define DHT11_OK 0
#define DHT11_ERROR_TIMEOUT -1
#define DHT11_ERROR_CHECKSUM -2
#define DHT11_CHECK_PERIOD_US 2
#define DHT11_CHECK_TIMEOUT_US 1000

gpio_num_t pin = GPIO_NUM_4;

int wait_gpio_level(gpio_num_t pin, int level, unsigned int timeout)
{
    unsigned int response_time = timeout;
    while (gpio_get_level(pin) == level)
    {
        if (response_time == 0)
        {
            printf("Error: Timeout\n");
            return DHT11_ERROR_TIMEOUT;
        }
        usleep(DHT11_CHECK_PERIOD_US);
        response_time--;
    }
    return DHT11_OK;
}

int dht11_read(gpio_num_t pin, float *humidity, float *temperature)
{
    unsigned int arry[5] = {0};
    unsigned int cnt = 7;
    unsigned int idx = 0;
    
    // Start signal
    gpio_set_direction(pin, GPIO_MODE_OUTPUT);
    gpio_set_level(pin, 0);
    usleep(20000);  // 20ms
    gpio_set_level(pin, 1);
    usleep(40);     // 40us
    gpio_set_direction(pin, GPIO_MODE_INPUT);
    
    if (wait_gpio_level(pin, 0, DHT11_CHECK_TIMEOUT_US) != DHT11_OK)
    {
        return DHT11_ERROR_TIMEOUT;
    }
    
    if (wait_gpio_level(pin, 1, DHT11_CHECK_TIMEOUT_US) != DHT11_OK)
    {
        return DHT11_ERROR_TIMEOUT;
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
                return DHT11_ERROR_TIMEOUT;
            }
            usleep(DHT11_CHECK_PERIOD_US);
            response_time--;
        }
        response_time = 1000;
        while (gpio_get_level(pin) == 1)
        {
            if (response_time == 0)
            {
                printf("Error: Timeout\n");
                return DHT11_ERROR_TIMEOUT;
            }
            usleep(DHT11_CHECK_PERIOD_US);
            response_time--;
        }
        
        if (response_time > 40)
        {
            arry[idx] |= (1 << cnt);
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

    if (arry[0] + arry[1] + arry[2] + arry[3] != arry[4])
    {
        return DHT11_ERROR_CHECKSUM;
    }

    *humidity = (float)arry[0];
    *temperature = (float)arry[2];
    return DHT11_OK;
}

void app_main()
{
    printf("DHT11 reading\n");
    float humidity = 0.0f;
    float temperature = 0.0f;
    int result = dht11_read(pin, &humidity, &temperature);
      if (result == DHT11_OK)
    {
        printf("Temperature: %.1f°C, Humidity: %.1f%%\n", temperature, humidity);
    }
    else if (result == DHT11_ERROR_TIMEOUT)
    {
        printf("Error: Timeout\n");
    }
    else if (result == DHT11_ERROR_CHECKSUM)
    {
        printf("Error: Checksum mismatch\n");
    }
    else
    {
        printf("Error: Unknown error\n");
    }
}
