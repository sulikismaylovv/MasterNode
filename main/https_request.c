/*
 * HTTPS GET Example using plain Mbed TLS sockets
 *
 * Contacts the howsmyssl.com API via TLS v1.2 and reads a JSON
 * response.
 *
 * Adapted from the ssl_client1 example in Mbed TLS.
 *
 * SPDX-FileCopyrightText: The Mbed TLS Contributors
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * SPDX-FileContributor: 2015-2022 Espressif Systems (Shanghai) CO LTD
 */

#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include <stdlib.h>
#include <inttypes.h>
#include <time.h>
#include <sys/time.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_log.h"
#include "esp_system.h"
#include "esp_timer.h"
#include "nvs_flash.h"
#include "nvs.h"
#include "protocol_examples_common.h"
#include "esp_sntp.h"
#include "esp_netif.h"

#include "lwip/err.h"
#include "lwip/sockets.h"
#include "lwip/sys.h"
#include "lwip/netdb.h"
#include "lwip/dns.h"

#include "esp_tls.h"
#include "sdkconfig.h"
#if CONFIG_MBEDTLS_CERTIFICATE_BUNDLE
#include "esp_crt_bundle.h"
#endif
#include "time_sync.h"
#include "url_handle.c"
#include <https_request.h>
#include <pthread.h>
static const char *TAG_HTTP = "HTTP_TO_SQL";

#define MAX_RESPONSE_SIZE 1024

char response[MAX_RESPONSE_SIZE];

pthread_mutex_t mutexlock = PTHREAD_MUTEX_INITIALIZER;

/* Timer interval once every day (24 Hours) */
#define TIME_PERIOD (86400000000ULL)
//database buffer
//int database_buffer[BUFF_SIZE];


/* Root cert for howsmyssl.com, taken from server_root_cert.pem

   The PEM file was extracted from the output of this command:
   openssl s_client -showcerts -connect www.howsmyssl.com:443 </dev/null

   The CA root cert is the last cert given in the chain of certs.

   To embed it in the app binary, the PEM file is named
   in the component.mk COMPONENT_EMBED_TXTFILES variable.
*/
extern const uint8_t server_root_cert_pem_start[] asm("_binary_server_root_cert_pem_start");
extern const uint8_t server_root_cert_pem_end[]   asm("_binary_server_root_cert_pem_end");

extern const uint8_t local_server_cert_pem_start[] asm("_binary_local_server_cert_pem_start");
extern const uint8_t local_server_cert_pem_end[]   asm("_binary_local_server_cert_pem_end");

#ifdef CONFIG_EXAMPLE_CLIENT_SESSION_TICKETS
static esp_tls_client_session_t *tls_client_session = NULL;
static bool save_client_session = false;
#endif

int database_buffer[BUFF_SIZE] ,send_bf[BUFF_SIZE];


static void parse_html_response(const char* response, int* parsed_ints) {
    // Find the position of the opening and closing HTML tags
    const char* start_tag = "<html>";
    const char* end_tag = "</html>";
    const char* start_pos = strstr(response, start_tag);
    const char* end_pos = strstr(response, end_tag);

    if (start_pos == NULL || end_pos == NULL) {
        //printf("Error: HTML tags not found in response\n");
        return;
    }

    // Calculate the length of the data between the HTML tags
    size_t data_len = end_pos - start_pos - strlen(start_tag);

    // Allocate a buffer to store the data
    char* data_buf = (char*) malloc(data_len + 1);

    if (data_buf == NULL) {
        //printf("Error: Failed to allocate memory for data buffer\n");
        return;
    }

    // Copy the data between the HTML tags to the buffer
    memcpy(data_buf, start_pos + strlen(start_tag), data_len);
    data_buf[data_len] = '\0';

    // Split the data into four integers and store them in the array
    //printf("%s\n" ,data_buf);
    sscanf(data_buf, "%1d%1d%1d%1d", &parsed_ints[0], &parsed_ints[1], &parsed_ints[2], &parsed_ints[3]);

    // Free the data buffer
    free(data_buf);
}

static void https_get_request(esp_tls_cfg_t cfg, const char *WEB_SERVER_URL, const char *REQUEST , int database_buffer[])
{
    char buf[512];
    int ret, len;

    esp_tls_t *tls = esp_tls_init();
    if (!tls) {
        //ESP_LOGE(TAG_HTTP, "Failed to allocate esp_tls handle!");
        goto exit;
    }

    if (esp_tls_conn_http_new_sync(WEB_SERVER_URL, &cfg, tls) == 1) {
        //ESP_LOGI(TAG_HTTP, "Connection established...");
    } else {
        //ESP_LOGE(TAG_HTTP, "Connection failed...");
        goto cleanup;
    }

#ifdef CONFIG_EXAMPLE_CLIENT_SESSION_TICKETS
    /* The TLS session is successfully established, now saving the session ctx for reuse */
    if (save_client_session) {
        esp_tls_free_client_session(tls_client_session);
        tls_client_session = esp_tls_get_client_session(tls);
    }
#endif

    size_t written_bytes = 0;
    do {
        ret = esp_tls_conn_write(tls,
                                 REQUEST + written_bytes,
                                 strlen(REQUEST) - written_bytes);
        if (ret >= 0) {
            //ESP_LOGI(TAG_HTTP, "%d bytes written", ret);
            written_bytes += ret;
        } else if (ret != ESP_TLS_ERR_SSL_WANT_READ  && ret != ESP_TLS_ERR_SSL_WANT_WRITE) {
            //ESP_LOGE(TAG_HTTP, "esp_tls_conn_write  returned: [0x%02X](%s)", ret, esp_err_to_name(ret));
            goto cleanup;
        }
    } while (written_bytes < strlen(REQUEST));

    //ESP_LOGI(TAG_HTTP, "Reading HTTP response...");

    if(strcmp(REQUEST , SQL_REQUEST ) == 0 ){
        len = sizeof(buf) - 1;
        memset(buf, 0x00, sizeof(buf));
        ret = esp_tls_conn_read(tls, (char *)buf, len);

        if (ret == ESP_TLS_ERR_SSL_WANT_WRITE  || ret == ESP_TLS_ERR_SSL_WANT_READ) {
            
        } else if (ret < 0) {
        //ESP_LOGE(TAG_HTTP, "esp_tls_conn_read  returned [-0x%02X](%s)", -ret, esp_err_to_name(ret));
            goto cleanup;
        } else if (ret == 0) {
            //ESP_LOGI(TAG_HTTP, "connection closed");
            goto cleanup;
        }

        len = ret;
        ///ESP_LOGD(TAG_HTTP, "%d bytes read", len);
        int parsed_ints[4] = {0};
        parse_html_response(buf, parsed_ints);
        for (int i = 0; i< BUFF_SIZE ; i++){
            database_buffer[i] = parsed_ints[i];
            send_bf[i] = parsed_ints[i];
        }
        //printf("Parsed integers: %d %d %d %d\n", database_buffer[0], database_buffer[1], database_buffer[2], database_buffer[3]);
        }
    else{
        //printf("Pushed to SQL\n");
    }

cleanup:
    esp_tls_conn_destroy(tls);
exit:
    // for (int countdown = 10; countdown >= 0; countdown--) {
    //     printf("%d...\n", countdown);
    //     vTaskDelay(1000 / portTICK_PERIOD_MS);
    // }
    return;
}

#if CONFIG_MBEDTLS_CERTIFICATE_BUNDLE
static void https_get_request_using_crt_bundle(int port , int data , int database_buffer[])
{
    //ESP_LOGI(TAG_HTTP, "https_request using crt bundle");
    esp_tls_cfg_t cfg = {
        .crt_bundle_attach = esp_crt_bundle_attach,
    };
    //case here to ask for certain http address
    //printf("PORT: %d \n" , port);
    switch(port){
        case 8888:
        //printf("HEre \n");
            if (data < 19){
                char url_LED[200];
                char led_request[200];
                https_get_request(cfg, create_url(port  , data%10 , url_LED , sizeof(url_LED)), create_http_request(port , data , led_request , sizeof(led_request)) ,database_buffer);
            }
            else{
                char url_SHUTTER[200];
                char SHUTTER_request[200];
                https_get_request(cfg, create_url(port  , data%10 , url_SHUTTER , sizeof(url_SHUTTER)), create_http_request(port , data , SHUTTER_request , sizeof(SHUTTER_request)) ,  database_buffer);

            }
            break;
        case 8887:
            if(data < 6){
            char url_NTC[200];
            char NTC_request[200];
            https_get_request(cfg, create_url(port  , data , url_NTC , sizeof(url_NTC)), create_http_request(port , data , NTC_request , sizeof(NTC_request)) , database_buffer);
            }
            else{
                char url_TEMP[200];
                char TEMP_request[200];
                https_get_request(cfg, create_url(port  , data , url_TEMP , sizeof(url_TEMP)), create_http_request(port , data , TEMP_request , sizeof(TEMP_request)) , database_buffer);
            }
           break;
        case 8886:
            char url_DOOR[200];
            char DOOR_request[200];
            https_get_request(cfg, create_url(port  , data , url_DOOR , sizeof(url_DOOR)), create_http_request(port , data , DOOR_request , sizeof(DOOR_request)) , database_buffer);
            break;
        default:
            https_get_request(cfg, WEB_URL_SQL, SQL_REQUEST , database_buffer);
            
            break;

            
    }
    //https_get_request(cfg, WEB_URL_SQL, SQL_REQUEST);
}
#endif // CONFIG_MBEDTLS_CERTIFICATE_BUNDLE


static void https_request_task(void *pvparameters)
{
    //static pthread_mutex_t mutex;

    pthread_mutex_lock(&mutexlock);
    https_request_params_t *req_params = (https_request_params_t *)pvparameters;
    int port = req_params->port;
    int data = req_params->data;
    memcpy(database_buffer, req_params->database_buffer, sizeof(req_params->database_buffer));
    //ESP_LOGI(TAG_HTTP, "Start https_request example");

#ifdef CONFIG_EXAMPLE_CLIENT_SESSION_TICKETS
    char *server_url = NULL;
#ifdef CONFIG_EXAMPLE_LOCAL_SERVER_URL_FROM_STDIN
    char url_buf[SERVER_URL_MAX_SZ];
    if (strcmp(CONFIG_EXAMPLE_LOCAL_SERVER_URL, "FROM_STDIN") == 0) {
        example_configure_stdin_stdout();
        fgets(url_buf, SERVER_URL_MAX_SZ, stdin);
        int len = strlen(url_buf);
        url_buf[len - 1] = '\0';
        server_url = url_buf;
    } else {
        //ESP_LOGE(TAG_HTTP, "Configuration mismatch: invalid url for local server");
        abort();
    }
    //printf("\nServer URL obtained is %s\n", url_buf);
#else
    server_url = CONFIG_EXAMPLE_LOCAL_SERVER_URL;
#endif /* CONFIG_EXAMPLE_LOCAL_SERVER_URL_FROM_STDIN */
    https_get_request_to_local_server(server_url);
    https_get_request_using_already_saved_session(server_url);
#endif

#if CONFIG_MBEDTLS_CERTIFICATE_BUNDLE
    https_get_request_using_crt_bundle(port , data, database_buffer);
    //ESP_LOGI(TAG_HTTP, "Finish https_request example");
#endif
    //ESP_LOGI(TAG_HTTP, "Minimum free heap size: %" PRIu32 " bytes", esp_get_minimum_free_heap_size());
    //https_get_request_using_cacert_buf();
    //https_get_request_using_global_ca_store();
    //ESP_LOGI(TAG_HTTP, "Finish https_request example");
    pthread_mutex_unlock(&mutexlock);
    vTaskDelete(NULL);
}

int* return_buffer(){
    return send_bf;
}

