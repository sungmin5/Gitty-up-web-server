//An HTTP Response
#ifndef RESPONSE_CODES_H
#define RESPONSE_CODES_H

#include <string>
#include <unordered_map>

const std::unordered_map<unsigned int, std::string> response_codes({
    {200, "OK"},
    {201, "Created"},
    {303, "See Other"},
    {400, "Bad Request"},
    {404, "Not Found"},
    {405, "Method Not Allowed"},
    {500, "Internal Server Error"}
});

#endif

