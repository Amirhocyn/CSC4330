#ifndef GENERIC_EVENT_H
#define GENERIC_EVENT_H

#include <sstream>
#include <iomanip>
#include <string>
#include <vector>
#include <cmath>



namespace EventCreation
{

    class GenericEvent
    {
    public:
        std::string eventTitle;
        std::string eventDate;
        std::string eventStartTime;
        std::string eventEndTime;
        std::string timeZone;
        std::string location;
        std::string details;
        std::vector<std::string> eventAlerts;

        std::string getEventTitle()
        {
            return eventTitle;
        }

        std::string getDetails()
        {
            return details;
        }

        std::string getEventDate()
        {
            return eventDate;
        }

        std::string getEventStartTime()
        {
            return eventStartTime;
        }

        std::string getEndTime()
        {
            return eventEndTime;
        }

        std::string getTimeZone()
        {
            return timeZone;
        }

        std::string getLocation()
        {
            return location;
        }

        void addHourToTime(char type);

        void addMinuteToTime(char type);

        std::string eventDateConverter(std::string date, char calendar) {};

        std::string eventTimeConverter(char type, std::string eventTime, char calendar) {};

        void setEventTitle(const std::string newEventTitle)
        {
            eventTitle = newEventTitle;
            return;
        }

        void setDetails(const std::string newEventDetail)
        {
            details = newEventDetail;
            return;
        }

        void setEventDate(const std::string newEventDate)
        {
            eventDate = newEventDate;
            return;
        }

        void setEventStartTime(const std::string newEventStartTime)
        {
            eventStartTime = newEventStartTime;
            return;
        }

        void setEventEndTime(const std::string newEventEndTime)
        {
            eventEndTime = newEventEndTime;
            return;
        }

        void setTimeZone(const std::string newTimeZone)
        {
            timeZone = newTimeZone;
            return;
        }

        void setLocation(const std::string newLocation)
        {
            location = newLocation;
            return;
        }
    };

    // end

};

#endif