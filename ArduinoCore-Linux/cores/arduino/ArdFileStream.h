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
class FileStream : public Stream {
  public:
    
    FileStream(const char* outDevice="/dev/stdout", const char* inDevice="/dev/stdin"){
        open(outDevice, inDevice);
    }

    ~FileStream(){
        in.close();
        out.close();
    }
    
    void open(const char* outDevice, const char* inDevice){
        if (outDevice!=nullptr)
            out.open(outDevice,std::ios::out); 
        if (inDevice!=nullptr)
            in.open(inDevice,std::ios::in); 
    }
    
    virtual void begin(int speed){
        // nothing to be done
    }
    
    virtual void print(const char* str){
        out << str;
        out.flush();
    }
    
    virtual void println(const char* str=""){
        out << str << "\n";
        out.flush();
    }
    
    virtual void print(int str){
        out << str;
        out.flush();
    }
    
    virtual void println(int str){
        out << str << "\n";
        out.flush();
    }
    
    virtual void flush() {
        out.flush();
    }
    
    virtual void write(const char* str, int len) {
        out.write(str, len);
    }

    virtual void write(uint8_t* str, int len) {
        out.write((const char*)str, len);
    }
    
    virtual size_t write(int32_t value){
        out.put(value);
        return 1;
    }
    
    virtual size_t write(uint8_t value){
        out.put(value);    
        return 1;
    }
    
    virtual int available() {
        return in.rdbuf()->in_avail();
    };
    
    virtual int read() {
        return in.get();
    } 
    
    virtual int peek() {
        return in.peek();
    }
   
  protected:
    std::fstream out;
    std::fstream in;

};
    
}

#endif // __ALTETC_H__
