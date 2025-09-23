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

#include "CSVData.h"
#include "Text.h"

// #define NORMAL_TOGGLE_ENABLE  //(Un)commenting toggles the normalized/real values swap with a
// trigram limit bar

#ifdef NORMAL_TOGGLE_ENABLE
// value_t: holds both real and normalized values
struct value_t {
    float real;
    float normalized;
};
#endif

// algorithmSetting_t: indicates which similarity model to use
typedef enum { ALGORITHM_COSINE, ALGORITHM_JACCARD, ALGORITHM_CAVNARTRENKLE } algorithmSetting_t;
// valueProcessingSetting_t: toggles real or normalized values to process
typedef enum { VALUE_NORMALIZE = 0, VALUE_REAL } valueProcessingSetting_t;

// settings_t: determines settings across the programs
struct settings_t {
    algorithmSetting_t algorithmSetting = ALGORITHM_COSINE;
#ifdef NORMAL_TOGGLE_ENABLE
    valueProcessingSetting_t valueProcessingSetting = VALUE_NORMALIZE;
#else
    const valueProcessingSetting_t valueProcessingSetting = VALUE_NORMALIZE;
    unsigned int trigramLimit = 100;
    unsigned int trigramCurrentCount = 0;
#endif
    unsigned int lineLimit = 100;
};

// TrigramProfile: map of trigram -> frequency
// Swapped map for unordered_map
#ifdef NORMAL_TOGGLE_ENABLE
typedef std::unordered_map<std::string, value_t> TrigramProfile;
#else
typedef std::unordered_map<std::string, float> TrigramProfile;
#endif

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

void addToTrigramProfile(const std::string& text, TrigramProfile& profile);

#endif
