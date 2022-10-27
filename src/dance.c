#include <stdio.h>
#include <stdlib.h>
#include <string.h>  // for memset
#include <assert.h>
#include "dance.h"

//int maxPats;  // for testing only
int maxb = 0;
int maxl = 0;

typedef struct {  // for testing only
    int flushes;
    int straights;
    int fullHouses;
} Value;

enum {
    CLUBS,
    DIAMONDS,
    HEARTS,
    SPADES
};

Node *currentNode = nodes;
int cards[16][5]; // no rank 0; room for high Ace, rank and suit totals
int inverted[53];    // inverted list of the column headers  
#define SUIT_TOTAL 15
#define RANK_TOTAL 4

void print_row(Node *p) {
    //for debugging
    Node *q = p;
    do {
        fprintf(stderr, " %d", q->col->name);
        q = q->right;
    } while (q != p);
    fprintf(stderr,"\n");   
}

void print_columns() {
    //for debugging
    Column *c = root.next;
    while (c != &root) {
        fprintf(stderr,"%d (%d) ", c->name, c->len);
        c = c->next;
    }
    fprintf(stderr, "\n\n");
 }

 void store_solution(int *answer) {
    for (int k = 0; k < 5; k++) {
        Node *p = choice[k];
        for (int j = 0; j < 5; j++) {
            *answer = p->col->name;
            p  = p->right;
            answer++;
        } 
    }
 }

 int bitcount(Hand n) {
    int count = 0;
    while (n) {
        n &= (n-1);
        count += 1;
    }
    return count;
 }

//  int isFlush(Node *n, char seek) {
//     for (int k = 0; k < 5; k++) {
//         if (n->col->name[1]  != seek)
//             return 0;
//         n = n->right;
//     }
//     return 1;
//  }

void cover(Column *c) {
    Column *l, *r;
    Node *rr, *nn, *uu, *dd;
    l = c->prev;
    r = c->next;
    l->next = r;
    r->prev = l;
    for (rr = c->head.down; rr != &(c->head); rr = rr->down) 
    for (nn = rr->right; nn != rr; nn = nn->right) {
        uu = nn->up;
        dd = nn->down;
        uu->down = dd;
        dd->up = uu;
        nn->col->len--;
    }
}

void uncover(Column *c) {
    Column *l, *r;
    Node *rr, *nn, *uu, *dd;
    for (rr = c->head.up; rr != &(c->head); rr = rr->up) 
    for (nn = rr->left; nn != rr; nn = nn->left) {
        uu = nn->up;
        dd = nn->down;
        uu->down = dd->up = nn;
        nn->col->len++;
    }
    l = c->prev;
    r = c->next;
    l->next = r->prev = c;
}

int pats;

void addHand(int hand[]) {

    Node *theNode = currentNode;
    Column *currentColumn;
    
    pats += 1;
    if (pats > MAXPATS) {
        fprintf(stderr,"MAXPATS exceeded; recompile\n");
        exit(1);
    } 

    for (int k = 0; k < 5; k++) {
        if (k != 0) 
            theNode->left = theNode - 1;
        if (k != 4)
            theNode->right = theNode + 1;
        currentColumn = columns + inverted[hand[k]];
        theNode->col = currentColumn;
        theNode->up = currentColumn->head.up;
        currentColumn->head.up->down = theNode;
        currentColumn->head.up = theNode;
        theNode->down = &currentColumn->head;
        currentColumn->len++;
        theNode++;
    }
    // To circularly link the row, we have to adjust the leftmost left
    // and the rightmost right pointers.
    theNode -= 1;
    theNode->right = currentNode;
    currentNode->left = theNode;
    theNode = currentNode;
    for (int k = 0; k < 5; k++) 
        theNode = theNode->right;
    currentNode += 5;
}

int findFlushes() {
    // find all flushes
    int flushes = 0;
    for (int s = CLUBS; s <= SPADES; ++s) {
        int length = cards[SUIT_TOTAL][s];
        if (length < 5)
            continue;
        int suit[length];
        int k = 0;
        for (int r = ACE; r <= KING; r++) {
            if (cards[r][s]) {
                suit[k] = cards[r][s];
                k++;
            }
        }

        int hand[5];
        for (int i0 = 0; i0 < length - 4; i0++) {
            hand[0] = suit[i0];        
            for (int i1 = i0 + 1; i1 < length - 3; i1++) {
                hand[1] = suit[i1];
                for (int i2 = i1 + 1; i2 < length - 2; i2++) {
                    hand[2] = suit[i2];
                    for (int i3 = i2 + 1; i3 < length - 1; i3++) {
                        hand[3] = suit[i3];
                        for (int i4 = i3 + 1; i4 < length; i4++) {
                            hand[4] = suit[i4];
                            addHand(hand);
                            flushes++;
                        }
                    }
                }
            }
        }
    }
    return flushes;
}

int findFullHouses() {
    // find all full houses

    // Bring all cards to the front of the row
    for (int r = ACE; r <= KING; r++) {
        if (cards[r][RANK_TOTAL]==0) continue;
        for (int k = 2; k >=0; k--) {
            if (cards[r][k]==0 && cards[r][k+1] != 0) {
                for (int j = k; j < 3; j++)
                    cards[r][j] = cards[r][j+1];
            }
        }
        // Don't have to worry about duplicates left behind,
        // because cards[RANK_TOTAL] tells us how many cards 
        // of rank r there are in the deal. 
    }


    int f = 0; // number of fours of a kind
    int t = 0; // number of trips
    int p = 0; // number of pairs
    int fulls = 0;
    for (int k = ACE; k <= KING; ++k) {
        if (cards[k][RANK_TOTAL] == 4)
            f++;
        else if (cards[k][RANK_TOTAL] == 3)
            t++;
        else if (cards[k][RANK_TOTAL] == 2)
            p++;
    }
    int T = 4*f + t;
    int P = 6*f + 3*t + p;
    int trips[T][3];
    int pairs[P][2];

    memset(trips, 0, sizeof(trips));
    memset(pairs, 0, sizeof(pairs));

    t = 0;
    p = 0;
    for (int r = ACE; r <= KING; r++) {
        int length = cards[r][RANK_TOTAL];
        if (length == 4) {
            for (int j0 = 0; j0<2; j0++)
            for (int j1 = j0+1; j1 < 3; j1++)
            for( int j2 = j1+1; j2 < 4; j2++) {
                trips[t][0] = cards[r][j0];
                trips[t][1] = cards[r][j1];
                trips[t][2] = cards[r][j2];
                t++;
            }
            for (int j0 = 0; j0<3; j0++)
            for (int j1 = j0+1; j1 < 4; j1++) {
                pairs[p][0] = cards[r][j0];
                pairs[p][1] = cards[r][j1];
                p++;
            }
        }
        else if (length == 3) {
            for (int j = 0; j < 3; j++) 
                trips[t][j] = cards[r][j];
            t++;
            for (int j0 = 0; j0 < 2; j0++)
            for (int j1 = j0+1; j1 < 3; j1++) {
                pairs[p][0] = cards[r][j0];
                pairs[p][1] = cards[r][j1];
                p++;
            }                
        }
        else if (length == 2) {
            for (int j = 0; j < 2; j++) 
                pairs[p][j] = cards[r][j];
            p++;
        }
    }
    //assert(t==T && p==P);
    
    for (int t = 0; t < T; t++) {
        int hand[5];
        hand[0] =trips[t][0];
        hand[1] =trips[t][1];
        hand[2] =trips[t][2];
        for (int p = 0; p < P; p++) {
            if ((trips[t][0] - pairs[p][0]) % 13 == 0)
                continue;  // trips and pair can't have same rank
            hand[3] = pairs[p][0];
            hand[4] = pairs[p][1];
            addHand(hand);
            fulls++;
        }
    }
    return fulls;
}

int findStraights() {
    // Find the straights.  First, add a high Ace,
    // if there is an Ace in the suit
    int straights = 0;
    for (int s = CLUBS; s <= SPADES; s++)
        if (cards[ACE][s]) 
        cards[14][s] = cards[ACE][s];
    cards[14][RANK_TOTAL] = cards[ACE][RANK_TOTAL];

    #define none(r) (cards[(r)][RANK_TOTAL]==0)
    for (int low = ACE; low <= 10; low++) {
        if (none(low) || none(low+1) || none(low+2) || none(low+3) || none(low+4))
            continue;
        int hand[5];
        int i[5];
        for (i[0]=0; i[0]<cards[low][RANK_TOTAL]; i[0]++) {
            hand[0] = cards[low][i[0]];
            for (i[1]=0; i[1]<cards[low+1][RANK_TOTAL]; i[1]++) { 
                hand[1] = cards[low+1][i[1]]; 
                for (i[2]=0; i[2]<cards[low+2][RANK_TOTAL]; i[2]++) { 
                    hand[2] = cards[low+2][i[2]];
                    for (i[3]=0; i[3]<cards[low+3][RANK_TOTAL]; i[3]++) {
                        hand[3] = cards[low+3][i[3]];
                        for (i[4]=0; i[4]<cards[low+4][RANK_TOTAL]; i[4]++) {
                            hand[4] = cards[low+4][i[4]]; 
                            addHand(hand);
                            straights++;
                        }
                    }
                }
            }
        }
     }
    return straights;
}      

// void pip(int card, char buf[]) {
//     buf[0] = rankCode[(card-1)%13 +1];
//     buf[1] = suitCode[(card-1)/13];
//     buf[2] = '\0';
// }

int solver(Hand spades, Hand hearts,
            Hand diamonds, Hand clubs, int answer[]) {

    Hand bitsets[4];
    bitsets[0] = clubs;
    bitsets[1] = diamonds;
    bitsets[2] = hearts;
    bitsets[3] = spades;
    currentNode = nodes; 

    #define RANK_TOTAL 4

    memset(cards, 0, sizeof(cards));

    for (int s = CLUBS; s <= SPADES; s++) 
    for (int r = ACE; r <= KING; r++) {
        int mask = (1 << (r-1));
        if (mask & bitsets[s]) {
            cards[r][s] = 13*s + r;
            cards[r][RANK_TOTAL] += 1;
            cards[SUIT_TOTAL][s] += 1;
        }
    }
    
    // Make the column headers and an inverted list of them 
  
    Column *currentColumn = columns + 1;
    Node *currentNode = nodes;
    for (int s = CLUBS; s <= SPADES; s++) 
    for (int r = ACE; r <= KING; r++){
        if (cards[r][s] == 0)
            continue;   
        currentColumn->head.up = currentColumn->head.down = &currentColumn->head;
        currentColumn->len = 0;
        currentColumn->prev = currentColumn - 1;
        currentColumn->name = cards[r][s];
        inverted[cards[r][s]] = currentColumn - columns;
        (currentColumn - 1)->next = currentColumn;
        currentColumn++;
    }
    (currentColumn - 1)->next = &root;
    root.prev = currentColumn - 1;

    pats = 0;
    findFlushes();
    findFullHouses();
    findStraights();

    // value->flushes = flushes;
    // value->straights = straights;
    // value->fullHouses = fulls;
    //int tmp = flushes + straights + fulls;
    //if (tmp > maxPats) maxPats = tmp;

    // Matrix constructed.  Find a solution, if one exists.

    int level = 0;
    Column *bestColumn = columns+1; // best column for branching has minimum length
    int minLength;

forward:

    minLength = 100000; // infinity
    for (currentColumn = root.next; currentColumn != &root; currentColumn = currentColumn->next) {
        if (currentColumn->len < minLength) {
            bestColumn = currentColumn;
            minLength = currentColumn->len;
        }
    }
  
    cover(bestColumn);
    //print_columns();
    currentNode = choice[level] = bestColumn->head.down;

advance:

    if (currentNode == &(bestColumn->head))
        goto backup;
    //print_row(choice[level]);
    for (Node* pp = currentNode->right; pp != currentNode; pp = pp->right)
        cover(pp->col);
    //print_columns();

    if (root.next == &root) { // all columns covered
        store_solution(answer);
        return 1;
    }
    level++;
    goto forward;

backup:
    uncover(bestColumn);
    if (level == 0) 
        return 0;  // we'd have short-circuited had there been a solution
    level--;
    currentNode = choice[level];
    bestColumn = currentNode->col;

    // Uncover all other columns of currentNode
    // We included left links, thereby making the rows doubly linked, so
    // that columns would be uncovered in the correct LIFO order in this
    // part of the program. (The uncover routine itself could have done its
    // job with right links only.) (Think about it.)

    for (Node* pp = currentNode->left; pp != currentNode; pp = pp->left) 
         uncover(pp->col);

    currentNode = choice[level] = currentNode->down;
    goto advance;
}