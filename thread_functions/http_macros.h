#define HTTP_RESPONCE_BLUEPRINT "%s\r\nContent-Length: %u\r\nContent-Type: %s\r\n\r\n%s"
#define HTTP_RESPONCE_REDIRECT "HTTP/1.0 308 Permanent Redirect\r\n\
Location: /state\r\n\r\n"
#define HTTP_RESPONCE_404 "HTTP/1.0 404 not found"

#define HTML_ROW_LOG_MESSAGE "        <li class=\"log-entry\">%s</li>\r\n"
#define HTML_ROW_LOG_MESSAGE_SIZE (sizeof(HTML_ROW_LOG_MESSAGE) - 3)

#define HTML_STATE "<!DOCTYPE html>\r\n\
<html lang=\"en\">\r\n\
<head>\r\n\
  <meta charset=\"UTF-8\">\r\n\
  <meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0\">\r\n\
  <title>DNS Updater Status</title>\r\n\
  <style>\r\n\
    body {\r\n\
      font-family: Arial, sans-serif;\r\n\
      margin: 0;\r\n\
      padding: 0;\r\n\
      background-color: #111;\r\n\
      color: white;\r\n\
    }\r\n\
\r\n\
    header {\r\n\
      background-color: #333;\r\n\
      padding: 10px;\r\n\
      text-align: center;\r\n\
    }\r\n\
\r\n\
    .container {\r\n\
      display: flex;\r\n\
      flex-wrap: wrap;\r\n\
      justify-content: center;\r\n\
      padding: 10px;\r\n\
    }\r\n\
\r\n\
    .section {\r\n\
      flex: 1 1 calc(50%% - 20px);\r\n\
      margin: 10px;\r\n\
      padding: 10px;\r\n\
      border-radius: 5px;\r\n\
      background-color: rgba(255, 255, 255, 0.05);\r\n\
      box-shadow: 0 2px 8px rgba(0, 0, 0, 0.2);\r\n\
    }\r\n\
\r\n\
    .status {\r\n\
      display: inline-block;\r\n\
      padding: 5px 10px;\r\n\
      border-radius: 5px;\r\n\
      font-weight: bold;\r\n\
      margin-bottom: 5px;\r\n\
    }\r\n\
\r\n\
    .undefined { background-color: #777; color: #333; }\r\n\
    .error { background-color: #ff5656; color: white; }\r\n\
    .warn { background-color: #ffd25b; color: #333; }\r\n\
    .okay { background-color: #67c97b; color: #333; }\r\n\
    .unused { background-color: #444; color: white; }\r\n\
\r\n\
    /* Additional styling for log entries */\r\n\
    .log-list {\r\n\
      list-style: none;\r\n\
    }\r\n\
\r\n\
    .log-entry {\r\n\
      margin-bottom: 5px;\r\n\
      background-color: #333;\r\n\
      color: white;\r\n\
      padding: 5px 10px;\r\n\
      border-radius: 5px;\r\n\
    }\r\n\
  </style>\r\n\
</head>\r\n\
<body>\r\n\
  <header>\r\n\
    <h1>DNS Updater Status</h1>\r\n\
  </header>\r\n\
  <div class=\"container\">\r\n\
    <div class=\"section\">\r\n\
      <h2>Global Health</h2>\r\n\
      <div class=\"status %s\">%s</div>\r\n\
    </div>\r\n\
    <div class=\"section\">\r\n\
      <h2>IPv4 State</h2>\r\n\
      <div class=\"status %s\">%s</div>\r\n\
    </div>\r\n\
    <div class=\"section\">\r\n\
      <h2>IPv6 State</h2>\r\n\
      <div class=\"status %s\">%s</div>\r\n\
    </div>\r\n\
    <div class=\"section\">\r\n\
      <h2>Managed DNS Entries</h2>\r\n\
      <ul>\r\n\
%s      </ul>\r\n\
    </div>\r\n\
    <div class=\"section\">\r\n\
      <h2>Recently Occurred Log Entries</h2>\r\n\
      <ul class=\"log-list\">\r\n\
%s      </ul>\r\n\
    </div>\r\n\
  </div>\r\n\
</body>\r\n\
</html>\r\n\
"
