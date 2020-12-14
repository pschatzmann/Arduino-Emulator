
#pragma once

#include "Common.h"
#include "RingBufferExt.h"
#include "IPAddress.h"
#include "Client.h"
#include "SocketImpl.h"
#include "ArduinoLogger.h"

namespace arduino {


class EthernetImpl {
  public:
    IPAddress &localIP(){
        SocketImpl sock;
        adress.fromString(sock.getIPAddress());
        return adress;
    }
    
  protected:
    IPAddress adress;
    
};
    
inline EthernetImpl Ethernet;

    
class EthernetClient : public Client {
    public:
        EthernetClient(int bufferSize=256, long timeout=2000){
            this->bufferSize = bufferSize;
            readBuffer = RingBufferExt(bufferSize);
            writeBuffer = RingBufferExt(bufferSize);
            setTimeout(timeout);
        }

        // checks if we are connected - using a timeout
        virtual uint8_t connected() {
            long timeout = millis() + getTimeout();
            uint8_t result = sock.connected();
            while (result<=0 && millis() < timeout){
                delay(200);
                result = sock.connected();
            }
            return result;
        }
    
        // support conversion to bool
        operator bool() { return connected(); }
    
        // opens a conection
        virtual int connect(IPAddress ipAddress, uint16_t port) {
            String str = String(ipAddress[0]) + String(".") +\
                  String(ipAddress[1]) + String(".") +\
                  String(ipAddress[2]) + String(".") +\
                  String(ipAddress[3]) ;

            return connect(str.c_str(),port);  
        }
    
        // opens a conection
        virtual int connect(const char* address, uint16_t port){
            Logger.info(WIFICLIENT,"connect");
            if (connectedFast()){
                sock.close();
            }
            sock.connect(address, port);
            is_connected = true;
            return 1;
        }

        virtual size_t write(char c){
            return write((uint8_t) c);
        }
    
        // writes an individual character into the buffer. We flush the buffer when it is full
        virtual size_t write(uint8_t c){
            if (writeBuffer.availableToWrite()==0){
                flush();
            }
            return writeBuffer.write(c);            
        }

        virtual size_t write(const char* str, int len){
            return write((const uint8_t*)str, len);
        }

        // direct write - if we have anything in the buffer we write that out first
        virtual size_t write(const uint8_t* str, size_t len){
            flush();
            return sock.write(str, len); 
        }
    
        virtual int print(const char* str=""){
            int len = strlen(str);
            return write(str, len);
        }

        virtual int println(const char* str=""){
            int len = strlen(str);
            int result = write(str, len);
            char eol[1];
            eol[0] = '\n';
            write(eol, 1);
            return result;
        }

        // flush write buffer
        virtual void flush() {
            Logger.debug(WIFICLIENT,"flush");

            int flushSize = writeBuffer.available();
            if (flushSize>0){
                uint8_t rbuffer[flushSize];
                writeBuffer.read(rbuffer, flushSize);
                sock.write(rbuffer, flushSize);
            }
        }

        // provides the available bytes from the read buffer or from the socket
        virtual int available() {
            Logger.debug(WIFICLIENT,"available");
            if (readBuffer.available()>0){
                return readBuffer.available();
            }
            long timeout = millis() + getTimeout();
            int result = sock.available();            
            while (result<=0 && millis() < timeout){
                delay(200);
                result = result = sock.available(); 
            }
            return result;
        }

        // read via ring buffer
        virtual int read() {
            Logger.debug(WIFICLIENT,"read-1");
            if (readBuffer.available()==0){
                uint8_t buffer[bufferSize];
                int len = read((uint8_t*)buffer, bufferSize);
                readBuffer.write(buffer, len);
            }
            return readBuffer.read();
        }

        virtual int read(char* buffer, size_t len){
            return read((uint8_t* )buffer, len);
        }
    
        // direct read with timeout
        virtual int read(uint8_t* buffer, size_t len){
            Logger.debug(WIFICLIENT,"read");
            int result = 0;
            if (readBuffer.available()>0){
                result = readBuffer.read(buffer, len);
            } else {            
                long timeout = millis() + getTimeout();
                result = sock.read(buffer, len);
                while (result<=0 && millis() < timeout){
                    delay(200);
                    result = sock.read(buffer, len);
                }
            }
            char lenStr[16];
            sprintf(lenStr,"%d",result);
            Logger.debug( WIFICLIENT,"read->",lenStr);
            
            return result;
        }
        
        // peeks one character
        virtual int peek(){
            Logger.debug(WIFICLIENT, "peek");
            if (readBuffer.available()>0){
                return readBuffer.peek();
            }
            return sock.peek();
        }
    
        // close the connection
        virtual void stop() {
            sock.close();
        }

    protected:
        const char *WIFICLIENT = "EthernetClient";
        SocketImpl sock;
        int bufferSize;
        RingBufferExt readBuffer;
        RingBufferExt writeBuffer;
        bool is_connected;
    
        bool connectedFast() {
            return is_connected;
        }
};
    

}

