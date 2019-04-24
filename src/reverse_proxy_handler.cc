#include <cstdlib>
#include <iostream>
#include <string>
#include <unordered_map>
#include "reverse_proxy_handler.h"

using boost::asio::ip::tcp;

Reverse_proxy_handler::Reverse_proxy_handler(std::shared_ptr<NginxConfig> config, std::string root_path) : route_handler(config, root_path) 
{
    std::cout << config->ToString() << std::endl;
    std::string location;
    for(auto stmt : config->statements_)
    {
        if(stmt->tokens_[0] == "location")
        {
            location = stmt->tokens_[1];
        }
        else if(stmt->tokens_[0] == "root")
        {
            location2proxy.insert(std::make_pair(location, stmt->tokens_[1]));
        }
        else if(stmt->tokens_[0] == "port")
        {
            port_num = atoi(stmt->tokens_[1].c_str());
        }
    }
}

std::shared_ptr<route_handler> Reverse_proxy_handler::create_handler (std::shared_ptr<NginxConfig> config, std::string root_path)
{
    return std::shared_ptr<Reverse_proxy_handler>(new Reverse_proxy_handler(config, root_path));
}

std::shared_ptr<response> Reverse_proxy_handler::handle_request (std::shared_ptr<request> req)
{
    std::string response_str = constructGetRequest(req->get_uri());
    if(redirect)
    {
        // behaves like /echo
        std::shared_ptr<response> resp(new response(200, std::string(req->get_raw_request())));
        resp->set_header("Content-Type", "text/plain");
        resp->set_header("Content-Length", std::to_string(resp->get_body().length()));
        return resp;
    }
    else 
    {
        std::shared_ptr<response> resp(new response(200, response_str));
        resp->set_header("Content-Type", mime_type);
        resp->set_header("Content-Length", std::to_string(response_str.length()));
        BOOST_LOG_TRIVIAL(info) << "Content-Length: " << resp->get_body().length();
        return resp;
    }
}


// part of the implementations are adapted from https://blog.csdn.net/duzixi/article/details/48195071
std::string Reverse_proxy_handler::sendGetRequest(char* host, char* path)
{
    boost::asio::io_service io_service;
    
    tcp::resolver resolver(io_service);
    tcp::resolver::query query(host, std::to_string(port_num));
    tcp::resolver::iterator iter = resolver.resolve(query);
 
    tcp::socket socket(io_service);
    boost::asio::connect(socket, iter);
    BOOST_LOG_TRIVIAL(info) << "host is : " << host;
    // construct request header
    boost::asio::streambuf request;
    std::ostream request_stream(&request);
    request_stream << "GET " << path << " HTTP/1.1\r\n";
    request_stream << "Host: " << host << "\r\n";
    request_stream << "Accept: */*\r\n";
    request_stream << "Content-Base: " << host << "\r\n";
    request_stream << "Connection: close\r\n\r\n";
    
    boost::asio::write(socket, request);
    
    boost::asio::streambuf response;
    boost::asio::read_until(socket, response, "\r\n");
    
    // check whether response is ok
    std::istream response_stream(&response);
    std::string http_version;
    response_stream >> http_version;
    unsigned int status_code;
    response_stream >> status_code;
    std::string status_message;
    getline(response_stream, status_message);
    if (!response_stream || http_version.substr(0, 5) != "HTTP/")
    {
        BOOST_LOG_TRIVIAL(info) << "Invalid response";
    }
    if (status_code != 200)
    {
        BOOST_LOG_TRIVIAL(info) << "response status code: " << status_code;
    }
    
    // read response header
    boost::asio::read_until(socket, response, "\r\n\r\n");
    
    std::string header;
    int len = 0;
    while (getline(response_stream, header) && header != "\r")
    {
        if (header.find("Content-Length: ") == 0) {
            std::stringstream stream;
            int offset = 16; // here 16 is the length of string "Content-Length: ", we need the number after this string
            stream << header.substr(offset); 
            stream >> len;
        }
        else if (header.find("Content-Type: ") == 0) {
            int offset = 14; // here 14 is the length of string "Content-Type: ", we need the type after this string
            int end = header.find(";");
            mime_type = header.substr(offset, end - offset); 
        }
    }
    
    long size = response.size();
    
    boost::system::error_code error;  // read error
    
    // keep reading until the file ends
    while (boost::asio::read(socket, response, boost::asio::transfer_at_least(1), error))
    {
        // get response length
        size = response.size();
        if (len != 0) {
            BOOST_LOG_TRIVIAL(info) << size << "  Byte  " << (size * 100) / len << "%";
        }
    }
 
    // if not reach the end of the file, throw an error
    if (error != boost::asio::error::eof)
    {
        throw boost::system::system_error(error);
    }
    
    BOOST_LOG_TRIVIAL(info) << size << " Contents downloaded.";
    
    // convert streambuf to string
    std::istream is(&response);
    is.unsetf(std::ios_base::skipws);
    std::string sz;
    sz.append(std::istream_iterator<char>(is), std::istream_iterator<char>());
    // BOOST_LOG_TRIVIAL(info) << sz;
    if(status_code == 301 || status_code == 302)
    {
        // return sendGetRequestToRedirectedSite(extractRedirectedHost(sz), path);
        redirect = true;
    }
    // else
    // {
    //     return sz;
    // }
    return sz;
}

std::string Reverse_proxy_handler::constructGetRequest(std::string uri)
{    
    uri = uri.substr(1);
    if(uri[uri.size() - 1] != '/') uri += '/';
    size_t index = uri.find("/");


    std::unordered_map<std::string, std::string>::iterator iter = location2proxy.find("/" + uri.substr(0, index));
    // here we don't need to check whether iter == location2proxy.end()
    // since the proxy is guaranteed to exist if the program reaches here
    std::string host_str = iter->second;
    char* host = new char[host_str.size() + 1];
    strcpy(host, host_str.c_str());

    char* urlPath = new char[uri.size() - index + 1];
    strcpy(urlPath, uri.substr(index, uri.size() - index).c_str());
    BOOST_LOG_TRIVIAL(info) << "host: " << host;
    BOOST_LOG_TRIVIAL(info) << "urlPath: " << urlPath;

    return sendGetRequest(host, urlPath);
}

char* Reverse_proxy_handler::extractRedirectedHost(std::string res)
{
    std::string http = std::string("http://");
    int start = res.find(http);
    start += http.length();
    int end = start;
    while(end < res.length() && res[end] != '/' && res[end] != '"')
    {
        end++;
    }
    std::string host_str = res.substr(start, end - start);
    char* host = new char[host_str.size() + 1];
    strcpy(host, host_str.c_str());
    return host;
}

std::string Reverse_proxy_handler::sendGetRequestToRedirectedSite(char* host, char* path){
    boost::asio::io_service io_service;
    
    tcp::resolver resolver(io_service);
    tcp::resolver::query query(host, std::to_string(port_num));
    tcp::resolver::iterator iter = resolver.resolve(query);
 
    tcp::socket socket(io_service);
    boost::asio::connect(socket, iter);
    BOOST_LOG_TRIVIAL(info) << "Redirected host is : " << host;
    // construct request header
    boost::asio::streambuf request;
    std::ostream request_stream(&request);
    request_stream << "GET " << path << " HTTP/1.1\r\n";
    request_stream << "Host: " << host << "\r\n";
    request_stream << "Accept: */*\r\n";
    request_stream << "Connection: close\r\n\r\n";
    
    boost::asio::write(socket, request);
    
    boost::asio::streambuf response;
    boost::asio::read_until(socket, response, "\r\n");
    
    // check whether response is ok
    std::istream response_stream(&response);
    std::string http_version;
    response_stream >> http_version;
    unsigned int status_code;
    response_stream >> status_code;
    std::string status_message;
    getline(response_stream, status_message);
    if (!response_stream || http_version.substr(0, 5) != "HTTP/")
    {
        BOOST_LOG_TRIVIAL(info) << "Invalid response";
    }
    if (status_code != 200)
    {
        BOOST_LOG_TRIVIAL(info) << "response status code: " << status_code;
    }
    
    // read response header
    boost::asio::read_until(socket, response, "\r\n\r\n");
    
    std::string header;
    int len = 0;
    while (getline(response_stream, header) && header != "\r")
    {
        if (header.find("Content-Length: ") == 0) {
            std::stringstream stream;
            int offset = 16; // here 16 is the length of string "Content-Length: ", we need the number after this string
            stream << header.substr(offset); 
            stream >> len;
        }
        else if (header.find("Content-Type: ") == 0) {
            int offset = 14; // here 14 is the length of string "Content-Type: ", we need the type after this string
            int end = header.find(";");
            mime_type = header.substr(offset, end - offset); 
        }
    }
    
    long size = response.size();
    
    boost::system::error_code error;  // read error
    
    // keep reading until the file ends
    while (boost::asio::read(socket, response, boost::asio::transfer_at_least(1), error))
    {
        // get response length
        size = response.size();
        if (len != 0) {
            BOOST_LOG_TRIVIAL(info) << size << "  Byte  " << (size * 100) / len << "%";
        }
    }
 
    // if not reach the end of the file, throw an error
    if (error != boost::asio::error::eof)
    {
        throw boost::system::system_error(error);
    }
    
    BOOST_LOG_TRIVIAL(info) << size << " Contents downloaded.";
    
    // convert streambuf to string
    std::istream is(&response);
    is.unsetf(std::ios_base::skipws);
    std::string sz;
    sz.append(std::istream_iterator<char>(is), std::istream_iterator<char>());
    return sz;
}