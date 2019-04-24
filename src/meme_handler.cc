//A handler for HTTP requests that creates and serves new memes

#include "meme_handler.h"

//overriden constructor takes a config
//for this route, a config is required that contains a path to the static images
meme_handler::meme_handler (std::shared_ptr<NginxConfig> config, std::string root_path) : route_handler(config, root_path) 
{
    parse_config();

    // connect to sql db and create a table
    sql_manager_ = std::unique_ptr<sql_manager>(new sql_manager("meme.db"));
    table_name_ = "meme_table";
    sql_manager_->connect();
    query_params qp;
    field_names_ = {"id", "img_path", "top_caption", "bot_caption", "times_viewed"};
    qp.push(field_names_[0], "TEXT PRIMARY KEY NOT NULL");
    qp.push(field_names_[1], "TEXT NOT NULL");
    qp.push(field_names_[2], "TEXT NOT NULL");
    qp.push(field_names_[3], "TEXT NOT NULL");
    qp.push(field_names_[4], "TEXT NOT NULL");
    sql_manager_->create_table(table_name_, qp);
}

//overriden factory method to create a new instance
std::shared_ptr<route_handler> meme_handler::create_handler (std::shared_ptr<NginxConfig> config, std::string root_path)
{
    return std::shared_ptr<meme_handler>(new meme_handler(config, root_path));
}

//overridden method in parent class to handle a request
std::shared_ptr<response> meme_handler::handle_request (std::shared_ptr<request> req)
{
    std::string method = req->get_method();
    if (method == "POST")
    {
        return create_meme(req);
    }
    else if (method == "PUT")
    {
        std::string check = check_uri(req->get_uri());
        return update_meme(req, check);
    }
    else if (method == "DELETE")
    {
        std::string check = check_uri(req->get_uri());
        return delete_meme(req, check);
    }
    else if (method == "GET")
    {
        //check the URI to determine whether to serve the entire meme list or just a single meme
        std::string check = check_uri(req->get_uri());
        if (check == "create" || check == "create/") { return redirect_request(req, "create"); }
        else if (check == "view" || check == "view/") { return redirect_request(req, "list"); }
        else if (check.length() > 7 && check.substr(0, 7) == "manage/") { return redirect_request(req, check); }
        else if (check.length() > 5 && check.substr(0, 5) == "view/") { return redirect_request(req, check.substr(5, check.length() - 5)); }
        else if (check == "" || check == "/") { return meme_list(req); }
        else if (check.length() > 7 && check.substr(0, 7) == "search/") { return meme_search(req, check.substr(7, check.length() - 7)); }
        else { return get_meme_by_id(req, check); }
    }
    else
    {
        return invalid_method(req);
    }
}

bool meme_handler::compareMemes(std::vector<std::string> meme1, std::vector<std::string> meme2) {
    return (std::stoi(meme1[4]) < std::stoi(meme2[4]));
}

//return the appropriate MIME type given a file extension
std::string meme_handler::get_mime_type (std::string extension)
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

//create a meme
std::shared_ptr<response> meme_handler::create_meme (std::shared_ptr<request> req)
{
    //check body of the request for validity
    std::vector<std::string> params;
    bool check = body_check(req->get_body(), params);
    
    if (!check)
    {
        //if body is not valid, return 400
        std::shared_ptr<response> resp(new response(400, "The provided meme creation request is invalid!\n"));
        resp->set_header("Content-Type", "text/plain");
        return resp;
    }
    
    //if valid, create meme using info in the body and store (???)
    std::string new_id;
    bool create = generate_new_meme(params, new_id);
    
    if (create)
    {
        //if creation successful, respond with 201
        std::shared_ptr<response> resp(new response(201, "{\"id\": \"" + new_id + "\"}"));
        resp->set_header("Content-Type", "application/json");
        return resp;
    }
    else
    {
        //if creation unsuccessful, respond with 500
        std::shared_ptr<response> resp(new response(500, "An error occurred while trying to create the provided meme!\n"));
        resp->set_header("Content-Type", "text/plain");
        return resp;
    }
}

//update an existing meme
std::shared_ptr<response> meme_handler::update_meme (std::shared_ptr<request> req, std::string meme_id)
{
    //check body of the request for validity
    std::vector<std::string> params;
    bool test = check_id(meme_id);
    bool check = body_check(req->get_body(), params);
    
    if (!(test & check))
    {
        //if body is not valid, return 400
        std::shared_ptr<response> resp(new response(400, "The provided meme update request is invalid!\n"));
        resp->set_header("Content-Type", "text/plain");
        return resp;
    }
    
    //TODO: SQL update command
    //TODO: check if a meme actually got updated (i.e., ID was valid)
    // update meme associated with id meme_id
    query_params qps;
    qps.push(field_names_[1], params[0]);
    qps.push(field_names_[2], params[1]);
    qps.push(field_names_[3], params[2]);
    query_params qpw;
    qpw.push(field_names_[0], meme_id);
    bool update = sql_manager_->update_record(table_name_, qps, qpw);
    
    if (update)
    {
        //if update successful, respond with 200
        std::shared_ptr<response> resp(new response(200, "{\"id\": \"" + meme_id + "\"}"));
        resp->set_header("Content-Type", "application/json");
        return resp;
    }
    else
    {
        //if update unsuccessful, respond with 500
        std::shared_ptr<response> resp(new response(500, "An error occurred while trying to update the provided meme!\n"));
        resp->set_header("Content-Type", "text/plain");
        return resp;
    }
}

//delete an existing meme
std::shared_ptr<response> meme_handler::delete_meme (std::shared_ptr<request> req, std::string meme_id)
{   
    bool test = check_id(meme_id);
    if (!test)
    {
        //if body is not valid, return 400
        std::shared_ptr<response> resp(new response(400, "The provided meme delete request is invalid!\n"));
        resp->set_header("Content-Type", "text/plain");
        return resp;
    }
    
    //TODO: SQL delete command
    //TODO: check if a meme actually got delete (i.e., ID was valid)
    // delete meme with id meme_id
    query_params qp;
    qp.push(field_names_[0], meme_id);
    bool del = sql_manager_->delete_record(table_name_, qp);
    
    if (del)
    {
        //if delete successful, respond with 200
        std::shared_ptr<response> resp(new response(200, "{\"id\": \"" + meme_id + "\"}"));
        resp->set_header("Content-Type", "application/json");
        return resp;
    }
    else
    {
        //if delete unsuccessful, respond with 500
        std::shared_ptr<response> resp(new response(500, "An error occurred while trying to delete the provided meme!\n"));
        resp->set_header("Content-Type", "text/plain");
        return resp;
    }
}


//serve web page with list of all memes (and URL's to view them)
std::shared_ptr<response> meme_handler::redirect_request (std::shared_ptr<request> req, std::string location)
{
    if (location == "create")
    {
        //redirect to creation web page
        std::shared_ptr<response> resp(new response(303, "Location: /web/memes/create.html"));
        resp->set_header("Location", "/web/memes/create.html");
        return resp;
    }
    else if (location == "list")
    {
        //redirect to master list web page
        std::shared_ptr<response> resp(new response(303, "Location: /web/memes/list.html"));
        resp->set_header("Location", "/web/memes/list.html");
        return resp;
    }
    else if (location.substr(0, 7) == "manage/")
    {
        //redirect to manage web page
        std::string id = location.substr(7, location.length() - 7);
        std::shared_ptr<response> resp(new response(303, "Location: /web/memes/manage.html?id=" + id));
        resp->set_header("Location", "/web/memes/manage.html?id=" + id);
        return resp;
    }
    else
    {
        //redirect to viewer web page
        std::shared_ptr<response> resp(new response(303, "Location: /web/memes/view.html?id=" + location));
        resp->set_header("Location", "/web/memes/view.html?id=" + location);
        return resp;
    }
}

//return a list of all memes as a JSON array
std::shared_ptr<response> meme_handler::meme_list (std::shared_ptr<request> req)
{
    query_params qp;
    std::vector<std::vector<std::string>> result = sql_manager_->select_record(table_name_, qp);
    std::stable_sort(result.begin(), result.end(), compareMemes);
    std::string output = "[";
    bool add_comma = false;
    // loops through the directory containing all of the memes and outputs an array of the file names
    for (auto i = 0; i < result.size(); i++)
    {
        // append comma before objects (except first object)
        if (add_comma) {
            output += ",";
        }
        add_comma = true;
        // append ids
        output += "\"" + result[result.size() - 1 - i][0] + ":" + result[result.size() - 1 - i][4] + ":" + result[result.size() - 1 - i][1] +
        ":" + result[result.size() - 1 - i][2] + ":" + result[result.size() - 1 - i][3] + "\"";
    }
    output += "]";
  
    //return JSON array
    std::shared_ptr<response> resp(new response(200, output));
    resp->set_header("Content-Type", get_mime_type(".json"));
    return resp;
}

std::shared_ptr<response> meme_handler::meme_search (std::shared_ptr<request> req, std::string query)
{
    query_params qp;
    std::string decodedQuery = UriDecode(query);
    // qp.push(field_names_[1], query);
    qp.push(field_names_[2], decodedQuery);
    qp.push(field_names_[3], decodedQuery);
    std::vector<std::vector<std::string>> result = sql_manager_->search_record(table_name_, qp);
    std::stable_sort(result.begin(), result.end(), compareMemes);
    std::string output = "[";
    bool add_comma = false;
    // loops through the directory containing all of the memes and outputs an array of the file names
    for (auto i = 0; i < result.size(); i++)
    {
        // append comma before objects (except first object)
        if (add_comma) {
            output += ",";
        }
        add_comma = true;
        // append ids
        output += "\"" + result[result.size() - 1 - i][0] + ":" + result[result.size() - 1 - i][4] + ":" + result[result.size() - 1 - i][1] +
        ":" + result[result.size() - 1 - i][2] + ":" + result[result.size() - 1 - i][3] + "\"";
    }
    output += "]";
  
    //return JSON array
    std::shared_ptr<response> resp(new response(200, output));
    resp->set_header("Content-Type", get_mime_type(".json"));
    return resp;
}

//return the data associated with a particular meme (template, captions)
//TODO: do we need any file locking???
std::shared_ptr<response> meme_handler::get_meme_by_id (std::shared_ptr<request> req, std::string id)
{
    std::string output = "{";
    std::string meme_template = "";
    std::string top_caption = "";
    std::string bottom_caption = "";
    
    // get meme from db
    query_params qp;
    qp.push(field_names_[0], id);
    std::vector<std::vector<std::string>> result = sql_manager_->select_record(table_name_, qp);
    if (result.size() == 1) {
        // result[0][0] is id
        meme_template = result[0][1];
        top_caption = result[0][2];
        bottom_caption = result[0][3];

        int curr_times_viewed = std::stoi(result[0][4]);
        query_params qps;
        qps.push(field_names_[4], std::to_string(curr_times_viewed + 1));
        query_params qpw;
        qpw.push(field_names_[0], result[0][0]);
        bool update = sql_manager_->update_record(table_name_, qps, qpw);

    }
    else {
        std::shared_ptr<response> resp(new response(404, "The requested file could not be found!\n"));
        resp->set_header("Content-Type", "text/plain");
        return resp;   
    }
    
    //all values should be non-empty after reading the file
    if (meme_template == "" || top_caption == "" || bottom_caption == "")
    {
        std::shared_ptr<response> resp(new response(400, "The provided meme has been corrupted!"));
        resp->set_header("Content-Type", get_mime_type(".txt"));
        return resp;
    }
    
    output += ("\"memeSelect\": \"" + meme_template + "\", ");
    output += ("\"topCaption\": \"" + top_caption + "\", ");
    output += ("\"bottomCaption\": \"" + bottom_caption + "\"");
    output += "}";
    
    //return JSON array
    std::shared_ptr<response> resp(new response(200, output));
    resp->set_header("Content-Type", get_mime_type(".json"));
    return resp;
}

//the method used in the request wasn't valid so return a 405
std::shared_ptr<response> meme_handler::invalid_method (std::shared_ptr<request> req)
{
    std::shared_ptr<response> resp(new response(405, "This route only supports the HTTP GET and POST methods!"));
    resp->set_header("Content-Type", "text/plain");
    return resp;
}

//parse URI to determine whether to serve the entire meme list or a specific meme
std::string meme_handler::check_uri (std::string uri)
{
    std::regex r("\\/memes/(.*)$");
    std::smatch m;
    std::regex_search(uri, m, r);
    
    std::string output = "";
    if (m.size() == 2) 
    {
        output = m.str(1);
    }
    
    return output;
}

//check the provided meme ID for correctness
bool meme_handler::check_id (std::string id)
{
    std::regex r("^[0-9a-fA-F]{8}\\-[0-9a-fA-F]{4}\\-[0-9a-fA-F]{4}\\-[0-9a-fA-F]{4}\\-[0-9a-fA-F]{12}$");
    std::smatch m;
    std::regex_search(id, m, r);
    
    return m.size() > 0;
}

//parse config to necessary attributes
bool meme_handler::parse_config ()
{
    path_to_memes_ = "";
    
    std::lock_guard<std::mutex> lock(config_lock_);
    
    std::string output;
    for (const auto& statement : config_->statements_) 
    {
        //statement we're looking for has exactly 2 tokens
         if (statement->tokens_.size() == 2)
         {
             if (statement->tokens_[0] == "memePath")
             {
                 path_to_memes_ = (root_path_ + statement->tokens_[1]);
             }
             else if (statement->tokens_[0] == "memeTemplatePath")
             {
                 path_to_meme_templates_ = (root_path_ + statement->tokens_[1]);
             }
         }
    }
    
    return !(path_to_memes_ == "");
}

//check the provided body for valid input data
//TODO: should eventually implement a better body parser...
bool meme_handler::body_check(std::string body, std::vector<std::string>& params)
{
    //parse out necessary body parameters from request - into params
    body = UriDecode(body);
    std::string meme_template = "";
    std::string top_caption = "";
    std::string bottom_caption = "";
    
    std::regex r("(\\w+)=([\\w\\.\\,\\?! ]+)&*");
    std::sregex_iterator iter(body.begin(), body.end(), r);
    std::sregex_iterator end;
  
    while (iter != end)
    {
        if (iter->size() == 3)
        {
            std::string key = (*iter)[1];
            if (key == "memeSelect") { meme_template = (*iter)[2]; }
            else if (key == "topCaption") { top_caption = (*iter)[2]; }
            else if (key == "bottomCaption") { bottom_caption = (*iter)[2]; }
        }
        ++iter;
    }

    //check memeSelect to ensure it matches the name of one of the template memes
    std::regex r1("\\w+.jpg");
    std::smatch m1;
    std::regex_search(meme_template, m1, r1);
    if (m1.size() == 0) { return false; }
    
    //check topCaption for a valid caption string
    std::regex r2("[\\w\\.\\,\\?! ]+");
    std::smatch m2;
    std::regex_search(top_caption, m2, r2);
    if (m2.size() == 0) { return false; }
    
    //check bottomCaption for a valid caption string
    std::regex r3("[\\w\\.\\,\\?! ]+");
    std::smatch m3;
    std::regex_search(bottom_caption, m3, r3);
    if (m3.size() == 0) { return false; }
    
    params.push_back(meme_template);
    params.push_back(top_caption);
    params.push_back(bottom_caption);
    return true;
}

//use the provided (valid) parameters to generate a new meme
bool meme_handler::generate_new_meme (std::vector<std::string> params, std::string& new_id)
{
    //generate a new GUID to be the ID for the new meme - push its value to new_id
    boost::uuids::uuid uuid = boost::uuids::random_generator()();
    new_id = boost::lexical_cast<std::string>(uuid);
    
    // return true if successfully inserted
    query_params qp;
    qp.push(field_names_[0], new_id);
    qp.push(field_names_[1], params[0]);
    qp.push(field_names_[2], params[1]);
    qp.push(field_names_[3], params[2]);
    qp.push(field_names_[4], "0");
    return sql_manager_->insert_record(table_name_, qp);
}

