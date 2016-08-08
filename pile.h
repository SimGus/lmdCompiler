#ifndef _PILE_H_
#define _PILE_H_

#include <stdlib.h>
#include <stdio.h>

typedef enum
{
   NORMAL, ITALIC, BOLD, UNDERLINE, STRIKETHROUGH, EMPHASIZED, QUOTE, PLAIN_TEXT, TELETYPE, LATEX
} Environment;

typedef struct Node Node;
struct Node
{
   Environment environmentType;
   Node* next;
};

typedef struct Pile Pile;
struct Pile
{
   Node* top;
};

void pilePush(Pile* pile, Environment newEnvironmentType);
Environment pilePop(Pile* pile);

Environment getTop(const Pile* pile);

void pileFree(Pile* pile);

#endif //_PILE_H_
