#pragma once
#include <boost/asio.hpp>
#include <boost/thread.hpp>

class reverse_proxy_session;

using namespace boost::asio;
using namespace boost::asio::ip;

class reverse_proxy
{
public:
    unsigned short listen_port;
    std::string proxy_host;
    unsigned short proxy_port;

    boost::shared_ptr<io_context> context;
    boost::shared_ptr<tcp::acceptor> acceptor;

private:
    std::vector<reverse_proxy_session*> sessions;

public:
    reverse_proxy(unsigned short listen_port, std::string proxy_host, unsigned short proxy_port);

    void run();
    void erase_session(reverse_proxy_session* session);

private:
    void handle_accept();
};
