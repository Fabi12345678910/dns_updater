#define HTTP_RESPONCE_BLUEPRINT "%s\r\nContent-Length: %u\r\nContent-Type: %s\r\n\r\n%s"
#define HTTP_RESPONCE_REDIRECT "HTTP/1.0 308 Permanent Redirect\r\n\
Location: /state\r\n\r\n"
#define HTTP_RESPONCE_404 "HTTP/1.0 404 not found"

#define HTML_ROW_LOG_MESSAGE "<li><h3>%s</h3></li>\r\n"
#define HTML_ROW_LOG_MESSAGE_SIZE (sizeof(HTML_ROW_LOG_MESSAGE) - 3)

#define HTML_STATE "<!DOCTYPE html>\r\n\
<html>\r\n\
<head>\r\n\
    <title>dns updater state</title>\r\n\
    <style>\r\n\
        .dot_okay {\r\n\
            height: 25px;\r\n\
            width: 25px;\r\n\
            background-color: #00ff00;\r\n\
            border-radius: 50%%;\r\n\
            display: inline-block;\r\n\
        }\r\n\
    </style>\r\n\
    <style>\r\n\
        .dot_undefined {\r\n\
            height: 25px;\r\n\
            width: 25px;\r\n\
            background-color: #ff0088;\r\n\
            border-radius: 50%%;\r\n\
            display: inline-block;\r\n\
        }\r\n\
    </style>\r\n\
    <style>\r\n\
        .dot_warning {\r\n\
            height: 25px;\r\n\
            width: 25px;\r\n\
            background-color: #ffb300;\r\n\
            border-radius: 50%%;\r\n\
            display: inline-block;\r\n\
        }\r\n\
    </style>\r\n\
    <style>\r\n\
        .dot_unused {\r\n\
            height: 25px;\r\n\
            width: 25px;\r\n\
            background-color: #575757;\r\n\
            border-radius: 50%%;\r\n\
            display: inline-block;\r\n\
        }\r\n\
    </style>\r\n\
    <style>\r\n\
        .dot_error {\r\n\
            height: 25px;\r\n\
            width: 25px;\r\n\
            background-color: #ff2600;\r\n\
            border-radius: 50%%;\r\n\
            display: inline-block;\r\n\
        }\r\n\
    </style>\r\n\
</head>\r\n\
<body>\r\n\
\r\n\
<h1 align=center>global health state: <span class= %s></span></h1>\r\n\
<h2>ipv4 state: <span class= %s></span></h2>\r\n\
<h2>ipv6 state: <span class= %s></span></h2>\r\n\
<h1>dns entry states:</h1>\r\n\
<ul>\r\n\
%s\r\n\
</ul>\r\n\
<h1>last log entries:</h1>\r\n\
<ul>\r\n\
%s\r\n\
</ul>\r\n\
\r\n\
</body>\r\n\
</html>\r\n"
