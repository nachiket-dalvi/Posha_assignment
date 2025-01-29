/* Posha Assignment
   Author: Nachiket Dalvi

    -Receives data from the PC on uart
    -Stores the data on SPI flash present on esp32(no eeprom on esp32)
    -Reads from the flash and retransmits the data back to the PC

    For accessing the flash storage, a partition of 1MB in the flash is created, the partition table is in 'partitions.csv'

.
*/
#include <stdio.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/uart.h"
#include "driver/gpio.h"
#include "sdkconfig.h"
#include "esp_log.h"
#include "driver/gptimer.h"
#include "esp_spiffs.h"




#define ECHO_TEST_RTS (UART_PIN_NO_CHANGE)
#define ECHO_TEST_CTS (UART_PIN_NO_CHANGE)

#define UART_PORT      UART_NUM_2
#define BAUD_RATE     2400
#define TASK_STACK_SIZE    (CONFIG_EXAMPLE_TASK_STACK_SIZE)

static const char *TAG = "UART TEST";



#define BUF_SIZE (50)

int count;
uint64_t stime,ctime;
float transmission_speed;       //Was earlier being used to calculate the speed to receive the data from the UART buffer(NOT speed of transmission as such)
char writedata[50];             //buffer to store the data incoming from the PC

 gptimer_handle_t gptimer = NULL;                                   //Timer config
    gptimer_config_t timer_config = {
    .clk_src = GPTIMER_CLK_SRC_DEFAULT,
    .direction = GPTIMER_COUNT_UP,
    .resolution_hz = 1 * 1000 * 1000, // 1MHz, 1 tick = 1us
    };



static void echo_task(void *arg)
{
    /* Configure parameters of an UART driver,
     * communication pins and install the driver */
    uart_config_t uart_config = {
        .baud_rate = BAUD_RATE,
        .data_bits = UART_DATA_8_BITS,
        .parity    = UART_PARITY_DISABLE,
        .stop_bits = UART_STOP_BITS_1,
        .flow_ctrl = UART_HW_FLOWCTRL_DISABLE,
        .source_clk = UART_SCLK_DEFAULT,
    };
    int intr_alloc_flags = 0;

#if CONFIG_UART_ISR_IN_IRAM
    intr_alloc_flags = ESP_INTR_FLAG_IRAM;
#endif

    ESP_ERROR_CHECK(uart_driver_install(UART_PORT, 1024 * 2, 0, 0, NULL, intr_alloc_flags));
    ESP_ERROR_CHECK(uart_param_config(UART_PORT, &uart_config));
    ESP_ERROR_CHECK(uart_set_pin(UART_PORT, 17, 16, ECHO_TEST_RTS, ECHO_TEST_CTS));

    // Configure a temporary buffer for the incoming data
    uint8_t *data = (uint8_t *) malloc(BUF_SIZE);

    while (1) {
        // Read data from the UART
    
        FILE *f = fopen("/storage/myfile.txt","w");             /*Opening the file to write, this also resets the previously stored data. */
        fclose(f);
        int len = uart_read_bytes(UART_PORT, &writedata, (BUF_SIZE), 100 / portTICK_PERIOD_MS);     /*Reading from UART buffer*/
        //printf("bytes %d\n",len);

        while(len>0)                                
        {
            FILE *f = fopen("/storage/myfile.txt","a");             /*Opening the file again to append, so it does not reset previous data*/
            if(f==NULL)
            {
                printf("ERROR opening the file\n"); //Handle this case
            }
            else{
  
                fprintf(f,writedata);
                fclose(f);
                memset(writedata,0,BUF_SIZE);       //clear write buffer


            }          
            len = uart_read_bytes(UART_PORT, &writedata, (BUF_SIZE), 100 / portTICK_PERIOD_MS);
            //printf("bytes %d\n",len);
            writedata[len+1] = '\0';

        }
        /*When the code comes out of this loop means there is no more data to receive as of now*/
        

        /*Reading data from the file that was modified*/
        char line[100];
        FILE *g = fopen("/storage/myfile.txt","r");
        if(g == NULL)
        {
            printf("failed to open the file\n");
        }
        else{
            
            while(fgets(line, sizeof(line), g)!=NULL){

                //printf(line);
                uart_write_bytes(UART_PORT, line, sizeof(line));        //Sends 100 bytes at once
                memset(line,0,sizeof(line));
            }
            fclose(g);
             //printf("Done reading the file\n");
            //printf("%s\n", line);
        }
        

    }
}

void app_main(void)
{
   
   /*SPIFFS configuration*/
    esp_vfs_spiffs_conf_t config ={
        .base_path = "/storage",
        .partition_label = NULL,
        .max_files = 5,
        .format_if_mount_failed = true
    };
    esp_err_t result = esp_vfs_spiffs_register(&config);

    size_t total = 0, used = 0;
    result = esp_spiffs_info(config.partition_label, &total, &used);
    
    
    if(result != ESP_OK)
    {
        printf("Failed to get partition info\n");     
    }

    FILE *f = fopen("/storage/myfile.txt","r");             //Opening file to check previously stored data. 
    if(f == NULL)
    {
        printf("failed to open the file\n");
    }
    else{
        char line[64];
        fgets(line, sizeof(line), f);
        fclose(f);
        printf("%s\n", line);
    }
    ESP_ERROR_CHECK(gptimer_new_timer(&timer_config, &gptimer));            /*Timer initialisation,enabling and starting*/
    gptimer_enable(gptimer);                        
    gptimer_start(gptimer);
    xTaskCreate(echo_task, "uart_echo_task", TASK_STACK_SIZE, NULL, 10, NULL);  /*Task creation*/
}
