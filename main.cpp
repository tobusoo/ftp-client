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

int STOR(FTPClient& ftp, Lexems& lexems)
{
    if (lexems.size() != 3) {
        std::cerr << "Wrong argument for STOR\n";
        std::cerr << "STOR <local-file> <remote-file>\n";
        return -1;
    }

    return ftp.STOR(lexems[1], lexems[2]);
}

int APPE(FTPClient& ftp, Lexems& lexems)
{
    if (lexems.size() != 3) {
        std::cerr << "Wrong argument for APPE\n";
        std::cerr << "APPE <local-file> <remote-file>\n";
        return -1;
    }

    return ftp.APPE(lexems[1], lexems[2]);
}

int DELE(FTPClient& ftp, Lexems& lexems)
{
    if (lexems.size() != 2) {
        std::cerr << "Wrong argument for DELE\n";
        std::cerr << "DELE <filename>\n";
        return -1;
    }

    return ftp.DELE(lexems[1]);
}

int MKD(FTPClient& ftp, Lexems& lexems)
{
    if (lexems.size() != 2) {
        std::cerr << "Wrong argument for MKD\n";
        std::cerr << "MKD <directory>\n";
        return -1;
    }

    return ftp.MKD(lexems[1]);
}

int RMD(FTPClient& ftp, Lexems& lexems)
{
    if (lexems.size() != 2) {
        std::cerr << "Wrong argument for RMD\n";
        std::cerr << "RMD <directory>\n";
        return -1;
    }

    return ftp.RMD(lexems[1]);
}

int action(FTPClient& ftp, Lexems& lexems)
{
    int resp = 0;
    auto cmd = lexems[0];

    if (cmd == "CWD") {
        resp = CWD(ftp, lexems);
    } else if (cmd == "PWD") {
        resp = ftp.PWD();
    } else if (cmd == "LIST") {
        resp = ftp.LIST(std::cout);
    } else if (cmd == "RETR") {
        resp = RETR(ftp, lexems);
    } else if (cmd == "STOR") {
        resp = STOR(ftp, lexems);
    } else if (cmd == "APPE") {
        resp = APPE(ftp, lexems);
    } else if (cmd == "DELE") {
        resp = DELE(ftp, lexems);
    } else if (cmd == "MKD") {
        resp = MKD(ftp, lexems);
    } else if (cmd == "RMD") {
        resp = RMD(ftp, lexems);
    } else {
        std::cerr << "Unknown command\n";
    }

    return resp;
}

void inputHandler(FTPClient& ftp)
{
    int resp = 0;
    const char* promt = "ftp> ";
    std::string input_line;
    std::cout << promt;

    while (std::getline(std::cin, input_line, '\n')) {
        auto lexems = get_lexems(input_line);
        if (lexems.size() == 0)
            continue;
        if (lexems[0] == "QUIT")
            return;

        resp = action(ftp, lexems);
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