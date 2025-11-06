#ifndef LINK_CREATOR_H
#define LINK_CREATOR_H

#include <iostream>
#include <string>
#include <sstream>
#include <iomanip> // Include for std::setw
#include <cstdlib>

std::string urlEncode(const std::string &value);

std::string createGoogleCalendarLink(const std::string &title, const std::string &details,
                                     const std::string &location, const std::string &startDateTime,
                                     const std::string &endDateTime);

void openURL(const std::string &url);

#endif