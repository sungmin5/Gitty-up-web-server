//A router to delegate HTTP requests to appropriate handlers

#include <cstdlib>
#include <iostream>
#include <string>
#include <unordered_map>
#include "router.h"
#include "echo_handler.h"
#include "static_file_handler.h"
#include "default_handler.h"
#include "status_handler.h"
#include "server_status_recorder.h"
#include "reverse_proxy_handler.h"
#include "meme_handler.h"
#include "health_handler.h"

//constructor takes a config
router::router (std::shared_ptr<NginxConfig> config, std::string server_root)
{
    config_ = config;
    server_root_ = server_root;
}

//register a handler to a particular URI
//inputs are a URI and handler name as strings
//handler names should correspond to defined handlers
//prioritize first handler registered for a given uri
bool router::register_route (std::string uri, std::string handler_name)
{
    std::lock_guard<std::mutex> lock(route_map_lock_);
    
    auto it = route_map_.find(uri);
    if (it == route_map_.end())
    {
        std::pair<std::string, std::string> h (uri, handler_name);
        //record uri prefix and handler name pair
        server_status_recorder::get_instance().prefix_recorder(h);
        route_map_.insert(h);
    }
    
    return true;
}

//use the provided configuration file to register all routes
bool router::register_routes_from_config ()
{
    std::lock_guard<std::mutex> lock(config_lock_);
    
    for (const auto& statement : config_->statements_) 
    {
        //pick out all handler configurations
        if (statement->tokens_.size() > 1 && statement->tokens_[0] == "handler" && statement->child_block_)
        {
            std::string handlerKey = statement->tokens_[1];
            std::shared_ptr<NginxConfig> rConfig = statement->child_block_;
            for (const auto& s : rConfig->statements_) 
            {
                //pick out location config param
                if (s->tokens_.size() > 0 && s->tokens_[0] == "location")
                {
                    std::string uri = s->tokens_[1];
                    register_route(uri, handlerKey);
                    // here we cannot break since there might be multiple proxies.
                    // break;
                }
            }
        }
    }
    
    return true;
}

//capture a default HTTP header to apply to all responses from the server
//inputs are strings for the header name and value
bool router::register_default_header (std::string header_name, std::string header_value)
{   
    std::lock_guard<std::mutex> lock(header_lock_);
    
    auto it = headers_.find(header_name);
    if (it != headers_.end())
    {
	    it->second = header_value;
    }
    else
    {
        std::pair<std::string, std::string> h (header_name, header_value);
        headers_.insert(h);
    }
    
    return true;
}

//main method to route the request to a particular handler and return the generated response
//input is a valid HTTP request
std::shared_ptr<response> router::route_request (std::shared_ptr<request> req)
{
    std::shared_ptr<route_handler> rh = select_handler(req->get_uri());
    std::shared_ptr<response> output = rh->handle_request(req);
    //record uri and response code
    server_status_recorder::get_instance().request_recorder(req->get_uri(), output->get_status_code());
    //logging response code and handler name
    log_lock_.lock();
    LOG_INFO << "ResponseCode::" << output->get_status_code() << "::RequestHandler::" << get_route_handler(longest_prefix_match(req->get_uri()));
    log_lock_.unlock();
    apply_default_headers(output);
    return output;
}

//get a specific route handler by URI
std::string router::get_route_handler (std::string uri)
{    
    std::lock_guard<std::mutex> lock(route_map_lock_);
    
    std::unordered_map<std::string, std::string>::const_iterator found = route_map_.find(uri);
    if (!(found == route_map_.end()))
    {
        return found->second;
    }
    else
    {
        std::shared_ptr<NginxConfig> proxy_config = get_handler_config("proxy");
        return proxy_config != NULL && validProxy(proxy_config, uri) ? "proxy" : "default";
    }
}

//get a specific header by name
std::string router::get_header (std::string name)
{    
    std::lock_guard<std::mutex> lock(header_lock_);
    
    std::unordered_map<std::string, std::string>::const_iterator found = headers_.find(name);
    if (!(found == headers_.end()))
    {
        return found->second;
    }
    else
    {
        return "";
    }
}

//get a nested config from the master config for a specific route handler
std::shared_ptr<NginxConfig> router::get_handler_config (std::string handler_name)
{
    std::lock_guard<std::mutex> lock(config_lock_);
    
    std::shared_ptr<NginxConfig> output;
    for (const auto& statement : config_->statements_) 
    {
        //the config we're looking for is nested
        if (statement->child_block_)
        {
            //must have at least one element if the child block is not null
            if (statement->tokens_[0] == "handler")
            {
                if (statement->tokens_.size() > 1 && statement->tokens_[1] == handler_name)
                {
                    output = statement->child_block_;
                }
            }
        }
    }
    return output;
}

//apply the default headers for the server to the given response
bool router::apply_default_headers (std::shared_ptr<response> res)
{
    std::lock_guard<std::mutex> lock(header_lock_);
    
    for (std::pair<std::string, std::string> header : headers_)
    {
	   res->set_header(header.first, header.second);
    }
    return true;
}

//pick registered route that is best match for given URI
std::string router::longest_prefix_match (std::string uri)
{
    std::lock_guard<std::mutex> lock(route_map_lock_);
    
    std::string longest = "";
    unsigned int current_count = 0;
    for (std::pair<std::string, std::string> mapping : route_map_)
    {
        if (uri.length() < mapping.first.length()) { continue; }
        bool match = true;
        for (unsigned int i = 0; i<mapping.first.length(); i++)
        {
            if (mapping.first[i] != uri[i]) { match = false; }
            else { current_count++; }
        }
        if (match)
        {
            longest = (mapping.first.length() > longest.length()) ? mapping.first : longest;
        }
    }
    return longest;
}

//return the appropriate route_handler given the provided URI 
//TODO: this is a little clunky right now...
std::shared_ptr<route_handler> router::select_handler (std::string uri)
{
    std::string route = longest_prefix_match(uri);
    std::string handler = get_route_handler(route);

    if (handler == "echo")
    {
        return echo_handler::create_handler(get_handler_config("echo"), server_root_);
    }
    else if (handler == "static1")
    {
        return static_file_handler::create_handler(get_handler_config("static1"), server_root_);
    }
    else if (handler == "static2")
    {
        return static_file_handler::create_handler(get_handler_config("static2"), server_root_);
    }
    else if (handler == "webMeme")
    {
        return static_file_handler::create_handler(get_handler_config("webMeme"), server_root_);
    }
    else if (handler == "imageMeme")
    {
        return static_file_handler::create_handler(get_handler_config("imageMeme"), server_root_);
    }
    else if (handler == "status") 
    {
        return status_handler::create_handler(get_handler_config("status"), server_root_);
    }
    else if (handler == "proxy")
    {
        return Reverse_proxy_handler::create_handler(get_handler_config("proxy"), server_root_);
    }
    else if (handler == "meme")
    {
	return meme_handler::create_handler(get_handler_config("meme"), server_root_);
    }
    else if (handler  == "health")
    {
        return health_handler::create_handler(get_handler_config("health"), server_root_);
    }
    else
    {       
        // std::cout<<handler<<"***"<<std::endl;
        // // first check whether this uri matches one of the proxies

        // // extract the proxy name
        // // i.e. ucla
        // int idx = 1;
        // while(idx < uri.size() && uri[idx] != '/')
        // {
        //     idx++;
        // }
        // std::string name = uri.substr(0, idx);

        // if(validProxy(get_handler_config("proxy"), name))
        // {
        //     std::cout<<"proxy"<<std::endl;
        //     return Reverse_proxy_handler::create_handler(get_handler_config("proxy"), server_root_);
        // }
        // else
        // {
        //     std::cout<<"default"<<std::endl;
        //     return default_handler::create_handler(get_handler_config("default"), server_root_);
        // }
        return default_handler::create_handler(get_handler_config("default"), server_root_);
    }
}

bool router::validProxy(std::shared_ptr<NginxConfig> config, std::string name)
{
    for(auto stmt: config->statements_)
    {
        if(stmt->tokens_[0] == "location" && stmt->tokens_[1] == name)
        {
            return true;
        }
    }
    return false;
}
