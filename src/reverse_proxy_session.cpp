#include "reverse_proxy_session.h"
#include "reverse_proxy.h"
#include <iostream>

reverse_proxy_session::reverse_proxy_session(reverse_proxy* proxy)
{
    this->proxy = proxy;
    this->client_socket = boost::make_shared<tcp::socket>(*proxy->context);
}

void reverse_proxy_session::connect(std::string proxy_host, unsigned short proxy_port)
{
    client_connected = true;
    tcp::resolver resolver(*proxy->context);
    tcp::resolver::iterator endpoints = resolver.resolve(proxy_host, std::to_string(proxy_port));
    std::cout << "Proxy connecting\n";
    this->proxy_socket = boost::make_shared<tcp::socket>(*proxy->context);
    proxy_socket->async_connect(endpoints->endpoint(),
                                [this](const boost::system::error_code& ec)
                                {
                                    if (!ec)
                                    {
                                        std::cout << "Proxy connected\n";
                                        proxy_connected = true;
                                        handle_client_read();
                                        handle_proxy_read();
                                    }
                                    else
                                    {
                                        std::cout << "Proxy connection failed: " << ec.what() << "\n";
                                        handle_close();
                                    }
                                });
}

void reverse_proxy_session::handle_proxy_read()
{
    proxy_socket->async_read_some(boost::asio::buffer(server_buffer, sizeof(server_buffer))
                                  , [this](
                                  const boost::system::error_code& ec, size_t bytes_read)
                                  {
                                      if (!ec && bytes_read > 0)
                                      {
                                          client_socket->async_write_some(
                                              boost::asio::buffer(server_buffer, bytes_read),
                                              [](const boost::system::error_code& write_ec, size_t size)
                                              {
                                              });
                                          handle_proxy_read();
                                      }
                                      else
                                      {
                                          if (ec)
                                          {
                                              std::cout << "Proxy read failed: " << ec.message() << "\n";
                                          }
                                          std::cout << "Proxy closed\n";
                                          proxy_connected = false;
                                          handle_close();
                                      }
                                  });
}

void reverse_proxy_session::handle_client_read()
{
    client_socket->async_read_some(boost::asio::buffer(client_buffer, sizeof(client_buffer))
                                   , [this](
                                   const boost::system::error_code& ec, size_t bytes_read)
                                   {
                                       if (!ec && bytes_read > 0)
                                       {
                                           proxy_socket->async_write_some(
                                               boost::asio::buffer(client_buffer, bytes_read),
                                               [](const boost::system::error_code& write_ec, size_t size)
                                               {
                                               });
                                           handle_client_read();
                                       }
                                       else
                                       {
                                           if (ec)
                                           {
                                               std::cout << "Client read failed: " << ec.message() << "\n";
                                           }
                                           std::cout << "Client closed\n";
                                           client_connected = false;
                                           handle_close();
                                       }
                                   });
}

void reverse_proxy_session::handle_close()
{
    if (client_socket.get() != nullptr)
    {
        client_socket->close();
    }
    if (proxy_socket.get() != nullptr)
    {
        proxy_socket->close();
    }

    if (!proxy_connected && !client_connected && !session_closed)
    {
        session_closed = true;
        proxy->erase_session(this);
    }
}
