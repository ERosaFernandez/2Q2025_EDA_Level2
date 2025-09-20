/**
 * @brief Lequel? language identification based on trigrams
 * @author Marc S. Ressl
 *
 * @copyright Copyright (c) 2022-2023
 *
 * @cite
 * https://towardsdatascience.com/understanding-cosine-similarity-and-its-application-fd42f585296a
 */

#include "Lequel.h"

#include <cmath>
#include <codecvt>
#include <iostream>
#include <locale>

#define LINE_LIMIT 50

using namespace std;

// ============ UNUSED =============//

/**
 * @brief Builds a trigram profile from a given text.
 *
 * @param text Vector of lines (Text)
 * @return TrigramProfile The trigram profile
 */
TrigramProfile buildTrigramProfile(const Text& text) {
    wstring_convert<std::codecvt_utf8_utf16<wchar_t>> converter;

    // Your code goes here...
    for (auto line : text) {
        if ((line.length() > 0) && (line[line.length() - 1] == '\r'))
            line = line.substr(0, line.length() - 1);
    }

    // Tip: converts UTF-8 string to wstring
    // wstring unicodeString = converter.from_bytes(textLine);

    // Tip: convert wstring to UTF-8 string
    // string trigram = converter.to_bytes(unicodeTrigram);

    return TrigramProfile();  // Fill-in result here
}

// ==================================//

/**
 * @name addToTrigramProfile
 * @brief Adds data to a previously created trigram profile from a given text.
 *
 * @param text String of UTF-8 Characters
 */
static void addToTrigramProfile(const std::string& text, TrigramProfile& profile) {
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
        unsigned char character = text[text_position];
        if (!(character & 0b10000000)) {
            text_position += 1;  // 1 Byte
        } else if ((character & 0b11100000) == 0b11000000) {
            text_position += 2;  // 2 Bytes
        } else if ((character & 0b11110000) == 0b11100000) {
            text_position += 3;  // 3 Bytes
        } else {
            text_position += 4;  // 4 Bytes
        }
        // text_position += (byte < 0x80) ? 1 : (byte < 0xE0) ? 2 : (byte < 0xF0) ? 3 : 4;

        char_count++;

        // Extracts trigram
        if (char_count == 3) {
            trigram.assign(text, trigram_start, text_position - trigram_start);
            profile[trigram]++;

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
    for (auto& element : trigramProfile) {
        sumSquares += element.second * element.second;
    }

    // Calculates the L2 norm
    float norm = sqrtf(sumSquares);
    if (norm == 0.0f)
        return;

    const float invNorm = 1.0f / norm;

    // Normalizes each trigram frequency by dividing by the norm
    for (auto& element : trigramProfile) {
        element.second *= invNorm;
    }
}

/**
 * @brief Calculates the cosine similarity between two trigram profiles
 *
 * @param textProfile The text trigram profile
 * @param languageProfile The language trigram profile
 * @return float The cosine similarity score
 */
float getCosineSimilarity(TrigramProfile& textProfile, TrigramProfile& languageProfile) {
    const size_t text_size = textProfile.size();
    const size_t lang_size = languageProfile.size();

    // Early exit for empty profiles
    if (text_size == 0 || lang_size == 0) {
        return 0.0f;
    }

    float dotProduct = 0.0f;

    // Computes dot product by iterating over the smaller profile
    if (text_size <= lang_size) {
        for (const auto& entry : textProfile) {
            const auto it = languageProfile.find(entry.first);
            dotProduct += (it != languageProfile.end()) ? entry.second * it->second : 0.0f;
        }
    } else {
        for (const auto& entry : languageProfile) {
            const auto it = textProfile.find(entry.first);
            dotProduct += (it != textProfile.end()) ? entry.second * it->second : 0.0f;
        }
    }

    return dotProduct;
}

/**
 * @brief Identifies the language of a text.
 *
 * @param text A Text (vector of lines)
 * @param languages A list of Language objects
 * @return string The language code of the most likely language
 */
string identifyLanguage(const Text& text, LanguageProfiles& languages) {
    // Your code goes here...

    return "";  // Fill-in result here
}

/**
 * @name identifyLanguageFromPath
 * @brief Identifies the language of a text given the file path;
 *
 * @param path string of characters for the file path
 * @param languages A list of Language objects
 * @return string The language code of the most likely language
 */
std::string identifyLanguageFromPath(char* path, LanguageProfiles& languages) {
    std::ifstream file(path);
    std::string extractedText;
    TrigramProfile profile;

    float max_cosine = 0;
    float temp_cosine = 0;
    std::string max_cosine_name;
    // wstring_convert<std::codecvt_utf8_utf16<wchar_t>> converter;

    if (!file.is_open()) {
        perror(("Error while opening file " + std::string(path)).c_str());
        return "";
    }

    for (int counter = 0; (counter < LINE_LIMIT) && (std::getline(file, extractedText));
         counter++) {
        addToTrigramProfile(extractedText, profile);
    }

    normalizeTrigramProfile(profile);

    for (auto languageProfile : languages) {
        temp_cosine = getCosineSimilarity(profile, languageProfile.trigramProfile);
        if (temp_cosine > max_cosine) {
            max_cosine = temp_cosine;
            max_cosine_name = languageProfile.languageCode;
        }
    }

    return max_cosine_name;
}

/**
 * @name identifyLanguageFromClipboard
 * @brief Identifies the language of a text given the clipboard contents
 *
 * @param path string of characters from the clipboard
 * @param languages A list of Language objects
 * @return string The language code of the most likely language
 */
std::string identifyLanguageFromClipboard(std::string& clipboard, LanguageProfiles& languages) {
    std::string extractedText;
    TrigramProfile profile;

    float max_cosine = 0;
    float temp_cosine = 0;
    std::string max_cosine_name;

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

    while (line_count < LINE_LIMIT && start < clipboard.length()) {
        // Find next line
        while ((end = clipboard.find('\n', start)) - start < 3) {
            start = end + 1;  // Special case: Blank line
        }

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

    normalizeTrigramProfile(profile);

    for (auto languageProfile : languages) {
        temp_cosine = getCosineSimilarity(profile, languageProfile.trigramProfile);
        if (temp_cosine > max_cosine) {
            max_cosine = temp_cosine;
            max_cosine_name = languageProfile.languageCode;
        }
    }

    return max_cosine_name;
}
