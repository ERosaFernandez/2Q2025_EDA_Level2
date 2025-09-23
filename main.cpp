/**
 * @brief Lequel? main module
 * @author Marc S. Ressl
 *
 * @copyright Copyright (c) 2022-2023
 */

#include <chrono>
#include <iostream>
#include <map>
#include <string>

#include "CSVData.h"
#include "Lequel.h"
#include "raylib.h"

#define LINE_WIDTH 5.0f
#define MIN_LIMIT 10
#define MED_LIMIT 200
#define HIGH_LIMIT 1000

using namespace std;

const string LANGUAGECODE_NAMES_FILE = "resources/languagecode_names_es.csv";
const string TRIGRAMS_PATH = "resources/trigrams/";

struct buttons_t {
    Rectangle algoCosine;
    Rectangle algoJaccard;
    Rectangle algoCavnarTrenkle;
#ifdef NORMAL_TOGGLE_ENABLE
    Rectangle toggleNormalize;
#else
    Rectangle trigramLimit;
#endif
    Rectangle lineLimit;
};

/**
 * @brief Loads trigram data.
 *
 * @param languageCodeNames Map of language code vs. language name (in i18n locale).
 * @param languages The trigram profiles.
 * @return true Succeeded
 * @return false Failed
 */
bool loadLanguagesData(unordered_map<string, string>& languageCodeNames,
                       LanguageProfiles& languages) {
    // Reads available language codes
    cout << "Reading language codes..." << endl;

    CSVData languageCodesCSVData;
    if (!readCSV(LANGUAGECODE_NAMES_FILE, languageCodesCSVData))
        return false;

    // Reads trigram profile for each language code
    for (auto& fields : languageCodesCSVData) {
        if (fields.size() != 2)
            continue;

        string languageCode = fields[0];
        string languageName = fields[1];

        languageCodeNames[languageCode] = languageName;

        cout << "Reading trigram profile for language code \"" << languageCode << "\"..." << endl;

        CSVData languageCSVData;
        if (!readCSV(TRIGRAMS_PATH + languageCode + ".csv", languageCSVData))
            return false;

        languages.push_back(LanguageProfile());
        LanguageProfile& language = languages.back();

        language.languageCode = languageCode;

        for (auto& fields : languageCSVData) {
            if (fields.size() != 2)
                continue;

            string trigram = fields[0];
            float frequency = (float)stoi(fields[1]);

#ifdef NORMAL_TOGGLE_ENABLE
            language.trigramProfile[trigram].real = frequency;
#else
            language.trigramProfile[trigram] = frequency;
#endif
        }

        normalizeTrigramProfile(language.trigramProfile);
    }

    return true;
}

/**
 * @name timestamp_millis
 * @brief Returns the number of milliseconds since the program started, similar to Arduino's
 * millis().
 * @return Current timestamp in milliseconds as a long long.
 */
unsigned long long timestamp_millis() {
    auto now = std::chrono::steady_clock::now();
    auto ms = std::chrono::time_point_cast<std::chrono::milliseconds>(now);
    return ms.time_since_epoch().count();
}

double timestamp_millis_high_resolution() {
    auto now = std::chrono::high_resolution_clock::now();
    auto ns = std::chrono::time_point_cast<std::chrono::nanoseconds>(now);
    return ns.time_since_epoch().count() / 1000000.0;  // Convert ns to ms
}

/**
 * @name setupButtons
 * @brief Sets the position, height and width for all buttons
 *
 * @param buttons The struct containing all button data
 */
void setupButtons(buttons_t& buttons) {
    buttons.algoCavnarTrenkle.y = 130;
    buttons.algoCosine.y = buttons.algoCavnarTrenkle.y;
    buttons.algoJaccard.y = buttons.algoCavnarTrenkle.y;

    buttons.lineLimit.y = 220;

    buttons.algoCavnarTrenkle.height = 50;
    buttons.algoCosine.height = buttons.algoCavnarTrenkle.height;
    buttons.algoJaccard.height = buttons.algoCavnarTrenkle.height;

    buttons.lineLimit.height = buttons.algoJaccard.height;

    buttons.algoCavnarTrenkle.width = 265;
    buttons.algoCosine.width = buttons.algoCavnarTrenkle.width - 135;
    buttons.algoJaccard.width = buttons.algoCavnarTrenkle.width - 120;

    buttons.lineLimit.width = buttons.algoCavnarTrenkle.width - 45;

    buttons.algoCavnarTrenkle.x = 20;
    buttons.algoCosine.x =
        buttons.algoCavnarTrenkle.x + buttons.algoCavnarTrenkle.width - LINE_WIDTH;
    buttons.algoJaccard.x = buttons.algoCavnarTrenkle.x + buttons.algoCavnarTrenkle.width +
                            buttons.algoCosine.width - 2 * LINE_WIDTH;

    buttons.lineLimit.x = buttons.algoCavnarTrenkle.x + buttons.algoCavnarTrenkle.width + 160;

#ifdef NORMAL_TOGGLE_ENABLE
    buttons.toggleNormalize.y = 220;
    buttons.toggleNormalize.height = buttons.algoJaccard.height;
    buttons.toggleNormalize.width = buttons.algoCavnarTrenkle.width - 45;
    buttons.toggleNormalize.x = buttons.algoCavnarTrenkle.x;
#else
    buttons.trigramLimit.y = 220;
    buttons.trigramLimit.height = buttons.algoJaccard.height;
    buttons.trigramLimit.width = buttons.algoCavnarTrenkle.width - 45;
    buttons.trigramLimit.x = buttons.algoCavnarTrenkle.x;
#endif
}

/**
 * @name drawButtons
 * @brief Draws the buttons
 *
 * @param buttons The struct containing all button data
 * @param globalSettings The struct containing all the settings data
 */
void drawButtons(buttons_t& buttons, settings_t& globalSettings) {
    Color CavTrenkle;
    Color Cosine;
    Color Jaccard;
    static char lineBuffer[8];
    static char trigramBuffer[8];
    sprintf(lineBuffer, "%d", globalSettings.lineLimit);
#ifndef NORMAL_TOGGLE_ENABLE
    sprintf(trigramBuffer, "%d", globalSettings.trigramLimit);
#endif

    switch (globalSettings.algorithmSetting) {
        case ALGORITHM_CAVNARTRENKLE:
            DrawRectangleRec(buttons.algoCavnarTrenkle, BROWN);
            DrawRectangleLinesEx(buttons.algoCosine, LINE_WIDTH, BROWN);
            DrawRectangleLinesEx(buttons.algoJaccard, LINE_WIDTH, BROWN);
            CavTrenkle = BEIGE;
            Cosine = BROWN;
            Jaccard = BROWN;
            break;

        case ALGORITHM_COSINE:
            DrawRectangleRec(buttons.algoCosine, BROWN);
            DrawRectangleLinesEx(buttons.algoCavnarTrenkle, LINE_WIDTH, BROWN);
            DrawRectangleLinesEx(buttons.algoJaccard, LINE_WIDTH, BROWN);
            CavTrenkle = BROWN;
            Cosine = BEIGE;
            Jaccard = BROWN;
            break;

        case ALGORITHM_JACCARD:
            DrawRectangleRec(buttons.algoJaccard, BROWN);
            DrawRectangleLinesEx(buttons.algoCavnarTrenkle, LINE_WIDTH, BROWN);
            DrawRectangleLinesEx(buttons.algoCosine, LINE_WIDTH, BROWN);
            CavTrenkle = BROWN;
            Cosine = BROWN;
            Jaccard = BEIGE;
            break;

        default:
            break;
    }

    DrawText("Algoritmo de comparación",
             buttons.algoCavnarTrenkle.x,
             buttons.algoCavnarTrenkle.y - 30,
             28,
             BROWN);

    DrawText("Cavnar Trenkle",
             buttons.algoCavnarTrenkle.x + 10,
             buttons.algoCavnarTrenkle.y + 10,
             30,
             CavTrenkle);
    DrawText("Coseno", buttons.algoCosine.x + 10, buttons.algoCosine.y + 10, 30, Cosine);
    DrawText("Jaccard", buttons.algoJaccard.x + 10, buttons.algoJaccard.y + 10, 30, Jaccard);

#ifdef NORMAL_TOGGLE_ENABLE
    DrawText("Valores de frecuencia",
             buttons.toggleNormalize.x,
             buttons.toggleNormalize.y - 30,
             28,
             BROWN);

    if (globalSettings.valueProcessingSetting == VALUE_NORMALIZE) {
        DrawRectangleRec(buttons.toggleNormalize, BROWN);
        DrawText("Normalizados",
                 buttons.toggleNormalize.x + 10,
                 buttons.toggleNormalize.y + 10,
                 30,
                 BEIGE);
    } else {
        DrawRectangleLinesEx(buttons.toggleNormalize, LINE_WIDTH, BROWN);
        DrawText(
            "Reales", buttons.toggleNormalize.x + 60, buttons.toggleNormalize.y + 10, 30, BROWN);
    }
#else
    DrawText("Límite de trigramas (scroll)",
             buttons.trigramLimit.x,
             buttons.trigramLimit.y - 30,
             28,
             BROWN);
    DrawRectangleLinesEx(buttons.trigramLimit, LINE_WIDTH, BROWN);
    DrawText(trigramBuffer, buttons.trigramLimit.x + 15, buttons.trigramLimit.y + 10, 30, BROWN);
#endif

    DrawText("Límite de líneas (scroll)", buttons.lineLimit.x, buttons.lineLimit.y - 30, 28, BROWN);
    DrawRectangleLinesEx(buttons.lineLimit, LINE_WIDTH, BROWN);
    DrawText(lineBuffer, buttons.lineLimit.x + 15, buttons.lineLimit.y + 10, 30, BROWN);
}

int main(int, char*[]) {
    // Swapped map for unordered_map
    unordered_map<string, string> languageCodeNames;
    LanguageProfiles languages;

    settings_t globalSettings;

    buttons_t buttons;

    Vector2 mousePosition;

    setupButtons(buttons);

    double timer_start = 0, timer_end = 0;

    float mouseWheel;

    if (!loadLanguagesData(languageCodeNames, languages)) {
        cout << "Could not load trigram data." << endl;
        return 1;
    }

    int screenWidth = 800;
    int screenHeight = 450;

    InitWindow(screenWidth, screenHeight, "Lequel?");

    SetTargetFPS(60);

    string languageCode = "---";

    while (!WindowShouldClose()) {
        if (IsKeyPressed(KEY_V) && (IsKeyDown(KEY_LEFT_CONTROL) || IsKeyDown(KEY_RIGHT_CONTROL) ||
                                    IsKeyDown(KEY_LEFT_SUPER) || IsKeyDown(KEY_RIGHT_SUPER))) {
            timer_start = timestamp_millis_high_resolution();
            std::string clipboard = GetClipboardText();
            languageCode = identifyLanguageFromClipboard(clipboard, languages, globalSettings);
            timer_end = timestamp_millis_high_resolution();
        }

        if (IsFileDropped()) {
            FilePathList droppedFiles = LoadDroppedFiles();
            timer_start = timestamp_millis_high_resolution();

            if (droppedFiles.count == 1) {

                languageCode =
                    identifyLanguageFromPath(droppedFiles.paths[0], languages, globalSettings);
                timer_end = timestamp_millis_high_resolution();

                UnloadDroppedFiles(droppedFiles);
            }
        }
        if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
            mousePosition = GetMousePosition();

            if (CheckCollisionPointRec(mousePosition, buttons.algoCavnarTrenkle))
                globalSettings.algorithmSetting = ALGORITHM_CAVNARTRENKLE;
            else if (CheckCollisionPointRec(mousePosition, buttons.algoCosine))
                globalSettings.algorithmSetting = ALGORITHM_COSINE;
            else if (CheckCollisionPointRec(mousePosition, buttons.algoJaccard))
                globalSettings.algorithmSetting = ALGORITHM_JACCARD;
#ifdef NORMAL_TOGGLE_ENABLE
            else if (CheckCollisionPointRec(mousePosition, buttons.toggleNormalize))
                if (globalSettings.algorithmSetting != ALGORITHM_COSINE) {
                    if (globalSettings.valueProcessingSetting == VALUE_NORMALIZE)
                        globalSettings.valueProcessingSetting = VALUE_REAL;
                    else
                        globalSettings.valueProcessingSetting = VALUE_NORMALIZE;
                }
#endif
        }

        if ((mouseWheel = GetMouseWheelMove())) {
            mousePosition = GetMousePosition();
            if (CheckCollisionPointRec(mousePosition, buttons.lineLimit)) {
                if (mouseWheel > 0) {
                    if (globalSettings.lineLimit < MIN_LIMIT)
                        globalSettings.lineLimit += 1;
                    else if (globalSettings.lineLimit < MED_LIMIT)
                        globalSettings.lineLimit += 10;
                    else if (globalSettings.lineLimit < HIGH_LIMIT)
                        globalSettings.lineLimit += 100;
                    else
                        globalSettings.lineLimit += 1000;
                }

                else {
                    if (globalSettings.lineLimit <= MIN_LIMIT)
                        globalSettings.lineLimit -= 1;
                    else if (globalSettings.lineLimit <= MED_LIMIT)
                        globalSettings.lineLimit -= 10;
                    else if (globalSettings.lineLimit <= HIGH_LIMIT)
                        globalSettings.lineLimit -= 100;
                    else
                        globalSettings.lineLimit -= 1000;
                }

                if (globalSettings.lineLimit == 0)
                    globalSettings.lineLimit = 1;
            }
#ifndef NORMAL_TOGGLE_ENABLE
            else if (CheckCollisionPointRec(mousePosition, buttons.trigramLimit)) {
                if (mouseWheel > 0) {
                    if (globalSettings.trigramLimit < MIN_LIMIT)
                        globalSettings.trigramLimit += 1;
                    else if (globalSettings.trigramLimit < MED_LIMIT)
                        globalSettings.trigramLimit += 10;
                    else if (globalSettings.trigramLimit < HIGH_LIMIT)
                        globalSettings.trigramLimit += 100;
                    else
                        globalSettings.trigramLimit += 1000;
                }

                else {
                    if (globalSettings.trigramLimit <= MIN_LIMIT)
                        globalSettings.trigramLimit -= 1;
                    else if (globalSettings.trigramLimit <= MED_LIMIT)
                        globalSettings.trigramLimit -= 10;
                    else if (globalSettings.trigramLimit <= HIGH_LIMIT)
                        globalSettings.trigramLimit -= 100;
                    else
                        globalSettings.trigramLimit -= 1000;
                }

                if (globalSettings.trigramLimit == 0)
                    globalSettings.trigramLimit = 1;
            }
#endif
        }

#ifdef NORMAL_TOGGLE_ENABLE
        if (globalSettings.algorithmSetting == ALGORITHM_COSINE)
            globalSettings.valueProcessingSetting = VALUE_NORMALIZE;
#endif

        BeginDrawing();

        ClearBackground(BEIGE);

        DrawText("Lequel?", 20, 10, 50, BROWN);
        DrawText("Copia y pega con Ctrl+V, o arrastra un archivo...", 20, 60, 20, BROWN);

        string languageString;
        if (languageCode != "---") {
            if (languageCodeNames.find(languageCode) != languageCodeNames.end())
                languageString = languageCodeNames[languageCode];
            else
                languageString = "Desconocido";
        }
        DrawText(languageString.c_str(), 20, 310, 48, BROWN);

        if (timer_end != 0) {
            char result[20];
            sprintf(result, "%.4f", timer_end - timer_start);
            DrawText("Resultado:", 20, 280, 24, BROWN);
            DrawText("Tiempo necesario (ms):", 20, 365, 24, BROWN);
            DrawText(result, 20, 400, 36, BROWN);
        }

        drawButtons(buttons, globalSettings);

        EndDrawing();
    }

    CloseWindow();

    return 0;
}
