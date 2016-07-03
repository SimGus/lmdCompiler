#include "pile.h"

void pilePush(Pile* pile, Environment newEnvironmentType)
{
   Node* newNode = malloc(sizeof(Node));
   newNode->environmentType = newEnvironmentType;
   newNode->next = pile->top;
   pile->top = newNode;
}

Environment pilePop(Pile* pile)
{
   Environment answer = pile->top->environmentType;

   Node* poppedTop = pile->top;
   pile->top = pile->top->next;
   free(poppedTop);

   return answer;
}

Environment getTop(const Pile* pile)
{
   if (pile != NULL && pile->top != NULL)
      return pile->top->environmentType;
   return NORMAL;
}

void pileFree(Pile* pile)
{
   Node* currentPoppedNode;
   while (pile->top != NULL)
   {
      currentPoppedNode = pile->top;
      pile->top = pile->top->next;
      free(currentPoppedNode);
   }
}
