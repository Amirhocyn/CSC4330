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
            ImGui::BeginChild("item view", ImVec2(0, -ImGui::GetFrameHeightWithSpacing())); // Leave room for 1 line below us
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
    std::string validExt[] = {".txt", ".doc", ".docx", ".pdf", ".png", ".jpg", ".tiff"};
    std::string fext = fp.extension().string();
    if (fext == validExt[0])   // Text file
    {
        return this->ExtractTxt(fp);
    }
    else if (fext == validExt[1] || fext == validExt[2]) // Doc/Docx file
    {
        return this->ExtractDoc(fp);
    }
    else if (fext == validExt[3]) // PDF file
    {
        return this->ExtractPdf(fp);
    }
    else if (fext == validExt[4] || fext == validExt[5] || fext == validExt[6]) // Valid Tesseract Img File
    {
        return this->ExtractImg(fp);
    }
    else
    {
        std::cout << "Unsupported Filetype";
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
    // Changing library so old function removed
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
    delete readText;
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