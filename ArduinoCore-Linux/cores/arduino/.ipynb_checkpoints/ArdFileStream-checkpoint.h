#ifndef SERIAL_H
#define SERIAL_H

#include <iostream>
#include <fstream>
#include "Stream.h"

namespace arduino {

/**
 * @brief We use the SerialDef class to be able to provide Serail, Serial1 and Serial2 outside of the
 * Arduino environment;  
 * 
 */
class SerialDev : public Stream {
  public:
    SerialDev() {
        file.open ("/dev/tty",std::ios::in|std::ios::out);
    }
    
    SerialDev(const char* device){
        file.open(device); 
    }
    
    ~SerialDev(){
        file.close();
    }
    
    virtual void begin(int speed){
        // nothing to be done
    }
    
    virtual void print(const char* str){
        file << str;
        file.flush();
    }
    
    virtual void println(const char* str=""){
        file << str << "\n";
        file.flush();
    }
    
    virtual void print(int str){
        file << str;
        file.flush();
    }
    
    virtual void println(int str){
        file << str << "\n";
        file.flush();
    }
    
    virtual void flush() {
        file.flush();
    }
    
    virtual void write(const char* str, int len) {
        file.write(str, len);
    }

    virtual void write(uint8_t* str, int len) {
        file.write((const char*)str, len);
    }
    
    virtual size_t write(int32_t value){
        file.put(value);
        return 1;
    }
    
    virtual size_t write(uint8_t value){
        file.put(value);    
        return 1;
    }
    

    virtual int available() {
        return file.rdbuf()->in_avail();
    };
    
    virtual int read() {
        return file.get();
    } 
    
    virtual int peek() {
        return file.peek();
    }
   

  protected:
    std::fstream file;

};

extern SerialDev Serial;

}

#endif // __ALTETC_H__
