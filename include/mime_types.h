//A constant map of extensions to MIME types
#ifndef MIME_TYPES_H
#define MIME_TYPES_H

#include <string>
#include <unordered_map>

const std::unordered_map<std::string, std::string> mime_types({
   { ".txt", "text/plain" },
   { ".html", "text/html" },
   { ".htm", "text/html" },
   { ".jpg", "image/jpeg" },
   { ".jpeg", "image/jpeg" },
   { ".zip", "application/zip" },
   { ".json", "application/json" },
   { ".xml", "application/xml" },
   { ".css", "text/css" },
   { ".js", "application/javascript" },
   { ".ico", "image/x-icon" }
});

#endif

