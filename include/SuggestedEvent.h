#ifndef SUGGESTED_EVENT_H
#define SUGGESTED_EVENT_H

#include <string>


namespace EventCreation
{

    struct SuggestedEvent
    {
        std::string fileName;
        std::string suggestedTitle;
        std::string suggestedDate;
        std::string suggestedIdea;
        std::string suggestedTime;

        SuggestedEvent(std::string file, std::string date, std::string classFound, std::string idea, std::string timeOf)
        {
            fileName = file;
            suggestedTitle = classFound;
            suggestedDate = date;
            suggestedIdea = idea;
            suggestedTime = timeOf;
        }
    };
};

#endif