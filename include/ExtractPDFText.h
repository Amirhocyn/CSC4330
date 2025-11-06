#ifndef EXTRACTPDFTEXT_H
#define EXTRACTPDFTEXT_H

#include <iostream>
#include <poppler-document.h>
#include <poppler-page.h>
#include <poppler-global.h>
#include <string>
#include <vector>
#include <memory>
#include <filesystem>

namespace fs = std::filesystem;

std::vector<std::string> ExtractPDFText(fs::path filepath);

#endif