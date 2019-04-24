//A set of functions to deal with URI operations
#ifndef URILIB_H
#define URILIB_H

#include <sstream>
#include <stdexcept>
#include <iomanip>

std::string CharToHex(unsigned char c); //convert char to hex
unsigned char HexToChar(const std::string& str); //convert hex to char
std::string UriEncode(const std::string& source); //URI-encode an input string
std::string UriDecode(const std::string& source); //URI-decode an input string

#endif

