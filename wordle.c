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
    // Markierung ob ein Buchstabe aus dem Wort bereits benutzt wurde
    bool used[WORD_LENGTH];
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
    for (int i = 0; words[i] != NULL; ++i)
    {
        if (strncmp(word, words[i], WORD_LENGTH) == 0)
            return true;
    }
    return false;
}

// Das gesuchte Wort nach einem Zeichen des geratenen Wortes durchsuchen,
// wenn das gesuchte Zeichen noch nicht als benutzt markiert wurde dieses
// markieren und true zurueckgeben
bool is_character_unmarked(game_state* state, char c)
{
    for (int i = 0; i < WORD_LENGTH; ++i)
    {
        if (state->word[i] == c && state->used[i] == false) {
            state->used[i] = true;
            return true;
        }
    }
    return false;
}

// den Zustand der laufenden Raterunde aktualisieren
void update_state(game_state *state)
{
    // Jedes Zeichen als unmarkiert kennzeichnen
    for (int i = 0; i < WORD_LENGTH; ++i) {
        state->result[i] = UNMARKED;
        state->used[i] = false;
    }

    // korrekt platzierte Zeichen finden und als solche markieren
    for (int i = 0; i < WORD_LENGTH; ++i)
    {
        if (state->guess[i] == state->word[i]) {
            state->result[i] = CORRECT;
            state->used[i] = true;
        }
    }

    // noch mal alle Zeichen durchgehen und die als
    // vorhanden, aber fehlplatziert markieren, die nicht
    // bereits markiert sind
    for (int i = 0; i < WORD_LENGTH; ++i)
    {
        // Zeichen ist als korrekt markiert, also überspringen
        if (state->result[i] == CORRECT)
            continue;
        state->result[i] = is_character_unmarked(state, state->guess[i])
            ? PRESENT : NOT_PRESENT;
    }
}

void get_input(game_state* state)
{
    // solange eine Eingabe anfordern, bis sie gültig ist
    bool bad_word;
    do
    {
        printf("\n%d. Versuch: ", state->n_tries);
        // Eingabe lesen
        bad_word = 0;
        for (int i = 0; i < WORD_LENGTH; i++) {
            state->guess[i] = getchar();
            if (state->guess[i] == '\n') {
                state->guess[i] = '\0';
                bad_word = 1;
                break;
            }
        }
        // überflüssige Zeichen verwerfen
        if (!bad_word)
            while (getchar() != '\n')
                ;
        // nach dem 5. Zeichen abschneiden
        state->guess[WORD_LENGTH] = '\0';
#ifdef DEBUG
        printf("Eingabe: '%s'\n", state->guess);
#endif
        if (bad_word) {
            printf("Bitte %d Buchstaben eingeben.\n", WORD_LENGTH);
	} else {
	    bad_word = !word_is_allowed(state->guess);
	    if (bad_word)
		printf("Das Wort ist nicht in der Liste erlaubter Wörter.\n");
	}

    }
    while (bad_word);
}

// dem User Feedback geben, wie gut sein Rateversuch war
void print_result(const game_state* state)
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
            printf("\x1b[37;43;1m");
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
    // überflüssige Zeichen verwerfen
    if (answer != '\n')
        while (getchar() != '\n')
            ;
    bool yes = answer == 'j' || answer == '\n';
    if (yes)
        printf("\nSuper! Dann mal los ...\n");
    return yes;
}

// Einstieg ins Programm
int main(int argc, char* argv[])
{
    unsigned int seed = (argc > 1)
        ? (unsigned int)atoi(argv[1])
        : (unsigned int)time(NULL);
    srand(seed);
    printf("\nNERD WORD\n\n"
           "Errate das Wort mit %d Buchstaben in maximal %d Versuchen.\n"
           "(Abbrechen mit Strg+C bzw. Ctrl+C)\n",
           WORD_LENGTH, MAX_TRIES);

    bool keepRunning = true;
    while (keepRunning)
    {
        game_state state;
        int num_words;
        // worte zaehlen und ein Wort zufällig auswählen
        for (num_words = 0; words[num_words];  num_words++);
        state.word = words[rand() % num_words];
#ifdef DEBUG
        state.word = "cebit";
        printf("Hint: %s\n", state.word);
#endif
        // eine Raterunde läuft über maximal 6 (`MAX_TRIES`) Versuche
        bool doRestart = false;
        for (state.n_tries = 1;
             state.n_tries <= MAX_TRIES && !doRestart;
             ++state.n_tries)
        {
            // User raten lassen
            get_input(&state);
            // geratenes Wort auswerten
            update_state(&state);
            // Feedback geben
            print_result(&state);

            // geratenes Wort mit dem gesuchten vergleichen, wenn identisch,
            // hat der User das Spiel gewonnen
            if (strncmp(state.guess, state.word, WORD_LENGTH) == 0)
            {
                printf("\nHurra, Du hast das Wort im %d. Versuch gefunden!\n",
                       state.n_tries);
                doRestart = true;
                keepRunning = another_round();
            }
            else if (state.n_tries == MAX_TRIES)
            {
                printf("\nSchade, Du hast das Wort nicht erraten.\n"
                       "Es lautete: %s.\n",
                       state.word);
                keepRunning = another_round();
            }
        }
    }
    printf("\nDanke, es war schön mit dir :-)\n\n");
    return EXIT_SUCCESS;
}
