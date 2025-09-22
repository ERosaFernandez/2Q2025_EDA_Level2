/**
 * @brief Lequel? language identification based on trigrams
 * @author Marc S. Ressl
 *
 * @copyright Copyright (c) 2022-2023
 *
 * @cite
 * https://towardsdatascience.com/understanding-cosine-similarity-and-its-application-fd42f585296a
 *
 * @cite
 * https://www.geeksforgeeks.org/python/jaccard-similarity/
 * https://rpubs.com/lgadar/weighted-jaccard
 * info about Jaccard similarity
 *
 * @cite
 * https://dsacl3-2019.github.io/materials/CavnarTrenkle.pdf
 * https://www.let.rug.nl/vannoord/TextCat/textcat.pdf
 * info about Cavnar Trenkle similarity
 */

#include "Lequel.h"

#include <cmath>
#include <codecvt>
#include <iostream>
#include <locale>

using namespace std;

/**
 * @name addToTrigramProfile
 * @brief Adds data to a previously created trigram profile from a given text.
 *
 * @param text String of UTF-8 Characters
 */
void addToTrigramProfile(const std::string& text, TrigramProfile& profile) {
    if (text.length() < 3)
        return;

    // One-time string creation
    // It's reused for every function call
    static std::string trigram;
    trigram.clear();
    trigram.reserve(12);

    // Native UTF-8 iteration
    unsigned short int char_count = 0;
    unsigned short int trigram_start = 0;
    unsigned short int trigram_next = 0;
    unsigned short int text_position = 0;

    unsigned char character;

    while (text_position < text.length()) {
        // Saves first position of the trigram
        if (char_count == 0) {
            trigram_start = text_position;
        }
        // Saves first position of the next trigram
        else if (char_count == 1) {
            trigram_next = text_position;
        }

        // Identifies UTF-8 character length
        // KNOWN ISSUE: It never expects a middle byte
        // SOLUTION: Never send a middle byte :)
        character = text[text_position];
        if (!(character & 0b10000000)) {
            text_position += 1;  // 1 Byte
        } else if ((character & 0b11100000) == 0b11000000) {
            text_position += 2;  // 2 Bytes
        } else if ((character & 0b11110000) == 0b11100000) {
            text_position += 3;  // 3 Bytes
        } else {
            text_position += 4;  // 4 Bytes
        }

        char_count++;

        // Extracts trigram
        if (char_count == 3) {
            trigram.assign(text, trigram_start, text_position - trigram_start);
            profile[trigram].real++;

            // Resets starting from the second position
            text_position = trigram_next;
            char_count = 0;
        }
    }
}

/**
 * @brief Normalizes a trigram profile.
 *
 * @param trigramProfile The trigram profile.
 */
void normalizeTrigramProfile(TrigramProfile& trigramProfile) {
    // Sums the squares of the trigram frequencies
    float sumSquares = 0.0f;

    auto trigramIterator = trigramProfile.begin();

    // for (auto& element : trigramProfile) {
    //     element.second.normalized = element.second.real;
    //     sumSquares += element.second.normalized * element.second.normalized;
    // }
    while (trigramIterator != trigramProfile.end()) {
        trigramIterator->second.normalized = trigramIterator->second.real;
        sumSquares += trigramIterator->second.normalized * trigramIterator->second.normalized;
        trigramIterator++;
    }

    // Calculates the L2 norm
    float norm = sqrtf(sumSquares);
    if (norm == 0.0f)
        return;

    const float invNorm = 1.0f / norm;

    // Normalizes each trigram frequency by dividing by the norm
    trigramIterator = trigramProfile.begin();
    while (trigramIterator != trigramProfile.end()) {
        trigramIterator->second.normalized *= invNorm;
        trigramIterator++;
    }
}

/**
 * @name getCosineSimilarity
 * @brief Calculates the cosine similarity between two trigram profiles
 *
 * @param textProfile The text trigram profile
 * @param languageProfile The language trigram profile
 * @param globalSettings The struct containing all the settings data
 * @return The cosine similarity score
 */
float getCosineSimilarity(TrigramProfile& textProfile,
                          TrigramProfile& languageProfile,
                          settings_t& globalSettings) {
    const size_t text_size = textProfile.size();
    const size_t lang_size = languageProfile.size();

    // Early exit for empty profiles
    if (text_size == 0 || lang_size == 0) {
        return 0.0f;
    }

    float dotProduct = 0.0f;

    // Computes dot product by iterating over the smaller profile
    auto textIterator = textProfile.begin();
    auto languageIterator = languageProfile.begin();
    if (text_size <= lang_size) {
        // for (const auto& entry : textProfile) {
        //     const auto it = languageProfile.find(entry.first);
        //     dotProduct += (it != languageProfile.end())
        //                       ? entry.second.normalized * it->second.normalized
        //                       : 0.0f;
        // }
        while (textIterator != textProfile.end()) {
            languageIterator = languageProfile.find(textIterator->first);
            if (languageIterator != languageProfile.end())
                dotProduct += textIterator->second.normalized * textIterator->second.normalized;
            textIterator++;
            //     dotProduct += (it != languageProfile.end())
            //                       ? entry.second.normalized * it->second.normalized
            //                       : 0.0f;
        }
    } else {
        // for (const auto& entry : languageProfile) {
        //     const auto it = textProfile.find(entry.first);
        //     dotProduct +=
        //         (it != textProfile.end()) ? entry.second.normalized * it->second.normalized :
        //         0.0f;
        // }
        while (languageIterator != languageProfile.end()) {
            textIterator = textProfile.find(languageIterator->first);
            if (textIterator != textProfile.end())
                dotProduct +=
                    languageIterator->second.normalized * languageIterator->second.normalized;
            languageIterator++;
            //     dotProduct += (it != languageProfile.end())
            //                       ? entry.second.normalized * it->second.normalized
            //                       : 0.0f;
        }
    }

    return dotProduct;
}

/**
 * @name getJaccardSimilarity
 * @brief Calculates the Jaccard similarity between two trigram profiles.
 * More info about Jaccard similarity:
 * https://www.geeksforgeeks.org/python/jaccard-similarity/
 * https://rpubs.com/lgadar/weighted-jaccard
 *
 * @param textProfile The text trigram profile
 * @param languageProfile The language trigram profile
 * @param globalSettings The struct containing all the settings data
 * @return The Jaccard similarity score
 */
static float getJaccardSimilarity(TrigramProfile& profile,
                                  TrigramProfile& language,
                                  settings_t& globalSettings) {
    float in_common = 0;
    float total = 0;

    auto profile_iterator = profile.begin();
    auto language_iterator = language.begin();

    // Calculates the amount of elements in common, then the elements in total
    if (globalSettings.valueProcessingSetting == VALUE_NORMALIZE) {
        while (profile_iterator != profile.end()) {
            language_iterator = language.find(profile_iterator->first);
            if (language_iterator != language.end()) {
                in_common += std::min(profile_iterator->second.normalized,
                                      language_iterator->second.normalized);
            }
            total += profile_iterator->second.normalized;
            profile_iterator++;
        }

        for (language_iterator = language.begin(); language_iterator != language.end();
             language_iterator++) {
            total += language_iterator->second.normalized;
        }
    } else {
        while (profile_iterator != profile.end()) {
            language_iterator = language.find(profile_iterator->first);
            if (language_iterator != language.end()) {
                in_common +=
                    std::min(profile_iterator->second.real, language_iterator->second.real);
            }
            total += profile_iterator->second.real;
            profile_iterator++;
        }

        for (language_iterator = language.begin(); language_iterator != language.end();
             language_iterator++) {
            total += language_iterator->second.real;
        }
    }

    // Intersection divided by the union
    return in_common / (total - in_common);
}

/**
 * @name getCavnarTrenkleSimilarity
 * @brief Calculates the Cavnar Trenkle similarity between two trigram profiles.
 * More info about Cavnar Trenkle similarity:
 * https://dsacl3-2019.github.io/materials/CavnarTrenkle.pdf
 * https://www.let.rug.nl/vannoord/TextCat/textcat.pdf
 *
 * @param textProfile The text trigram profile
 * @param languageProfile The language trigram profile
 * @param globalSettings The struct containing all the settings data
 * @return The Cavnar Trenkle similarity score
 */
static float getCavnarTrenkleSimilarity(TrigramProfile& profile,
                                        TrigramProfile& language,
                                        settings_t& globalSettings) {
    float totalDistance = 0.0f;

    auto profileIterator = profile.begin();
    auto languageIterator = language.begin();

    // Calculates |profileNormalValue - languageNormalValue|
    if (globalSettings.valueProcessingSetting == VALUE_NORMALIZE) {
        // for (auto& input : profile) {
        //     auto iterator = language.find(input.first);
        //     if (iterator != language.end()) {
        //         totalDistance += std::abs(input.second.normalized - iterator->second.normalized);
        //     } else
        //         totalDistance += 1.0f;
        // }
        while (profileIterator != profile.end()) {
            languageIterator = language.find(profileIterator->first);
            if (languageIterator != language.end()) {
                totalDistance += std::abs(profileIterator->second.normalized -
                                          languageIterator->second.normalized);
            } else
                totalDistance += 1.0f;
            profileIterator++;
        }
    } else {
        // for (auto& input : profile) {
        //     auto iterator = language.find(input.first);
        //     if (iterator != language.end()) {
        //         totalDistance += std::abs(input.second.real - iterator->second.real);
        //     } else
        //         totalDistance += 1.0f;
        // }
        while (profileIterator != profile.end()) {
            languageIterator = language.find(profileIterator->first);
            if (languageIterator != language.end()) {
                totalDistance +=
                    std::abs(profileIterator->second.real - languageIterator->second.real);
            } else
                totalDistance += 1.0f;
            profileIterator++;
        }
    }

    // Convert distance to similarity
    return 1.0f / (1.0f + totalDistance);
}

/**
 * @name compareLanguages
 * @brief Identifies the language of a text.
 *
 * @param profile The profile created from the extracted text
 * @param languages A list of Language objects
 * @param globalSettings The struct containing all the settings data
 * @return The language code of the most likely language
 */
static std::string compareLanguages(TrigramProfile& profile,
                                    LanguageProfiles& languages,
                                    settings_t& globalSettings) {
    float max_value = 0;
    float temp_value = 0;
    std::string* max_value_name;

    auto languageIterator = languages.begin();

    switch (globalSettings.algorithmSetting) {
        case ALGORITHM_JACCARD:
            // for (auto &languageProfile : languages) {
            //     temp_value =
            //         getJaccardSimilarity(profile, languageProfile.trigramProfile,
            //         globalSettings);
            //     if (temp_value > max_value) {
            //         max_value = temp_value;
            //         max_value_name = &languageProfile.languageCode;
            //     }
            // }
            while (languageIterator != languages.end()) {
                temp_value =
                    getJaccardSimilarity(profile, languageIterator->trigramProfile, globalSettings);
                if (temp_value > max_value) {
                    max_value = temp_value;
                    max_value_name = &languageIterator->languageCode;
                }
                languageIterator++;
            }
            break;
        case ALGORITHM_CAVNARTRENKLE:
            // for (auto &languageProfile : languages) {
            //     temp_value = getCavnarTrenkleSimilarity(
            //         profile, languageProfile.trigramProfile, globalSettings);
            //     if (temp_value > max_value) {
            //         max_value = temp_value;
            //         max_value_name = &languageProfile.languageCode;
            //     }
            // }
            while (languageIterator != languages.end()) {
                temp_value = getCavnarTrenkleSimilarity(
                    profile, languageIterator->trigramProfile, globalSettings);
                if (temp_value > max_value) {
                    max_value = temp_value;
                    max_value_name = &languageIterator->languageCode;
                }
                languageIterator++;
            }
            break;
        case ALGORITHM_COSINE:
            // for (auto &languageProfile : languages) {
            //     temp_value =
            //         getCosineSimilarity(profile, languageProfile.trigramProfile, globalSettings);
            //     if (temp_value > max_value) {
            //         max_value = temp_value;
            //         max_value_name = &languageProfile.languageCode;
            //     }
            // }
            while (languageIterator != languages.end()) {
                temp_value =
                    getCosineSimilarity(profile, languageIterator->trigramProfile, globalSettings);
                if (temp_value > max_value) {
                    max_value = temp_value;
                    max_value_name = &languageIterator->languageCode;
                }
                languageIterator++;
            }
            break;
        default:
            return "";
    }

    return *max_value_name;
}

/**
 * @name identifyLanguageFromPath
 * @brief Identifies the language of a text given the file path;
 *
 * @param path string of characters for the file path
 * @param languages A list of Language objects
 * @param globalSettings The struct containing all the settings data
 * @return The language code of the most likely language
 */
std::string identifyLanguageFromPath(char* path,
                                     LanguageProfiles& languages,
                                     settings_t& globalSettings) {
    std::ifstream file(path);
    std::string extractedText;
    TrigramProfile profile;

    if (!file.is_open()) {
        perror(("Error while opening file " + std::string(path)).c_str());
        return "";
    }

    for (int counter = 0;
         (counter < globalSettings.lineLimit) && (std::getline(file, extractedText));
         counter++) {
        addToTrigramProfile(extractedText, profile);
    }

    if (globalSettings.valueProcessingSetting == VALUE_NORMALIZE) {
        normalizeTrigramProfile(profile);
    }

    return compareLanguages(profile, languages, globalSettings);
}

/**
 * @name identifyLanguageFromClipboard
 * @brief Identifies the language of a text given the clipboard contents
 *
 * @param path string of characters from the clipboard
 * @param languages A list of Language objects
 * @param globalSettings The struct containing all the settings data
 * @return The language code of the most likely language
 */
std::string identifyLanguageFromClipboard(std::string& clipboard,
                                          LanguageProfiles& languages,
                                          settings_t& globalSettings) {
    static std::string extractedText;
    static TrigramProfile profile;

    // Should avoid constant reallocations
    profile.reserve(5000);
    extractedText.reserve(500);
    profile.clear();
    extractedText.clear();

    // Special case: empty clipboard
    if (clipboard.empty()) {
        perror(("Error while opening Clipboard"));
        return "";
    }

    // Line by line iteration
    unsigned int line_count = 0;
    size_t start = 0;
    size_t line_end = 0;
    size_t end = 0;

    while (line_count < globalSettings.lineLimit && start < clipboard.length()) {
        // Find next line
        if ((end = clipboard.find('\n', start)) == std::string::npos) {
            end = clipboard.length();  // Special case: One long line
        }

        if (end > 0 && clipboard[end - 1] == '\r') {
            line_end = end - 1;  // Windows style end symbol '\r'
        } else {
            line_end = end;
        }

        std::string line = clipboard.substr(start, line_end - start);
        addToTrigramProfile(line, profile);

        line_count++;
        start = end + 1;  // Move past the newline
    }

    if (globalSettings.valueProcessingSetting == VALUE_NORMALIZE) {
        normalizeTrigramProfile(profile);
    }

    return compareLanguages(profile, languages, globalSettings);
}
