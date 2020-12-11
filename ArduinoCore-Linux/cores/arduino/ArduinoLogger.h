#ifndef __ARDLOGGER_H__
#define __ARDLOGGER_H__

#include "Stream.h"

namespace arduino {

/**
 * @brief Supported log levels
 * 
 */
enum LogLevel { 
    Debug,
    Info, 
    Warning, 
    Error
};

/**
 * @brief A simple Logger that writes messages dependent on the log level
 * 
 */

class ArduinoLogger {
    public:
        ArduinoLogger(){}
        ~ArduinoLogger(){}
        // activate the logging
        virtual void setLogger(Stream& out, LogLevel level=Error){
            this->log_stream_ptr = &out;
            this->log_level = level;
        }

        // checks if the logging is active
        virtual bool isLogging(){
            return log_stream_ptr!=nullptr;
        }

        // write an message to the log
        virtual void log(LogLevel current_level, const char *str, const char* str1=nullptr, const char* str2=nullptr){
            if (log_stream_ptr!=nullptr){
                if (current_level >= log_level){
                    log_stream_ptr->print((char*)str);
                    if (str1!=nullptr){
                        log_stream_ptr->print(" ");
                        log_stream_ptr->print((char*)str1);
                    }
                    if (str2!=nullptr){
                        log_stream_ptr->print(" ");
                        log_stream_ptr->print((char*)str2);
                    }
                    log_stream_ptr->println();
                    log_stream_ptr->flush();
                }
            }
        }

    protected:
        Stream *log_stream_ptr;
        LogLevel log_level;  

};

extern ArduinoLogger Logger;

}

// e.g. used by UDP
static void log_e(const char* msg, int errorNo){
    char errorNoStr[10];
    sprintf(errorNoStr,"%d", errorNo);
    Logger.log(Error, msg, errorNoStr);
}
    



#endif // __HttpLogger_H__