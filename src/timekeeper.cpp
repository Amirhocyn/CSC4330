#include "timekeeper.h"
#include "ExtractPDFText.h"
#include <imgui.h>
#include <stdio.h>
#include <vector>
#include <fstream>
#include <iostream>
#include <filesystem>
#include <string>
#include <sstream>
#include <tesseract/baseapi.h>
#include <leptonica/allheaders.h>

#include <cstdio>
#include <memory>
#include <stdexcept>
#include <array>


TimeKeeper::TimeKeeper(DropFolder &dfp)
{
    df = &dfp;
    extractedText.clear();
    for (int i=0; i<df->getSize(); i++)
    {
        if (Extract(df->getPath(i)))
        {
            filesRead++;
        }
    }
    if (extractedText.size()==filesRead)
    {
        goodReads = true;
    }
}

void DropFolder::handle_dropped_file(const char* path) {
    fs::path p(path);
    // Simple filter to prevent random files
    std::string ext = p.extension().string();
    if (ext == ".txt" || ext == ".pdf" || ext == ".png" || ext == ".jpg" || ext == ".jpeg"|| ext == ".docx") {
        filePaths.push_back(p);
        numFiles++;
    }
}

void TimeKeeper::ShowFileTexts(bool* p_open)
{
    ImGui::SetNextWindowSize(ImVec2(500, 440), ImGuiCond_FirstUseEver);
    if (ImGui::Begin("File text confirmation", p_open, ImGuiWindowFlags_MenuBar))
    {
        if (ImGui::BeginMenuBar())
        {
            if (ImGui::BeginMenu("File"))
            {
                if (ImGui::MenuItem("Close", "Ctrl+W")) { *p_open = false; }
                ImGui::EndMenu();
            }
            ImGui::EndMenuBar();
        }

        // Left side is files in
        static int selected = 0;
        {
            ImGui::BeginChild("file list", ImVec2(150, 0), ImGuiChildFlags_Border | ImGuiChildFlags_ResizeX);
            for (int i = 0; i < df->getSize(); i++)
            {
                char label[128];
                sprintf(label, "File %d: %s", i+1, df->getFileName(i).c_str());
                if (ImGui::Selectable(label, selected == i))
                    selected = i;
            }
            ImGui::EndChild();
        }
        ImGui::SameLine();

        // Right
        {
            ImGui::BeginGroup();
            ImGui::BeginChild("item view", ImVec2(0, -ImGui::GetFrameHeightWithSpacing())); 
            ImGui::Text("File: %s", df->getFileName(selected).c_str());
            ImGui::Separator();
            if (ImGui::BeginTabBar("##Tabs", ImGuiTabBarFlags_None))
            {
                if (ImGui::BeginTabItem("Extracted Text"))
                {
                    ImGui::TextWrapped(extractedText.at(selected).c_str());
                    ImGui::EndTabItem();
                }
                if (ImGui::BeginTabItem("Details"))
                {
                    ImGui::Text("File Name: %s", df->getFileName(selected).c_str());
                    ImGui::Text("File Type: %s", df->getExtension(selected).c_str());
                    ImGui::Text("Full Path: %s", df->getPath(selected).c_str());
                    ImGui::EndTabItem();
                }
                ImGui::EndTabBar();
            }
            ImGui::EndChild();
            if (ImGui::Button("Revert")) {}
            ImGui::SameLine();
            if (ImGui::Button("Save")) {}
            ImGui::EndGroup();
        }
    }
    ImGui::End();
}

bool TimeKeeper::Extract(fs::path fp)
{
    // Define supported extensions
    std::string validExt[] = {".txt", ".doc", ".docx", ".pdf", ".png", ".jpg", ".jpeg"};
    std::string fext = fp.extension().string();

    // 1. Documents (Txt, Docx, PDF) - PASS PATH TO PYTHON
    // We treat PDF just like the others now.
    if (fext == ".txt" || fext == ".doc" || fext == ".docx" || fext == ".pdf") {
        extractedText.push_back(fp.string()); 
        return true;
    }
    // 2. Images - PASS PATH TO PYTHON
    else if (fext == ".png" || fext == ".jpg" || fext == ".jpeg") {
        extractedText.push_back(fp.string()); 
        return true; 
    }
    else {
        std::cout << "Unsupported Filetype: " << fext << "\n";
        return false;
    }
}

bool TimeKeeper::ExtractTxt(fs::path fp)
{
    std::ifstream fin;
    std::stringstream txt;
    fin.open(fp.c_str());
    if (fin.is_open())
    {
        txt << fin.rdbuf();
        extractedText.push_back(txt.str());
        fin.close();
        return true;
    }
    fin.close();
    return false;
    
}

bool TimeKeeper::ExtractDoc(fs::path fp)
{
    return false;
}

bool TimeKeeper::ExtractPdf(fs::path fp)
{
    std::vector<std::string> pageTexts = ExtractPDFText(fp);
    if (pageTexts.empty() || pageTexts.at(0) == "ERROR") {
        return false;
    }
    std::string compiledText = "";
    for (const auto& text : pageTexts) {
        compiledText = compiledText + text + "\n";
    }
    extractedText.push_back(compiledText);
    return true;
}

bool TimeKeeper::ExtractImg(fs::path fp)
{

    std::string filepath = fp.string();

    tesseract::TessBaseAPI *api = new tesseract::TessBaseAPI();
    // Initialize tesseract-ocr with English, without specifying tessdata path
    if (api->Init(NULL, "eng")) {
        fprintf(stderr, "Could not initialize tesseract.\n");
        return false;
    }

    std::cout << "File path determined: " << filepath << "\n";
    // Open input image with leptonica library
    Pix *image = pixRead(filepath.c_str());
    api->SetImage(image);
    // Get OCR result
    std::cout << "Reading \n";
    char *readText = api->GetUTF8Text();
    std::cout << "read \n";

    if (readText)
    {
        // Input read text to extractedText
        extractedText.push_back(std::string(readText));
    }
    else
    {
        std::cout << "No text read for " << filepath << "\n";
    }

    // Destroy used object and release memory
    api->End();
    delete api;
    delete[] readText;
    pixDestroy(&image);

    return true;
}

bool TimeKeeper::ExtractImg(PIX *image)
{

    tesseract::TessBaseAPI *api = new tesseract::TessBaseAPI();
    // Initialize tesseract-ocr with English, without specifying tessdata path
    if (api->Init(NULL, "eng")) {
        fprintf(stderr, "Could not initialize tesseract.\n");
        return false;
    }

    // Open input image with leptonica library
    api->SetImage(image);
    // Get OCR result
    char *readText = api->GetUTF8Text();
    
    // Input read text to extractedText
    extractedText.push_back(std::string(readText));

    // Destroy used object and release memory
    api->End();
    delete api;
    delete readText;
    pixDestroy(&image);

    return true;
}

std::string TimeKeeper::execPython(const char* cmd) {
    std::array<char, 128> buffer;
    std::string result;
    
    // Use _popen on Windows, popen on Mac/Linux
    #ifdef _WIN32
        std::unique_ptr<FILE, decltype(&_pclose)> pipe(_popen(cmd, "r"), _pclose);
    #else
        std::unique_ptr<FILE, decltype(&pclose)> pipe(popen(cmd, "r"), pclose);
    #endif

    if (!pipe) {
        return "Error: popen() failed!";
    }
    
    while (fgets(buffer.data(), buffer.size(), pipe.get()) != nullptr) {
        result += buffer.data();
    }
    return result;
}

void TimeKeeper::generateICS(std::string filename, std::string title, std::string startDateTime, std::string endDateTime, std::string description) {
    // Open in binary mode to prevent Windows from messing with our explicit line endings
    std::ofstream icsFile(filename, std::ios::binary); 
    
    if (icsFile.is_open()) {
        // STRICT \r\n LINE ENDINGS FOR IPHONE COMPATIBILITY
        icsFile << "BEGIN:VCALENDAR\r\n";
        icsFile << "VERSION:2.0\r\n";
        icsFile << "PRODID:-//LSU//TimeKeeper v1.0//EN\r\n";
        icsFile << "CALSCALE:GREGORIAN\r\n";
        icsFile << "METHOD:PUBLISH\r\n";
        
        icsFile << "BEGIN:VEVENT\r\n";
        
        // Metadata
        icsFile << "UID:" << startDateTime << "-" << title.substr(0, 3) << "@timekeeper.app\r\n";
        icsFile << "DTSTAMP:" << startDateTime << "\r\n";
        
        // Start Time
        icsFile << "DTSTART:" << startDateTime << "\r\n";
        
        // End Time (Logic: If empty, use 1 hour duration)
        if (!endDateTime.empty()) {
             icsFile << "DTEND:" << endDateTime << "\r\n";
        } else {
             icsFile << "DURATION:PT1H\r\n"; 
        }
        
        icsFile << "STATUS:CONFIRMED\r\n";
        icsFile << "SEQUENCE:0\r\n";
        icsFile << "SUMMARY:" << title << "\r\n";
        
        // Description (Escape newlines to prevent breaking the file)
        // Simple replacement of \n with space for safety in prototype
        std::string cleanDesc = description;
        for (auto & c: cleanDesc) if (c == '\n') c = ' ';
        icsFile << "DESCRIPTION:" << cleanDesc << "\r\n";
        
        icsFile << "END:VEVENT\r\n";
        icsFile << "END:VCALENDAR\r\n";
        
        icsFile.close();
    }
}