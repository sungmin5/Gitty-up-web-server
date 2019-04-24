//An HTTP Request
#ifndef REQUEST_H
#define REQUEST_H

#include <cstdlib>
#include <iostream>
#include <string>
#include <unordered_map>

class request
{
public:
  //Methods
  request(char* data, size_t data_size); //constructor
  char* get_raw_request(); //write out a request as an HTTP string
  size_t get_request_size(); //retrieve the size of the raw request
  std::string get_method(); //retrieve the request method
  std::string get_uri(); //retrieve the URI for the request
  std::string get_header(std::string name); //retrieve a request header by name
  std::string get_body(); //retrieve the request body
  std::string get_query_string_param(std::string name); //retrieve query string parameter
  bool is_valid(); //retrieve whether or not the request is valid for HTTP
private:
  //Methods
  void parse_request(); //main request information extraction method
  bool parse_status_line(std::string req); //extract status line info from raw
  bool parse_headers(std::string req); //extract headers from raw
  bool parse_body(std::string req); //extract body from raw
  bool parse_query_string(); //extract query string from URI

  //Attributes
  char* raw_request_;
  size_t request_size_;
  std::string method_;
  std::string uri_;
  std::unordered_map<std::string, std::string> request_headers_;
  std::string request_body_;
  std::unordered_map<std::string, std::string> query_string_;
  bool valid_;
};

#endif