#pragma once
#include <boost/asio.hpp>

using namespace boost::asio;
using namespace boost::asio::ip;

class reverse_proxy;

class reverse_proxy_session
{
public:
    reverse_proxy* proxy;
    boost::shared_ptr<tcp::socket> client_socket;
    boost::shared_ptr<tcp::socket> proxy_socket;
    boost::shared_ptr<tcp::resolver::iterator> resolver_iterator;

private:
    char server_buffer[4096];
    char client_buffer[4096];

    bool client_connected = false;
    bool proxy_connected = false;
    bool session_closed = false;

public:
    reverse_proxy_session(reverse_proxy* proxy);

    // Connect to the proxied server
    void connect(std::string proxy_host, unsigned short proxy_port);

    // Forward data from the proxied server to the client
    void handle_proxy_read();

    // Forward data from the client to the proxied server
    void handle_client_read();

private:
    // Close sockets and erase session
    void handle_close();
};
