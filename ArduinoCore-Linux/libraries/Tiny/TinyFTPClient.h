#pragma once

/**
#pragma once

/**
 * @file ArduinoFTPClient.h
 * @author Phil Schatzmann (phil.schatzmann@gmail.com)
 * @brief A simple FTP client for Arduino using Streams
 * 
 * Sometimes you might have the need to write your sensor data into a remote file or for your local 
 * processing logic you need more data than you can store on your local Arduino device. 
 *
 * If your device is connected to the Internet, this library is coming to your rescue: The FTP protocol 
 * is one of oldest communication protocols. It is easy to implement and therefore rather efficient and
 * you can find free server implementations on all platforms.
 *
 * Here is a simple but powerful FTP client library that uses a Stream based API. 
 *
 * We support
 *
 * - The reading of Remote Files (File Download)
 * - The writing to Remote Files (File Upload)
 * - The listing of Remote Files and Directories
 * - The creation and deletion of remote directories
 * - The deletion of remote files
 *
 * @version 1.0
 * @date 2020-Nov-07
 * 
 * @copyright Copyright (c) 2020 Phil Schatzmann
 * 
 */

#include "Stream.h"
#include "IPAddress.h"

// make sure that the solution is working in my emulator which uses the arduino namespace
namespace arduino {}
using namespace arduino;

// Common Constants
static const int MAXFILE_NAME_LENGTH = 512;
static const int COMMAND_PORT = 21;
static const int DATA_PORT = 1000;

// Common Enums
enum FileMode  { READ_MODE, WRITE_MODE, WRITE_APPEND_MODE };
enum CurrentOperation  { READ_OP, WRITE_OP, LS_OP, NOP };
enum ObjectType { TypeFile, TypeDirectory, TypeUndefined };
enum FTPLogLevel  { LOG_DEBUG, LOG_INFO, LOG_WARN, LOG_ERROR };

/**
 * @brief IPConnect
 * Unfortunatly there are different TCP/IP APIs in Arduino. We currently support
 * Ethernet and WiFi
 */

class FtpIpClient {
  public:
    virtual ~FtpIpClient() {};
    virtual bool connect(IPAddress address, int port) = 0;
    virtual bool isConnected() = 0;
    virtual IPAddress localAddress() = 0;
    virtual Stream* stream() = 0;
    virtual void stop() = 0;
};

#include <Ethernet.h>

class IPConnectEthernet : public FtpIpClient {
    virtual bool connect(IPAddress address, int port){
       bool ok = client.connect(address, port);
       return ok;
    }

    virtual IPAddress localAddress(){
      return Ethernet.localIP();
    }

    virtual bool isConnected(){
      return client.connected();
    }

    virtual Stream* stream() {
      return &client;
    }

    virtual void stop() {
      client.stop();
    }  

    EthernetClient client;
};

#if defined(ESP32) || defined(ESP8266) 
#include <WiFi.h>

class FtpIpClientWifi : public FtpIpClient {
  public:
    virtual bool connect(IPAddress address, int port){
       bool ok = client.connect(address, port);
       return ok;
    }

    virtual IPAddress localAddress(){
      return WiFi.localIP();
    }

    virtual bool isConnected(){
      return client.available() || client.connected();
    }

    virtual Stream* stream() {
      return &client;
    }

    virtual void stop() {
      client.stop();
    }

    WiFiClient client;
};

#endif

/**
 * @brief CStringFunctions
 * We implemented some missing C based string functions for character arrays
 */

class CStringFunctions {
  public:
    static int findNthInStr(char* str,  char ch, int n) { 
        int occur = 0; 
        // Loop to find the Nth 
        // occurence of the character 
        for (int i = 0; i < strlen(str); i++) { 
            if (str[i] == ch) { 
                occur += 1; 
            } 
            if (occur == n) 
                return i; 
        } 
        return -1; 
    } 

    static int readln(Stream &stream, char* str, int maxLen) {
      int len = 0;
      if (maxLen>stream.available()){
        maxLen = stream.available();
      }
      for (int j=0;j<maxLen;j++){
        len = j;
        char c = stream.read();
        if (c==0 || c=='\n'){
            break;
        } 
        str[j] = c;   
      }
      memset(str+len,0,maxLen-len);
      return len;
    } 
};

/**
 * @brief FTPLogger
 * To activate logging define the output stream e.g. with FTPFTPLogger.setOutput(Serial);
 * and (optionally) set the log level
 */
class FTPLoggerImpl {
  public:

    void setFTPLogLevel(FTPLogLevel level){
          min_log_level  = level;   
    }

    FTPLogLevel getFTPLogLevel(){
          return min_log_level;
    }

    void setOutput(Stream &out){
          out_ptr = &out;
    }

    void writeLog(FTPLogLevel level, const char* module, const char* msg=nullptr){
        if (out_ptr!=nullptr && level>=min_log_level) {
          out_ptr->print("FTP ");
          switch (level){
              case LOG_DEBUG:
                  out_ptr->print("DEBUG - ");
                  break;
              case LOG_INFO:
                  out_ptr->print("INFO - ");
                  break;
              case LOG_WARN:
                  out_ptr->print("WARN - ");
                  break;
              case LOG_ERROR:
                  out_ptr->print("ERROR - ");
                  break;
          }
          out_ptr->print(module);
          if (msg!=nullptr) {
              out_ptr->print(" ");
              out_ptr->print(msg);
          }
          out_ptr->println();
      } else {
          //delay(100);
      }
    }

  protected:
    FTPLogLevel min_log_level;
    Stream *out_ptr; 
};

FTPLoggerImpl FTPLogger;

/**
 * @brief FTPBasicAPI
 * Implementation of Low Level FTP protocol. In order to simplify the logic we always use Passive FTP where
 * it is our responsibility to open the data conection.
 */

class FTPBasicAPI {
  friend class FTPFile;
  public:
    FTPBasicAPI(){
        FTPLogger.writeLog( LOG_DEBUG, "FTPBasicAPI");      
    }

    ~FTPBasicAPI(){
        FTPLogger.writeLog( LOG_DEBUG, "~FTPBasicAPI");      
    }

    virtual  bool open(FtpIpClient *cmdPar, FtpIpClient *dataPar, IPAddress &address, int port, int data_port, const char* username, const char *password){
        FTPLogger.writeLog( LOG_DEBUG, "FTPBasicAPI ", "open");
        command_ptr = cmdPar;
        data_ptr = dataPar;  
        remote_address = address;

        bool ok = connect( address, port, command_ptr, true);
        if (ok && username!=nullptr) {
            const char* ok_result[] = {"331","230","530", nullptr};
            ok = cmd("USER", username, ok_result);
        }
        if (ok && password!=nullptr) {
            const char* ok_result[] = {"230","202",nullptr};
            ok = cmd("PASS", password, ok_result);
        }
        if (ok) {
            ok = passv();
        }
        is_open = ok;
        return ok;
    }

    virtual  bool quit(){
        FTPLogger.writeLog( LOG_DEBUG, "FTPBasicAPI", "quit");      
        const char* ok_result[] = {"221","226", nullptr};
        return cmd("QUIT", nullptr, ok_result, false);    
    }

    virtual  bool isConnected(){
        return is_open;
    }

    virtual  bool passv(){
        FTPLogger.writeLog( LOG_DEBUG, "FTPBasicAPI","passv");      
        bool ok = cmd("PASV", nullptr, "227");
        if (ok) {
            char buffer[80];
            FTPLogger.writeLog(LOG_DEBUG, "FTPBasicAPI::passv", result_reply);
            // determine data port
            int start1 = CStringFunctions::findNthInStr(result_reply,',',4)+1;
            int p1 = atoi(result_reply+start1);
            
            sprintf(buffer,"*** port1 -> %d ",  p1 );
            FTPLogger.writeLog(LOG_DEBUG,"FTPBasicAPI::passv", buffer);

            int start2 = CStringFunctions::findNthInStr(result_reply,',',5)+1;
            int p2 = atoi(result_reply+start2);
            
            sprintf(buffer,"*** port2 -> %d ", p2 );
            FTPLogger.writeLog(LOG_DEBUG,"FTPBasicAPI::passv", buffer);

            int data_port = (p1 * 256) + p2;
            sprintf(buffer,"*** data port: %d", data_port);
            FTPLogger.writeLog(LOG_DEBUG, "FTPBasicAPI::passv", buffer);

            ok = connect(remote_address, data_port, data_ptr) == 1;
        }
        return ok;      
    }

    virtual  bool del(const char * file){
        FTPLogger.writeLog( LOG_DEBUG, "FTPBasicAPI","del");      
        return cmd("DELE", file, "250");
    }

    virtual  bool mkdir(const char * dir) {
        FTPLogger.writeLog( LOG_DEBUG, "FTPBasicAPI","mkdir");      
        return cmd("MKD", dir, "257");
    }

    virtual  bool rmd(const char * dir){
        FTPLogger.writeLog( LOG_DEBUG, "FTPBasicAPI","rd");      
        return cmd("RMD", dir, "250");
    }

    virtual bool abort(){
        bool result = false;
        if (current_operation!=NOP) {
            FTPLogger.writeLog( LOG_DEBUG, "FTPBasicAPI","abort");      
            const char* ok[] = {"225", "226",nullptr};
            current_operation = NOP;
            result = cmd("ABOR", nullptr, ok);
        }
        return result;
    }

    virtual  Stream *read(const char* file_name ) {
        if (current_operation!=READ_OP) {
            FTPLogger.writeLog( LOG_DEBUG, "FTPBasicAPI", "read");      
            abort();
            const char* ok[] = {"150","125", nullptr};
            cmd("RETR", file_name, ok);
            current_operation = READ_OP;
        }
        checkClosed(data_ptr);
        return data_ptr->stream();
    }

    virtual  Stream *write(const char* file_name, FileMode mode) {
        if (current_operation!=WRITE_OP) {
            FTPLogger.writeLog( LOG_DEBUG, "FTPBasicAPI", "write");      
            abort();
            const char* ok_write[] = {"125", "150", nullptr};
            cmd(mode == WRITE_APPEND_MODE ? "APPE": "STOR", file_name, ok_write);
            current_operation = WRITE_OP;
        }
        checkClosed(data_ptr);
        return data_ptr->stream();
    }

    virtual  Stream * ls(const char* file_name){
        FTPLogger.writeLog( LOG_DEBUG, "FTPBasicAPI", "ls");      
        abort();
        const char* ok[] = {"125", "150", nullptr};
        cmd("NLST",file_name, ok);
        current_operation = LS_OP;
        return data_ptr->stream();
    }

    virtual  void closeData(){
        FTPLogger.writeLog( LOG_DEBUG, "FTPBasicAPI","closeData");      
        data_ptr->stop();
        setCurrentOperation(NOP);
    }

    virtual  void setCurrentOperation(CurrentOperation op) {
        current_operation = op;
    }

    virtual CurrentOperation currentOperation() { 
      return current_operation;
    }

    virtual  void flush(){
        FTPLogger.writeLog( LOG_DEBUG, "FTPBasicAPI","flush");      
        data_ptr->stream()->flush();
    }

    virtual size_t size(const char * file){
        FTPLogger.writeLog( LOG_DEBUG, "FTPBasicAPI","size");      
        if (cmd("SIZE", file, "213")) {
            return atol(result_reply+4);
        }
        return 0;

    }
    virtual ObjectType objectType(const char * file){
        FTPLogger.writeLog( LOG_DEBUG, "FTPBasicAPI","objectType"); 
        const char* ok_result[] = {"213","550", nullptr}; 
        ObjectType result =  TypeDirectory;   
        if (cmd("SIZE", file, ok_result)) {
            if (strncmp(result_reply,"213",3)==0){
                result = TypeFile;
            } 
        }
        return result;
    }

  protected:
    virtual bool connect(IPAddress adr, int port, FtpIpClient *client_ptr, bool doCheckResult=false){
        char buffer[80];
        bool ok = true;
        sprintf(buffer,"connect %s:%d", toStr(adr), port);
        FTPLogger.writeLog( LOG_DEBUG, "FTPBasicAPI::connect", buffer);          
        ok = client_ptr->connect(adr, port);
        if (ok && doCheckResult){
            const char* ok_result[] = {"220","200",nullptr};
            ok = checkResult(ok_result, "connect");
        }
        if (!ok){
            FTPLogger.writeLog( LOG_ERROR, "FTPBasicAPI::connect", buffer);      
        }
        return ok;
    }

    const char *itoa(uint8_t value, char buffer[]){
        sprintf(buffer,"%ud", value);
        return (const char*)buffer;
    }

    const char* toStr(IPAddress adr){
        static char result[12];
        char number[5];
        strcat(result,itoa(adr[0],number));
        strcat(result, ".");
        strcat(result,itoa(adr[1],number));
        strcat(result, ".");
        strcat(result,itoa(adr[2],number));
        strcat(result,".");
        strcat(result,itoa(adr[3],number));
        return result;
    }

    virtual bool cmd(const char* command, const char* par, const char* expected, bool wait_for_data=true){
        const char* expected_array[] = { expected, nullptr };
        return cmd(command, par, expected_array, wait_for_data);
    }

    virtual bool cmd(const char* command_str, const char* par, const char* expected[], bool wait_for_data=true){
        char command_buffer[512];
        Stream *stream_ptr = command_ptr->stream();
        if (par==nullptr){
            strcpy(command_buffer, command_str);
            stream_ptr->println(command_buffer);
        } else {
            sprintf(command_buffer,"%s %s", command_str, par);
            stream_ptr->println(command_buffer);
        }
        FTPLogger.writeLog( LOG_DEBUG, "FTPBasicAPI::cmd", command_buffer);

        return checkResult(expected, command_buffer, wait_for_data);      
    }

    virtual void checkClosed(FtpIpClient *client){
        if (!client->isConnected()){
            FTPLogger.writeLog( LOG_DEBUG, "FTPBasicAPI","checkClosed -> client is closed"); 
            // mark the actual command as completed     
            current_operation = NOP;
        }

    }
    
    virtual bool checkResult(const char* expected[], const char* command, bool wait_for_data=true){
        // consume all result lines
        bool ok = false;
        result_reply[0] = '\0';
        Stream *stream_ptr = command_ptr->stream();

        char result_str[80];
        if (wait_for_data || stream_ptr->available()) {
            // wait for reply
            while(stream_ptr->available()==0){
                delay(100);
            }
            // read reply
            //result_str = stream_ptr->readStringUntil('\n').c_str();
            CStringFunctions::readln(*stream_ptr, result_str, 80);

            if (strlen(result_str)>3) {  
                FTPLogger.writeLog( LOG_DEBUG, "FTPBasicAPI::checkResult", result_str);
                strncpy(result_reply, result_str, 80);
                // if we did not expect anything
                if (expected[0]==nullptr){
                    ok = true;
                    FTPLogger.writeLog( LOG_DEBUG,"FTPBasicAPI::checkResult", "success because of not expected result codes");
                } else {
                    // check for valid codes
                    char msg[80];
                    for (int j=0; expected[j]!=nullptr; j++) {
                        sprintf(msg,"- checking with %s", expected[j]);
                        FTPLogger.writeLog( LOG_DEBUG, "FTPBasicAPI::checkResult", msg);
                        if (strncmp(result_str, expected[j], 3)==0) {
                            sprintf(msg," -> success with %s", expected[j]);
                            FTPLogger.writeLog( LOG_DEBUG, "FTPBasicAPI::checkResult", msg);
                            ok = true;
                            break;
                        }
                    }
                }
            } else {
                // if we got am empty line and we dont need to wait we are still ok
                if (!wait_for_data)
                    ok = true;
            }
        } else {
            ok = true;
        }

        // log error
        if (!ok){
            FTPLogger.writeLog( LOG_ERROR,"FTPBasicAPI::checkResult", command);  
            FTPLogger.writeLog( LOG_ERROR,"FTPBasicAPI::checkResult", result_reply);
        }
        return ok;         
    }

    CurrentOperation current_operation = NOP; // currently running op -> do we need to cancel ?
    FtpIpClient *command_ptr; // Client for commands
    FtpIpClient *data_ptr; // Client for upload and download of files
    IPAddress remote_address;
    bool is_open;
    char result_reply[80];

};


/**
 * @brief FTPFile
 * A single file which supports read and write operations. This class is implemented
 * as an Arduino Stream and therfore provides all corresponding functionality
 * 
 */

class FTPFile : public Stream {
  public:
    FTPFile(FTPBasicAPI *api_ptr, const char* name, FileMode mode){
        FTPLogger.writeLog( LOG_DEBUG, "FTPFile");
        if (name!=nullptr)
            this->file_name = strdup(name);
        this->mode = mode;
        this->api_ptr = api_ptr;
    }

    FTPFile(FTPFile &cpy){
        file_name = strdup(cpy.file_name);
        eol = cpy.eol;
        mode = cpy.mode;
        api_ptr = cpy.api_ptr;
        object_type = cpy.object_type;
    }

    FTPFile(FTPFile &&move){
        file_name = move.file_name;
        eol = move.eol;
        mode = move.mode;
        api_ptr = move.api_ptr;
        object_type = move.object_type;
        // clear source
        move.file_name = nullptr;
        api_ptr = nullptr;
    }

    ~FTPFile(){
        if (this->file_name!=nullptr)
            free((void*)this->file_name);
    }

    virtual size_t write(uint8_t data){
        FTPLogger.writeLog( LOG_DEBUG, "FTPFile", "write");
        if (mode==READ_MODE) {
            FTPLogger.writeLog( LOG_DEBUG, "FTPFile", "Can not write - File has been opened in READ_MODE");
            return 0;
        }
        Stream *result_ptr = api_ptr->write(file_name, mode );
        return result_ptr->write(data);
    }

    virtual size_t write(char* data, int len){
        int count = 0;
        for (int j=0;j<len;j++){
            count += write(data[j]);
        }
        return count;
    }

    virtual  int read(){
        FTPLogger.writeLog( LOG_DEBUG, "FTPFile", "read");
        Stream *result_ptr =  api_ptr->read(file_name);
        return result_ptr->read();
    }

    virtual  int read(void *buf, size_t nbyte){
        FTPLogger.writeLog( LOG_INFO, "FTPFile", "read-n");
        memset(buf,0, nbyte);
        Stream *result_ptr =  api_ptr->read(file_name);
        return result_ptr->readBytes((char*)buf, nbyte);
    }

    virtual  int readln(void *buf, size_t nbyte){
        FTPLogger.writeLog( LOG_INFO, "FTPFile", "readln");
        memset(buf,0, nbyte);
        Stream *result_ptr =  api_ptr->read(file_name);
        return result_ptr->readBytesUntil(eol[0], (char*)buf, nbyte );
    }

    virtual  int peek(){
        FTPLogger.writeLog( LOG_DEBUG, "FTPFile", "peek");
        Stream *result_ptr =  api_ptr->read(file_name);
        return result_ptr->peek();
    }

    virtual  int available(){
        char msg[80];
        Stream *result_ptr =  api_ptr->read(file_name);
        int len = result_ptr->available();
        sprintf(msg,"available: %d", len);
        FTPLogger.writeLog( LOG_DEBUG, "FTPFile", msg);
        return len;
    }

    virtual size_t size(){
        char msg[80];
        size_t size =  api_ptr->size(file_name);
        sprintf(msg,"size: %ld", size);
        FTPLogger.writeLog( LOG_DEBUG, "FTPFile", msg);
        return size;
    }

    virtual bool isDirectory(){
        FTPLogger.writeLog( LOG_DEBUG, "FTPFile", "isDirectory");
        return  api_ptr->objectType(file_name)==TypeDirectory;
    }

    virtual  void flush(){
        if (api_ptr->currentOperation()==WRITE_OP) {
            FTPLogger.writeLog( LOG_DEBUG, "FTPFile", "flush");
            Stream *result_ptr =  api_ptr->write(file_name, mode);
            result_ptr->flush();
        }
    }

    virtual  void close(){
        FTPLogger.writeLog( LOG_INFO, "FTPFile", "close");
        const char* ok[] = {"226", nullptr};
        api_ptr->checkResult(ok, "close", false);
        if (api_ptr->currentOperation()==WRITE_OP) {
            flush();
        }
        api_ptr->closeData();
    }

    virtual  const char * name(){
        return file_name;
    }

    virtual void setEOL(char* eol){
        FTPLogger.writeLog( LOG_DEBUG, "FTPFile", "setEOL");
        this->eol = eol;
    }

  protected:
    const char* file_name;
    const char* eol = "\n";
    FileMode mode;
    FTPBasicAPI *api_ptr;
    ObjectType object_type = TypeUndefined;
};

/**
 * @brief FTPFileIterator
 * The file name iterator can be used to list all available files and directories. We open 
 * a separate session for the ls operation so that we do not need to keep the result in memory
 * and we dont loose the data when we mix it with read and write operations.
 */
class FTPFileIterator {
  public:
    FTPFileIterator(FTPBasicAPI *api, const char* dir, FileMode mode){
        FTPLogger.writeLog( LOG_DEBUG, "FTPFileIterator()");
        this->directory_name = dir;
        this->api_ptr = api;
        this->file_mode = mode;
    }

    FTPFileIterator(FTPFileIterator &copy){
        FTPLogger.writeLog( LOG_DEBUG, "FTPFileIterator()-copy");
        this->api_ptr = copy.api_ptr;
        this->stream_ptr = copy.stream_ptr;
        this->directory_name = copy.directory_name;  
        this->file_mode = copy.file_mode;
    }

    FTPFileIterator(FTPFileIterator &&copy){
        FTPLogger.writeLog( LOG_DEBUG, "FTPFileIterator()-move");
        this->api_ptr = copy.api_ptr;
        this->stream_ptr = copy.stream_ptr;
        this->directory_name = copy.directory_name;  
        this->file_mode = copy.file_mode;
        copy.api_ptr = nullptr;
        copy.stream_ptr = nullptr;
        copy.directory_name = nullptr;
    }
    
    ~FTPFileIterator(){
        FTPLogger.writeLog( LOG_DEBUG, "~FTPFileIterator()");
    }

    virtual FTPFileIterator &begin(){
        FTPLogger.writeLog( LOG_DEBUG, "FTPFileIterator", "begin");
        if (api_ptr!=nullptr && directory_name!=nullptr) {
            stream_ptr = api_ptr->ls(directory_name);
            readLine();
        } else {
            FTPLogger.writeLog( LOG_ERROR, "FTPFileIterator", "api_ptr is null");
        }
        return *this;
    }

    virtual FTPFileIterator end(){
        FTPLogger.writeLog( LOG_DEBUG, "FTPFileIterator", "end");
        FTPFileIterator end = *this;
        end.buffer[0]=0;
        return end;
    }

    virtual FTPFileIterator &operator++(){
        FTPLogger.writeLog( LOG_DEBUG, "FTPFileIterator", "++");
        readLine();
        return *this;
    }

    virtual FTPFileIterator &operator++(int _na){
        FTPLogger.writeLog( LOG_DEBUG, "FTPFileIterator", "++(1)");
        readLine();
        return *this;
    }

    virtual FTPFile operator*(){
        FTPLogger.writeLog( LOG_DEBUG, "FTPFileIterator", "*");
        //return buffer;
        return FTPFile(api_ptr,buffer,file_mode);
    }

    virtual bool operator!=(const FTPFileIterator& comp){ return strcmp(this->buffer,comp.buffer)!=0; }
    virtual bool operator>(const FTPFileIterator& comp){ return strcmp(this->buffer,comp.buffer)>0; }
    virtual bool operator<(const FTPFileIterator& comp){ return strcmp(this->buffer,comp.buffer)<0; }
    virtual bool operator>=(const FTPFileIterator& comp){ return strcmp(this->buffer,comp.buffer)>=0; }
    virtual bool operator<=(const FTPFileIterator& comp){ return strcmp(this->buffer,comp.buffer)<=0; }

  protected:
    virtual void readLine(){
        FTPLogger.writeLog( LOG_DEBUG, "FTPFileIterator", "readLine");
        if (stream_ptr!=nullptr) {
            String str = stream_ptr->readStringUntil('\n');
            strcpy(buffer, str.c_str());
            if (strlen(buffer)){
                api_ptr->setCurrentOperation(NOP);
            }
            if (strcmp(buffer, directory_name)==0){
                // when ls is called on a file it returns the file itself
                // which we just ignore
                strcpy(buffer, "");
            }
        } else {
            FTPLogger.writeLog( LOG_ERROR, "FTPFileIterator", "stream_ptr is null");
            strcpy(buffer, "");
        }
    }

    FTPBasicAPI* api_ptr;
    Stream* stream_ptr;
    FileMode file_mode;
    const char* directory_name = (char*) "";
    char buffer[MAXFILE_NAME_LENGTH];
};

/**
 * @brief FTPClient 
 * Basic FTP access class which supports directory operatations and the opening of a files 
 * 
 */
class FTPClient {
  public:
    // default construcotr
    FTPClient(FtpIpClient &command, FtpIpClient &data, int port = COMMAND_PORT, int data_port = DATA_PORT ){
        FTPLogger.writeLog( LOG_INFO, "FTPClient()");
        init(&command, &data, port, data_port);
    }

    // plotform specific easy constructor
    FTPClient(int port = COMMAND_PORT, int data_port = DATA_PORT){
        FTPLogger.writeLog( LOG_INFO, "FTPClient()");
        FtpIpClient *command;
        FtpIpClient *data; 

    #ifdef ESP32 
        command = new FtpIpClientWifi();
        data = new FtpIpClientWifi();
        cleanup_clients = true;
    #else
        command = new IPConnectEthernet();
        data = new IPConnectEthernet();
        cleanup_clients = true;
    #endif

        init(command, data, port, data_port);
    }

    // destructor to clean up memory
     ~FTPClient(){
        if (cleanup_clients)   {
            delete command_ptr;
            delete data_ptr;
        } 
     }

    // opens the ftp connection  
    virtual  bool begin(IPAddress remote_addr, const char* user="anonymous", const char* password=nullptr){
        FTPLogger.writeLog( LOG_INFO, "FTPClient", "begin");
        this->userid = user;
        this->password = password;
        this->remote_addr = remote_addr;

        return api.open(this->command_ptr, this->data_ptr, remote_addr, this->port, this->data_port, user, password);
    }

    //call this when a card is removed. It will allow you to inster and initialise a new card.
    virtual  bool end(){
        FTPLogger.writeLog( LOG_INFO, "FTPClient", "end");
        return api.quit();
    }

    // get the file 
    virtual  FTPFile& open(const char *filename, FileMode mode = READ_MODE){
        FTPLogger.writeLog( LOG_INFO, "FTPClient", "open");
        return *(new FTPFile(&api, filename, mode ));
    }

    // Create the requested directory heirarchy--if intermediate directories
    // do not exist they will be created.
    virtual  bool mkdir(const char *filepath){
        FTPLogger.writeLog( LOG_INFO, "FTPClient", "mkdir");
        return api.mkdir(filepath);
    } 
    // Delete the file.
    virtual  bool remove(const char *filepath){
        FTPLogger.writeLog( LOG_INFO, "FTPClient", "remove");
        return api.del(filepath);
    }

    // Removes a directory
    virtual  bool rmdir(const char *filepath){
        FTPLogger.writeLog( LOG_INFO, "FTPClient", "rmdir");
        return api.rmd(filepath);
    }

    // lists all file names in the specified directory
    virtual FTPFileIterator ls(const char* path, FileMode mode = WRITE_MODE){
        FTPLogger.writeLog( LOG_INFO, "FTPClient", "ls");
        FTPFileIterator it(&api, path, mode);
        return it;
    }

  protected:
    void init(FtpIpClient *command, FtpIpClient *data, int port = COMMAND_PORT, int data_port = DATA_PORT){
        FTPLogger.writeLog( LOG_DEBUG, "FTPClient");
        this->command_ptr = command;
        this->data_ptr = data;
        this->port = port;
        this->data_port = data_port;
    }

    FTPBasicAPI api;    
    FtpIpClient *command_ptr;
    FtpIpClient *data_ptr;
    IPAddress remote_addr;
    const char* userid;
    const char* password;
    int port;
    int data_port;
    bool cleanup_clients;
};
