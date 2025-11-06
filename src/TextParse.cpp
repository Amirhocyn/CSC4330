#include <iostream>
#include <string>
#include <fstream>
#include <vector>
#include <sstream>
#include <regex>
#include <stdlib.h>
#include <ctype.h>
#include "SuggestedEvent.h"
#include "FileParse.h"

using namespace std;

std::string readWholeFile(const std::string &filename)
{
    std::ifstream file(filename);
    if (!file.is_open())
    {
        std::cerr << "Failed to open file: " << filename << std::endl;
        return "";
    }

    std::stringstream buffer;
    buffer << file.rdbuf(); // Read the whole file into the buffer
    file.close();           // Close the file after reading

    return buffer.str(); // Convert the buffer into a std::string and return
}

vector<EventCreation::SuggestedEvent> ParseStart(TextExtraction::FileParse grabFP)
{

    TextExtraction::FileParse FP = grabFP;
    vector<EventCreation::SuggestedEvent> sugEvents;
    string textToSearch = FP.fileText;
    string textName = FP.fileName;
    string classFound = "NotFound";
    vector<string> detail;
    vector<regex> masterTest;
    vector<string> timeOfThing;

    regex details("(final (exam)?)|(FINAL (exam)?)|(Final(exam)?)|(Exam(exam)?)|(EXAM)|(Midterm(exam)?)|(MIDTERM(exam)?)|(midterm(exam)?) \\d{1}?");

    regex TIMEOFTHINGFOUND("(1[0-2]|[1-9])(:[0-5][0-9]?)?");

    // yyyy-mm-dd
    regex test2("\\d{4}-\\d{1,2}-\\d{1,2}");

    // mm-dd-yyyy
    regex test("\\d{1,2}-\\d{1,2}-\\d{4}");
    regex test4("\\d{1,2}/\\d{1,2}(/(\\d{2}|\\d{4}))?");

    // Day, Month Day
    regex test1("((Jan(uary)?|Feb(ruary)?|Mar(ch)?|Apr(il)?|May|Jun(e)?|Jul(y)?|Aug(ust)?|Sep(tember)?|Oct(ober)?|Nov(ermber)?|Dec(ember)?) \\d{1,2})");

    // Day, Month Day Year at Start-End
    // regex test3("(Mon[day]|Tue[sday]|Wed[neday]|Thu[rsday]|Fri[day]|Sat[urday]|Sun[day]), (Jan[uary]|Feb[ruary]|Mar[ch]|Apr[il]|May|Jun[e]|Jul[y]|Aug[ust]|Sep[tember]|Oct[ober]|Nov[ermber]|Dec[ember]) \\d{1,2}, \\d{4} at \\d{1,2}(PM|AM|pm|am) - \\d{1,2}(PM|AM|pm|am)");

    // Time test for format ##:##, #:##, #:## (A or P).M., ##:## (A or P).M
    // regex timeTest("\\d{1,2}:\\d{2}[ (A|a|P|p)[.](M|m)[.]]");

    // regex test5("((Monday|Tuesday|Wednesday|Thursday|Friday|Saturday|Sunday), (January|February|March|April|May|June|July|August|September|October|November|December) \\d{1,2} \\d{1,2}(AM|PM|am|pm):\\d{1,2} (AM|PM|am|pm))");

    masterTest.push_back(test);
    masterTest.push_back(test1);
    masterTest.push_back(test2);
    masterTest.push_back(test4);
    // masterTest.push_back(test3);
    // masterTest.push_back(timeTest);
    // masterTest.push_back(test5);

    vector<string> dates;
    string currentSpot = textToSearch;
    size_t searchPM = currentSpot.find("PM");
    string foundTime;

    for (auto const &each : masterTest)
    {
        sregex_iterator end;
        sregex_iterator it(textToSearch.begin(), textToSearch.end(), (each));

        while (it != end)
        {
            std::smatch match = *it;
            std::string locate = match.str();
            int position = match.position();

            string idea = textToSearch.substr(position - 20, position);

            string timeOf = textToSearch.substr(position + 6, 10);

            sregex_iterator it2(idea.begin(), idea.end(), (details));

            while (it2 != end)
            {
                detail.push_back(it2->str());
                ++it2;
            }

            sregex_iterator it3(timeOf.begin(), timeOf.end(), (TIMEOFTHINGFOUND));
            while (it3 != end)
            {
                timeOfThing.push_back(it3->str());
                ++it3;
            }

            dates.push_back(it->str());
            ++it;
        }
    }

    classFound = textToSearch.substr(0, 8); // most syllabi have the name of the class as the first characters

    int i = 0;
    for (const auto &date : dates)
    {
        EventCreation::SuggestedEvent e(textName, date, classFound, detail[i], timeOfThing[i]);
        sugEvents.push_back(e);
        i++;
    }

    return sugEvents;
}