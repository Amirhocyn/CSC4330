#include <iostream>
#include <poppler-document.h>
#include <poppler-page.h>
#include <poppler-global.h>
#include <string>
#include <vector>
#include <memory>
#include <filesystem>
#include "ExtractPDFText.h"

namespace fs = std::filesystem;

const int CHUNK_SIZE = 1024; // Define the size of each text chunk

std::vector<std::string> ExtractPDFText(fs::path filepath) {
    
    std::vector<std::string> pageTexts;
    std::string filename = filepath.string();
    std::unique_ptr<poppler::document> doc(poppler::document::load_from_file(filename));
    
    if (!doc) {
        std::cerr << "Failed to open the PDF file." << std::endl;
        pageTexts.push_back("ERROR");
        return pageTexts;
    }

    int numPages = doc->pages();
    std::cout << "Number of pages: " << numPages << std::endl;

    // Extract text from each page and process it in smaller chunks
    for (int i = 0; i < numPages; ++i) {
        std::unique_ptr<poppler::page> pg(doc->create_page(i));
        if (!pg) {
            std::cerr << "Failed to read the page." << std::endl;
            continue;
        }
        
        poppler::ustring text = pg->text();
        poppler::byte_array textUTF8 = text.to_utf8();
        auto data = textUTF8.data();
        std::string convertedString(data);
        pageTexts.push_back(convertedString);
        // // Process text in chunks
        // int textLength = text.size();
        // for (int start = 0; start < textLength; start += CHUNK_SIZE) {
        //     int end = std::min(start + CHUNK_SIZE, textLength);
        //     poppler::ustring chunk;
        //     chunk.assign(text.begin() + start, text.begin() + end); // Assign substring to chunk
        //     std::vector<char> utf8_bytes = chunk.to_utf8();
        //     std::string utf8_chunk(utf8_bytes.begin(), utf8_bytes.end()); // Convert the chunk directly to a std::string
        //     std::cout << utf8_chunk;
        // }
        // std::cout << std::endl << std::endl;
    }

    return pageTexts;
}
