/**
 * @brief Lequel? language identification based on trigrams
 * @author Marc S. Ressl
 *
 * @copyright Copyright (c) 2022-2023
 *
 * @cite
 * https://towardsdatascience.com/understanding-cosine-similarity-and-its-application-fd42f585296a
 */

#ifndef LEQUEL_H
#define LEQUEL_H

#include <list>
#include <map>
#include <string>
#include <unordered_map>

#include "Text.h"

// value_t: holds both real and normalized values
struct value_t {
    float real;
    float normalized;
};

// algorithmSetting_t: indicates which similarity model to use
typedef enum { ALGORITHM_COSINE, ALGORITHM_JACCARD, ALGORITHM_CAVNARTRENKLE } algorithmSetting_t;
// valueProcessingSetting_t: toggles real or normalized values to process
typedef enum { VALUE_NORMALIZE = 0, VALUE_REAL } valueProcessingSetting_t;

// settings_t: determines settings across the programs
struct settings_t {
    algorithmSetting_t algorithmSetting = ALGORITHM_COSINE;
    valueProcessingSetting_t valueProcessingSetting = VALUE_NORMALIZE;
    unsigned int lineLimit = 100;
};

// TrigramProfile: map of trigram -> frequency
// Swapped map for unordered_map
typedef std::unordered_map<std::string, value_t> TrigramProfile;

// TrigramList: list of trigrams
typedef std::list<std::string> TrigramList;

struct LanguageProfile {
    std::string languageCode;
    TrigramProfile trigramProfile;
};

typedef std::list<LanguageProfile> LanguageProfiles;

// Functions
TrigramProfile buildTrigramProfile(const Text& text);
void normalizeTrigramProfile(TrigramProfile& trigramProfile);
float getCosineSimilarity(TrigramProfile& textProfile, TrigramProfile& languageProfile);
std::string identifyLanguage(const Text& text, LanguageProfiles& languages);

std::string identifyLanguageFromPath(char* path,
                                     LanguageProfiles& languages,
                                     settings_t& globalSettings);

std::string identifyLanguageFromClipboard(std::string& clipboard,
                                          LanguageProfiles& languages,
                                          settings_t& globalSettings);

#endif
