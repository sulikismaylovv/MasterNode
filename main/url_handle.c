#include <stdio.h>
#include <string.h>
#include <http_urls.h>

char* create_http_request(int port ,int data, char *http_request, size_t max_len) {
    
    if (strlen(SQL_REQUEST) + 1 + 1 > max_len) {
        printf("Error: max_len is too small!\n");
        return NULL;
    }

    switch(port){
        case 8888:
            if(data < 19){
                snprintf(http_request, max_len, "GET %s%d HTTP/1.1\r\n"
                                   "Host: %s\r\n"
                                   "User-Agent: esp-idf/1.0 esp32\r\n"
                                   "\r\n", WEB_URL_LED, data, WEB_SERVER);
            }
            else{
                snprintf(http_request, max_len, "GET %s%d HTTP/1.1\r\n"
                                   "Host: %s\r\n"
                                   "User-Agent: esp-idf/1.0 esp32\r\n"
                                   "\r\n", WEB_URL_SHUTTER, data, WEB_SERVER);
            }
            break;
        case 8887:
            if(data <6){
            snprintf(http_request, max_len, "GET %s%d HTTP/1.1\r\n"
                                   "Host: %s\r\n"
                                   "User-Agent: esp-idf/1.0 esp32\r\n"
                                   "\r\n", WEB_URL_NTC, data, WEB_SERVER);}
            else{
                snprintf(http_request, max_len, "GET %s%d HTTP/1.1\r\n"
                                   "Host: %s\r\n"
                                   "User-Agent: esp-idf/1.0 esp32\r\n"
                                   "\r\n", WEB_URL_TEMP, data, WEB_SERVER);

            }
            break;
        case 8886:
            snprintf(http_request, max_len, "GET %s%d HTTP/1.1\r\n"
                                   "Host: %s\r\n"
                                   "User-Agent: esp-idf/1.0 esp32\r\n"
                                   "\r\n", WEB_URL_DOOR, data, WEB_SERVER);
            break;
        default:
            snprintf(http_request, max_len, "GET %s HTTP/1.1\r\n"
                                   "Host: %s\r\n"
                                   "User-Agent: esp-idf/1.0 esp32\r\n"
                                   "\r\n", WEB_URL_SQL, WEB_SERVER);
            break;
    }

    return http_request;
    
}

char* create_url(int port ,int data, char *WEB_URL, size_t max_len){
        if (strlen(WEB_URL) + 1 + 1 > max_len) {
        printf("Error: max_len is too small!\n");
        return NULL;
    }

    switch(port){
        case 8888:
            if(data < 19){
                sprintf(WEB_URL, "%s%d", WEB_URL_LED, data);
            }
            else{
                sprintf(WEB_URL, "%s%d", WEB_URL_SHUTTER, data);
            }
            break;
        case 8887:
            if(data < 6){
            sprintf(WEB_URL, "%s%d", WEB_URL_NTC, data);}
            else{
                sprintf(WEB_URL, "%s%d", WEB_URL_TEMP, data);
            }
            break;
        case 8886:
            sprintf(WEB_URL, "%s%d", WEB_URL_DOOR, data);
            break;
        default:
           strcpy(WEB_URL, WEB_URL_SQL);
            break;
    }

    return WEB_URL;
}
