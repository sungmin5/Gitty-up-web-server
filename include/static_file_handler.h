//A handler for HTTP requests that serves static files
#ifndef STATIC_FILE_HANDLER_H
#define STATIC_FILE_HANDLER_H

#include <algorithm>
#include <cstdlib>
#include <iostream>
#include <unordered_map>
#include <vector>
#include <regex>
#include "route_handler.h"
#include "mime_types.h"

class static_file_handler : public route_handler
{
public:
    //Methods
    static std::shared_ptr<route_handler> create_handler (std::shared_ptr<NginxConfig> config, std::string root_path);
    std::shared_ptr<response> handle_request (std::shared_ptr<request> req); //given a request, generate an appropriate response
    std::string get_static_file_path (); //retrieve the file path for static files from the config
    std::vector<std::string> parse_file_info (std::string uri); //parse out file info from URI
    std::string get_mime_type (std::string extension); //get MIME type given an extension
private:    
    //Methods
    static_file_handler (std::shared_ptr<NginxConfig> config, std::string root_path); //constructor overload
    std::shared_ptr<response> serve_file (std::shared_ptr<request> req); //serve static file
    std::shared_ptr<response> invalid_method (std::shared_ptr<request> req); //generate invalid method response
    
    //Attributes
    std::string path_;
};

#endif

