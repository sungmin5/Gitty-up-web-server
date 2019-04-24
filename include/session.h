// A TCP session
#ifndef SESSION_H
#define SESSION_H

#include <cstdlib>
#include <iostream>
#include <boost/bind.hpp>
#include <boost/asio.hpp>
#include <thread>
#include "request.h"
#include "response.h"
#include "router.h"
#include "logger.h"

using boost::asio::ip::tcp;

class session 
{
public:
  //Methods
  session(boost::asio::io_service& io_service, std::shared_ptr<router> rout); //constructor
  tcp::socket& socket(); //get underlying socket object
  bool set_data (std::string d); //method to set data explicitly
  bool start(); //start a session by opening a socket
  bool read(); //read off of the socket
  bool write(std::vector<char> data); //write on the socket
  bool close(); //close the socket
  bool handle_success(size_t bytes_transferred); //handler for successful reads on the socket
  bool handle_error(const boost::system::error_code& error); //handler for errors while reading on the socket
private:
  //Methods
  void handle_read(const boost::system::error_code& error, size_t bytes_transferred); //handle async reads
  void handle_write(const boost::system::error_code& error); //handle async writes
  bool handle_valid_request(std::shared_ptr<request> req); //handler for valid requests
  bool handle_invalid_request(std::shared_ptr<request> req); //handler for invalid requests

  //Attributes
  tcp::socket socket_;
  std::string ip_addr_;
  enum { max_length = 1024 };
  char data_[max_length];
  std::shared_ptr<router> router_;
  std::shared_ptr<response> holder_;
};

#endif
