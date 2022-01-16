#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define PARSE_FUNC_TYPE Stack* stack, Queue* instructions, int *mem, Definitions* definitions, Token* token
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
  OP_WHILE,
  OP_THEN,
  OP_END,
  OP_DEFWORD,
  OP_ENDDEF,
  OP_MEM,
  OP_MEMR,
  OPS_COUNT // size of enum OPS
} OPS;

typedef struct Queue Queue;
typedef struct QueueElem QueueElem;
typedef struct Node Node;
typedef struct Stack Stack;
typedef struct Token Token;
typedef struct Definitions Definitions;
typedef struct DefWord DefWord;

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

typedef struct Definitions {
  DefWord *head;
} Definitions;

typedef struct DefWord {
  char word[MAX_WORD_SIZE];
  Queue *block;
  DefWord *next;
} DefWord;

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

/* Concats 2 queues, value is saved in the second. */
void concatQueues(Queue *block, Queue *instructions) {
  if (isEmptyQueue(block)) {
    /* do nothing */
  } else if (isEmptyQueue(instructions)) {
    instructions->size = block->size;
    instructions->head = block->head;
    instructions->tail = block->tail;
  } else {
    instructions->size += block->size;
    block->tail->next = instructions->head;
    instructions->head->prev = block->tail;
    instructions->head = block->head;
  }
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

/* Initialise Definitions. */
Definitions* newDefinitions() {
  Definitions *definitions;
  definitions = (Definitions*) malloc(sizeof(Definitions));
  definitions->head = NULL;
  return definitions;
}

/* Add a word definition. */
void addDefinition(Definitions *definitions, char *word, Queue *block) {
  DefWord *definition;
  definition = (DefWord*) malloc(sizeof(DefWord));
  strncpy(definition->word, word, MAX_WORD_SIZE);
  definition->block = block;
  definition->next = definitions->head;
  definitions->head = definition;
}

/* Find definition of a word, NULL if not found. */
DefWord* findDefinition(Definitions *definitions, char *word) {
  DefWord *definition = definitions->head;
  while (definition != NULL) {
    if (strncmp(word, definition->word, MAX_WORD_SIZE) == 0) {
      return definition;
    }
    definition = definition->next;
  }
  return NULL;
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
    } else if (i == 0 && current == '-' && word[i+1] != 0) {
      /* okay to have `-` at the start, and make sure it is not a single `-` */
    } else if (current < 48 || current > 57) {
      return 0;
    }
  }
  assert(0, "Word does not terminate.");
  return 0;
}

/* Declaration here to use it in controlIf. */
void parseQueue(Stack* stack, Queue* instructions, int *mem, Definitions* definitions, QueueElem* queueElem);

/* Jumps to one of the specified OP_TYPEs. Used in control flow. */
void jumpTo(Stack* stack, Queue* instructions, int *mem, Definitions* definitions, int *jumpPoints, size_t jumpPointsSize) {
  Token *startToken = peekQueue(instructions)->token;
  while (!isEmptyQueue(instructions)) {
    Token *token = peekQueue(instructions)->token;
    int type = token->OP_TYPE;
    int i;
    if (type == OP_IF || type == OP_WHILE) {
      parseQueue(stack, instructions, mem, definitions, pollQueue(instructions));
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
void jumpToEnd(Stack* stack, Queue* instructions, int *mem, Definitions* definitions) {
  int jumpPoints[1] = {OP_END};
  jumpTo(stack, instructions, mem, definitions, jumpPoints, 1);
}

void parseUNKNOWN(PARSE_FUNC_TYPE) {
  DefWord *definition = findDefinition(definitions, token->word);
  if (definition == NULL) {
    char *message;
    asprintf(&message, "Word `%s` not implemented yet.", token->word);
    assertWithToken(0, message, token);
  }
  /* Add block to instructions. */
  Queue *block = definition->block;
  concatQueues(block, instructions);
}

void parseINT(PARSE_FUNC_TYPE) {
  pushStack(stack, token->value);
}

void parseADD(PARSE_FUNC_TYPE) {
  int a = popStack(stack);
  int b = popStack(stack);
  pushStack(stack, b + a);
}

void parseSUB(PARSE_FUNC_TYPE) {
  int a = popStack(stack);
  int b = popStack(stack);
  pushStack(stack, b - a);
}

void parseMUL(PARSE_FUNC_TYPE) {
  int a = popStack(stack);
  int b = popStack(stack);
  pushStack(stack, b * a);
}

void parseDIV(PARSE_FUNC_TYPE) {
  int a = popStack(stack);
  int b = popStack(stack);
  pushStack(stack, b / a);
}

void parseREM(PARSE_FUNC_TYPE) {
  int a = popStack(stack);
  int b = popStack(stack);
  pushStack(stack, b % a);
}

void parseEQU(PARSE_FUNC_TYPE) {
  int a = popStack(stack);
  int b = popStack(stack);
  pushStack(stack, a == b ? 1 : 0);
}

void parseNEQU(PARSE_FUNC_TYPE) {
  int a = popStack(stack);
  int b = popStack(stack);
  pushStack(stack, a != b ? 1 : 0);
}

void parseGTE(PARSE_FUNC_TYPE) {
  int b = popStack(stack);
  int a = popStack(stack);
  pushStack(stack, a >= b ? 1 : 0);
}

void parseLTE(PARSE_FUNC_TYPE) {
  int b = popStack(stack);
  int a = popStack(stack);
  pushStack(stack, a <= b ? 1 : 0);
}

void parseGT(PARSE_FUNC_TYPE) {
  int b = popStack(stack);
  int a = popStack(stack);
  pushStack(stack, a > b ? 1 : 0);
}

void parseLT(PARSE_FUNC_TYPE) {
  int b = popStack(stack);
  int a = popStack(stack);
  pushStack(stack, a < b ? 1 : 0);
}

void parsePEEK(PARSE_FUNC_TYPE) {
  int top = peekStack(stack);
  printf("%d\n", top);
}

void parsePOP(PARSE_FUNC_TYPE) {
  int top = popStack(stack);
  printf("%d\n", top);
}

void parseEMIT(PARSE_FUNC_TYPE) {
  int top = popStack(stack);
  printf("%c\n", top);
}

void parseSIZE(PARSE_FUNC_TYPE) {
  int size = stack->size;
  printf("%d\n", size);
}

void parseCR(PARSE_FUNC_TYPE) {
  printf("\n");
}

void parseDUP(PARSE_FUNC_TYPE) {
  int top = peekStack(stack);
  pushStack(stack, top);
}

void parseDROP(PARSE_FUNC_TYPE) {
  popStack(stack);
}

void parseSWAP(PARSE_FUNC_TYPE) {
  int a = popStack(stack);
  int b = popStack(stack);
  pushStack(stack, a);
  pushStack(stack, b);
}

void parseOVER(PARSE_FUNC_TYPE) {
  int a = popStack(stack);
  int second = peekStack(stack);
  pushStack(stack, a);
  pushStack(stack, second);
}

void parseROT(PARSE_FUNC_TYPE) {
  int c = popStack(stack);
  int b = popStack(stack);
  int a = popStack(stack);
  pushStack(stack, b);
  pushStack(stack, c);
  pushStack(stack, a);
}

void parseSTR(PARSE_FUNC_TYPE) {
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
}

void parseIF(Stack* stack, Queue* instructions, int *mem, Definitions* definitions, Token* startToken) {
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
    parseQueue(stack, instructions, mem, definitions, queueElem);
  }
  assertWithToken(hasThen, "`then` not found after `if` or `elseif`", startToken);
  int truth = popStack(stack);
  if (truth == 0) {
    /* Jump to next block for evaluation. */
    int jumpPoints[2] = {OP_ELSEIF, OP_END};
    jumpTo(stack, instructions, mem, definitions, jumpPoints, 2);
    Token* next = pollQueue(instructions)->token;
    parseIF(stack, instructions, mem, definitions, next);
  } else {
    int hasEnd = 0;
    while (!isEmptyQueue(instructions)) {
      QueueElem *queueElem = pollQueue(instructions);
      Token *current = queueElem->token;
      if (current->OP_TYPE == OP_ELSEIF) {
        jumpToEnd(stack, instructions, mem, definitions);
        /* Poll `end` word off. */
        current = pollQueue(instructions)->token;
      }
      if (current->OP_TYPE == OP_END) {
        hasEnd = 1;
        break;
      }
      parseQueue(stack, instructions, mem, definitions, queueElem);
    }
    assertWithToken(hasEnd, "`end` not found after `if` or `elseif`", startToken);
  }
}

void parseELSEIF(PARSE_FUNC_TYPE) {
  assertWithToken(0, "`elseif` without if", token);
}

void parseWHILE(PARSE_FUNC_TYPE) {
  Queue *evalInstructions = newQueue();
  Queue *loopInstructions = newQueue();
  QueueElem *elem;
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
      parseQueue(stack, evalQueue, mem, definitions, pollQueue(evalQueue));
    }
    truth = popStack(stack);
    if (truth == 0) {
      break;
    }

    Queue *loopQueue = copyQueue(loopInstructions);
    while (!isEmptyQueue(loopQueue)) {
      parseQueue(stack, loopQueue, mem, definitions, pollQueue(loopQueue));
    }
  }
}

void parseTHEN(PARSE_FUNC_TYPE) {
  assertWithToken(0, "`then` word without starting", token);
}

void parseEND(PARSE_FUNC_TYPE) {
  assertWithToken(0, "`end` word without starting.", token);
}

void parseDEFWORD(PARSE_FUNC_TYPE) {
  Token *wordNameToken = pollQueue(instructions)->token;
  assertWithToken(wordNameToken->OP_TYPE == OP_UNKNOWN, "Word must not be defined before.", wordNameToken);
  char *wordName = wordNameToken->word;
  int hasEndDef = 0;
  Queue *block = newQueue();
  Token *defToken;
  int type;
  while (!isEmptyQueue(instructions)) {
    defToken = pollQueue(instructions)->token;
    type = defToken->OP_TYPE;
    assertWithToken(type != OP_DEFWORD, "No nested `defword`", defToken);
    if (type == OP_ENDDEF) {
      hasEndDef = 1;
      break;
    }
    pushQueue(block, defToken);
  }
  assertWithToken(hasEndDef, "`enddef` not found after `defword`", token);
  addDefinition(definitions, wordName, block);
}

void parseENDDEF(PARSE_FUNC_TYPE) {
  assertWithToken(0, "`enddef` without `defword`", token);
}

void parseMEM(PARSE_FUNC_TYPE) {
  Token *indexToken = pollQueue(instructions)->token;
  int index = indexToken->value;
  assertWithToken(index >= 0, "Memory index must be non-negative.", indexToken);
  assertWithToken(index < MEM_SIZE, "Memory index is too large.", indexToken);
  mem[index] = popStack(stack);
}

void parseMEMR(PARSE_FUNC_TYPE) {
  Token *indexToken = pollQueue(instructions)->token;
  int index = indexToken->value;
  assertWithToken(index >= 0, "Memory index must be non-negative.", indexToken);
  assertWithToken(index < MEM_SIZE, "Memory index is too large.", indexToken);
  pushStack(stack, mem[index]);
}

/* Parses a token. */
void parseQueue(Stack* stack, Queue* instructions, int *mem, Definitions* definitions, QueueElem* queueElem) {
  assert(OPS_COUNT == 33, "Update control flow in parse().");
  Token *token = queueElem->token;
  void (*parsers[OPS_COUNT]) (PARSE_FUNC_TYPE) = {
    parseUNKNOWN,
    parseINT,
    parseADD,
    parseSUB,
    parseMUL,
    parseDIV,
    parseREM,
    parseEQU,
    parseNEQU,
    parseGTE,
    parseLTE,
    parseGT,
    parseLT,
    parsePEEK,
    parsePOP,
    parseEMIT,
    parseSIZE,
    parseCR,
    parseDUP,
    parseDROP,
    parseSWAP,
    parseOVER,
    parseROT,
    parseSTR,
    parseIF,
    parseELSEIF,
    parseWHILE,
    parseTHEN,
    parseEND,
    parseDEFWORD,
    parseENDDEF,
    parseMEM,
    parseMEMR,
  };
  parsers[token->OP_TYPE](stack, instructions, mem, definitions, token);
}

Token* makeToken(int row, int col, char *word) {
  Token* token;
  token = (Token*) malloc(sizeof(Token));
  token->row = row;
  token->col = col;
  token->value = 0;
  token->OP_TYPE = OP_UNKNOWN;
  strncpy(token->word, word, MAX_WORD_SIZE);
  assert(OPS_COUNT == 33, "Update control flow in makeToken().");
  // control flow to decide type of operation
  char *types[OPS_COUNT] = {
    "UNKNOWN",
    "INT",
    "+",
    "-",
    "*",
    "/",
    "%",
    "=",
    "!=",
    ">=",
    "<=",
    ">",
    "<",
    ",",
    ".",
    "emit",
    ".s",
    "cr",
    "dup",
    "drop",
    "swap",
    "over",
    "rot",
    ".\"",
    "if",
    "elseif",
    "while",
    "then",
    "end",
    "defword",
    "enddef",
    "mem",
    "memr",
  };
  if (isNumber(word)) {
    token->OP_TYPE = OP_INT;
    token->value = atoi(word);
  } else {
    int i;
    for (i = 0; i < OPS_COUNT; i++) {
      if (strncmp(word, types[i], MAX_WORD_SIZE) == 0) {
        token->OP_TYPE = i;
        break;
      }
    }
  }
  return token;
}

/* Main Function */
int main(int argc, char* argv[]) {
  assert(argc > 1, "Not enough arguments.\nPlease use `./stackc filename` or `./stackc -s \"1 .\"`");
  int flagShort = strncmp(argv[1], "-s", 2) == 0;
  Queue *instructions = newQueue();
  Stack *stack = newStack();
  Definitions *definitions = newDefinitions();
  int isMemUsed = 0;
  /* `mem` is initialised only if actually used in the program. */
  int mem[MEM_SIZE];
  /* word size is limited to MAX_WORD_SIZE */
  char word[MAX_WORD_SIZE];
  memset(word, 0, sizeof(word));

  if (flagShort) {
    assert(argc > 2, "Not enough arguments. Please provide a short script to run.");
    int current = 1;
    size_t argvI = 0;
    size_t wordI = 0;
    while (current != 0) {
      current = argv[2][argvI++];
      if (current == 0 || current == 32) {
        /* 0 is the EOF character, 32 is space character. */
        if (wordI == 0) {
          continue;
        }
        Token *token = makeToken(0, argvI - wordI, word);
        if (token->OP_TYPE == OP_MEM || token->OP_TYPE == OP_MEMR) {
          isMemUsed = 1;
        }
        pushQueue(instructions, token);
        wordI = 0;
        memset(word, 0, sizeof(word));
      } else {
        word[wordI++] = current;
      }
    }
  } else {
    char* filename = argv[1];
    FILE *source;
    source = fopen(filename, "r");

    if (source == NULL) {
      char *message;
      asprintf(&message, "File %s not found", filename);
      assert(source == NULL, message);
    }

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
    fclose(source);
  }

  if (isMemUsed) {
    memset(mem, 0, sizeof(mem));
  }
  while (!isEmptyQueue(instructions)) {
    parseQueue(stack, instructions, mem, definitions, pollQueue(instructions));
  }

  return 0;
}