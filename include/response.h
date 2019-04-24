//An HTTP Response
#ifndef RESPONSE_H
#define RESPONSE_H

#include <cstdlib>
#include <iostream>
#include <string>
#include <unordered_map>
#include "response_codes.h"

class response
{
public:
    //Methods
    response (unsigned int status_code, std::string body); //constructor overload
    unsigned int get_status_code (); //retrieve status code
    std::string get_header(std::string name); //retrieve a given header by name
    std::string get_body (); //retrieve response body
    void set_status_code (unsigned int sc); //set the status code
    void set_header (std::string h_name, std::string h_value); //set a specific header
    void set_body (std::string body); //set the body
    std::string generate_response (); //generate a proper HTTP response
private:
    //Methods
    std::string generate_status_line (); //create the status line of the response
    std::string generate_header_lines (); //create the header lines of the response
    std::string get_status_word (unsigned int status); //get the status description
    
    //Attributes
    unsigned int status_code_;
    std::unordered_map<std::string, std::string> response_headers_;
    std::string response_body_;
    std::string response_;
};

#endif

