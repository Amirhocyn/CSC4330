#include "genericEvent.h"
#include <sstream>
#include <iomanip>
#include <ctime>
#include <iostream>

// Converts date to format for google or outlook depending on user input
std::string eventDateConverter(std::string eventDate, char calendar)
{
    time_t now = time(0);
    tm *ltm = localtime(&now);
    int year = 1900 + ltm->tm_year;

    int counter;
    std::string month;
    std::string newEventDate;
    std::string yearString = std::to_string(year);
    newEventDate += yearString;

    if (calendar == 'g')
    {
        if (isalpha(eventDate[0]))
        {
            for (int i = 0; i < 3; i++)
            {
                month += toupper(eventDate[i]);
            }
            if (month == "JAN")
            {
                newEventDate += "01";
            }
            else if (month == "FEB")
            {
                newEventDate += "02";
            }
            else if (month == "MAR")
            {
                newEventDate += "03";
            }
            else if (month == "APR")
            {
                newEventDate += "04";
            }
            else if (month == "MAY")
            {
                newEventDate += "05";
            }
            else if (month == "JUN")
            {
                newEventDate += "06";
            }
            else if (month == "JUL")
            {
                newEventDate += "07";
            }
            else if (month == "AUG")
            {
                newEventDate += "08";
            }
            else if (month == "SEP")
            {
                newEventDate += "09";
            }
            else if (month == "OCT")
            {
                newEventDate += "10";
            }
            else if (month == "NOV")
            {
                newEventDate += "11";
            }
            else if (month == "DEC")
            {
                newEventDate += "12";
            }
        }
        else
        {
            if (eventDate[0] == '0')
            {
                newEventDate += "0";
                newEventDate += newEventDate[1];
            }
            else
            {
                if (eventDate[1] != '-' && eventDate[1] != '/')
                {
                    newEventDate += eventDate.substr(0, 2);
                }
                else
                {
                    newEventDate += '0';
                    newEventDate += eventDate[0];
                }
            }
        }
    }
    else if (calendar == 'o')
    {
        if (isalpha(eventDate[0]))
        {
            for (int i = 0; i < 3; i++)
            {
                month += toupper(eventDate[i]);
            }
            if (month == "JAN")
            {
                newEventDate += "-01";
            }
            else if (month == "FEB")
            {
                newEventDate += "-02";
            }
            else if (month == "MAR")
            {
                newEventDate += "-03";
            }
            else if (month == "APR")
            {
                newEventDate += "-04";
            }
            else if (month == "MAY")
            {
                newEventDate += "-05";
            }
            else if (month == "JUN")
            {
                newEventDate += "-06";
            }
            else if (month == "JUL")
            {
                newEventDate += "-07";
            }
            else if (month == "AUG")
            {
                newEventDate += "-08";
            }
            else if (month == "SEP")
            {
                newEventDate += "-09";
            }
            else if (month == "OCT")
            {
                newEventDate += "-10";
            }
            else if (month == "NOV")
            {
                newEventDate += "-11";
            }
            else if (month == "DEC")
            {
                newEventDate += "-12";
            }
        }
        else
        {
            if (eventDate[0] == '0')
            {
                newEventDate += "-0";
                newEventDate += newEventDate[1];
            }
            else
            {
                if (eventDate[1] != '-' && eventDate[1] != '/')
                {
                    newEventDate += ("-" + eventDate.substr(0, 2));
                }
                else
                {
                    newEventDate += "-0";
                    newEventDate += (eventDate[0]);
                }
            }
        }
    }
    if (calendar == 'g')
    {
        if (isalpha(eventDate[0]))
        {
            counter = 0;
            for (char s : eventDate)
            {
                if (isdigit(s))
                {
                    if (counter == eventDate.size() - 1)
                    {
                        newEventDate += "0";
                        newEventDate += eventDate.substr(counter);
                        break;
                    }
                    else
                    {
                        newEventDate += eventDate.substr(counter);
                        break;
                    }
                }
                ++counter;
            }
        }
        else
        {
            counter = 0;
            for (char s : eventDate)
            {
                if (s == '/' || s == '-')
                {
                    ++counter;
                    if (counter == eventDate.size() - 1)
                    {
                        newEventDate += "0";
                        newEventDate += eventDate.substr(counter);
                        break;
                    }
                    else
                    {
                        newEventDate += eventDate.substr(counter);
                        break;
                    }
                }
                ++counter;
            }
        }
    }
    else if (calendar == 'o')
    {
        if (isalpha(eventDate[0]))
        {
            counter = 0;
            for (char s : eventDate)
            {
                if (isdigit(s))
                {
                    if (counter == eventDate.size() - 1)
                    {
                        newEventDate += "-0";
                        newEventDate += eventDate.substr(counter);
                        break;
                    }
                    else
                    {
                        newEventDate += ("-" + eventDate.substr(counter));
                        break;
                    }
                }
                ++counter;
            }
        }
        else
        {
            counter = 0;
            for (char s : eventDate)
            {
                if (s == '/' || s == '-')
                {
                    ++counter;
                    if (counter == eventDate.size() - 1)
                    {
                        newEventDate += "-0";
                        newEventDate += eventDate.substr(counter);
                        break;
                    }
                    else
                    {
                        newEventDate += ("-" + eventDate.substr(counter));
                        break;
                    }
                }
                ++counter;
            }
        }
    }
    eventDate = newEventDate;
    return eventDate;
}

std::string eventTimeConverter(char type, std::string eventTime, char calendar)
{
    std::string strTime;
    if (type == 's')
    {
        strTime = eventTime;
    }
    if (type == 'e')
    {
        strTime = eventTime;
    }

    std::string zero = "0";
    std::string dzero = "00";
    std::string colon = ":";

    if (strTime.size() == 6)
    {
    }
    else if (strTime.size() == 4)
    {
        strTime = zero + strTime.substr(0, 1) + strTime.substr(2) + dzero;
    }
    // Testing 

    // else if (strTime.size() == 1)
    // {
    //     strTime = zero + strTime + dzero + dzero;
    // }
    // else if (strTime.size() == 2)
    // {
    //     strTime = strTime + dzero + dzero;
    // }
    else
    {
        strTime = strTime.substr(0, 2) + strTime.substr(3) + dzero;
    }

    int stoitime = std::stoi(strTime);
    if (stoitime < 73000)
    {
        stoitime = stoitime + 120000;
        strTime = std::to_string(stoitime);
    }

    if (strTime.size() == 5)
        strTime = zero + strTime;

    if (calendar == 'o')
    {
        strTime = strTime.substr(0, 2) + ":" + strTime.substr(2, 2) + ":" + strTime.substr(4, 2);
    }

    if (type == 's')
    {
        eventTime = strTime;
    }
    if (type == 'e')
    {
        eventTime = strTime;
    }
    return eventTime;
}

namespace EventCreation
{

    void GenericEvent::addHourToTime(char type)
    {
        std::string strTime;
        std::string strHours;
        if (type == 's')
        {
            strTime = eventStartTime;
        }
        if (type == 'e')
        {
            strTime = eventEndTime;
        }

        strHours = strTime.substr(0, 2);
        int hour = std::stoi(strHours);

        if (strHours == "00")
        {
            strHours = "01";
            strTime = strHours + strTime.substr(2);
        }
        else if (strHours == "23")
        {
            strTime = "00" + strTime.substr(2);
            hour = 0;
        }
        else if (hour < 23)
        {
            hour = hour + 1;
        }

        std::ostringstream oss;
        oss << std::setw(2) << std::setfill('0') << hour;
        strTime = oss.str() + strTime.substr(2);

        if (type == 's')
        {
            eventStartTime = strTime;
        }
        if (type == 'e')
        {
            eventEndTime = strTime;
        }
    }

    void GenericEvent::addMinuteToTime(char type)
    {
        std::string strTime;
        std::string strMinute;
        if (type == 's')
        {
            strTime = eventStartTime;
        }
        if (type == 'e')
        {
            strTime = eventEndTime;
        }

        strMinute = strTime.substr(3, 2);
        int minute = std::stoi(strMinute);
        minute = (minute + 15) % 60;
        int carry = (minute + 15) / 60;

        std::ostringstream oss;
        oss << std::setw(2) << std::setfill('0') << minute;
        strTime = strTime.substr(0, 3) + oss.str() + strTime.substr(5);

        if (carry > 0)
        {
            addHourToTime(type);
        }

        if (type == 's')
        {
            eventStartTime = strTime;
        }
        if (type == 'e')
        {
            eventEndTime = strTime;
        }
    }
}