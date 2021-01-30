#pragma once

#include "TcpConnection.h"
#include "ServerLogic.h"
#include "ThreadSafeVector.h"
#include <atomic>

class TcpServer
{
public:
    using tcp = boost::asio::ip::tcp;

    TcpServer(int port) : m_acceptor(m_ioContext, tcp::endpoint(tcp::v4(), port))
    {
        m_connections.reserve(m_maxConnectionsCount);
    }

    ~TcpServer()
    {
        stop();
    }

    void start()
    {
        m_serverRunning = true;
        startAccept();
        // run server logic in other thread
        m_serverLogicThread = std::thread(&TcpServer::handleClients, this);

        // run read/write operations + handlers in other thread
        // should have started work before we call run()
        // all call back handlers will calling in thread where m_ioContext.run() called
        // even if async read/send was called in other thread.
        //m_ioContextThread = std::thread([this]() { m_ioContext.run(); });
        m_ioContext.run();

        std::cout << "start()" << std::endl;
    }

    void stop()
    {
        for (int i = 0; i < m_connections.size(); ++i)
        {
            if (m_connections[i]->isConnected()) // stop all clients
                m_connections[i]->disconnect();
        }

        m_serverRunning = false;

        if (!m_ioContext.stopped()) { m_ioContext.stop(); }
        if (m_serverLogicThread.joinable()) m_serverLogicThread.join();
        //if (m_ioContextThread.joinable()) m_ioContextThread.join();
    }

private:
    void startAccept()
    {
        TcpConnection::pointer newConnection = TcpConnection::create(m_ioContext);

        m_acceptor.async_accept(newConnection->getSocket(),
            boost::bind(&TcpServer::handleAccept,
                this,
                newConnection,
                boost::asio::placeholders::error));
    }

    void handleAccept(TcpConnection::pointer newConnection, const boost::system::error_code& error)
    {
        if (!error)
        {
            bool haveDisconnected = false;
            for (int i = 0; i < m_connections.size(); ++i)
            {
                if (!m_connections[i]->isConnected()) // look for disconnected client
                {
                    m_connections.setElement(i, newConnection);
                    newConnection->start();
                    haveDisconnected = true;
                    break;
                }
            }

            if (!haveDisconnected && m_connections.size() < m_maxConnectionsCount)
            {
                m_connections.pushBack(newConnection);
                newConnection->start();
            }
            std::cout << "m_connections.size(): " << m_connections.size() << std::endl;
        }

        startAccept();
    }

    void handleClients()
    {
        while (m_serverRunning)
        {
            // do server logic
            m_serverLogic.handleClientsMessages(m_connections);

            // send answer for all connected clients
            for (int i = 0; i < m_connections.size(); ++i)
            {
                if (m_connections[i]->isConnected())
                {
                    m_connections[i]->writeAnswer();
                }
            }
        }
    }

    std::atomic<bool> m_serverRunning = false;

    ServerLogic m_serverLogic;
    std::thread m_serverLogicThread;

    ThreadSafeVector<TcpConnection::pointer> m_connections; // clients
    const int m_maxConnectionsCount = 1000; // for preventing vector reallocation. reserve that and dont push more

    boost::asio::io_context m_ioContext;
    std::thread m_ioContextThread;
    tcp::acceptor m_acceptor;
};