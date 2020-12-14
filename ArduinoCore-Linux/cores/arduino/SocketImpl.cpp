
/**
 * Separate implementation class for the WIFI client to prevent import conflicts
 ***/

#include <sys/socket.h> 
#include <sys/ioctl.h>
#include <unistd.h>
#include <fcntl.h>
#include <arpa/inet.h> 

#include <sys/types.h>
#include <ifaddrs.h>
#include <netinet/in.h>
#include <net/if.h>
#include <string.h>

#include "SocketImpl.h"
#include "ArduinoLogger.h"

namespace arduino {

SocketImpl::SocketImpl(){
}

const char* SOCKET_IMPL = "SocketImpl";

// checks if we are connected
uint8_t SocketImpl::connected() {
    char buf[2];
    int result = ::recv(sock, &buf, 1, MSG_PEEK | MSG_DONTWAIT);
    // if peek is working we are connected - if not we do further checks
    is_connected = result >= 0;
    if (!is_connected){
        //int getsockopt(int sockfd, int level, int optname,void *optval, socklen_t *optlen);
        int error_code;
        socklen_t error_code_size;
        int result = getsockopt(sock, SOL_SOCKET, SO_ERROR, &error_code, &error_code_size);
        if (result!=0){
            char msg[50];
            sprintf(msg,"%d",result);
            Logger.error(SOCKET_IMPL,"getsockopt->",msg);
        }    
        
       is_connected = (result == 0);
    }

    return is_connected;
}

// opens a conection
int SocketImpl::connect(const char* address, uint16_t port){
    if ((sock = ::socket(AF_INET, SOCK_STREAM, 0)) < 0) { 
        Logger.error(SOCKET_IMPL,"could not create socket");
        return -1; 
    } 

    serv_addr.sin_family = AF_INET; 
    serv_addr.sin_port = htons(port); 

    // Convert IPv4 and IPv6 addresses from text to binary form 
    if(::inet_pton(AF_INET, address, &serv_addr.sin_addr)<=0)  { 
        Logger.error(SOCKET_IMPL,"invalid address");
        return -2; 
    } 

    if (::connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) { 
        Logger.error(SOCKET_IMPL,"could not connect");
        return -3; 
    } 

    is_connected = true;
    Logger.info(SOCKET_IMPL,"connected!");
    return 1;
}

// send the data via the socket - returns the number of characters written or -1=>Error
size_t SocketImpl::write(const uint8_t* str, size_t len){
    Logger.debug(SOCKET_IMPL,"write");
    return ::send(sock , str , len , 0 ); 
}

// provides the available bytes
size_t SocketImpl::available() {
    int bytes_available;
    ioctl(sock, FIONREAD, &bytes_available);
    char msg[50];
    sprintf(msg,"%d",bytes_available);
    Logger.debug(SOCKET_IMPL,"available->",msg);
    return bytes_available;
}

// direct read
size_t SocketImpl::read(uint8_t* buffer, size_t len){
    size_t result = ::recv(sock, buffer, len, MSG_DONTWAIT );
    char lenStr[80];
    sprintf(lenStr,"%ld -> %ld",len, result);
    Logger.debug(SOCKET_IMPL,"read->", lenStr);
    return result;
}

// peeks one character
int SocketImpl::peek(){
    Logger.debug(SOCKET_IMPL,"peek");
    char buf[1];
    int result = ::recv(sock, &buf, 1, MSG_PEEK | MSG_DONTWAIT);
    return result>0 ? buf[0] : -1;
}

void SocketImpl::close() {
    Logger.info(SOCKET_IMPL,"close");
    ::close(sock);
}
    
// determines the IP Adress
const char* SocketImpl::getIPAddress(){
    Logger.info(SOCKET_IMPL,"getIPAddress");
    const char* valid[] = {"eth0", "en0","en1",nullptr};
    return getIPAddress(valid);
}
    
// determines the IP Adress    
const char* SocketImpl::getIPAddress(const char* validEntries[]){
    Logger.info(SOCKET_IMPL,"getIPAddress-1");
    struct ifaddrs *interfaces = NULL;
    struct ifaddrs *temp_addr = NULL;
    char* resultAddress = nullptr;
    int success = 0;
    // retrieve the current interfaces - returns 0 on success
    success = getifaddrs(&interfaces);
    if (success == 0) {
        // Loop through linked list of interfaces
        temp_addr = interfaces;
        while(temp_addr != NULL) {
            if(temp_addr->ifa_addr->sa_family == AF_INET && (temp_addr->ifa_flags & IFF_UP)) {
                // check if ifa_name is in the list of relevant names
                bool isValid = false;
                for(int j=0;validEntries[j]!=nullptr;j++){
                    if (strcmp(temp_addr->ifa_name, validEntries[j])==0){
                        isValid = true;
                        break;
                    }
                }
                if(isValid) {
                    auto addr = ((struct sockaddr_in*)temp_addr->ifa_addr)->sin_addr;
                    resultAddress = inet_ntoa(addr);
                    break;
                }
            }
            temp_addr = temp_addr->ifa_next;
        }
    }
    // Free memory
    freeifaddrs(interfaces);
    return resultAddress;
}
    
    
}