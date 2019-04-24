//An HTTP Request

#include <cstdlib>
#include <iostream>
#include <string>
#include <regex>
#include "request.h"


//constructor takes an array of characters and size for that array
request::request(char* data, size_t data_size)
{
    //capture raw request (copy the data)
    raw_request_ = new char[data_size];
    request_size_ = data_size;
    for (size_t i=0; i<data_size; i++)
    {
        raw_request_[i] = data[i];
    }
    
    //extract useful information from the request
    parse_request();
}

//get_raw_request returns the raw request as an array of characters
char* request::get_raw_request () 
{
    return raw_request_;
}

//get_request_size returns the size of the raw request
size_t request::get_request_size () 
{
    return request_size_;
}

//get_method returns the HTTP method of the request
std::string request::get_method () 
{
    return method_;
}

//get_uri returns the uri of the request as a string
std::string request::get_uri () 
{
    return uri_;
}

//get_header returns the header value as a string corresponding to the provided header name
std::string request::get_header (std::string name) 
{
    std::unordered_map<std::string, std::string>::const_iterator found = request_headers_.find(name);
    if (!(found == request_headers_.end()))
    {
        return found->second;
    }
    else
    {
        return "";
    }
}

//get_query_string_param returns the value corresponding to the given query string parameter name (if it exists)
std::string request::get_query_string_param (std::string name) 
{
    std::unordered_map<std::string, std::string>::const_iterator found = query_string_.find(name);
    if (!(found == query_string_.end()))
    {
        return found->second;
    }
    else
    {
        return "";
    }
}

//get_body returns the body of the request as a string
//TODO: how to represent empty body?
std::string request::get_body () 
{
    return request_body_;
}

//is_valid returns whether or not the request is valid for HTTP
bool request::is_valid () 
{
    return valid_;
}

//parse_request sets all request data attributes for the request object
void request::parse_request ()
{
    std::string req = "";
    for (size_t i=0; i<request_size_; i++)
    {
        req += raw_request_[i];
    }
    
    valid_ = false;
    bool out1 = parse_status_line(req);
    if (out1)
    {
        bool out2 = parse_headers(req);
        if (out2)
        {
            bool out3 = parse_body(req);
            if (out3)
            {
                valid_ = true;
            }
        }
    }
}

//parse_status_line captures the HTTP status line for the request
bool request::parse_status_line(std::string req)
{
    std::regex r("^(\\w+) (.+) HTTP\\/(\\d\\.\\d)\\\r\\\n");
    std::smatch m;
    std::regex_search(req, m, r);
    
    if (m.size() == 4) 
    {
        method_ = m.str(1);
        uri_ = m.str(2);
        bool qs = parse_query_string();
        return qs;
    }
    else
    {
        return false;
    }
    
}

//parse_headers captures the HTTP headers for the request
//the method loops the raw request and outputs the offset corresponding to the end of the headers
bool request::parse_headers(std::string req)
{
    std::regex r("\\\n(.+)\\: (.+)\\\r");
    std::sregex_iterator iter(req.begin(), req.end(), r);
    std::sregex_iterator end;
    
    while (iter != end)
    {
        if (iter->size() == 3)
        {
            std::pair<std::string, std::string> h ((*iter)[1], (*iter)[2]);
            request_headers_.insert(h);
        }
        else
        {
            return false;
        }
        ++iter;
    }
    
    return true;
}

//parse_body captures the body for the request
//the method loops the raw request and outputs the offset corresponding to the end of the request
bool request::parse_body(std::string req)
{
    std::regex r("\\\r\\\n\\\r\\\n(.+)$");
    std::smatch m;
    std::regex_search(req, m, r);
    
    if (m.size() > 0)
    {
        request_body_ = m.str(1);
    }
    
    return true;
}

//extract the query string from the URI if it is present
bool request::parse_query_string()
{
    std::regex r("^(.+)\\?(.*)$");
    std::smatch m;
    std::regex_search(uri_, m, r);
    
    //query string is present
    if (m.size() == 3)
    {
        uri_ = m.str(1);
        std::string qs = m.str(2);
        std::regex r1("(.+?)=(.+?)(&*)");
        std::sregex_iterator iter(qs.begin(), qs.end(), r1);
        std::sregex_iterator end;
        
        while (iter != end)
        {
            if (iter->size() == 4)
            {
                std::pair<std::string, std::string> h ((*iter)[1], (*iter)[2]);
                query_string_.insert(h);
            }
            else
            {
                return false;
            }
            ++iter;
        }
    }
    
    return true;
}