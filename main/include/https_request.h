#ifndef https_variables
#define https_variables

#include <pthread.h>

extern pthread_mutex_t mutexlock;

int* return_buffer();

//takes html response and extracts number as database from it
static void parse_html_response(const char* response, int* parsed_ints);

//standardget request function
static void https_get_request(esp_tls_cfg_t cfg, const char *WEB_SERVER_URL, const char *REQUEST , int database_buffer[]);

//get request using certificate value
static void https_get_request_using_crt_bundle(int port , int data , int database_buffer[]);


static void https_request_task(void *pvparameters);



#endif 