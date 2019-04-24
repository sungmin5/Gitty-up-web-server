//A handler for HTTP requests that creates and serves new memes
#ifndef MEME_HANDLER_H
#define MEME_HANDLER_H

#include <algorithm>
#include <cstdlib>
#include <iostream>
#include <fstream> 
#include <unordered_map>
#include <string>
#include <regex>
#include <vector>
#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_generators.hpp>
#include <boost/uuid/uuid_io.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/filesystem.hpp>
#include "route_handler.h"
#include "mime_types.h"
#include "uri_lib.h"
#include "sql_lib.h"

class meme_handler : public route_handler
{
public:
    //Methods
    static std::shared_ptr<route_handler> create_handler (std::shared_ptr<NginxConfig> config, std::string root_path);
    std::shared_ptr<response> handle_request (std::shared_ptr<request> req); //given a request, generate an appropriate response
    std::string get_mime_type (std::string extension); //get MIME type given an extension
private:    
    //Methods
    meme_handler (std::shared_ptr<NginxConfig> config, std::string root_path); //constructor overload
    std::shared_ptr<response> create_meme (std::shared_ptr<request> req); //create a new meme
    std::shared_ptr<response> update_meme (std::shared_ptr<request> req, std::string meme_id); //update an existing meme
    std::shared_ptr<response> delete_meme (std::shared_ptr<request> req, std::string meme_id); //delete an existing meme
    std::shared_ptr<response> redirect_request (std::shared_ptr<request> req, std::string location); //redirect to a meme web page
    std::shared_ptr<response> meme_list (std::shared_ptr<request> req); //retrieve all existing memes
    std::shared_ptr<response> meme_search (std::shared_ptr<request> req, std::string query); //retrieve memes with prefix
    std::shared_ptr<response> get_meme_by_id (std::shared_ptr<request> req, std::string meme_id); //retrieve a particular meme's information
    std::shared_ptr<response> invalid_method (std::shared_ptr<request> req); //generate invalid method response
    std::string check_uri (std::string uri); //check the URI to determine routing action
    bool check_id (std::string id); //check the meme ID for correctness
    bool parse_config (); //retrieve all necessary info from the input config
    bool body_check(std::string body, std::vector<std::string>& params); //check body and parse values into vector
    bool generate_new_meme (std::vector<std::string> params, std::string& new_id); //generate new meme and output its ID
    static bool compareMemes(std::vector<std::string> meme1, std::vector<std::string> meme2);
    
    //Attributes
    std::string path_to_meme_templates_;
    std::string path_to_memes_;
    std::unique_ptr<sql_manager> sql_manager_;
    std::string table_name_;
    std::vector<std::string> field_names_;

};

#endif