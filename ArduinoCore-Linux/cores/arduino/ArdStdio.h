#pragma once

#include <iostream>
#include <streambuf>
#include "Stream.h"
#include "Printable.h"

namespace arduino {

/**
 * @brief We use the SerialDef class to be able to provide Serail, Serial1 and Serial2 outside of the
 * Arduino environment;  
 * 
 */
class StdioDevice : public Stream {
  public:
    
    StdioDevice(bool autoFlush=true){
        auto_flush = autoFlush;
    }
        
    ~StdioDevice(){
    }

    operator bool() const { return true; } // For classic while(!Serial) { ... } pattern for USB ready wait

    virtual void begin(int speed){
        // nothing to be done
    }
    
    virtual void print(const char* str){
        std::cout << str;
        if (auto_flush) flush();
    }
    
    virtual void println(const char* str=""){
        std::cout << str << "\n";
        if (auto_flush) flush();
    }
    
    virtual void print(int val, int radix = DEC){
        if (radix == DEC) { std::cout << val; } else { Stream::print(val, radix); }
        if (auto_flush) flush();
    }
    
    virtual void println(int val, int radix = DEC){
        if (radix == DEC) { std::cout << val << "\n"; } else { Stream::println(val, radix); }
        if (auto_flush) flush();
    }
    
    virtual void println(String &str){
        println(str.c_str());
    }

    virtual void print(String &str){
        print(str.c_str());
    }
    
    
    virtual void println(Printable &p){
        p.printTo(*this);
        std::cout << "\n";
        if (auto_flush) flush();
    }

    virtual void print(Printable &p){
        p.printTo(*this);
        if (auto_flush) flush();
    }
    
    virtual void flush() {
        std::cout.flush();
    }
    
    virtual void write(const char* str, int len) {
        std::cout.write(str, len);
        if (auto_flush) flush();
    }

    virtual void write(uint8_t* str, int len) {
        std::cout.write((const char*)str, len);
        if (auto_flush) flush();
    }
    
    virtual size_t write(int32_t value){
        std::cout.put(value);
        if (auto_flush) flush();
        return 1;
    }
    
    virtual size_t write(uint8_t value){
        std::cout.put(value);    
        if (auto_flush) flush();
        return 1;
    }
    
    virtual int available() {
        return std::cin.rdbuf()->in_avail();
    };
    
    virtual int read() {
        return std::cin.get();
    } 
    
    virtual int peek() {
        return std::cin.peek();
    }

  protected:
    bool auto_flush;
   

};

extern StdioDevice Serial;    
    
}

