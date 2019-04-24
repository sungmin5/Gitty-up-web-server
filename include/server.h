// A TCP web server
#ifndef SERVER_H
#define SERVER_H

#include "session.h"
#include "config_parser.h"
#include "logger.h"
#include "router.h"
#include <boost/thread/thread.hpp>

using boost::asio::ip::tcp;

class server 
{
public:
  //Methods
  server(boost::asio::io_service& io_service, NginxConfig config, short port, unsigned int threads); //constructor
  void run(); // run server with multiple threads
  bool create_router(std::string server_root); //create a router for the server to use
  bool start_accept(); //accept an incoming connection
  bool get_status();
  bool register_route(std::string uri, std::string route_handler); //register a route in the router
  bool register_default_header(std::string header_name, std::string header_value); //reguster a default header in the router
private:
  //Methods
  void handle_accept(session* new_session, const boost::system::error_code& error); //handle async accept

  //Attributes
  boost::asio::io_service& io_service_;
  tcp::acceptor acceptor_;
  bool isRunning = false;
  NginxConfig config_;
  std::shared_ptr<router> router_;
  unsigned int thread_count_;
};

#endif