#include <iostream>
#include <string>
#include <sstream>
#include <iomanip>
#include <cstdlib>
#include "linkCreator.h"

std::string urlEncode(const std::string &value)
{
    std::ostringstream escaped;
    escaped.fill('0');
    escaped << std::hex;

    for (char c : value)
    {
        if (isalnum(c) || c == '-' || c == '_' || c == '.' || c == '~')
        {
            escaped << c;
        }
        else
        {
            escaped << std::uppercase;
            escaped << '%' << std::setw(2) << int((unsigned char)c);
            escaped << std::nouppercase;
        }
    }

    return escaped.str();
}

// Creates link for Google Calendar
std::string createGoogleCalendarLink(const std::string &title, const std::string &details,
                                     const std::string &location, const std::string &startDateTime,
                                     const std::string &endDateTime)
{
    std::string base_url = "https://calendar.google.com/calendar/render?action=TEMPLATE";
    std::string url = base_url + "&text=" + urlEncode(title) +
                      "&details=" + urlEncode(details) +
                      "&location=" + urlEncode(location) +
                      "&dates=" + startDateTime + "/" + endDateTime;
    return url;
}

// Creates link for Outlook Calendar
std::string createOutlookCalendarLink(const std::string &title, const std::string &details,
                                      const std::string &location, const std::string &startDateTime,
                                      const std::string &endDateTime)
{
    std::string base_url = "https://outlook.live.com/calendar/0/deeplink/compose?path=/calendar/action/compose";
    std::string url = base_url + "&startdt=" + urlEncode(startDateTime) +
                      "&enddt=" + urlEncode(endDateTime) +
                      "&subject=" + urlEncode(title) +
                      "&body=" + urlEncode(details) +
                      "&location=" + urlEncode(location);

    return url;
}

void openURL(const std::string &url)
{
#ifdef _WIN32 // Correct quoting for the Windows command line
    std::string command = "start \"\" \"" + url + "\""; 
#elif __APPLE__ // Correct quoting for macOS
    std::string command = "open \"" + url + "\""; 
#else // Correct quoting for Linux
    std::string command = "xdg-open \"" + url + "\""; 
#endif
    system(command.c_str());
}
