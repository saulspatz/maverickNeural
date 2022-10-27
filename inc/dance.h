/*  dance.h

* Solve using Knuth's DLX algorithm.  If we consider the pat hands as subsets
* of the deal, then this becomes a set exact cover problem.  In Knuth's terminology,
* the cards in the deal become the columns of a 0-1 matrix, the hands are the rows, 
* and a entry is 1 if and only if the card corresponding to the column occurs in
* the hand crresponding to the row.  Much of the code here, and many of the comments, are
* copied from the dance.w program available on Knuth's website.
* Each column of the input matrix is represented by a column struct,
* and each row is represented as a linked list of node structs. There's one
* node for each nonzero entry in the matrix.

* More precisely, the nodes are linked circularly within each row, in
* both directions. The nodes are also linked circularly within each column;
* the column lists each include a header node, but the row lists do not.
* Column header nodes are part of a column struct, which
* contains further info about the column.

* Each node contains five fields. Four are the pointers of doubly linked lists,
* already mentioned; the fifth points to the column containing the node.
*/

#ifndef DANCE_H
#define DANCE_H
#include <stdint.h>

typedef uint16_t Hand;
struct col_struct;          // Forward declaration needed because Node 
                        // and Column refer to one another. 

typedef struct node_struct {
  struct node_struct *left,*right;  /* predecessor and successor in row */
  struct node_struct *up,*down;     /* predecessor and successor in column */
  struct col_struct *col;           /* the column containing this node */
} Node; 

typedef struct col_struct {
  Node head;                        // the list header 
  int len;                          // the number of non-header items currently in this column's list 
  int name;                         // identification of the column 
  struct col_struct *prev,*next;    // neighbors of this column 
} Column;

// gateway to the unsettled columns 
#define root columns[0] 

#define MAXPATS 5200      

Column columns[26];             // one extra for the root
Node nodes[5*MAXPATS];          // the ones in the matrix
Node *choice[5];                // the hand chosen on each level
// char rankCode[] = " A23456789TJQK";
// char suitCode[] = "CDHS";

#define ACE 1
#define KING 13

int solver(Hand spades, Hand hearts,
            Hand diamonds, Hand clubs, int answer[]);

#endif