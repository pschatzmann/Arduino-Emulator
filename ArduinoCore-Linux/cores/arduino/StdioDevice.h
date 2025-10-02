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
    
    virtual size_t print(const char* str){
        std::cout << str;
        if (auto_flush) flush();
        return strlen(str);
    }
    
    virtual size_t println(const char* str=""){
        std::cout << str << "\n";
        if (auto_flush) flush();
        return strlen(str) + 1;
    }
    
    virtual size_t print(int val, int radix = DEC){
        size_t result = Stream::print(val, radix); 
        if (auto_flush) flush();
        return result;
    }
    
    virtual size_t println(int val, int radix = DEC){
        size_t result = Stream::println(val, radix); 
        if (auto_flush) flush();
        return result;
    }
    
    virtual size_t println(String &str){
        return println(str.c_str());
    }

    virtual size_t print(String &str){
        return print(str.c_str());
    }
    
    
    virtual size_t println(Printable &p){
        size_t result = p.printTo(*this);
        std::cout << "\n";
        if (auto_flush) flush();
        return result + 1;
    }

    virtual size_t print(Printable &p){
        auto result = p.printTo(*this);
        if (auto_flush) flush();
        return result;
    }
    
    void flush() override {
        std::cout.flush();
    }
    
    virtual size_t write(const char* str, size_t len) {
        std::cout.write(str, len);
        if (auto_flush) flush();
        return len;
    }

    virtual size_t write(uint8_t* str, size_t len) {
        std::cout.write((const char*)str, len);
        if (auto_flush) flush();
        return len;

    }
    size_t write(const uint8_t* str, size_t len) override {
        std::cout.write((const char*)str, len);
        if (auto_flush) flush();
        return len;
    }
    
    virtual size_t write(int32_t value){
        std::cout.put(value);
        if (auto_flush) flush();
        return 1;
    }
    
    size_t write(uint8_t value) override {
        std::cout.put(value);    
        if (auto_flush) flush();
        return 1;
    }
    
    int available() override {
        return std::cin.rdbuf()->in_avail();
    };
    
    int read() override {
        return std::cin.get();
    } 
    
    int peek() override {
        return std::cin.peek();
    }

  protected:
    bool auto_flush = true;
   

};

static StdioDevice Serial;    
    
}

