//A handler for HTTP requests that serves static files

#include <algorithm>
#include <cstdlib>
#include <iostream>
#include <sstream>
#include <fstream>
#include <string>
#include <unordered_map>
#include "static_file_handler.h"
#include <vector>

#include <limits.h>
#include <unistd.h>

//overriden constructor takes a config
//for this route, a config is required that contains a path to the static images
static_file_handler::static_file_handler (std::shared_ptr<NginxConfig> config, std::string root_path) : route_handler(config, root_path) 
{
    path_ = (root_path_ + get_static_file_path());
}

//overriden factory method to create a new instance
std::shared_ptr<route_handler> static_file_handler::create_handler (std::shared_ptr<NginxConfig> config, std::string root_path)
{
    return std::shared_ptr<static_file_handler>(new static_file_handler(config, root_path));
}

//overridden method in parent class to handle a request
std::shared_ptr<response> static_file_handler::handle_request (std::shared_ptr<request> req)
{
    std::string method = req->get_method();
    if (method == "GET")
    {
        return serve_file(req);
    }
    else
    {
        return invalid_method(req);
    }
}


//parse config to retrieve the static file path
std::string static_file_handler::get_static_file_path ()
{
    std::lock_guard<std::mutex> lock(config_lock_);
    
    std::string output;
    for (const auto& statement : config_->statements_) 
    {
        //statement we're looking for has exactly 2 tokens
         if (statement->tokens_.size() == 2)
         {
             if (statement->tokens_[0] == "root")
             {
                 output = statement->tokens_[1];
             }
         }
    }
    if (!output.empty())
    {
        return output;
    }
    else
    {
        return output;
    }
}

//parse relevant information from the file requested in the URI
std::vector<std::string> static_file_handler::parse_file_info (std::string uri)
{
    std::vector<std::string> file_info;
    std::regex r("\\/(\\w+)(\\.\\w+)$");
    std::smatch m;
    std::regex_search(uri, m, r);
    
    if (m.size() == 3) 
    {
        file_info.push_back(m.str(1)); //file name
        file_info.push_back(m.str(2)); //file extension
    }
    
    return file_info;
}

//return the appropriate MIME type given a file extension
std::string static_file_handler::get_mime_type (std::string extension)
{
    //mime_types is defined in mime_types.h
    std::unordered_map<std::string, std::string>::const_iterator found = mime_types.find(extension);
    if (!(found == mime_types.end()))
    {
        return found->second;
    }
    else
    {
        //this is a catch-all MIME type in case the requested extension isn't currently supported
        return "application/octet-stream";
    }   
}

//serve the file indicated by the URI
//if the file isn't present, return 404
std::shared_ptr<response> static_file_handler::serve_file (std::shared_ptr<request> req)
{
    try
    {
        std::vector<std::string> file_info = parse_file_info(req->get_uri());
        if (file_info.size() < 2)
        {
            std::shared_ptr<response> resp(new response(404, "The requested file could not be found!\n"));
            resp->set_header("Content-Type", "text/plain");
            return resp;
        }
        
        std::ifstream t(path_ + "/" + file_info[0] + file_info[1], std::ios::in | std::ios::binary);
        if (!t.is_open())
        {
            std::shared_ptr<response> resp(new response(404, "The requested file could not be found!\n"));
            resp->set_header("Content-Type", "text/plain");
            return resp;
        }
        std::ostringstream oss;
        oss << t.rdbuf();
        std::string f(oss.str());
        
        std::shared_ptr<response> resp(new response(200, f));
        resp->set_header("Content-Type", get_mime_type(file_info[1]));
        return resp;
    }
    catch (int e)
    {
        std::shared_ptr<response> resp(new response(500, "An error occurred while trying to retrieve the requested file!\n"));
        resp->set_header("Content-Type", "text/plain");
        return resp;
    }
}

//the method used in the request wasn't valid so return a 405
std::shared_ptr<response> static_file_handler::invalid_method(std::shared_ptr<request> req)
{
    std::shared_ptr<response> resp(new response(405, "This route only supports the HTTP GET method!"));
    resp->set_header("Content-Type", "text/plain");
    return resp;
}