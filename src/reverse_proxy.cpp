#include "reverse_proxy.h"
#include "reverse_proxy_session.h"
#include <iostream>

reverse_proxy::reverse_proxy(unsigned short listen_port, std::string proxy_host, unsigned short proxy_port)
{
    this->listen_port = listen_port;
    this->proxy_host = proxy_host;
    this->proxy_port = proxy_port;
    context = boost::make_shared<io_context>();
}

void reverse_proxy::run()
{
    const auto listen_endpoint = boost::make_shared<tcp::endpoint>(tcp::endpoint(tcp::v4(), listen_port));
    acceptor = boost::make_shared<tcp::acceptor>(*context);
    acceptor->open(listen_endpoint->protocol());
    acceptor->bind(*listen_endpoint);
    acceptor->listen();

    handle_accept();
    std::cout << "Listening on port " << listen_port << "\n";
    try
    {
        boost::system::error_code ec;
        context->run(ec);
    }
    catch (std::runtime_error& ex)
    {
        std::cout << "Runtime error: " << ex.what() << "\n";
    }
}

void reverse_proxy::erase_session(reverse_proxy_session* session)
{
    sessions.erase(std::remove(sessions.begin(), sessions.end(), session), sessions.end());
    delete session;
}

void reverse_proxy::handle_accept()
{
    auto session = new reverse_proxy_session(this);
    sessions.push_back(session);
    boost::shared_ptr<tcp::socket>& client_socket = session->client_socket;
    acceptor->async_accept(*client_socket, [this, &client_socket, session](const boost::system::error_code& ec)
    {
        if (!ec)
        {
            std::cout << "Client " << client_socket->remote_endpoint().address() << " connected\n";
            session->connect(proxy_host, proxy_port);
        }
        else
        {
            std::cout << "Client connection failed: " << ec.message() << "\n";
            erase_session(session);
        }

        handle_accept();
    });
}
