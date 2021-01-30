#pragma once

#include "TcpConnection.h"
#include "ThreadSafeVector.h"

class ServerLogic
{
public:
    ServerLogic();
    ~ServerLogic();

    void handleClientsMessages(ThreadSafeVector<TcpConnection::pointer>& clients);

private:

};

