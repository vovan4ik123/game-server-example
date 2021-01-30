#include "TcpServer.h"

int main()
{
    // take language from OS
    setlocale(LC_ALL, "");

    try
    {
        TcpServer tcpServer(100);
        tcpServer.start();

        //while (1) {}
    }
    catch (std::exception& e)
    {
        std::cerr << "catch (std::exception& e) " << e.what() << std::endl;
    }

    //std::cout <<  << std::endl;
    //std::cout << "" << std::endl;
    //std::cout << "" << << std::endl;

    std::cout << "Hello World!\n";
    system("PAUSE");
}