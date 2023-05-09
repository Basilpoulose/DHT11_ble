#include <stdio.h>
#include "driver/gpio.h"
#include <unistd.h>
#define DHT11_OK             0
#define DHT11_ERROR_TIMEOUT -1
#define DHT11_ERROR_CHECKSUM -2
#define DHT11_CHECK_PERIOD_US 2
#define DHT11_CHECK_TIMEOUT_US 1000
int wait_gpio_level(gpio_num_t pin,int level, unsigned int timeout){
    unsigned int response_time=timeout;
    while(gpio_get_level(pin) == level){
        if(response_time --==0){
            printf("eroor");
            return DHT11_ERROR_TIMEOUT;
        }
        usleep(DHT11_CHECK_PERIOD_US);
    }
    return DHT11_OK;
}

int dht11_read(gpio_num_t pin, float *humidity, float *temperature) {
    unsigned int arry[5];
    unsigned int cnt = 7;
    unsigned int idx = 0;

    //  start signal
    gpio_set_direction(gpio_num_t pin , GPIO_MODE_OUTPUT);
    gpio_set_level(pin, 0);
    usleep(1800);
    gpio_set_level(pin, 1);
    usleep(40);
    gpio_set_direction(gpio_num_t pin, GPIO_MODE_INPUT);
    if(wait_gpio_level(gpio_num_t pin,0,DHT11_CHECK_TIMEOUT_US) !=DHT11_OK){
        return DHT11_ERROR_TIMEOUT;
    }
    if(wait_gpio_level(gpio_num_t pin,1,DHT11_CHECK_TIMEOUT_US) !=DHT11_OK){
        return DHT11_ERROR_TIMEOUT;
    }

    
    // read data
    for (int i = 0; i < 40; i++) {
        unsigned int response_time = 1000;
        while (gpio_get_level(pin) == 0) {
            if (response_time--==0) {
                printf("error");
                return DHT11_ERROR_TIMEOUT;
            }
            usleep(DHT11_CHECK_PERIOD_US);
        }
        if (response_time > 40) {
            arry[idx] |= (1 << cnt);
        }
        if (cnt == 0) {
            cnt = 7;
            idx++;
        } else {
            cnt--;
        }
    }

    if (arry[0] + arry[1] + arry[2] + arry[3] != arry[4]) {
        return DHT11_ERROR_CHECKSUM;
    }

    *humidity = (float)arry[0];
    *temperature = (float)arry[2];

    return DHT11_OK;
}
void app_main() {
    printf("DHT11 reading\n");
    float humidity = 0.0f;
    float temperature = 0.0f;
    gpio_num_t pin = GPIO_NUM_4;
    int result = dht11_read(GPIO_NUM_18, &humidity, &temperature);
    if (result == DHT11_OK) {
        printf("Temperature: %.1f C, Humidity: %.1f%%\n", temperature, humidity);
    } else {
        printf("Error reading DHT11 sensor: %d\n", result);
    }
}