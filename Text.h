/**
 * @brief Reads text files
 * @author Marc S. Ressl
 *
 * @copyright Copyright (c) 2022-2023
 */

#ifndef TEXT_H
#define TEXT_H

#include <list>
#include <string>
#include <fstream>

// Text: list of strings
typedef std::list<std::string> Text;

// Functions
bool getTextFromString(const std::string &s, Text &text);
bool getTextFromFile(const std::string path, Text &text);
// FILE * getTextFromFile(const char* path, Text &text);

#endif
