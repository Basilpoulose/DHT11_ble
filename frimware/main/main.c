#include <stdio.h>
#include "driver/gpio.h"
#include <unistd.h>
#define DHT_PIN GPIO_NUM_4

/*#define GPIO_OUTPUT_IO_0   4

#define GPIO_OUTPUT_PIN_SEL  (1ULL<<GPIO_OUTPUT_IO_0)

#define GPIO_INPUT_IO_0     4

#define GPIO_INPUT_PIN_SEL  (1ULL<<GPIO_INPUT_IO_0)


// cofigure gpio pin 

void config_in()
{
        gpio_config_t dht_conf ;
        dht_conf.mode = GPIO_MODE_INPUT,
        dht_conf.pin_bit_mask = GPIO_INPUT_PIN_SEL,
        dht_conf.pull_down_en = 0,
        dht_conf.pull_up_en = 0;

    gpio_config(&dht_conf);
}


void config_out()
{
        gpio_config_t dht_conf ;
        dht_conf.mode = GPIO_MODE_OUTPUT,
        dht_conf.pin_bit_mask = GPIO_OUTPUT_PIN_SEL,
        dht_conf.pull_down_en = 0,
        dht_conf.pull_up_en = 0;

    gpio_config(&dht_conf);
}*/




void dht_read(uint8_t* data)
{
    uint8_t last_read = 1;
    uint8_t counter = 0;
    uint8_t j = 0;
    uint8_t i;

    data[0]=data[1]=data[2]=data[3]=data[4]=0;

    //config_out();
    gpio_set_direction(DHT_PIN, GPIO_MODE_OUTPUT);
    gpio_set_level(DHT_PIN,0);
    usleep(2000);

    gpio_set_level(DHT_PIN,1);
    usleep(40);

    //config_in();
    gpio_set_direction(DHT_PIN, GPIO_MODE_INPUT);

    for(i=0;i<=85;i++)
    {
        counter=0;
        while(gpio_get_level(DHT_PIN)==last_read)
        {
            counter++;
            usleep(1);
            if(counter==255)
            break;
        }
        last_read = gpio_get_level(DHT_PIN);

        if (counter == 255)
            break;

        if ((i >= 4) && (i % 2 == 0)) {
            data[j / 8] <<= 1;
            if (counter > 16)
                data[j / 8] |= 1;
            j++;
            
        }
    }

}
void app_main() {
    uint8_t data[5];

    float temp, humidity;

    while (1) {
        dht_read(data);

        humidity = data[0]+ data[1] * 0.1;
        temp = data[2];
        if(data[3] & 0x80)
        {
            temp = -1 - temp;

        }
        temp += (data[3] & 0x0f) * 0.1;

        printf("Temperature: %.1f °C\n", temp);
        printf("Humidity: %.1f %%\n", humidity);

        sleep(2);
    }
}