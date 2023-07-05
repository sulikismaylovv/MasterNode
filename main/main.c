#include <stdio.h>
#include <unistd.h>
#include <pthread.h>
#include <inttypes.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"
#include "esp_pthread.h"
#include "tcp_server.c"
#include "https_request.c"

//SemaphoreHandle_t bin_sem = NULL;     // Waits for parameter to be read
//static SemaphoreHandle_t mutex;       // Lock access to buffer and Serial
int buf[BUFF_SIZE];             // Shared buffer

static void* SQL_mngr(void *arg){
    TaskHandle_t myTaskHandleSQL = NULL;

    // Create the task with the port parameter
    int port = 7777;  // Example port value
    int data = 0;
    https_request_params_t task_params = {
        .port = port,
        .data = data,
        .database_buffer = {buf[0], buf[1], buf[2], buf[3]}
    };

    while(1){
    //vmesto NULL napishi &task_params
    xTaskCreate(&https_request_task, "https_get_task", 8192, &task_params, 5, &myTaskHandleSQL);
    //xSemaphoreGive(bin_sem);
    for (int countdown = 5; countdown >= 0; countdown--) {
        vTaskDelay(500 / portTICK_PERIOD_MS);}
    }
    return NULL;
}

static void* LDR_mngr(void *arg){
    int data = 0;
    https_request_params_t task_params = {
        .port = 8888,
        .data = data,
        .database_buffer = {buf[0], buf[1], buf[2], buf[3]}
    };
    tcp_server_task((void*) &task_params);
    return NULL;
}

static void* NTC_mngr(void *arg){
    int data = 0;
    https_request_params_t task_params = {
        .port = 8887,
        .data = data,
        .database_buffer = {buf[0], buf[1], buf[2], buf[3]}
    };
    tcp_server_task((void*) &task_params);
    return NULL;
}

static void* DOOR_mngr(void *arg){
    int data = 0;
    https_request_params_t task_params = {
        .port = 8886,
        .data = data,
        .database_buffer = {buf[0], buf[1], buf[2], buf[3]}
    };
    tcp_server_task((void*) &task_params);
    return NULL;
}

void app_main(void)
{
    //clean up and establish connection
    ESP_ERROR_CHECK(nvs_flash_init());
    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());

    /* This helper function configures Wi-Fi or Ethernet, as selected in menuconfig.
     * Read "Establishing Wi-Fi or Ethernet Connection" section in
     * examples/protocols/README.md for more information about this function.
     */
    ESP_ERROR_CHECK(example_connect());


    pthread_attr_t attr;
    pthread_t thread1, thread2 , thread3 , thread4;
    int res;

    if (esp_reset_reason() == ESP_RST_POWERON) {
        //ESP_LOGI(TAG_HTTP, "Updating time from NVS");
        ESP_ERROR_CHECK(update_time_from_nvs());
    }

    const esp_timer_create_args_t nvs_update_timer_args = {
            .callback = &fetch_and_store_time_in_nvs,
    };

    esp_timer_handle_t nvs_update_timer;
    ESP_ERROR_CHECK(esp_timer_create(&nvs_update_timer_args, &nvs_update_timer));
    ESP_ERROR_CHECK(esp_timer_start_periodic(nvs_update_timer, TIME_PERIOD));
    


    // Create a pthread with the default parameters
    res = pthread_attr_init(&attr);
    assert(res == 0);
    res = pthread_create(&thread1, &attr, NTC_mngr, NULL);
    assert(res == 0);
    //printf("Created thread 0x%"PRIx32"\n", thread1);

    // Create a pthread with a larger stack size using the standard API
    pthread_attr_setstacksize(&attr, 16384);
    res = pthread_create(&thread2, &attr, LDR_mngr, NULL);
    assert(res == 0);
    //printf("Created larger stack thread 0x%"PRIx32"\n", thread2);

    res = pthread_create(&thread3, &attr, DOOR_mngr, NULL);
    assert(res == 0);
    //printf("Created second larger stack thread 0x%"PRIx32"\n", thread3);

    
    //if http request breaks a lot , comment out these 3 lines below
    res = pthread_create(&thread4, &attr, SQL_mngr, NULL);
    assert(res == 0);
    printf("Created second larger stack thread 0x%"PRIx32"\n", thread4);

    // Create mutexes and semaphores before starting tasks
    //bin_sem = xSemaphoreCreateBinary();
    //mutex = xSemaphoreCreateMutex();

    res = pthread_join(thread1, NULL);
    assert(res == 0);
    res = pthread_join(thread2, NULL);
    assert(res == 0);
    res = pthread_join(thread3, NULL);
    assert(res == 0);
    //and then these 2
    res = pthread_join(thread4, NULL);
    assert(res == 0);
    //printf("Threads have exited\n\n");

}
