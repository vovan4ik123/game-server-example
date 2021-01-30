#pragma once

#if defined(_WIN32) && !defined(_WIN32_WINNT)
    #define _WIN32_WINNT 0x0601  // minimum suported version = win 7, (0x0A00 for win 10)
#endif

#include <iostream>
#include <string>
#include <boost/asio.hpp>
#include <boost/array.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/enable_shared_from_this.hpp>
#include <boost/bind/bind.hpp>
#include <thread>
#include <chrono>
#include <atomic>
#include "ThreadSafeString.h"

class TcpConnection : public boost::enable_shared_from_this<TcpConnection>
{
public:
    using pointer = boost::shared_ptr<TcpConnection>;
    using tcp = boost::asio::ip::tcp;

    static pointer create(boost::asio::io_context& ioContext)
    {
        return pointer(new TcpConnection(ioContext));
    }

    ~TcpConnection()
    {
        disconnect();

        std::cout << "~TcpConnection()" << std::endl;
    }

    tcp::socket& getSocket()
    {
        return m_socket;
    }

    bool isConnected()
    {
        return m_connected;
    }

    void start()
    {
        m_connected = true;
        readFromClient();
    }

    void disconnect()
    {
        m_connected = false;
        if (m_socket.is_open())
        {
            m_socket.close();
        }
    }

    void writeAnswer()
    {
        std::scoped_lock lock(mutexWriting);

        if (!m_connected || messageOut.empty() || !m_writingFinished) { return; }

        m_serverAnswer = messageOut.getMove();
        m_serverAnswer += m_endOfMessage;

        boost::asio::async_write(m_socket,
                                 boost::asio::buffer(m_serverAnswer),
                                 boost::bind(&TcpConnection::writeHandler,
                                             shared_from_this(),
                                             boost::asio::placeholders::error,
                                             boost::asio::placeholders::bytes_transferred));

        m_writingFinished = false;
    }

    ThreadSafeString messageIn;
    ThreadSafeString messageOut;

private:
    TcpConnection(boost::asio::io_context& ioContext)
        : m_socket(ioContext)
    {
        std::cout << "TcpConnection() " << std::endl;
    }

    void readFromClient()
    {
        if (!m_connected) { return; }
        //std::cout << "readFromClient()" << std::endl;

        m_clientRequest.clear();
        m_clientRequest.reserve(m_maxDataLength);

        boost::asio::async_read_until(m_socket,
                                      boost::asio::dynamic_buffer(m_clientRequest),
                                      m_endOfMessage, // indicates that received message is completed. stop receiving, go to call back
                                      boost::bind(&TcpConnection::readHandler,
                                                  shared_from_this(),
                                                  boost::asio::placeholders::error,
                                                  boost::asio::placeholders::bytes_transferred));
    }
    void readHandler(const boost::system::error_code& error, size_t bytes_transferred)
    {
        if (error &&
            (error == boost::asio::error::eof || // clients socket is closed
             error.category() == boost::asio::error::system_category)) // clients system error (app is closed)
        {
            std::cout << "disconnect: " << error.message() << std::endl;
            disconnect();
            return;
        }

        //std::cout << "Received: " << m_clientRequest << std::endl;
        messageIn.setMove(std::move(m_clientRequest));

        readFromClient();
    }

    void writeHandler(const boost::system::error_code& error, size_t bytes_transferred)
    {
        std::scoped_lock lock(mutexWriting);
        m_writingFinished = true;
    }

    tcp::socket m_socket;

    std::atomic<bool> m_connected = false;
    bool m_writingFinished = true;
    std::mutex mutexWriting;

    const int m_maxDataLength = 1024;
    std::string m_clientRequest;
    std::string m_serverAnswer;
    const std::string m_endOfMessage = " E_O_M "; // end of every sended/received completed message

};