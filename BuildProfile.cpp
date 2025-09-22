#include "BuildProfile.h"
#include <iostream>
#include <unordered_map>
#include <string>
#include <vector>
#include <fstream>
#include <algorithm>

/**
 * @name buildLanguageProfile
 * @brief Builds the trigram profile for a given language from a text corpus.
 *
 * @param inputPath Path to the text corpus (UTF-8).
 * @param outputPath Path to save the trigram profile CSV.
 * @param languageCode Short language code (e.g., "grn", "cat", "cpp").
 * @return True if the profile was successfully created, false otherwise.
 */
bool buildLanguageProfile(const std::string &inputPath,
                          const std::string &outputPath,
                          const std::string &languageCode)
{
    std::ifstream file(inputPath);
    if (!file.is_open()) {
        std::cerr << "Error: could not open corpus " << inputPath << std::endl;
        return false;
    }

    TrigramProfile profile;
    std::string line;

    // 1. Reads the corpus line by line and update trigram profile
    while (std::getline(file, line)) {
        addToTrigramProfile(line, profile);
    }

    // 2. Copies trigram-frequency pairs to a vector for sorting
    std::vector<std::pair<std::string, int>> trigramList;
    trigramList.reserve(profile.size());
    for (const auto &entry : profile) {
        trigramList.emplace_back(entry.first, entry.second.real);
    }

    // 3. Sorts by frequency (descending).
    std::sort(trigramList.begin(), trigramList.end(),
              [](const auto &a, const auto &b) {
                  if (a.second != b.second)
                      return a.second > b.second; // higher count first
                  return a.first < b.first;       
              });

    // 4. Converts sorted list to CSVData
    CSVData data;
    data.reserve(trigramList.size());
    for (const auto &entry : trigramList) {
        std::vector<std::string> row;
        row.push_back(entry.first);                
        row.push_back(std::to_string(entry.second)); 
        data.push_back(row);
    }

    // 5. Writes to CSV
    if (!writeCSV(outputPath, data)) {
        std::cerr << "Error: could not write profile to " << outputPath << std::endl;
        return false;
    }

    std::cout << "Profile created for " << languageCode
              << " -> " << outputPath << std::endl;

    return true;
}


bool addLanguageToNamesCSV(const std::string &languageCode,
                           const std::string &languageName,
                           const std::string &csvPath)
{
    // Verifica si ya existe
    std::ifstream infile(csvPath);
    if (infile.is_open()) {
        std::string line;
        while (std::getline(infile, line)) {
            if (line.find("\"" + languageCode + "\"") != std::string::npos) {
                return true; // ya existe, no agrega
            }
        }
    }

    std::ofstream file(csvPath, std::ios::app); // modo append
    if (!file.is_open()) {
        std::cerr << "Error: could not open " << csvPath << std::endl;
        return false;
    }

    file << "\"" << languageCode << "\",\"" << languageName << "\"\n";
    return true;
}


int main()
{
    // Creates profiles for natural languages
    buildLanguageProfile("resources/corpus/corpus_guarani.txt",
                         "resources/trigrams/grn.csv",
                         "grn");
    addLanguageToNamesCSV("grn", "Guaraní", "resources/languagecode_names_es.csv");

    buildLanguageProfile("resources/corpus/corpus_catalan.txt",
                         "resources/trigrams/cat.csv",
                         "cat");
    addLanguageToNamesCSV("cat", "Catalán", "resources/languagecode_names_es.csv");

    buildLanguageProfile("resources/corpus/corpus_asturian.txt",
                         "resources/trigrams/ast.csv",
                         "ast");
    addLanguageToNamesCSV("ast", "Asturiano", "resources/languagecode_names_es.csv");

    // Creates profiles for programming languages
    buildLanguageProfile("resources/corpus/corpus_c.txt",
                         "resources/trigrams/c.csv",
                         "c");
    addLanguageToNamesCSV("c", "C", "resources/languagecode_names_es.csv");

    buildLanguageProfile("resources/corpus/corpus_cpp.txt",
                         "resources/trigrams/cpp.csv",
                         "cpp");
    addLanguageToNamesCSV("cpp", "C++", "resources/languagecode_names_es.csv");

    buildLanguageProfile("resources/corpus/corpus_python.txt",
                         "resources/trigrams/py.csv",
                         "py");
    addLanguageToNamesCSV("py", "Python", "resources/languagecode_names_es.csv");

    return 0;
}