#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <stdint.h>
#include <assert.h>
#include "dance.h"

void shuffle(int deck[]) {
    for (int i = 0; i < 13; ++i) {
        int j = i + rand()%(13-i);
        int temp = deck[i];
        deck[i] = deck[j];
        deck[j] = temp;
    }
}

int main(int argc, char *argv[]) {
    if (argc != 6) {
        fprintf(stderr, "Args: spades hearts diamonds clubs deals\n");
        exit(1);
    }
    int needed = atoi(argv[5]);
    int suits[4];
    for (int k = 0; k < 4; k++) {
        suits[k] = atoi(argv[k+1]);
    }
    srand(time(NULL));
    FILE *fout = fopen("maverick.csv", "w");

    int decks[4][13];
    for (int i = 0; i < 4; i++) 
    for (int j = 0; j < 13; j++ ) 
        decks[i][j] = j+1;

    if (suits[0]+suits[1]+suits[2]+suits[3]!= 25) {
        fprintf(stderr, "Suit counts must sum to 25\n");
        exit(1);
    }

    int answer[25];

    int found = 0;
    while (found < needed) {
        for (int i = 0; i < 4; i++)
            shuffle(decks[i]);
        // A liitle bit-fiddling here to get the deal
        // into the format the solver expects
        uint16_t spades, hearts, diamonds, clubs;
        spades = 0;
        hearts = 0;
        diamonds = 0;
        clubs = 0;
        for (int j = 0; j < suits[0]; j++)
            spades |= 1 << decks[0][j];
        for (int j = 0; j < suits[1]; j++)
            hearts |= 1 << decks[1][j];
        for (int j = 0; j < suits[2]; j++)
            diamonds |= 1 << decks[2][j];
        for (int j = 0; j < suits[3]; j++)
            clubs |= 1 << decks[3][j];

        int value = solver(spades, hearts, diamonds, clubs, answer);
        if (value) {
            found++;
            for (int j = 0; j < 13; j++) {
                fprintf(fout, "%u ", clubs & 1);
                clubs >>= 1;
            }
            for (int j = 0; j < 13; j++) {
                fprintf(fout, "%u ", diamonds & 1);
                diamonds >>= 1;
            }
            for (int j = 0; j < 13; j++) {
                fprintf(fout, "%u ", hearts & 1);
                hearts >>= 1;
            }
            for (int j = 0; j < 13; j++) {
                fprintf(fout, "%u ", spades & 1);
                spades >>= 1;
            }
            fprintf(fout, ",");
            for (int i = 0; i < 25; i++) 
                fprintf(fout, "%d ", answer[i]);
            fprintf(fout, "\n");
        }
    }
}

int isPat(int hand[]) {
    // Cards are numbered from 1 to 52, but 0-based numbering is more 
    // convenient here.

    for (int k =  0; k < 5; k++)
        hand[k] -= 1;
    int suits[5];
    int ranks[5];
    for (int k = 0; k < 5; k++) {
        suits[k] = hand[k]/13;
        ranks[k] = hand[k]%13;
    } 
    int flush = 1;
    for (int j = 1; j < 5; ++j) {
        if (suits[j] != suits[0]) {
            flush = 0;
            break;
        }
    }
    if (flush) return 1;
    // insertion sort ranks
    int i, temp, j;
    for (i = 1; i < 5; i++) {
        j = i-1;
        temp = ranks[i];
        while (j >= 0 && ranks[j] > temp) {
            ranks[j+1] = ranks[j];
            j -= 1;
        }
        ranks[j+1] = temp;
    }

    //check for full house
    if ((ranks[0]==ranks[1]) && (ranks[2]==ranks[4]))
        return 1;
    if ((ranks[0]==ranks[2]) && (ranks[3]==ranks[4]))
        return 1;

    // check for straight
    if ((ranks[1]=ranks[0]+1) &&
        (ranks[2]=ranks[1]+1) && 
        (ranks[3]=ranks[2]+1) &&
        (ranks[4]=ranks[3]+1))  return 1;

    // check for high straight
    return ((ranks[0] == 0) && 
            (ranks[1] == 9) &&
            (ranks[2] == 10) &&
            (ranks[3] == 11) &&
            (ranks[4] == 12) );
}

int score(int problem[], int solution[]) {
    // Score a proposed solution to the probelm
    // Problem is array of 52 booleans
    // Solution is array of 25 ints

    int cards = 0;     // number of cards from the problem in the solution
    int goodPats = 0;  // number of pats hands in the solution containing only cards from the problem
    int badPats = 0;   // number of other pat hands in the solution
   
    int cardScore[26];  // scoring points
    int goodPatScore[6] = {0, 15, 30, 45, 60, 75};
    int badPatScore[6] = {0, 0, 0, 0, 0, 0};
    for (int i = 0; i <= 25; i++)
        cardScore[i] = i;
    for (int hand = 0; hand < 5; hand++) {
        int good = 1;
        for (int j = 5*hand; j < 5*hand+5; j++) {
            if (problem[solution[j]]) 
                cards++;
            else good = 0;
            problem[solution[j]] = 0;  // can't use same card twice
        }
        if (isPat(&solution[5*hand])) {
            if (good)
                goodPats++;
            else
                badPats++;
        } 
    }
    return cardScore[cards] + goodPatScore[goodPats] + badPatScore[badPats];    
}