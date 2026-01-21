#include "log.h"
#include <fstream>

bool is_tty = isatty(STDOUT_FILENO);

void Logger::log(LogLevel lvl, const std::string& s)
{
    if(this->logInConsole){
        if (isatty(STDOUT_FILENO)) {
            std::cout
                << color(lvl)
                << s
                << "\033[0m\n";
        } else {
            std::cout << s << '\n';
        }
    }
    if(this->logInFile){

        std::ofstream LogFile;
        LogFile.open(this->FileName, std::ios_base::app);
        LogFile << s << "\n";
        LogFile.close();
    }
}

void Logger::Clear(){
    std::ofstream LogFile(this->FileName);
    LogFile << "";
    LogFile.close();
}