/* BSD Socket API Example

   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/
#include <string.h>
#include <stdbool.h>
#include <sys/param.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_log.h"
#include "nvs_flash.h"
#include "esp_netif.h"
#include "protocol_examples_common.h"
#include <stdio.h>
#include <unistd.h>
#include <pthread.h>

#include "lwip/err.h"
#include "lwip/sockets.h"
#include "lwip/sys.h"
#include <lwip/netdb.h>

#include "analyze.c"
#include "esp_tls.h"
#include <http_urls.h>
#include <https_request.h>
#include <ctype.h>


#define KEEPALIVE_IDLE              CONFIG_EXAMPLE_KEEPALIVE_IDLE
#define KEEPALIVE_INTERVAL          CONFIG_EXAMPLE_KEEPALIVE_INTERVAL
#define KEEPALIVE_COUNT             CONFIG_EXAMPLE_KEEPALIVE_COUNT


//mutex 
static pthread_mutex_t mutex;
int buffer_tcp[BUFF_SIZE] , old_buffer[BUFF_SIZE];             // Shared buffers
bool first = true;

static const char *TAG = "Master Node";


static void send_data(const int sock,int data , int len){
    int to_write = len;
    char rx_buffersend[128];
    sprintf(rx_buffersend , "%d" , data);
    //printf("number :%s \n" , rx_buffersend);
    while (to_write > 0) {
        int written = send(sock, rx_buffersend + (len - to_write), to_write, 0);
        if (written < 0) {
            //ESP_LOGI(TAG, "Error occurred during sending: errno %d", errno);
            break;
        }
        to_write -= written;
    }
}


static void check_buffer(const int sock , int PORT ,int old_buffer[] , pthread_mutex_t* mutex){
    //if(pthread_mutex_lock(mutex) == 0){
            int* Array_new = return_buffer();
            for (int i = 0; i < BUFF_SIZE; i++) {
                 buffer_tcp[i] = *(Array_new + i); // copy element i from intPtr to intArray
            }
            //printf("buffer_tcp integers: %d %d %d %d\n", buffer_tcp[0], buffer_tcp[1], buffer_tcp[2], buffer_tcp[3]);
            for (int i = 0; i < BUFF_SIZE; i++) {
                 if (buffer_tcp[i] != old_buffer[i]) {
                     if(i == 0 && PORT == 8888){
                        //printf("Send %d to port 8888 \n" ,buffer_tcp[i]+10 );
                        send_data(sock , buffer_tcp[i]+10 , sizeof(buffer_tcp[i]+10));
                        old_buffer[i] = buffer_tcp[i];
                     }
                     else if(i == 1 && PORT == 8888){
                        //printf("Send %d to port 8888 2 \n" ,buffer_tcp[i]+20 );
                        send_data(sock , buffer_tcp[i]+20 , sizeof(buffer_tcp[i]+20));
                        old_buffer[i] = buffer_tcp[i];
                     }
                     else if(i == 3 && PORT == 8887){
                        //printf("Send %d to port 8887 \n" ,buffer_tcp[i]);
                        send_data(sock , buffer_tcp[i] , sizeof(buffer_tcp[i]));
                        old_buffer[i] = buffer_tcp[i];
                     }
                     else if(i == 2 && PORT == 8886){
                        //printf("Send %d to port 8886 \n" ,buffer_tcp[i]);
                        send_data(sock , buffer_tcp[i] , sizeof(buffer_tcp[i]));
                        old_buffer[i] = buffer_tcp[i];
                     }

                 }
             }
             //do_retransmit(sock , PORT ,  buffer_tcp);

             //pthread_mutex_unlock(mutex);
        //}
}


static int do_retransmit(const int sock , int PORT , int buffer_send[] , pthread_mutex_t* mutex , int old_buffer[])
{
    int len;
    int data = 0;
    //rx_buffer 1 and rx_buffer 2 are used to retransmit data from master node to slave node
    char rx_buffer[128];
    TaskHandle_t myTaskHandleTCP = NULL;
    


    do 
    {
        //receive
        check_buffer(sock ,PORT , old_buffer , mutex);
        len = recv(sock, rx_buffer, sizeof(rx_buffer), MSG_DONTWAIT);
        if (len < 0) {
            if (errno == EAGAIN || errno == EWOULDBLOCK) {
            // No data available to read, continue with other processing
            } else {
            //ESP_LOGI(TAG, "Error occurred during receiving: errno %d", errno);
            return -1;
            }
        } else if (len == 0) {
            //ESP_LOGI(TAG, "Connection closed");
            return -1;
        } else {
            data = atoi(rx_buffer);
            ESP_LOGI(TAG, "Received %d bytes from port %d: %d as integer", len, PORT , data);
            memset(rx_buffer, 0, sizeof(rx_buffer)); // clear the char array

            //once received send back
            //case statement to determine what calculation to perform based on the port
            // send() can return less bytes than supplied length.
            // Walk-around for robust implementation.
            https_request_params_t task_params = {
                        .port = PORT,
                        .data = 0,
                        .database_buffer = {buffer_send[0], buffer_send[1], buffer_send[2], buffer_send[3]}
            };
            switch(PORT){
                //LDR node
                case 8888:
                    task_params.data = LDR_command(data);
                    //printf("Data : %d \n" , task_params.data);
                    xTaskCreate(&https_request_task, "https_get_task", 8192, &task_params, 5, &myTaskHandleTCP);
                    send_data(sock ,  LDR_command(data) , len);
                    
                    task_params.data = SHUTTER_command(data);
                    //printf("Data : %d \n" , task_params.data);                    
                    xTaskCreate(&https_request_task, "https_get_task", 8192, &task_params, 5, &myTaskHandleTCP);
                    send_data(sock , SHUTTER_command(data) , len);
                    
                    break;
                //NTC node
                case 8887:
                    ESP_LOGI(TAG , "Calculated Temperature : %.2f" , convert_to_temp(data));
                    task_params.data = STEPPER_command(convert_to_temp(data));
                    xTaskCreate(&https_request_task, "https_get_task", 8192, &task_params, 5, &myTaskHandleTCP);
                    send_data(sock , STEPPER_command(convert_to_temp(data)) , len);
                    task_params.data = convert_to_temp(data) ;
                    xTaskCreate(&https_request_task, "https_get_task", 8192, &task_params, 5, &myTaskHandleTCP);

                    break;
                
                //door node
                case 8886:
                    task_params.data = 0;
                    xTaskCreate(&https_request_task, "https_get_task", 8192, &task_params, 5, &myTaskHandleTCP);
                    //send_data(sock , data , len);

                    break;
                
                default:
                    break;

            }
        }
        vTaskDelay(pdMS_TO_TICKS(500));
    } 
    while (1);

    return 0;
}


static void *tcp_server_task(void *pvParameters)
{
    if(pthread_mutex_init (&mutex, NULL) != 0){
        //printf("Failed to initialize the spiffs mutex");
    }

    //do not change
    https_request_params_t *req_params = (https_request_params_t *)pvParameters;
    char addr_str[128];
    int PORT = req_params->port;
    memcpy(buffer_tcp, req_params->database_buffer, sizeof(req_params->database_buffer));
    //int* Array_new = return_buffer();
    if(first){
        //printf("here\n");
        int* Array_new2 = return_buffer();
        for (int i = 0; i < BUFF_SIZE; i++) {
                old_buffer[i] = *(Array_new2 + i); // copy element i from intPtr to intArray
            }
        first = false;
        //printf("NULL integers: %d %d %d %d\n", old_buffer[0], old_buffer[1], old_buffer[2], old_buffer[3]);
    }
    int addr_family = (int)AF_INET;
    int ip_protocol = 0;
    int keepAlive = 1;
    int keepIdle = KEEPALIVE_IDLE;
    int keepInterval = KEEPALIVE_INTERVAL;
    int keepCount = KEEPALIVE_COUNT;
    struct sockaddr_storage dest_addr;

    struct sockaddr_in *dest_addr_ip4 = (struct sockaddr_in *)&dest_addr;
    dest_addr_ip4->sin_addr.s_addr = htonl(INADDR_ANY);
    dest_addr_ip4->sin_family = AF_INET;
    dest_addr_ip4->sin_port = htons(PORT);
    ip_protocol = IPPROTO_IP;

    int listen_sock = socket(addr_family, SOCK_STREAM, ip_protocol);
    if (listen_sock < 0) {
        //ESP_LOGE(TAG, "Unable to create socket: errno %d", errno);
        vTaskDelete(NULL);
        return NULL;
    }
    int opt = 1;
    setsockopt(listen_sock, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    //ESP_LOGI(TAG, "Socket created");

    int err = bind(listen_sock, (struct sockaddr *)&dest_addr, sizeof(dest_addr));
    if (err != 0) {
        //ESP_LOGE(TAG, "Socket unable to bind: errno %d", errno);
        //ESP_LOGE(TAG, "IPPROTO: %d", addr_family);
        goto CLEAN_UP;
    }
    ESP_LOGI(TAG, "Socket bound, port %d", PORT);

    err = listen(listen_sock, 1);
    if (err != 0) {
        //ESP_LOGE(TAG, "Error occurred during listen: errno %d", errno);
        goto CLEAN_UP;
    }

    while (1) {

        //ESP_LOGI(TAG, "Socket listening");

        struct sockaddr_storage source_addr; // Large enough for both IPv4 or IPv6
        socklen_t addr_len = sizeof(source_addr);
        int sock = accept(listen_sock, (struct sockaddr *)&source_addr, &addr_len);
        if (sock < 0) {
            //ESP_LOGE(TAG, "Unable to accept connection: errno %d", errno);
            break;
        }

        // Set tcp keepalive option
        setsockopt(sock, SOL_SOCKET, SO_KEEPALIVE, &keepAlive, sizeof(int));
        setsockopt(sock, IPPROTO_TCP, TCP_KEEPIDLE, &keepIdle, sizeof(int));
        setsockopt(sock, IPPROTO_TCP, TCP_KEEPINTVL, &keepInterval, sizeof(int));
        setsockopt(sock, IPPROTO_TCP, TCP_KEEPCNT, &keepCount, sizeof(int));
        // Convert ip address to string
        if (source_addr.ss_family == PF_INET) {
            inet_ntoa_r(((struct sockaddr_in *)&source_addr)->sin_addr, addr_str, sizeof(addr_str) - 1);
        }

        //ESP_LOGI(TAG, "Socket accepted ip address: %s", addr_str);

        //change this part accordingly
        
        while(do_retransmit(sock , PORT ,  buffer_tcp , &mutex , old_buffer) != -1){
            //wait till connection closes
        }

        shutdown(sock, 0);
        close(sock);
    }

CLEAN_UP:
    close(listen_sock);
    vTaskDelete(NULL);

    if(pthread_mutex_destroy (&mutex) == 0){
        //printf("Mutex destruction failed");
    } 

    return NULL;
}


