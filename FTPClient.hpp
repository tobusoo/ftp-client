#pragma once

#include <arpa/inet.h>
#include <string>

#define CONTROL_BUFFER_LEN 4084
#define DATA_BUFFER_LEN (4084 * 1024)

class FTPClient {
public:
    FTPClient(const char* host, int port = 21);
    void SetControlStream(std::ostream* out);
    int Connect();
    int Login(const std::string& user, const std::string& password);

    int CWD(const std::string& dir);
    int PWD();
    int LIST(std::ostream& result);
    int RETR(const std::string& remote_file, const std::string& local_file);

    void Disconnect();
    ~FTPClient();

private:
    int sendCommand(const std::string& cmd, bool need_out = true);
    bool checkResponseCode(const std::string& expect);
    int getControlResponse(bool need_out = true);
    int writeDataTo(std::ostream& out);

    int PASV();

private:
    struct sockaddr_in servaddr;
    int port;
    int control_sock;
    int data_sock;

    char control_buffer[CONTROL_BUFFER_LEN];
    char data_buffer[DATA_BUFFER_LEN];
    std::ostream* control_out;
};