/*
 * Copyright (c) 2024 Oliver Lau <ola@ct.de>, Heise Medien GmbH & Co. KG
 */

// Falls das Programm unter Windows laufen soll, die Windows-Header-
// Datei einbinden
#if _MSC_VER
#include <Windows.h>
#endif

// Die folgenden Zeilen laden Header-Dateien, die Zugriff auf Funktionen
// etwa für die Ein- und Ausgabe, zur Zufallszahlenerzeugung oder zur
// Stringverarbeitung bereitstellen oder Typen und Werte für den Umgang
// mit Boole'schen Werten definieren
#include <ctype.h>       // tolower()
#include <stdbool.h>     // bool, true und false
#include <stdio.h>       // stdin, getchar(), fgets()
#include <stdlib.h>      // atoi(), srand(), rand()
#include <string.h>      // strncmp(), strchr()
#include <time.h>        // time()

// Definition zur Wortliste in der Datei words.c einbinden
#include "words.h"

#define WORD_BUF_LEN (WORD_LENGTH + 1)
#define MAX_TRIES (6)

// Bei der Auswertung des geratenen Wortes wird jeder Buchstabe markiert:
// `NOT_PRESENT` steht für einen Buchstaben, der nicht im gesuchten Wort
// vorkommt, `PRESENT` für einen Buchstaben, der vorkommt, aber fehl-
// platziert ist, `CORRECT` für einen richtig platzierten Buchstaben.
enum status
{
    UNMARKED,     // 0
    NOT_PRESENT,  // 1
    PRESENT,      // 2
    CORRECT       // 3
};

typedef enum status state_t;

// Diese Struktur hält den aktuellen Zustand der Raterunde fest
typedef struct
{
    // Zeiger auf das gesuchte Wort in der Wortliste
    const char* word;
    // das aktuell geratene Wort
    char guess[WORD_BUF_LEN];
    // Markierungen für die Richtigkeit der geratenen Buchstaben
    state_t result[WORD_LENGTH];
    // Nummer des Rateversuchs
    int n_tries;
} game_state;

// Prüft, ob das übergebene Wort in der geladenen Wortliste
// enthalten ist
bool word_is_allowed(const char* word)
{
    // Sequenzielle Suche nach dem Wort in der Liste
    // der erlaubten Wörter. Im Kontext dieses Spielchens
    // ist das völlig okay, performanter wäre aber eine
    // binäre Suche ("divide & conquer"), zumal die Wortliste
    // bereits lexikografisch sortiert ist.
    for (int i = 0; i < NUM_WORDS; ++i)
    {
        if (strncmp(word, words[i], WORD_LENGTH) == 0)
            return true;
    }
    return false;
}

// Alle Markierungen durchgehen und gibt TRUE zurückgeben,
// wenn das gesuchte Zeichen bereits als vorhanden
// markiert wurde
bool is_character_marked(game_state* state, char c)
{
    for (int i = 0; i < WORD_LENGTH; ++i)
    {
        if (state->guess[i] == c && state->result[i] != UNMARKED)
            return true;
    }
    return false;
}

// den Zustand der laufenden Raterunde aktualisieren
void update_state(game_state *state)
{
    // Jedes Zeichen als unmarkiert kennzeichnen
    for (int i = 0; i < WORD_LENGTH; ++i)
        state->result[i] = UNMARKED;

    for (int i = 0; i < WORD_LENGTH; ++i)
    {
        char c = state->guess[i];

        // Markieren des Buchstabens als `CORRECT`, wenn er an der
        // richtigen Stelle steht
        if (c == state->word[i])
        {
            state->result[i] = CORRECT;
            continue;
        }

        // Prüfen, ob der Buchstabe im gesuchten Wort vorkommt
        if (strchr(state->word, c) != NULL)
        {
            // Markieren des Buchstabens als `PRESENT`, wenn er im
            // Wort vorkommt, aber nicht an der richtigen Stelle steht
            if (state->result[i] != CORRECT)
            {
                state->result[i] = PRESENT;
            }
        }
        else
        {
            // Markieren des Buchstabens als `NOT_PRESENT`, wenn er
            // nicht im Wort vorkommt
            state->result[i] = NOT_PRESENT;
        }
    }
}

void get_input(game_state *state, int trial)
{
    // solange eine Eingabe anfordern, bis sie gültig ist
    bool bad_word = true;
    while (bad_word)
    {
        printf("\n%d. Versuch:"
               "\n? ",
               trial);
        // Eingabe lesen
#ifdef _MSC_VER
        DWORD nread;
        HANDLE hstdin = GetStdHandle(STD_INPUT_HANDLE);
        if (!ReadConsole(hstdin, state->guess, WORD_BUF_LEN, &nread, NULL))
            return;
#else
        if (fgets(state->guess, WORD_BUF_LEN, stdin) == NULL)
            return;
#endif
        // überflüssige Zeichen verwerfen
        int ch;
        while ((ch = getchar()) != EOF && ch != '\n')
            /* */;
        // nach dem 5. Zeichen abschneiden
        state->guess[WORD_LENGTH] = '\0';
        // Prüfen, ob das geratene Wort in der Liste erlaubter Wörter
        // enthalten ist
        bad_word = !word_is_allowed(state->guess);
        if (bad_word)
        {
            printf("Das Wort ist nicht in der Liste erlaubter Wörter.\n");
        }
    }
}

// dem User Feedback geben, wie gut sein Rateversuch war
void print_result(const game_state *state)
{
    // Das Ergebnis ein bisschen hübsch aufbereiten
    printf("! ");
    for (int i = 0; i < WORD_LENGTH; ++i)
    {
        switch (state->result[i])
        {
        case CORRECT:
            // korrekt platzierte Buchstaben erscheinen auf
            // grünem Hintergrund
            printf("\033[37;42;1m");
            break;
        case PRESENT:
            // vorhandene, aber fehlplatzierte Buchstaben erscheinen auf
            // gelbem Hintergrund
            printf("\033[37;43;1m");
            break;
        case NOT_PRESENT:
            // fall-through
        default:
            // nicht vorhandene Buchstaben erscheinen auf
            // rotem Hintergrund
            printf("\033[37;41;1m");
            break;
        }
        printf("%c", state->guess[i]);
    }
    // Schrift- und Hintergrundfarbe auf Defaults zurücksetzen
    printf("\033[0m\n");
}

bool another_round(void)
{
    printf("Noch eine Runde? (J/n) ");
    char answer = (char)tolower(getchar());
    bool yes = answer == 'j' || answer == '\n';
    if (yes)
        printf("\nSuper! Dann mal los ...\n");
    return yes;
}

// Einstieg ins Programm
int main(void)
{
#ifndef DEBUG
    // Zufallszahlengenerator mit der aktuellen
    // Unix-Time initialisieren
    srand((unsigned int)time(NULL));
#else
    // im Debug-Modus den Pseudozufallszahlengenerator mit einem
    // konstanten Wert initialisieren, damit die Resultate
    // besser nachvollziehbar sind
    srand(0);
#endif
    printf("\nNERD WORD   ¯\\_(ツ)_/¯\n\n"
           "Errate das Wort mit %d Buchstaben in maximal %d Versuchen.\n"
           "(Abbrechen mit Strg+C bzw. Ctrl+C)\n",
           WORD_LENGTH, MAX_TRIES);

    bool finished = false;
    while (!finished)
    {
        game_state state;
        // ein Wort zufällig auswählen
        state.word = words[rand() % NUM_WORDS];
#ifdef DEBUG
        printf("Hint: %s\n", state.word);
#endif
        // eine Raterunde läuft über maximal 6 (`MAX_TRIES`) Versuche
        for (int n_tries = 1; n_tries <= MAX_TRIES && !finished; ++n_tries)
        {
            // User raten lassen
            get_input(&state, n_tries);
            // geratenes Wort auswerten
            update_state(&state);
            // Feedback geben
            print_result(&state);

            // geratenes Wort mit dem gesuchten vergleichen, wenn identisch,
            // hat der User das Spiel gewonnen
            if (strncmp(state.guess, state.word, WORD_LENGTH) == 0)
            {
                printf("\nHurra, du hast das Wort im %d. Versuch gefunden!\n",
                       n_tries);
                finished = !another_round();
            }
            else if (n_tries == MAX_TRIES)
            {
                printf("\nSchade, du hast das Wort nicht erraten.\n"
                       "Es lautete: %s.\n",
                       state.word);
                finished = !another_round();
            }
        }
    }
    printf("\nDanke, es war schön mit dir :-)\n\n");
    return EXIT_SUCCESS;
}
