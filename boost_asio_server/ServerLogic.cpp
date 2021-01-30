#include "ServerLogic.h"

ServerLogic::ServerLogic()
{

}

ServerLogic::~ServerLogic()
{

}

void ServerLogic::handleClientsMessages(ThreadSafeVector<TcpConnection::pointer>& clients)
{
    for (int i = 0; i < clients.size(); ++i)
    {
        if (clients[i]->isConnected())
        {
            if (!clients[i]->messageIn.empty())
            {
                //std::cout << "!clients[i]->messageIn.empty()" << std::endl;
                std::string request = clients[i]->messageIn.getMove();
            }

            std::string out = "answer to client " + std::to_string(i);
            clients[i]->messageOut.setMove(std::move(out));
        }
    }

    std::this_thread::sleep_for(std::chrono::milliseconds(15));
    
    //std::cout << "" << std::endl;
}