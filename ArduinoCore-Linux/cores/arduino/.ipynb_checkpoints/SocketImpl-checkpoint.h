#ifndef SOCKETIMPL_H
#define SOCKETIMPL_H

/**
 * Separate implementation class for the WIFI client to prevent import conflicts
 ***/

#include <sys/socket.h> 
#include <sys/ioctl.h>
#include <unistd.h>
#include <fcntl.h>
#include <arpa/inet.h> 

namespace arduino {

class SocketImpl {
    public:
        SocketImpl(){
        }

        // checks if we are connected
        virtual uint8_t connected() {
            if  (is_connected){
                char buf[2];
                int result = ::recv(sock, &buf, 1, MSG_PEEK | MSG_DONTWAIT);
                is_connected = result >= 0;
                // int error_code;
                // int error_code_size = sizeof(error_code);
                // int result = getsockopt(sock, SOL_SOCKET, SO_ERROR, &error_code, &error_code_size);
                // is_connected = (result == 0);
            }
            return is_connected;
        }
    
        // opens a conection
        virtual int connect(const char* address, uint16_t port){
            if ((sock = ::socket(AF_INET, SOCK_STREAM, 0)) < 0) { 
                return -1; 
            } 
        
            serv_addr.sin_family = AF_INET; 
            serv_addr.sin_port = htons(port); 
            
            // Convert IPv4 and IPv6 addresses from text to binary form 
            if(::inet_pton(AF_INET, address, &serv_addr.sin_addr)<=0)  { 
                return -2; 
            } 
        
            if (::connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) { 
                return -3; 
            } 

            is_connected = true;
            return 1;
        }
    
        virtual size_t write(const uint8_t* str, size_t len){
            return ::send(sock , str , len , 0 ); 
        }
    
        // provides the available bytes
        virtual int available() {
            int bytes_available;
            ioctl(sock,FIONREAD, &bytes_available) ;
            return bytes_available;
        }
    
        // direct read
        virtual int read(uint8_t* buffer, size_t len){
            return ::recv(sock, buffer, len, MSG_DONTWAIT );
        }
        
        // peeks one character
        virtual int peek(){
            char buf[1];
            int result = ::recv(sock, &buf, 1, MSG_PEEK | MSG_DONTWAIT);
            return result>0 ? buf[0] : -1;
        }
    
        virtual void close() {
            ::close(sock);
        }

    protected:
        bool is_connected;
        int sock = 0, valread; 
        struct sockaddr_in serv_addr; 

};
    
}

#endif