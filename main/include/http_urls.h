#ifndef variables
#define variables
#define BUFF_SIZE 4 // Size of buffer array that will taken from SQL database

typedef struct {
    int port;
    int data;
    int database_buffer[BUFF_SIZE];
} https_request_params_t;

/* Constants that aren't configurable in menuconfig */
#define WEB_SERVER "a22-home5.studev.groept.be"
#define WEB_PORT "443"
#define WEB_URL_SQL "https://a22-home5.studev.groept.be/dataOut.php?row=1"
#define WEB_URL_TEMP "https://a22-home5.studev.groept.be/tempIn.php?temp="
#define WEB_URL_NTC "https://a22-home5.studev.groept.be/dataIn.php?row=4&id="
#define WEB_URL_DOOR "https://a22-home5.studev.groept.be/dataIn.php?row=3&id="
#define WEB_URL_SHUTTER "https://a22-home5.studev.groept.be/dataIn.php?row=2&id="
#define WEB_URL_LED "https://a22-home5.studev.groept.be/dataIn.php?row=1&id="
#define WEB_URL_NAME "https://a22-home5.studev.groept.be/nameIn.php?name="


//to concatinate
//int data = 1;
//char url[100];  // buffer to store the concatenated string
//sprintf(url, "%s%d", WEB_URL_LED, data);

#define SERVER_URL_MAX_SZ 256

//SQL request
static const char SQL_REQUEST[] = "GET " WEB_URL_SQL " HTTP/1.1\r\n"
                             "Host: "WEB_SERVER"\r\n"
                             "User-Agent: esp-idf/1.0 esp32\r\n"
                             "\r\n";

//TEMP post
static const char TEMP_REQUEST[] = "GET " WEB_URL_TEMP " HTTP/1.1\r\n"
                             "Host: "WEB_SERVER"\r\n"
                             "User-Agent: esp-idf/1.0 esp32\r\n"
                             "\r\n";


//NTC post
static const char NTC_REQUEST[] = "GET " WEB_URL_NTC " HTTP/1.1\r\n"
                             "Host: "WEB_SERVER"\r\n"
                             "User-Agent: esp-idf/1.0 esp32\r\n"
                             "\r\n";

//LED post
static const char LED_REQUEST[] = "GET " WEB_URL_LED " HTTP/1.1\r\n"
                             "Host: "WEB_SERVER"\r\n"
                             "User-Agent: esp-idf/1.0 esp32\r\n"
                             "\r\n";

//SHUTTER post
static const char SHUTTER_REQUEST[] = "GET " WEB_URL_SHUTTER " HTTP/1.1\r\n"
                             "Host: "WEB_SERVER"\r\n"
                             "User-Agent: esp-idf/1.0 esp32\r\n"
                             "\r\n";

//DOOR post
static const char DOOR_REQUEST[] = "GET " WEB_URL_DOOR " HTTP/1.1\r\n"
                             "Host: "WEB_SERVER"\r\n"
                             "User-Agent: esp-idf/1.0 esp32\r\n"
                             "\r\n";

#ifdef CONFIG_EXAMPLE_CLIENT_SESSION_TICKETS
static const char LOCAL_SRV_REQUEST[] = "GET " CONFIG_EXAMPLE_LOCAL_SERVER_URL " HTTP/1.1\r\n"
                             "Host: "WEB_SERVER"\r\n"
                             "User-Agent: esp-idf/1.0 esp32\r\n"
                             "\r\n";
#endif

#endif 