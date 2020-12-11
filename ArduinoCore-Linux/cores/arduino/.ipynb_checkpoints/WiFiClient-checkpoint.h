#ifndef WIFICLIENT_H
#define WIFICLIENT_H

namespace arduino {

class WiFiClient : public Client {
    public:
        Client(int bufferSize=1024){
            readBuffer = RingBuffer(bufferSize);
            writeBuffer = RingBuffer(bufferSize);
        }

        // checks if we are connected
        bool connected() {
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
    
        // support conversion to bool
        operator bool() { return connected(); }

        // opens a conection
        int connect(Url &url){
            return connect(url.host(), url.port());
        }
    
        // opens a conection
        int connect(const char* address, int port){
            Log.log(Info, "Socket is connecting...");
            if ((sock = ::socket(AF_INET, SOCK_STREAM, 0)) < 0) { 
                Log.log(Error, "Socket creation error");
                return -1; 
            } 
        
            serv_addr.sin_family = AF_INET; 
            serv_addr.sin_port = htons(port); 
            
            
            // Convert IPv4 and IPv6 addresses from text to binary form 
            if(::inet_pton(AF_INET, address, &serv_addr.sin_addr)<=0)  { 
                Log.log(Error, "Invalid address: Address not supported");
                return -2; 
            } 
        
            if (::connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) { 
                Log.log(Error, "Connection Failed");
                return -3; 
            } 

            Log.log(Info, "Socket is connected");
            is_connected = true;
            return 1;
        }
    

        // writes an individual character into the buffer. We flush the buffer when it is full
        int write(char c){
            if (writeBuffer.availableToWrite()==0){
                flush();
            }
            return writeBuffer.write(c);
        }

        // direct write - if we have anything in the buffer we write that out first
        int write(const char* str, int len){
            flush();
            return send(sock , str , len , 0 ); 
        }

        int write(const uint8_t* str, int len){
            flush();
            return send(sock , str , len , 0 ); 
        }
    
        int print(const char* str){
            int len = strlen(str);
            return write(str, len);
        }

        int println(const char* str){
            int len = strlen(str);
            int result = write(str, len);
            char eol[1];
            eol[0] = '\n';
            write(eol, 1);
            return result;
        }
  
        // provides the available bytes
        int available() {
            int bytes_available;
            ioctl(sock,FIONREAD, &bytes_available) ;
            return bytes_available;
        }

        // read via ring buffer
        int read() {
            if (readBuffer.available()==0){
                uint8_t buffer[bufferSize];
                int len = ::recv(sock, (void*)buffer, bufferSize , MSG_DONTWAIT);
                readBuffer.write(buffer, len);
            }
            return readBuffer.read();
        }

        // direct read
        int read(char* buffer, int len){
            return ::recv(sock, buffer, len, MSG_DONTWAIT );
        }
    
        // direct read
        int read(uint8_t* buffer, int len){
            return ::recv(sock, buffer, len, MSG_DONTWAIT );
        }
        
        // peeks one character
        int peek(){
            char buf[1];
            int result = ::recv(sock, &buf, 1, MSG_PEEK | MSG_DONTWAIT);
            return result>0 ? buf[0] : -1;
        }
    
        int readBytesUntil(char untilChar,char* str, int len){
            for (int j=0;j<len;j++){
                int c = read();
                str[j] = c;
                if (c==untilChar){
                    return j;
                }
            }
            return len;
        }

        // flush write buffer
        void flush() {
            int flushSize = writeBuffer.available();
            if (flushSize>0){
                uint8_t rbuffer[flushSize];
                writeBuffer.read(rbuffer, flushSize);
                ::send(sock, rbuffer, flushSize, 0);
            }
        }

        // close the connection
        void stop() {
            close(sock);
            is_connected = false;
        }

    protected:
        bool is_connected;
        int sock = 0, valread; 
        int bufferSize = 512;
        RingBuffer readBuffer;
        RingBuffer writeBuffer;
        struct sockaddr_in serv_addr; 

};
    
}

#endif