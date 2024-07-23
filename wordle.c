/*
 * Copyright (c) 2024 Oliver Lau <ola@ct.de>, Heise Medien GmbH & Co. KG
 */

#if _MSC_VER
#include <Windows.h>
#endif

#include <ctype.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <time.h>

#include "words.h"

#define WORD_LENGTH 5
#define WORD_BUF_LEN (WORD_LENGTH + 1)
#define MAX_TRIES 6
#define TRUE 1
#define FALSE 0

// Bei der Auswertung des geratenen Wortes wird jeder Buchstabe markiert.
// `NOT_PRESENT` steht für einen Buchstaben, der nicht im gesuchten Wort
// vorkommt, `PRESENT` für einen Buchstaben, der vorkommt, aber fehl-
// platziert ist, `CORRECT` für einen richtig platzierten Buchstaben.
enum status
{
    UNMARKED,
    NOT_PRESENT,
    PRESENT,
    CORRECT
};

typedef enum status state_t;

// Diese Struktur hält den aktuellen Zustand der Raterunde fest
typedef struct
{
    // Zeiger auf das gesuchte Wort in der Wortliste
    const char *word;
    // das aktuell geratene Wort
    char guess[WORD_BUF_LEN];
    // Markierungen für die Richtigkeit der geratenen Buchstaben
    state_t result[WORD_LENGTH];
} game_state;

// Prüft, ob das übergebene Wort in der geladenen Wortliste
// enthalten ist
_Bool word_is_allowed(const char *word)
{
    for (size_t i = 0; i < NUM_WORDS; ++i)
    {
        if (strncmp(word, words[i], WORD_LENGTH) == 0)
            return TRUE;
    }
    return FALSE;
}

// eine 0-terminierte Zeichenfolge in Kleinbuchstaben wandeln
char *strtolower(char *s)
{
    for (char *p = s; *p != '\0'; p++)
    {
        *p = (char)tolower(*p);
    }
    return s;
}

inline void safe_free(void *p)
{
    if (p != NULL)
        free(p);
}


// Geht alle Markierungen durch und gibt TRUE zurück,
// wenn das gesuchte Zeichen bereits als vorhanden
// markiert wurde
_Bool is_character_marked(game_state *state, char c)
{
    for (int i = 0; i < WORD_LENGTH; ++i)
    {
        if (state->guess[i] == c && state->result[i] != UNMARKED)
            return TRUE;
    }
    return FALSE;
}

// den Zustand der laufenden Raterunde aktualisieren
void update_state(game_state *state)
{
    // Jedes Zeichen als unmarkiert kennzeichnen
    memset(state->result, UNMARKED, sizeof(int) * WORD_LENGTH);

    // korrekt platzierte Zeichen finden und als solche markieren
    for (int i = 0; i < WORD_LENGTH; ++i)
    {
        if (state->guess[i] == state->word[i])
        {
            state->result[i] = CORRECT;
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
        char c = state->guess[i];
        state->result[i] =
            (strchr(state->word, c) != NULL &&
             !is_character_marked(state, c))
                ? PRESENT
                : NOT_PRESENT;
    }
}

void get_input(game_state *state, int trial)
{
    // solange eine Eingabe anfordern, bis sie gültig ist
    _Bool bad_word = TRUE;
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
        // überflüssige Zeichen verwerfen
        int ch;
        while ((ch = getchar()) != EOF && ch != '\n')
            /* */;
#else        
        if (fgets(state->guess, WORD_BUF_LEN, stdin) == NULL)
            return;
        while ((ch = getch()) != EOF && ch != '\n')
            /* */;
#endif
        // nach dem 5. Zeichen abschneiden
        state->guess[WORD_LENGTH] = '\0';
        // Prüfen, ob das geratene Wort in der Liste erlaubter Wörter enthalten ist
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
        char c = state->guess[i];
        switch (state->result[i])
        {
        case CORRECT:
            // korrekt platzierte Buchstaben erscheinen auf grünem Hintergrund
            printf("\033[37;42;1m%c", c);
            break;
        case PRESENT:
            // vorhandene, aber fehlplatzierte Buchstaben erscheinen auf gelbem Hintergrund
            printf("\033[37;43;1m%c", c);
            break;
        case NOT_PRESENT:
            // fall-through
        default:
            // nicht vorhandene Buchstaben erscheinen auf rotem Hintergrund
            printf("\033[37;41;1m%c", c);
            break;
        }
    }
    // Schrift- und Hintergrundfarbe auf Defaults zurücksetzen
    printf("\033[0m\n");
}

_Bool another_round(void)
{
    printf("Noch eine Runde? (J/n) ");
    char answer = (char)tolower(getchar());
    return answer == 'j' || answer == '\n';
}

// das eigentliche Spiel beginnt hier
void play(void)
{
#ifndef DEBUG
    // Zufallszahlengenerators mit der aktuellen
    // Unix-Time initialisieren
    srand((unsigned int)time(NULL));
#else
    srand(1);
#endif
    printf("\nNERD WORD   ¯\\_(ツ)_/¯\n\n"
           "Errate das Wort mit %d Buchstaben in maximal %d Versuchen.\n"
           "(Abbrechen mit Strg+C bzw. Ctrl+C)\n",
           WORD_LENGTH, MAX_TRIES);

    _Bool finished = FALSE;
    while (!finished)
    {
        game_state state;
        // ein Wort zufällig auswählen
        state.word = words[rand() % NUM_WORDS];
#ifdef DEBUG
        printf("Hint: '%s'", state.word);
#endif
        // eine Raterunde läuft über maximal 6 Versuche
        for (int num_tries = 1; num_tries <= MAX_TRIES && !finished; ++num_tries)
        {
            // User raten lassen
            get_input(&state, num_tries);
            // geratenes Wort auswerten
            update_state(&state);
            // Feedback geben
            print_result(&state);

            // geratenes Wort mit dem gesuchten vergleichen, wenn identisch,
            // hat der User das Spiel gewonnen
            if (strncmp(state.guess, state.word, WORD_LENGTH) == 0)
            {
                printf("\nHurra, du hast das Wort im %d. Versuch gefunden!\n", num_tries);
                finished = !another_round();
                break;
            }
            else if (num_tries == MAX_TRIES)
            {
                printf("\nSchade, du hast das Wort nicht erraten.\n"
                       "Es lautete: %s.\n",
                       state.word);
                finished = !another_round();
                break;
            }
        }
    }
}

// Einstieg ins Programm
int main(void)
{
    // die erste Raterunde und ggf. weitere starten
    play();
    // den für die Wortliste belegten Speicher freigeben
    safe_free((void*)words);
    // Danke, es war schön mit dir ;-)
    printf("\033[0m\n");
    return EXIT_SUCCESS;
}
