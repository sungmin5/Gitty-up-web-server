//
// async_tcp_echo_server.cpp
// ~~~~~~~~~~~~~~~~~~~~~~~~~
//
//
// Copyright (c) 2003-2017 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#include <cstdlib>
#include <iostream>
#include <string>
#include <boost/bind.hpp>
#include <boost/asio.hpp>
#include "session.h"

using boost::asio::ip::tcp;

//constructor takes a Boost io_service object
session::session(boost::asio::io_service& io_service, std::shared_ptr<router> rout)
: socket_(io_service), router_(rout) 
{
}

//socket returns the underlying socket object for this session
tcp::socket& session::socket () 
{
    return socket_;
}

//this method should be used for testing purposes only!
//pushes the input string into data_
bool session::set_data (std::string d)
{
    for (int i=0; i<d.length(); i++)
    {
        data_[i] = d[i];
    }
    return true;
}

//start executes an asynchronous read in which the server
//waits on data from the requestor
bool session::start() 
{
    try{
        ip_addr_ = socket_.remote_endpoint().address().to_string();
    }catch(std::exception e){
        ip_addr_ = "not given";
    }
    LOG_INFO << "IP: " << ip_addr_;
    return read();
}

//async read off of the socket
bool session::read()
{
    socket_.async_read_some(boost::asio::buffer(data_, max_length),
       boost::bind(&session::handle_read, this,
       boost::asio::placeholders::error,
       boost::asio::placeholders::bytes_transferred));

     return true;
}

//async write onto the socket
bool session::write(std::vector<char> data)
{
    boost::asio::async_write(socket_,
        boost::asio::buffer(data),
        boost::bind(&session::handle_write, this,
        boost::asio::placeholders::error));
    
    return true;
}

//close the given session
bool session::close()
{
    socket_.shutdown(boost::asio::ip::tcp::socket::shutdown_send);
    socket_.close();
        
    delete this;
}

//public handler for successful reads off of the socket
bool session::handle_success(size_t bytes_transferred)
{
    //parse raw request to a request object
    char* d = data_;
    LOG_INFO << std::string(d);
    std::shared_ptr<request> req(new request(d, bytes_transferred));
    
    //check request for validity
    if (req->is_valid())
    {
	   try
       {
           handle_valid_request(req);
           return true;
       }
       catch (const std::exception& e)
       {
           LOG_ERROR << std::string(e.what());
           handle_error(make_error_code(boost::system::errc::bad_message));
           return false;
       }
    }
    else
    {
	   handle_invalid_request(req);
       return false;
    }
}

//public handler for unsuccessful reads off of the socket
bool session::handle_error(const boost::system::error_code& error)
{
    std::shared_ptr<response> resp(new response(500, "Oops! Looks like something went wrong on our side. Please try again later."));
    resp->set_header("Content-Type", "text/plain");
    LOG_INFO << "ResponseCode::500::RequestHandler::none"; 
    std::string response_string = resp->generate_response();
    std::vector<char> bytes(response_string.begin(), response_string.end());
    write(bytes);
    return true;
}


//handle_read is a callback for when the session reads data from the requestor
void session::handle_read(const boost::system::error_code& error,
  size_t bytes_transferred) 
{
    if (!error) 
    { 
      handle_success(bytes_transferred);
    }
    else 
    {
      handle_error(error);
    }
}



//handle_write is a callback for when the server writes data to the requestor
void session::handle_write(const boost::system::error_code& error) 
{
    if (!error) 
    {
      close();
    }
    else 
    {
      delete this;
    }
}

//handle a request that was deemed syntactically valid
bool session::handle_valid_request(std::shared_ptr<request> req)
{
    holder_ = router_->route_request(req);
    std::string response_string = holder_->generate_response();
    std::vector<char> bytes(response_string.begin(), response_string.end());
    write(bytes);
    return true;
}

//handle a request that was deemed syntactically invalid
bool session::handle_invalid_request(std::shared_ptr<request> req)
{
    std::shared_ptr<response> resp(new response(400, std::string(req->get_raw_request())));
    resp->set_header("Content-Type", "text/plain");
    LOG_INFO << "ResponseCode::400::RequestHandler::none";
    std::string response_string = resp->generate_response();
    std::vector<char> bytes(response_string.begin(), response_string.end());
    write(bytes);
    return true;
}
