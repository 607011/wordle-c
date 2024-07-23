/*
 * Copyright (c) 2024 Oliver Lau <ola@ct.de>, Heise Medien GmbH & Co. KG
 */

#include <ctype.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define WORD_LENGTH 5
#define WORD_BUF_LEN (WORD_LENGTH + 1)
#define MAX_TRIES 6
#define TRUE 1
#define FALSE 0

// Bei der Auswertung des geratenen Wortes wird jeder Buchstabe markiert.
// NOT_PRESENT steht für einen Buchstaben, der nicht im gesuchten Wort
// vorkommt, PRESENT für einen Buchstaben, der vorkommt, aber fehl-
// platziert ist, CORRECT für einen richtig platzierten Buchstaben.
enum status
{
    UNMARKED,
    NOT_PRESENT,
    PRESENT,
    CORRECT
};

typedef enum status state_t;

// Diese Struktur hält den aktuellen Zustand der Raterunde fest
struct game_state
{
    // Zeiger auf das gesuchte Wort in der Wortliste
    char *word;
    // das aktuell geratene Wort
    char guess[WORD_BUF_LEN];
    // Markierungen für die Richtigkeit der geratenen Buchstaben
    state_t result[WORD_LENGTH];
};

// Ein Zeiger auf die Liste der geladenen Wörter; wird von
// fill_wordlist_from_file() befüllt.
char *words = NULL;

// Anzahl der Wörter, auf die `words` zeigt.
size_t num_words_read = 0;

// Prüft, ob das übergebene Wort in der geladenen Wortliste
// enthalten ist
_Bool word_is_allowed(const char *word)
{
    for (size_t i = 0; i < num_words_read; ++i)
    {
        if (strncmp(word, words + i * WORD_BUF_LEN, WORD_LENGTH) == 0)
        {
            return TRUE;
        }
    }
    return FALSE;
}

// eine 0-terminierte Zeichenfolge in Kleinbuchstaben wandeln
char *strtolower(char *s)
{
    for (char *p = s; *p != '\0'; p++)
    {
        *p = tolower(*p);
    }
    return s;
}

// Textdatei mit den Wörtern lesen und den Beginn auf die Liste
// der globalen Variable `words` zuweisen.
int fill_wordlist_from_file(const char *filename)
{
    FILE *fp;
    char *line = NULL;
    size_t len = 0;
    ssize_t nread;

    fp = fopen(filename, "r");
    if (fp == NULL)
    {
        perror("fopen() failed");
        return EXIT_FAILURE;
    }

    while ((nread = getline(&line, &len, fp)) != -1)
    {
        if (nread != WORD_BUF_LEN)
        {
            fprintf(stderr, "WARNING: line %3zu has %zu characters (expected %d)\n",
                    num_words_read + 1, nread - 1, WORD_LENGTH);
            continue;
        }
        words = realloc(words, sizeof(char *) * num_words_read * WORD_BUF_LEN);
        if (words == NULL)
        {
            perror("realloc() failed");
            fclose(fp);
            if (line != NULL)
                free(line);
            return EXIT_FAILURE;
        }
        strlcpy(words + num_words_read * WORD_BUF_LEN, line, WORD_BUF_LEN);
        ++num_words_read;
    }

    if (ferror(fp))
    {
        perror("fread() failed");
        fclose(fp);
        if (line != NULL)
            free(line);
        free(words);
        return EXIT_FAILURE;
    }

    // Eingabedatei schließen
    fclose(fp);
    return EXIT_SUCCESS;
}

// Geht alle Markierungen durch und gibt TRUE zurück,
// wenn das gesuchte Zeichen bereits als vorhanden
// markiert wurde
_Bool is_character_marked(struct game_state *state, char c)
{
    for (int i = 0; i < WORD_LENGTH; ++i)
    {
        if (state->guess[i] == c && state->result[i] != UNMARKED)
            return TRUE;
    }
    return FALSE;
}

// den Zustand der laufenden Raterunde aktualisieren
void update_state(struct game_state *state)
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

void get_input(struct game_state *state, int trial)
{
    // solange eine Eingabe anfordern, bis sie gültig ist
    while (TRUE)
    {
        printf("\n%d. Versuch:"
               "\n? ",
               trial);
        // Eingabe lesen
        if (fgets(state->guess, WORD_BUF_LEN, stdin) == NULL)
            return;
        // überflüssige Zeichen verwerfen
        fflush(stdin);
        // Line feed am Ende der Eingabe durch String-Ende-Zeichen ersetzen
        // TODO: damit rutschen bei der folgenden Prüfung Wörter aus vier Zeichen durch
        state->guess[WORD_LENGTH] = '\0';
        // Prüfen auf korrekte Wortlänge
        if (strnlen(state->guess, WORD_LENGTH) != WORD_LENGTH)
        {
            printf("Bitte gib ein Wort mit %d Buchstaben ein!\n", WORD_LENGTH);
            continue;
        }
        // Prüfen, ob das geratene Wort in der Liste erlaubter Wörter enthalten ist
        if (word_is_allowed(state->guess))
            break;
        printf("Das Wort ist nicht in der Liste erlaubter Wörter.\n");
    }
    printf("! ");
}

void print_result(const struct game_state *state)
{
    // Das Ergebnis ein bisschen hübsch aufbereiten
    for (int i = 0; i < WORD_LENGTH; ++i)
    {
        char c = state->guess[i];
        switch (state->result[i])
        {
        case CORRECT:
            // korrekt platzierte Buchstaben erscheinen Weiß auf grünem Hintergrund
            printf("\033[37;42;1m%c", c);
            break;
        case PRESENT:
            // vorhandene, aber fehlplatzierte Buchstaben erscheinen Weiß auf gelbem Hintergrund
            printf("\033[37;43;1m%c", c);
            break;
        case NOT_PRESENT:
            // fall-through
        default:
            // nicht vorhandene Buchstaben erscheinen Weiß auf rotem Hintergrund
            printf("\033[37;41;1m%c", c);
            break;
        }
    }
    printf("\033[0m\n");
}

// das eigentliche Spiel beginnt hier
void play(void)
{
#ifndef DEBUG
    // Zufallszahlengenerators mit der aktuellen
    // Unix-Time initialisieren
    srand(time(NULL));
#else
    srand(1);
#endif
    printf("\nNERD WORD   ¯\\_(ツ)_/¯\n\n"
           "Errate das Wort mit %d Buchstaben in maximal %d Versuchen.\n"
           "(Abbrechen mit Ctrl+C)\n",
           WORD_LENGTH, MAX_TRIES);
    _Bool finished = FALSE;
    while (!finished)
    {
        struct game_state state;
        // ein Wort zufällig auswählen
        state.word = words + WORD_BUF_LEN * (rand() % num_words_read);

        // eine Raterunde läuft über maximal 6 Versuche
        for (int trial = 1; trial <= MAX_TRIES; ++trial)
        {
            // User raten lassen
            get_input(&state, trial);
            // geratenes Wort auswerten
            update_state(&state);
            // Feedback geben
            print_result(&state);

            // geratenes Wort mit dem gesuchten vergleichen, wenn identisch,
            // hat der User das Spiel gewonnen
            if (strncmp(state.guess, state.word, WORD_LENGTH) == 0)
            {
                printf("\nHurra, du hast das Wort im %d. Versuch gefunden!\n"
                       "Noch eine Runde? (J/n) ",
                       trial);
                char answer = getchar();
                finished = !(answer == 'j' || answer == '\n');
                break;
            }
        }
    }
}

// Einstieg ins Programm
int main(int argc, char *argv[])
{
    // Dateiname für die Wortliste aus den Kommandozeilenargument lesen,
    // wenn nicht vorhanden, die Vorgabe "words.txt" wählen.
    const char *words_filename = argc > 1
                                     ? argv[1]
                                     : "words.txt";
    // Wortliste lesen
    int rc = fill_wordlist_from_file(words_filename);
    if (rc != EXIT_SUCCESS)
        return rc;
    // die erste Raterunde und ggf. weitere starten
    play();
    // den für die Wortliste belegten Speicher freigeben
    free(words);
    // Danke, es war schön mit dir ;-)
    return EXIT_SUCCESS;
}
