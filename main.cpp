#include "mbed.h"
#include "HTS221Sensor.h"
#include "TCPSocket.h"
#include "ISM43362Interface.h"

static DevI2C devI2c(PB_11,PB_10);
static HTS221Sensor hum_temp(&devI2c);
ISM43362Interface wifi(false);

char Server_IP[] = "192.168.0.6";
int Server_PORT = 8000;

Thread thread;

void thread2(ISM43362Interface *net){
    TCPSocket socket;
    nsapi_error_t response;

    socket.open(net);
    response = socket.connect("192.168.255.40", 8000);
    if(0 != response) {
        printf("Error connecting: %d\n", response);
        socket.close();
        return;
    }

    char rbuffer[64];
    response = socket.recv(rbuffer, sizeof rbuffer);
    if (response < 0) {
        printf("Error receiving data: %d\n", response);
    } else {
        printf("recv %d [%.*s]\r\n", response, strstr(rbuffer, "\r\n")-rbuffer, rbuffer);
    }
 
    // Close the socket to return its memory and bring down the network interface
    socket.close();
}

void socket_demo(ISM43362Interface *net, float value1, float value2){
    TCPSocket socket;
    nsapi_error_t response;

    socket.open(net);
    response = socket.connect(Server_IP, Server_PORT);
    if(0 != response) {
        printf("Error connecting: %d\n", response);
        socket.close();
        return;
    }
    // Send a simple http request
    char sbuffer[] = "GET / HTTP/1.1\r\nHost: www.arm.com\r\n\r\n";
    nsapi_size_t size = strlen(sbuffer);
    response = 0;
    while(size)
    {
        response = socket.send(sbuffer+response, size);
        if (response < 0) {
            printf("Error sending data: %d\n", response);
            socket.close();
            return;
        } else {
            size -= response;
            // Check if entire message was sent or not
            printf("sent %d [%.*s]\r\n", response, strstr(sbuffer, "\r\n")-sbuffer, sbuffer);
        }
    }
 
    // Recieve a simple http response and print out the response line
    char rbuffer[64];
    response = socket.recv(rbuffer, sizeof rbuffer);
    if (response < 0) {
        printf("Error receiving data: %d\n", response);
    } else {
        printf("recv %d [%.*s]\r\n", response, strstr(rbuffer, "\r\n")-rbuffer, rbuffer);
    }
 
    // Close the socket to return its memory and bring down the network interface
    socket.close();
}

int main(){
    uint8_t id;
    float value1, value2;
    char buffer[20] = "";
    hum_temp.init(NULL);
    hum_temp.enable();
    hum_temp.read_id(&id);

    printf("HTS221:  [temp] %.2f C, [hum]   %.2f%%\r\n", value1, value2);

    printf("\nConnecting to %s...\n", MBED_CONF_APP_WIFI_SSID);
    int ret = wifi.connect(MBED_CONF_APP_WIFI_SSID, MBED_CONF_APP_WIFI_PASSWORD, NSAPI_SECURITY_WPA_WPA2);
    if (ret != 0) {
        printf("\nConnection error\n");
        return -1;
    }
 
    printf("Success\r\n");
    printf("MAC: %s\r\n", wifi.get_mac_address());
    printf("IP: %s\r\n", wifi.get_ip_address());
    printf("Netmask: %s\r\n", wifi.get_netmask());
    printf("Gateway: %s\r\n", wifi.get_gateway());
    printf("RSSI: %d\r\n", wifi.get_rssi());
    
    while(1){
        hum_temp.get_temperature(&value1);
        hum_temp.get_humidity(&value2);
        buffer[0] = (char)value1;
        buffer[1] = (char)value2;

        printf("HTS221:  [temp] %.2f C, [hum]   %.2f%%\r\n", value1, value2);
        socket_demo(&wifi, value1, value2);
        wait(0.5);
    }   
}