#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_WORD_SIZE 256
#define MAX_STRING_SIZE 1000
#define MEM_SIZE 100

typedef enum OPS {
  OP_UNKNOWN,
  OP_INT,
  OP_ADD,
  OP_SUB,
  OP_MUL,
  OP_DIV,
  OP_REM,
  OP_EQU,
  OP_NEQU,
  OP_GTE,
  OP_LTE,
  OP_GT,
  OP_LT,
  OP_PEEK,
  OP_POP,
  OP_EMIT,
  OP_SIZE,
  OP_CR,
  OP_DUP,
  OP_DROP,
  OP_SWAP,
  OP_OVER,
  OP_ROT,
  OP_STR,
  OP_IF,
  OP_ELSEIF,
  OP_THEN,
  OP_WHILE,
  OP_END,
  OP_MEM,
  OP_MEMR,
  OPS_COUNT // size of enum OPS
} OPS;

typedef struct Queue Queue;
typedef struct QueueElem QueueElem;
typedef struct Node Node;
typedef struct Stack Stack;
typedef struct Token Token;

/* Doubly Linked List implementation of a queue. */
/* Does not break links between elements when polling. */

typedef struct Queue {
  int size;
  QueueElem* head;
  QueueElem* tail;
} Queue;

typedef struct QueueElem {
  Token* token;
  QueueElem* prev;
  QueueElem* next;
} QueueElem;

/* Linked List implementation of a stack. */

typedef struct Node {
  int value;
  Node* next;
} Node;

typedef struct Stack {
  int size;
  Node* root;
} Stack;

typedef struct Token {
  int row;
  int col;
  OPS OP_TYPE;
  int value;
  char word[MAX_WORD_SIZE];
} Token;

/* Print Token for debugging. */
void printToken(Token* token) {
  printf("-- Token --\n");
  printf("Position: %d %d\n", token->row, token->col);
  printf("OP_TYPE: %d\n", token->OP_TYPE);
  printf("Value: %d\n", token->value);
  printf("Word: %s\n", token->word);
}

/* Assert with Token. */
int assertWithToken(int truth, char *message, Token* token) {
  if (truth == 0) {
    printf("Assertion Error: %s\n", message);
    if (token != NULL) {
      printToken(token);
    }
    exit(1);
  } else {
    return 1;
  }
}

/* Python-like assertions. */
int assert(int truth, char *message) {
  return assertWithToken(truth, message, NULL);
}

/* Initialise a new queue. */
Queue* newQueue(void) {
  Queue *queue;
  queue = (Queue*) malloc(sizeof(Queue));
  queue->size = 0;
  queue->head = NULL;
  queue->tail = NULL;
  return queue;
}

/* Checking if a queue is empty. */
int isEmptyQueue(Queue* queue) {
  return queue->size == 0;
}

/* Push to queue. */
void pushQueue(Queue* queue, Token* token) {
  QueueElem *elem;
  elem = (QueueElem*) malloc(sizeof(QueueElem));
  elem->token = token;
  elem->next = NULL;
  if (isEmptyQueue(queue)) {
    queue->head = elem;
    queue->tail = elem;
    elem->prev = NULL;
  } else {
    QueueElem *prevLast = queue->tail;
    prevLast->next = elem;
    elem->prev = prevLast;
    queue->tail = elem;
  }
  queue->size++;
}

/* Poll head of queue. */
QueueElem* pollQueue(Queue* queue) {
  assert(!isEmptyQueue(queue), "Polling from empty queue.");
  QueueElem *prevHead = queue->head;
  QueueElem *newHead = prevHead->next;
  queue->head = newHead;
  /* newHead will maintain the reference to the now polled prev. */
  queue->size--;
  return prevHead;
}

/* Peek head of queue. */
QueueElem* peekQueue(Queue* queue) {
  assert(!isEmptyQueue(queue), "Peeking empty queue.");
  return queue->head;
}

/* Get previous queue even when polled. */
QueueElem* prevQueueElem(QueueElem* currentElem) {
  QueueElem *prevElem = currentElem->prev;
  assert(prevElem != NULL, "Previous element should not be null.");
  return prevElem;
}

/* Copy a queue. */
Queue* copyQueue(Queue* queue) {
  Queue *result = newQueue();
  QueueElem *elem = queue->head;
  while (elem != NULL) {
    pushQueue(result, elem->token);
    elem = elem->next;
  }
  return result;
}

/* Initialise a new stack. */
Stack* newStack(void) {
  Stack *stack;
  stack = (Stack*) malloc(sizeof(Stack));
  stack->size = 0;
  stack->root = NULL;
  return stack;
}

/* Checking if a stack is empty. */
int isEmptyStack(Stack* stack) {
  return stack->size == 0;
}

/* Push an integer onto a stack. */
int pushStack(Stack* stack, int value) {
  Node* newNode;
  newNode = (Node*) malloc(sizeof(Node));
  newNode->value = value;
  newNode->next = stack->root;
  stack->root = newNode;
  stack->size++;
  return 1;
}

/* Peek at the first element of the stack. */
int peekStack(Stack* stack) {
  assert(!isEmptyStack(stack), "Stack underflow while peeking stack.\n");
  return stack->root->value;
}

/* Pops the first element of the stack. */
int popStack(Stack* stack) {
  assert(!isEmptyStack(stack), "Stack underflow while popping stack.\n");
  Node* poppedNode = stack->root;
  stack->root = poppedNode->next;
  stack->size--;
  return poppedNode->value;
}

/* Checks if a string is a representation of an integer. */
int isNumber(char* word) {
  char current;
  size_t i;
  for (i = 0; i < MAX_WORD_SIZE; i++) {
    current = word[i];
    if (current == 0) {
      /* 0 is the EOF character */
      return 1;
    } else if (current < 48 || current > 57) {
      return 0;
    }
  }
  assert(0, "Word does not terminate.");
  return 0;
}

/* Declaration here to use it in controlIf. */
void parseQueue(Stack* stack, Queue* instructions, int *mem, QueueElem* queueElem);

/* Jumps to one of the specified OP_TYPEs. Used in control flow. */
void jumpTo(Stack* stack, Queue* instructions, int *mem, int *jumpPoints, size_t jumpPointsSize) {
  Token *startToken = peekQueue(instructions)->token;
  while (!isEmptyQueue(instructions)) {
    Token *token = peekQueue(instructions)->token;
    int type = token->OP_TYPE;
    int i;
    if (type == OP_IF || type == OP_WHILE) {
      parseQueue(stack, instructions, mem, pollQueue(instructions));
      pollQueue(instructions);
    } else if (type == OP_STR) {
      /* Remove ." to end from instructions. */
      Token *stringToken = token;
      while (!isEmptyQueue(instructions) && stringToken->OP_TYPE != OP_END) {
        stringToken = pollQueue(instructions)->token;
      }
      continue;
    }
    for (i = 0; i < jumpPointsSize; i++) {
      if (type == jumpPoints[i]) {
        return;
      }
    }
    pollQueue(instructions);
  }
  assertWithToken(0, "No `end` word in control flow.", startToken);
}

/* Specific version of jumpTo, for convenience. */
void jumpToEnd(Stack* stack, Queue* instructions, int *mem) {
  int jumpPoints[1] = {OP_END};
  jumpTo(stack, instructions, mem, jumpPoints, 1);
}

/* Called when parseQueue() encounters a OP_IF. */
void controlIf(Stack* stack, Queue* instructions, int *mem, Token* startToken) {
  int startType = startToken->OP_TYPE;
  if (startType == OP_END) {
    return;
  }
  assertWithToken(startType == OP_IF || startType == OP_ELSEIF, "controlIf must be called with `if` or `elseif` token", startToken);
  /* Evaluate until `then` keyword */
  int hasThen = 0;
  while (!isEmptyQueue(instructions)) {
    QueueElem *queueElem = pollQueue(instructions);
    Token *current = queueElem->token;
    if (current->OP_TYPE == OP_THEN) {
      hasThen = 1;
      break;
    }
    parseQueue(stack, instructions, mem, queueElem);
  }
  assertWithToken(hasThen, "`then` not found after `if` or `elseif`", startToken);
  int truth = popStack(stack);
  if (truth == 0) {
    /* Jump to next block for evaluation. */
    int jumpPoints[2] = {OP_ELSEIF, OP_END};
    jumpTo(stack, instructions, mem, jumpPoints, 2);
    Token* next = pollQueue(instructions)->token;
    controlIf(stack, instructions, mem, next);
  } else {
    int hasEnd = 0;
    while (!isEmptyQueue(instructions)) {
      QueueElem *queueElem = pollQueue(instructions);
      Token *current = queueElem->token;
      if (current->OP_TYPE == OP_ELSEIF) {
        jumpToEnd(stack, instructions, mem);
        /* Poll `end` word off. */
        current = pollQueue(instructions)->token;
      }
      if (current->OP_TYPE == OP_END) {
        hasEnd = 1;
        break;
      }
      parseQueue(stack, instructions, mem, queueElem);
    }
    assertWithToken(hasEnd, "`end` not found after `if` or `elseif`", startToken);
  }
}

/* Called when parseQueue() encounters a OP_WHILE. */
void controlWhile(Stack* stack, Queue* instructions, int *mem, QueueElem* rootElem) {
  Queue *evalInstructions = newQueue();
  Queue *loopInstructions = newQueue();
  QueueElem *elem;
  Token *token;
  int type;
  /* Fill up evalInstructions. */
  while (!isEmptyQueue(instructions)) {
    elem = pollQueue(instructions);
    token = elem->token;
    type = token->OP_TYPE;
    if (type == OP_THEN) {
      break;
    }
    pushQueue(evalInstructions, token);
  }
  /* Fill up loopInstructions. */
  int ends = 1;
  while (ends > 0 && !isEmptyQueue(instructions)) {
    elem = pollQueue(instructions);
    token = elem->token;
    type = token->OP_TYPE;
    if (type == OP_IF || type == OP_WHILE || type == OP_STR) {
      ends++;
    } else if (type == OP_END) {
      ends--;
    }
    if (ends == 0) {
      break;
    }
    pushQueue(loopInstructions, token);
  }
  int truth;
  while (1) {
    /* Evaluate evalInstuctions. */
    Queue *evalQueue = copyQueue(evalInstructions);
    while (!isEmptyQueue(evalQueue)) {
      parseQueue(stack, evalQueue, mem, pollQueue(evalQueue));
    }
    truth = popStack(stack);
    if (truth == 0) {
      break;
    }

    Queue *loopQueue = copyQueue(loopInstructions);
    while (!isEmptyQueue(loopQueue)) {
      parseQueue(stack, loopQueue, mem, pollQueue(loopQueue));
    }
  }
}

/* Parses a token. */
void parseQueue(Stack* stack, Queue* instructions, int *mem, QueueElem* queueElem) {
  assert(OPS_COUNT == 31, "Update control flow in parse().");
  Token *token = queueElem->token;
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
    pushStack(stack, a == b ? 1 : 0);
  } else if (token->OP_TYPE == OP_NEQU) {
    int a = popStack(stack);
    int b = popStack(stack);
    pushStack(stack, a != b ? 1 : 0);
  } else if (token->OP_TYPE == OP_GTE) {
    int b = popStack(stack);
    int a = popStack(stack);
    pushStack(stack, a >= b ? 1 : 0);
  } else if (token->OP_TYPE == OP_LTE) {
    int b = popStack(stack);
    int a = popStack(stack);
    pushStack(stack, a <= b ? 1 : 0);
  } else if (token->OP_TYPE == OP_GT) {
    int b = popStack(stack);
    int a = popStack(stack);
    pushStack(stack, a > b ? 1 : 0);
  } else if (token->OP_TYPE == OP_LT) {
    int b = popStack(stack);
    int a = popStack(stack);
    pushStack(stack, a < b ? 1 : 0);
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
  } else if (token->OP_TYPE == OP_CR) {
    printf("\n");
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
  } else if (token->OP_TYPE == OP_STR) {
    /* Parsing a string. */
    char string[MAX_STRING_SIZE];
    memset(string, 0, sizeof(string));
    int index = 0;
    while (!isEmptyQueue(instructions)) {
      Token *word = pollQueue(instructions)->token;
      if (word->OP_TYPE == OP_END) {
        printf("%s\n", string);
        return;
      } else {
        char current;
        size_t i;
        for (i = 0; i < MAX_WORD_SIZE; i++) {
          current = word->word[i];
          if (current == 0) {
            /* 0 is the EOF character */
            break;
          }
          assertWithToken(index < MAX_STRING_SIZE, "String too long.", token);
          string[index++] = current;
        }
        /* Add space after each word */
        assertWithToken(index < MAX_STRING_SIZE, "String too long.", token);
        string[index++] = ' ';
      }
    }
    assertWithToken(0, "`.\"` without `end`.", token);
  } else if (token->OP_TYPE == OP_IF) {
    controlIf(stack, instructions, mem, token);
  } else if (token->OP_TYPE == OP_ELSEIF) {
    assertWithToken(0, "`elseif` without if", token);
  } else if (token->OP_TYPE == OP_WHILE) {
    controlWhile(stack, instructions, mem, queueElem);
  } else if (token->OP_TYPE == OP_THEN) {
    assertWithToken(0, "`then` word without starting", token);
  } else if (token->OP_TYPE == OP_END) {
    assertWithToken(0, "`end` word without starting.", token);
  } else if (token->OP_TYPE == OP_MEM) {
    Token *indexToken = pollQueue(instructions)->token;
    int index = indexToken->value;
    assertWithToken(index >= 0, "Memory index must be non-negative.", indexToken);
    assertWithToken(index < MEM_SIZE, "Memory index is too large.", indexToken);
    mem[index] = popStack(stack);
  } else if (token->OP_TYPE == OP_MEMR) {
    Token *indexToken = pollQueue(instructions)->token;
    int index = indexToken->value;
    assertWithToken(index >= 0, "Memory index must be non-negative.", indexToken);
    assertWithToken(index < MEM_SIZE, "Memory index is too large.", indexToken);
    pushStack(stack, mem[index]);
  } else if (token->OP_TYPE == OP_UNKNOWN) {
    char *message;
    asprintf(&message, "Word `%s` not implemented yet.", token->word);
    assertWithToken(0, message, token);
  } else {
    assertWithToken(0, "Unreachable code.", token);
  }
}

Token* makeToken(int row, int col, char *word) {
  Token* token;
  token = (Token*) malloc(sizeof(Token));
  token->row = row;
  token->col = col;
  token->value = 0;
  strncpy(token->word, word, MAX_WORD_SIZE);
  assert(OPS_COUNT == 31, "Update control flow in makeToken().");
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
  } else if (strcmp(word, "!=") == 0) {
    token->OP_TYPE = OP_NEQU;
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
  } else if (strcmp(word, "cr") == 0) {
    token->OP_TYPE = OP_CR;
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
  } else if (strcmp(word, ".\"") == 0) {
    token->OP_TYPE = OP_STR;
  } else if (strcmp(word, "if") == 0) {
    token->OP_TYPE = OP_IF;
  } else if (strcmp(word, "elseif") == 0) {
    token->OP_TYPE = OP_ELSEIF;
  } else if (strcmp(word, "while") == 0) {
    token->OP_TYPE = OP_WHILE;
  } else if (strcmp(word, "then") == 0) {
    token->OP_TYPE = OP_THEN;
  } else if (strcmp(word, "end") == 0) {
    token->OP_TYPE = OP_END;
  } else if (strcmp(word, "mem") == 0) {
    token->OP_TYPE = OP_MEM;
  } else if (strcmp(word, "memr") == 0) {
    token->OP_TYPE = OP_MEMR;
  } else {
    token->OP_TYPE = OP_UNKNOWN;
  }
  return token;
}

/* Main Function */
int main(int argc, char* argv[]) {
  assert(argc > 1, "No file given.\nPlease use `./forth filename`");
  char* filename = argv[1];
  FILE *source;
  source = fopen(filename, "r");

  if (source == NULL) {
    char *message;
    asprintf(&message, "File %s not found", filename);
    assert(source == NULL, message);
  }

  Queue* instructions = newQueue();
  Stack* stack = newStack();
  int isMemUsed = 0;
  /* `mem` is initialised only if actually used in the program. */
  int mem[MEM_SIZE];

  /* word size is limited to MAX_WORD_SIZE */
  char word[MAX_WORD_SIZE];
  memset(word, 0, sizeof(word));

  /* Parsing input line by line. */
  char *line = NULL;
  /* Signed integer, includes new line character. */
  int row = 0;
  ssize_t lengthOfLine;
  size_t len = 0;
  while ((lengthOfLine = getline(&line, &len, source)) != -1) {
    int wordIndex = 0;
    memset(word, 0, sizeof(word));
    int lineIndex;
    for (lineIndex = 0; lineIndex < lengthOfLine; lineIndex++) {
      char c = line[lineIndex];
      /* Catch comments and ignore the rest (by exiting for loop). */
      if (c == '-' && lineIndex < lengthOfLine - 1 && line[lineIndex+1] == '-') {
        break;
      } else if (c == ' ' || c == '\n') {
        if (wordIndex == 0) {
          /* Skipping multiple spaces. */
          continue;
        }
        Token *token = makeToken(row, lineIndex - wordIndex, word);
        /* Only initialises `mem` if actually used. */
        if (token->OP_TYPE == OP_MEM || token->OP_TYPE == OP_MEMR) {
          isMemUsed = 1;
        }
        pushQueue(instructions, token);
        wordIndex = 0;
        memset(word, 0, sizeof(word));
      } else {
        assert(wordIndex < MAX_WORD_SIZE, "Word is too long!");
        word[wordIndex++] = c;
      }
    }
    row++;
  }

  if (isMemUsed) {
    memset(mem, 0, sizeof(mem));
  }

  while (!isEmptyQueue(instructions)) {
    parseQueue(stack, instructions, mem, pollQueue(instructions));
  }

  fclose(source);
  return 0;
}
