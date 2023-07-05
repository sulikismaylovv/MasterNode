| Supported Targets | ESP32 | ESP32-C2 | ESP32-C3 | ESP32-S2 | ESP32-S3 |
| ----------------- | ----- | -------- | -------- | -------- | -------- |

### Software Implementation 

The ESP32S3 is programmed using ESP-IDF development tool. The code is divided in 3 different sections: initialization , threads creation and threads execution. The complete flow diagram of ESP32S3 is shown in Figure 1.



The initialization section creates various parameters required for the proper functioning of the master system, such as a list of HTTP requests, threads, and thread parameters (including mutexes, attributes, and thread sizes), as well as port numbers. This section also performs clean-up operations and ensures that the WiFi connection is established correctly.

Thread initialization section is mainly responsible for creating 4 threads (3 threads for 3 slave nodes and 1 thread for database connection). In the master node, the thread used for establishing a database connection was implemented with a standard thread size, whereas the threads utilized for communication with slave nodes were allocated a larger stack size. This is due to the substantial computational workload and data management requirements of the slave node communication threads in comparison to the database connection thread. Both types of threads are responsible for performing two tasks: the Transmission Control Protocol (TCP) server task, and generating Hypertext Transfer Protocol (HTTP) GET requests to ensure synchronization of the database.

The TCP server task creates a TCP socket with the specified port number and waits for a connection request from the client. After accepting a request from the client, connection between server and client is established and the application waits for some data to be received from the client. Received data is analyzed and the command is calculated. Once ready, the command is transmitted back to the client.

HTTP GET request function retrieves data from a remote server using the HTTP protocol. The code first creates a specific GET request with the required headers and parameters using the OpenSSL library. The request is then sent to the server, and the response is received and parsed to extract the relevant data. The retrieved data is then printed to the console for further analysis. This approach provides a secure and efficient way to retrieve data from remote servers. This code is used for both pushing data to the database as well as pulling data from it and updating shared buffer.

In order to maintain data consistency and prevent race conditions, mutexes were implemented as a more robust and reliable multi-threaded mechanism. Specifically, mutexes are important in both the TCP server and HTTP GET request tasks, whenever a thread must request access to the shared buffer before being allowed to modify its contents. 

### Configure the project

```
idf.py menuconfig
```
Open the project configuration menu (`idf.py menuconfig`) to configure Wi-Fi or Ethernet. See "Establishing Wi-Fi or Ethernet Connection" section in [examples/protocols/README.md](../../README.md) for more details. You can also configure the port numbers for specific slave nodes.

#### Configuring Client Session Tickets

Note: This example has client session tickets enabled by default.

* Open the project configuration menu (`idf.py menuconfig`)
* In the `Component Config` -> `ESP-TLS` submenu, select the `Enable client session tickets` option.

Ensure that the server has the session tickets feature enabled.

### Build and Flash

Build the project and flash it to the board, then run monitor tool to view serial output:

idf.py -p PORT flash monitor

(Replace PORT with the name of the serial port to use.)

