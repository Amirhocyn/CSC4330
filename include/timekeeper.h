#ifndef TIMEKEEPER_H
#define TIMEKEEPER_H

#include <stdio.h>
#include <vector>
#include <fstream>
#include <filesystem>
#include <tesseract/baseapi.h>
#include <leptonica/allheaders.h>
#include <string>
#include <sstream>

namespace fs = std::filesystem;

class DropFolder {
    
    std::vector<fs::path> filePaths;
    int numFiles=0;

    

    public:

    void handle_dropped_file(const char*);
    bool hasFiles() {return (numFiles>0);}
    int getNumFiles() {return numFiles;}
    int getSize() {return filePaths.size();}
    fs::path getPath(int i) {return filePaths.at(i);}
    std::string getPathName(int i) {return filePaths.at(i).string(); }
    std::string getFileName(int i) {return filePaths.at(i).filename().string(); }
    std::string getExtension(int i) {return filePaths.at(i).extension().string(); }
};

enum extractors {
    etxt,
    edoc,
    epdf,
    epng
};

class TimeKeeper {

    private:
    int filesRead = 0;
    bool goodReads = false;
    DropFolder *df;
    std::vector<std::string> extractedText;
    
    bool ExtractTxt(fs::path fp);
    bool ExtractDoc(fs::path fp);
    bool ExtractPdf(fs::path fp);
    bool ExtractImg(fs::path fp); // for extracting from an existing image
    bool ExtractImg(PIX *image); // for extracting from generated images of PDFs


    public:
    TimeKeeper(DropFolder &dfp);
    bool Extract(fs::path fp);
    void ShowFileTexts(bool* p_open);
    int getSize() {return extractedText.size();}
};



#endif