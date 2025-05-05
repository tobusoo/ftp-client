#include <FTPClient.hpp>

#include <iostream>
#include <sstream>
#include <vector>

using Lexems = std::vector<std::string>;

Lexems get_lexems(const std::string& line)
{
    auto iss = std::istringstream(line);
    Lexems lexems;
    std::string lexem;

    while (iss >> lexem) {
        lexems.push_back(lexem);
    }

    return lexems;
}

int CWD(FTPClient& ftp, Lexems& lexems)
{
    if (lexems.size() != 2) {
        std::cerr << "Wrong argument for CWD\n";
        std::cerr << "CWD <remote-dir>\n";
        return -1;
    }

    return ftp.CWD(lexems[1]);
}

int RETR(FTPClient& ftp, Lexems& lexems)
{
    if (lexems.size() != 3) {
        std::cerr << "Wrong argument for RETR\n";
        std::cerr << "RETR <remote-file> <local-file>\n";
        return -1;
    }

    return ftp.RETR(lexems[1], lexems[2]);
}

void inputHandler(FTPClient& ftp)
{
    int resp = 0;
    const char* promt = "ftp> ";
    std::string input_line;
    std::cout << promt;

    while (std::getline(std::cin, input_line, '\n')) {
        auto lexems = get_lexems(input_line);
        auto cmd = std::move(lexems[0]);

        if (cmd == "CWD") {
            resp = CWD(ftp, lexems);
        } else if (cmd == "PWD") {
            resp = ftp.PWD();
        } else if (cmd == "LIST") {
            resp = ftp.LIST(std::cout);
        } else if (cmd == "RETR") {
            resp = RETR(ftp, lexems);
        } else {
            std::cerr << "Unknown command\n";
        }

        if (resp != 0) {
            std::cerr << "Failed to: " << lexems[0] << '\n';
        }
        std::cout << promt;
    }
}

int main(int argc, char* argv[])
{
    if (argc != 4) {
        std::cerr << "Usage: ftp <host> <user> <password>\n";
        return EXIT_FAILURE;
    }

    auto host = argv[1];
    auto user = argv[2];
    auto password = argv[3];

    FTPClient ftp(host);
    ftp.SetControlStream(&std::cout);

    if (ftp.Connect() == -1) {
        std::cerr << "Failed to connect\n";
        return EXIT_FAILURE;
    }

    if (ftp.Login(user, password) == -1) {
        std::cerr << "Failed to login\n";
        return EXIT_FAILURE;
    }

    inputHandler(ftp);

    return 0;
}