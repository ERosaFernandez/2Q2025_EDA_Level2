#ifndef BUILDPROFILE_H
#define BUILDPROFILE_H

#include <string>

#include "Lequel.h"
#include "CSVData.h"
#include "Text.h"

// Build a trigram profile for a given language from a text corpus.
// inputPath:   path to a large text corpus (UTF-8).
// outputPath:  where to save the trigram profile (CSV).
// languageCode: short language code (e.g., "grn", "cat", "cpp").
bool buildLanguageProfile(const std::string &inputPath,
                          const std::string &outputPath,
                          const std::string &languageCode);

#endif 
