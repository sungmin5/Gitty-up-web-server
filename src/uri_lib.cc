//CREDIT FOR IMPLEMENTATION:
//https://stackoverflow.com/questions/18307429/encode-decode-url-in-c/35348028

#include "uri_lib.h"

//convert char to hex
std::string CharToHex(unsigned char c) 
{
    short i = c;

    std::stringstream s;

    s << "%" << std::setw(2) << std::setfill('0') << std::hex << i;

    return s.str();
}

//convert hex to char
unsigned char HexToChar(const std::string& str) 
{
    short c = 0;

    if(!str.empty()) 
    {
        std::istringstream in(str);

        in >> std::hex >> c;

        if(in.fail()) 
        {
            throw std::runtime_error("stream decode failure");
        }
    }

    return static_cast<unsigned char>(c);
}

//URI-encode an input string
std::string UriEncode(const std::string& toEncode)
{
    std::ostringstream out;

    for (std::string::size_type i=0; i < toEncode.length(); ++i) 
    {
        short t = toEncode.at(i);

        if (
            t == 45 ||	// hyphen
            (t >= 48 && t <= 57) ||	// 0-9
            (t >= 65 && t <= 90) ||	// A-Z
            t == 95 ||	// underscore
            (t >= 97 && t <= 122) ||	// a-z
            t == 126	// tilde
        ) 
        {
            out << toEncode.at(i);
        } 
        else 
        {
            out << CharToHex(toEncode.at(i));
        }
    }

    return out.str();
}

//URI-decode an input string
std::string UriDecode(const std::string& toDecode)
{
    std::ostringstream out;

    for(std::string::size_type i=0; i < toDecode.length(); ++i) 
    {
        if (toDecode.at(i) == '%') 
        {
            std::string str(toDecode.substr(i+1, 2));
            out << HexToChar(str);
            i += 2;
        }
        else if (toDecode.at(i) == '+')
        {
            out << ' ';
        }
        else 
        {
            out << toDecode.at(i);
        }
    }

    return out.str();
}