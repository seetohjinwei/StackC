#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Python-like assertions. */
int assert(int truth, char *message) {
  if (truth == 0) {
    printf("Assertion Error: %s\n", message);
    exit(1);
  } else {
    return 1;
  }
}

enum OPS {
  OP_UNKNOWN,
  OP_INT,
  OP_ADD,
  OP_SUB,
  OP_MUL,
  OP_DIV,
  OP_REM,
  OP_EQU,
  OP_GTE,
  OP_LTE,
  OP_GT,
  OP_LT,
  OP_PEEK,
  OP_POP,
  OP_EMIT,
  OP_SIZE,
  OP_DUP,
  OP_DROP,
  OP_SWAP,
  OP_OVER,
  OP_ROT,
  OP_CR,
  OPS_COUNT // size of enum OPS
};

/* Doubly Linked List implementation of a queue. */
/* Does not break links between elements when polling. */

struct Queue {
  int size;
  struct QueueElem* head;
  struct QueueElem* tail;
};

struct QueueElem {
  struct Token* token;
  struct QueueElem* prev;
  struct QueueElem* next;
};

/* Linked List implementation of a stack. */

struct Node {
  int value;
  struct Node* next;
};

struct Stack {
  int size;
  struct Node* root;
};

struct Token {
  int pos;
  enum OPS OP_TYPE;
  int value;
};

/* Print Token for debugging. */
void printToken(struct Token* token) {
  printf("-- Token --\n");
  printf("Position: %d\n", token->pos);
  printf("OP_TYPE: %d\n", token->OP_TYPE);
  printf("Value: %d\n", token->value);
}

/* Initialise a new queue. */
struct Queue* newQueue(void) {
  struct Queue *queue;
  queue = (struct Queue*) malloc(sizeof(struct Queue));
  queue->size = 0;
  queue->head = NULL;
  queue->tail = NULL;
  return queue;
}

int isEmptyQueue(struct Queue* queue) {
  return queue->size == 0;
}

/* Push to queue. */
void pushQueue(struct Queue* queue, struct Token* token) {
  struct QueueElem *elem;
  elem = (struct QueueElem*) malloc(sizeof(struct QueueElem));
  elem->token = token;
  elem->next = NULL;
  if (isEmptyQueue(queue)) {
    queue->head = elem;
    queue->tail = elem;
    elem->prev = NULL;
  } else {
    struct QueueElem *prevLast = queue->tail;
    prevLast->next = elem;
    elem->prev = prevLast;
    queue->tail = elem;
  }
  queue->size++;
}

/* Poll head of queue. */
struct QueueElem* pollQueue(struct Queue* queue) {
  assert(!isEmptyQueue(queue), "Polling from empty queue.");
  struct QueueElem *prevHead = queue->head;
  struct QueueElem *newHead = prevHead->next;
  queue->head = newHead;
  /* newHead will maintain the reference to the now polled prev. */
  queue->size--;
  return prevHead;
}

/* Peek head of queue. */
struct Token* peekQueue(struct Queue* queue) {
  assert(!isEmptyQueue(queue), "Peeking empty queue.");
  return queue->head->token;
}

/* Get previous token even when polled. */
struct Token* prevQueueElem(struct QueueElem* currentElem) {
  struct QueueElem *prevElem = currentElem->prev;
  assert(prevElem != NULL, "Previous element should not be null.");
  return prevElem->token;
}

/* Initialise a new stack. */
struct Stack* newStack(void) {
  struct Stack *stack;
  stack = (struct Stack*) malloc(sizeof(struct Stack));
  stack->size = 0;
  stack->root = NULL;
  return stack;
}

/* Checking if a stack is empty. */
int isEmptyStack(struct Stack* stack) {
  return stack->size == 0;
}

/* Push an integer onto a stack. */
int pushStack(struct Stack* stack, int value) {
  struct Node* newNode;
  newNode = (struct Node*) malloc(sizeof(struct Node));
  newNode->value = value;
  newNode->next = stack->root;
  stack->root = newNode;
  stack->size++;
  return 1;
}

/* Peek at the first element of the stack. */
int peekStack(struct Stack* stack) {
  assert(!isEmptyStack(stack), "Stack underflow.\n");
  return stack->root->value;
}

/* Pops the first element of the stack. */
int popStack(struct Stack* stack) {
  assert(!isEmptyStack(stack), "Stack underflow.\n");
  struct Node* poppedNode = stack->root;
  stack->root = poppedNode->next;
  stack->size--;
  return poppedNode->value;
}

/* Checks if a string is a representation of an integer. */
int isNumber(char* word) {
  char current;
  size_t i = 0;
  do {
    // 0 is the EOF character
    current = word[i];
    if (current != 0 && (current < 48 || current > 57)) {
      return 0;
    }
    i++;
  } while (current != '\0');
  return 1;
}

/* Parses a token. */
void parseQueueElem(struct Stack* stack, struct QueueElem* queueElem) {
  assert(OPS_COUNT == 22, "Update control flow in parse().");
  struct Token *token = queueElem->token;
  if (token->OP_TYPE == OP_INT) {
    pushStack(stack, token->value);
  } else if (token->OP_TYPE == OP_ADD) {
    int a = popStack(stack);
    int b = popStack(stack);
    pushStack(stack, b + a);
  } else if (token->OP_TYPE == OP_SUB) {
    int a = popStack(stack);
    int b = popStack(stack);
    pushStack(stack, b - a);
  } else if (token->OP_TYPE == OP_MUL) {
    int a = popStack(stack);
    int b = popStack(stack);
    pushStack(stack, b * a);
  } else if (token->OP_TYPE == OP_DIV) {
    int a = popStack(stack);
    int b = popStack(stack);
    pushStack(stack, b / a);
  } else if (token->OP_TYPE == OP_REM) {
    int a = popStack(stack);
    int b = popStack(stack);
    pushStack(stack, b % a);
  } else if (token->OP_TYPE == OP_EQU) {
    int a = popStack(stack);
    int b = popStack(stack);
    pushStack(stack, a != b ? 0 : 1);
  } else if (token->OP_TYPE == OP_GTE) {
    int a = popStack(stack);
    int b = popStack(stack);
    pushStack(stack, a >= b ? 0 : 1);
  } else if (token->OP_TYPE == OP_LTE) {
    int a = popStack(stack);
    int b = popStack(stack);
    pushStack(stack, a <= b ? 0 : 1);
  } else if (token->OP_TYPE == OP_GT) {
    int a = popStack(stack);
    int b = popStack(stack);
    pushStack(stack, a > b ? 0 : 1);
  } else if (token->OP_TYPE == OP_LT) {
    int a = popStack(stack);
    int b = popStack(stack);
    pushStack(stack, a < b ? 0 : 1);
  } else if (token->OP_TYPE == OP_PEEK) {
    int top = peekStack(stack);
    printf("%d\n", top);
  } else if (token->OP_TYPE == OP_POP) {
    int top = popStack(stack);
    printf("%d\n", top);
  } else if (token->OP_TYPE == OP_EMIT) {
    int top = popStack(stack);
    printf("%c\n", top);
  } else if (token->OP_TYPE == OP_SIZE) {
    int size = stack->size;
    printf("%d\n", size);
  } else if (token->OP_TYPE == OP_DUP) {
    int top = peekStack(stack);
    pushStack(stack, top);
  } else if (token->OP_TYPE == OP_DROP) {
    popStack(stack);
  } else if (token->OP_TYPE == OP_SWAP) {
    int a = popStack(stack);
    int b = popStack(stack);
    pushStack(stack, a);
    pushStack(stack, b);
  } else if (token->OP_TYPE == OP_OVER) {
    int a = popStack(stack);
    int second = peekStack(stack);
    pushStack(stack, a);
    pushStack(stack, second);
  } else if (token->OP_TYPE == OP_ROT) {
    int c = popStack(stack);
    int b = popStack(stack);
    int a = popStack(stack);
    pushStack(stack, b);
    pushStack(stack, c);
    pushStack(stack, a);
  } else if (token->OP_TYPE == OP_CR) {
    printf("\n");
  } else {
    assert(0, "Unreachable code.");
  }
}

struct Token* makeToken(int pos, char *word) {
  struct Token* token;
  token = (struct Token*) malloc(sizeof(struct Token));
  token->pos = pos;
  token->value = 0;
  assert(OPS_COUNT == 22, "Update control flow in makeToken().");
  // control flow to decide type of operation
  if (isNumber(word)) {
    token->OP_TYPE = OP_INT;
    token->value = atoi(word);
  } else if (strcmp(word, "+") == 0) {
    token->OP_TYPE = OP_ADD;
  } else if (strcmp(word, "-") == 0) {
    token->OP_TYPE = OP_SUB;
  } else if (strcmp(word, "*") == 0) {
    token->OP_TYPE = OP_MUL;
  } else if (strcmp(word, "/") == 0) {
    token->OP_TYPE = OP_DIV;
  } else if (strcmp(word, "%") == 0) {
    token->OP_TYPE = OP_REM;
  } else if (strcmp(word, "=") == 0) {
    token->OP_TYPE = OP_EQU;
  } else if (strcmp(word, ">=") == 0) {
    token->OP_TYPE = OP_GTE;
  } else if (strcmp(word, "<=") == 0) {
    token->OP_TYPE = OP_LTE;
  } else if (strcmp(word, ">") == 0) {
    token->OP_TYPE = OP_GT;
  } else if (strcmp(word, "<") == 0) {
    token->OP_TYPE = OP_LT;
  } else if (strcmp(word, ",") == 0) {
    token->OP_TYPE = OP_PEEK;
  } else if (strcmp(word, ".") == 0) {
    token->OP_TYPE = OP_POP;
  } else if (strcmp(word, "emit") == 0) {
    token->OP_TYPE = OP_EMIT;
  } else if (strcmp(word, ".s") == 0) {
    token->OP_TYPE = OP_SIZE;
  } else if (strcmp(word, "dup") == 0) {
    token->OP_TYPE = OP_DUP;
  } else if (strcmp(word, "drop") == 0) {
    token->OP_TYPE = OP_DROP;
  } else if (strcmp(word, "swap") == 0) {
    token->OP_TYPE = OP_SWAP;
  } else if (strcmp(word, "over") == 0) {
    token->OP_TYPE = OP_OVER;
  } else if (strcmp(word, "rot") == 0) {
    token->OP_TYPE = OP_ROT;
  } else if (strcmp(word, "cr") == 0) {
    token->OP_TYPE = OP_CR;
  } else {
    token->OP_TYPE = OP_UNKNOWN;
    char *message;
    asprintf(&message, "Word %s not implemented yet.", word);
    assert(0, message);
  }
  return token;
}

/* Main Function */
int main(int argc, char* argv[]) {
  /* assert(argc > 1, "No file given.\nPlease use `./forth filename`"); */
  char* filename = argv[1];
  FILE *sourceptr;
  sourceptr = fopen(filename, "r");

  if (sourceptr == NULL) {
    char *message;
    asprintf(&message, "File %s not found", filename);
    assert(sourceptr == NULL, message);
  }

  struct Queue* instructions = newQueue();
  struct Stack* stack = newStack();
  /* word size is limited to 256 */
  char word[256];
  int pos = 0;
  while(fscanf(sourceptr, "%s", word) != EOF) {
    struct Token *token = makeToken(pos, word);
    pushQueue(instructions, token);
    pos = ftell(sourceptr);
  }
  while(!isEmptyQueue(instructions)) {
    struct QueueElem* elem = pollQueue(instructions);
    parseQueueElem(stack, elem);
  }
  fclose(sourceptr);
  return 0;
}
