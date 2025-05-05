#include <FTPClient.hpp>

#include <fstream>
#include <iostream>
#include <regex>

#include <netdb.h>
#include <netinet/in.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

FTPClient::FTPClient(const char* host, int port)
{
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(port);

    hostent* host_entry = gethostbyname(host);
    if (!host_entry) {
        throw "Could not resolve hostname";
    }

    memcpy(&servaddr.sin_addr, host_entry->h_addr_list[0], host_entry->h_length);
}

void FTPClient::SetControlStream(std::ostream* out)
{
    control_out = out;
}

int FTPClient::Connect()
{
    control_sock = socket(AF_INET, SOCK_STREAM, 0);
    if (control_sock == -1) {
        perror("control socket");
        return -1;
    }

    if (connect(control_sock, (struct sockaddr*)&servaddr, sizeof(servaddr))) {
        perror("connect to control port");
        return -1;
    }

    int bytes_received = recv(control_sock, control_buffer, CONTROL_BUFFER_LEN, 0);
    if (bytes_received == -1) {
        close(control_sock);
        perror("recv from control port");
        return -1;
    }

    control_buffer[bytes_received] = '\0';
    if (control_out)
        *control_out << control_buffer;

    return 0;
}

int FTPClient::Login(const std::string& user, const std::string& password)
{
    if (sendCommand("USER " + user, false) == -1)
        return -1;
    if (!checkResponseCode("331"))
        return -1;

    if (sendCommand("PASS " + password) == -1)
        return -1;
    if (!checkResponseCode("230"))
        return -1;

    return 0;
}

int FTPClient::CWD(const std::string& dir)
{
    if (sendCommand("CWD " + dir) == -1)
        return -1;
    if (!checkResponseCode("250"))
        return -1;

    return 0;
}

int FTPClient::PWD()
{
    if (sendCommand("PWD") == -1)
        return -1;
    if (!checkResponseCode("257"))
        return -1;

    return 0;
}

int FTPClient::LIST(std::ostream& out)
{
    if (PASV() == -1)
        return -1;

    if (sendCommand("LIST") == -1)
        return -1;

    int resp = writeDataTo(out);
    close(data_sock);
    resp |= getControlResponse();

    return resp;
}

int FTPClient::RETR(const std::string& remote_file, const std::string& local_file)
{
    if (PASV() == -1)
        return -1;

    if (sendCommand("RETR " + remote_file) == -1)
        return -1;
    if (!checkResponseCode("150"))
        return -1;

    std::ofstream out_file(local_file, std::ios::binary);
    if (!out_file) {
        if (control_out)
            *control_out << "Can't create" << local_file << "file\n";
        close(data_sock);
        return -1;
    }

    int resp = writeDataTo(out_file);
    close(data_sock);
    resp |= getControlResponse();

    return resp;
}

void FTPClient::Disconnect()
{
    if (control_sock != -1) {
        if (*control_out)
            *control_out << '\n';
        sendCommand("QUIT");
        close(control_sock);
    }
    if (data_sock != -1) {
        close(data_sock);
    }
}

FTPClient::~FTPClient()
{
    Disconnect();
}

int FTPClient::sendCommand(const std::string& cmd, bool need_out)
{
    std::string full_cmd = cmd + "\r\n";
    send(control_sock, full_cmd.c_str(), full_cmd.size(), 0);

    return getControlResponse(need_out);
}

bool FTPClient::checkResponseCode(const std::string& expect)
{
    return strncmp(control_buffer, expect.c_str(), expect.size()) == 0;
}

int FTPClient::getControlResponse(bool need_out)
{
    int bytes_received = recv(control_sock, control_buffer, CONTROL_BUFFER_LEN, 0);
    if (bytes_received == -1) {
        perror("control response");
        return -1;
    }

    control_buffer[bytes_received] = '\0';
    if (need_out && control_out)
        *control_out << control_buffer;

    return 0;
}

int FTPClient::writeDataTo(std::ostream& out)
{
    while (true) {
        int bytes_received = recv(data_sock, data_buffer, DATA_BUFFER_LEN, 0);
        if (bytes_received == -1) {
            perror("writeDataTo recv");
            return -1;
        } else if (bytes_received == 0) {
            return 0;
        }

        data_buffer[bytes_received] = '\0';
        out << data_buffer;
    }
}

int FTPClient::PASV()
{
    if (sendCommand("PASV") == -1)
        return -1;
    if (!checkResponseCode("227"))
        return -1;

    const std::string response = control_buffer;
    std::regex digit_regex("(\\d+),(\\d+),(\\d+),(\\d+),(\\d+),(\\d+)");
    std::smatch matches;
    std::regex_search(response, matches, digit_regex);

    if (matches.size() != 7)
        return -1;

    int port = (std::stoi(matches[5].str()) << 8) + std::stoi(matches[6].str());

    data_sock = socket(AF_INET, SOCK_STREAM, 0);
    if (data_sock < 0) {
        perror("socket");
        return -1;
    }

    sockaddr_in data_addr{};
    data_addr.sin_family = AF_INET;
    data_addr.sin_port = htons(port);
    data_addr.sin_addr = servaddr.sin_addr;

    if (connect(data_sock, (struct sockaddr*)&data_addr, sizeof(data_addr))) {
        close(data_sock);
        perror("connect data");
        return false;
    }

    return 0;
}
