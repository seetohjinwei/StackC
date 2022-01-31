#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define DEF_SIZE 64
#define PARSE_FUNC_TYPE Stack* stack, Queue* instructions, Definitions* definitions, Token* token
#define MAX_WORD_SIZE 1024

typedef enum OPS {
  OP_UNKNOWN,
  OP_INT,
  OP_STR,
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
  OP_POP,
  OP_EMIT,
  OP_SIZE,
  OP_PRINT,
  OP_DUP,
  OP_DROP,
  OP_SWAP,
  OP_OVER,
  OP_ROT,
  OP_IF,
  OP_ELSEIF,
  OP_WHILE,
  OP_THEN,
  OP_END,
  OP_DEFWORD,
  OP_ENDDEF,
  OPS_COUNT /* size of enum OPS */
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
  Definitions *definitions = malloc(sizeof(*definitions) * DEF_SIZE);
  int i;
  for (i = 0; i < DEF_SIZE; i++) {
    Definitions *definition;
    definition = (Definitions*) malloc(sizeof(DefWord));
    definition->head = NULL;
    definitions[i] = *definition;
  }
  return definitions;
}

int getHeadIndex(char *word) {
  int headsSize = DEF_SIZE - 1;
  char heads[] = {
    'A','B','C','D','E','F','G','H','I','J','K','L','M','N','O','P','Q','R','S','T','U','V','W','X','Y','Z',
  'a','b','c','d','e','f','g','h','i','j','k','l','m','n','o','p','q','r','s','t','u','v','w','x','y','z',
  '0','1','2','3','4','5','6','7','8','9','_'
  };
  char first = word[0];
  int index;
  for (index = 0; index < headsSize; index++) {
    char head = heads[index];
    if (first == head) {
      break;
    }
  }
  return index;
}

/* Add a word definition. */
void addDefinition(Definitions definitions[], char *word, Queue *block) {
  int index = getHeadIndex(word);
  Definitions *group = &definitions[index];
  DefWord *definition;
  definition = (DefWord*) malloc(sizeof(DefWord));
  strncpy(definition->word, word, MAX_WORD_SIZE);
  definition->block = block;
  definition->next = group->head;
  group->head = definition;
}

/* Find definition of a word, NULL if not found. */
DefWord* findDefinition(Definitions definitions[], char *word) {
  int index = getHeadIndex(word);
  Definitions *group = &definitions[index];
  DefWord *definition = group->head;
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

int isString(char *word) {
  if (word[0] != '"') {
    return 0;
  }
  size_t i;
  for (i = 1; i < MAX_WORD_SIZE; i++) {
    if (word[i] == '"') {
      return word[i+1] == '\0';
    }
  }
  return 0;
}

/* Declaration here to use it in controlIf. */
void parseQueue(Stack* stack, Queue* instructions, Definitions* definitions, QueueElem* queueElem);

/* Jumps to one of the specified OP_TYPEs. Used in control flow. */
void jumpTo(Stack* stack, Queue* instructions, Definitions* definitions, int *jumpPoints, size_t jumpPointsSize) {
  Token *startToken = peekQueue(instructions)->token;
  while (!isEmptyQueue(instructions)) {
    Token *token = peekQueue(instructions)->token;
    int type = token->OP_TYPE;
    int i;
    if (type == OP_IF || type == OP_WHILE) {
      parseQueue(stack, instructions, definitions, pollQueue(instructions));
      pollQueue(instructions);
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
void jumpToEnd(Stack* stack, Queue* instructions, Definitions* definitions) {
  int jumpPoints[1] = {OP_END};
  jumpTo(stack, instructions, definitions, jumpPoints, 1);
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

void parseSTR(PARSE_FUNC_TYPE) {
  /* Parses a string. */
  /* The string size will be at the top of the stack, followed by `n` characters in ascii and then the terminating NULL character. */
  char *word = token->word;
  Stack *chars = newStack();
  int size = 0;
  int i;
  char current;
  int escapeCharacter = 0;
  int hasEnd = 0;
  for (i = 1; i < MAX_WORD_SIZE; i++) {
    current = word[i];
    if (current == '"') {
      hasEnd = 1;
      break;
    } else if (escapeCharacter == 1) {
      escapeCharacter = 0;
      /* Escape Characters */
      if (current == '\\') {
        pushStack(chars, '\\');
        size++;
      } else if (current == 'n') {
        pushStack(chars, '\n');
        size++;
      } else if (current == 't') {
        pushStack(chars, '\t');
        size++;
      } else {
        assertWithToken(0, "Invalid escape character.", token);
      }
    } else if (current == '\\') {
      escapeCharacter = 1;
    } else {
      pushStack(chars, current);
      size++;
    }
  }
  assertWithToken(hasEnd == 1, "String has no NULL terminating character.", token);
  pushStack(stack, 0);
  while (!isEmptyStack(chars)) {
    current = popStack(chars);
    pushStack(stack, current);
  }
  pushStack(stack, size);
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

void parsePOP(PARSE_FUNC_TYPE) {
  int top = popStack(stack);
  printf("%d", top);
}

void parseEMIT(PARSE_FUNC_TYPE) {
  int top = popStack(stack);
  printf("%c", top);
}

void parseSIZE(PARSE_FUNC_TYPE) {
  int size = stack->size;
  printf("%d", size);
}

void parsePRINT(PARSE_FUNC_TYPE) {
  int size = popStack(stack);
  char word[MAX_WORD_SIZE];
  int i;
  for (i = 0; i < size + 1; i++) {
    word[i] = popStack(stack);
  }
  printf("%s", word);
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

void parseIF(PARSE_FUNC_TYPE) {
  int startType = token->OP_TYPE;
  if (startType == OP_END) {
    return;
  }
  assertWithToken(startType == OP_IF || startType == OP_ELSEIF, "controlIf must be called with `if` or `elseif` token", token);
  /* Evaluate until `then` keyword */
  int hasThen = 0;
  while (!isEmptyQueue(instructions)) {
    QueueElem *queueElem = pollQueue(instructions);
    Token *current = queueElem->token;
    if (current->OP_TYPE == OP_THEN) {
      hasThen = 1;
      break;
    }
    parseQueue(stack, instructions, definitions, queueElem);
  }
  assertWithToken(hasThen, "`then` not found after `if` or `elseif`", token);
  int truth = popStack(stack);
  if (truth == 0) {
    /* Jump to next block for evaluation. */
    int jumpPoints[2] = {OP_ELSEIF, OP_END};
    jumpTo(stack, instructions, definitions, jumpPoints, 2);
    Token* next = pollQueue(instructions)->token;
    parseIF(stack, instructions, definitions, next);
  } else {
    int hasEnd = 0;
    while (!isEmptyQueue(instructions)) {
      QueueElem *queueElem = pollQueue(instructions);
      Token *current = queueElem->token;
      if (current->OP_TYPE == OP_ELSEIF) {
        jumpToEnd(stack, instructions, definitions);
        /* Poll `end` word off. */
        current = pollQueue(instructions)->token;
      }
      if (current->OP_TYPE == OP_END) {
        hasEnd = 1;
        break;
      }
      parseQueue(stack, instructions, definitions, queueElem);
    }
    assertWithToken(hasEnd, "`end` not found after `if` or `elseif`", token);
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
    if (type == OP_IF || type == OP_WHILE) {
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
      parseQueue(stack, evalQueue, definitions, pollQueue(evalQueue));
    }
    truth = popStack(stack);
    if (truth == 0) {
      break;
    }

    Queue *loopQueue = copyQueue(loopInstructions);
    while (!isEmptyQueue(loopQueue)) {
      parseQueue(stack, loopQueue, definitions, pollQueue(loopQueue));
    }
  }
}

void parseTHEN(PARSE_FUNC_TYPE) {
  assertWithToken(0, "`then` word without starting", token);
}

void parseEND(PARSE_FUNC_TYPE) {
  assertWithToken(0, "`end` word without starting.", token);
}

int validateWordName(char *word) {
  char first = word[0];
  if ('0' <= first || first <= '9') {
    /* first character cannot be a number */
    return 1;
  }
  /* no character in the word can be any of these */
  char invalidChars[] = {'"'};
  int invalidCharsSize = 1;
  char *w = word;
  while (*w != '\0') {
    int i;
    for (i = 0; i < invalidCharsSize; i++) {
      if (*w == invalidChars[i]) {
        return 0;
      }
    }
    w++;
  }
  return 1;
}

void parseDEFWORD(PARSE_FUNC_TYPE) {
  Token *wordNameToken = pollQueue(instructions)->token;
  assertWithToken(wordNameToken->OP_TYPE == OP_UNKNOWN, "Word must not be defined before.", wordNameToken);
  char *wordName = wordNameToken->word;
  assertWithToken(validateWordName(wordName) == 1, "Word name contains invalid characters.", wordNameToken);
  int hasEndDef = 0;
  Queue *block = newQueue();
  Token *defToken;
  int type;
  while (!isEmptyQueue(instructions)) {
    defToken = pollQueue(instructions)->token;
    type = defToken->OP_TYPE;
    assertWithToken(type != OP_DEFWORD, "No nested `def`", defToken);
    if (type == OP_ENDDEF) {
      hasEndDef = 1;
      break;
    }
    pushQueue(block, defToken);
  }
  assertWithToken(hasEndDef, "`enddef` not found after `def`", token);
  addDefinition(definitions, wordName, block);
}

void parseENDDEF(PARSE_FUNC_TYPE) {
  assertWithToken(0, "`enddef` without `def`", token);
}

/* Parses a token. */
void parseQueue(Stack* stack, Queue* instructions, Definitions* definitions, QueueElem* queueElem) {
  assert(OPS_COUNT == 30, "Update control flow in parse().");
  Token *token = queueElem->token;
  void (*parsers[OPS_COUNT]) (PARSE_FUNC_TYPE) = {
    parseUNKNOWN,
    parseINT,
    parseSTR,
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
    parsePOP,
    parseEMIT,
    parseSIZE,
    parsePRINT,
    parseDUP,
    parseDROP,
    parseSWAP,
    parseOVER,
    parseROT,
    parseIF,
    parseELSEIF,
    parseWHILE,
    parseTHEN,
    parseEND,
    parseDEFWORD,
    parseENDDEF,
  };
  parsers[token->OP_TYPE](stack, instructions, definitions, token);
}

Token* makeToken(int row, int col, char *word) {
  Token* token;
  token = (Token*) malloc(sizeof(Token));
  token->row = row;
  token->col = col;
  token->value = 0;
  token->OP_TYPE = OP_UNKNOWN;
  strncpy(token->word, word, MAX_WORD_SIZE);
  assert(OPS_COUNT == 30, "Update control flow in makeToken().");
  /* control flow to decide type of operation */
  char *types[OPS_COUNT] = {
    "", /* UNKNOWN */
    "", /* INT */
    "", /* STR */
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
    ".",
    "emit",
    ".s",
    "print",
    "dup",
    "drop",
    "swap",
    "over",
    "rot",
    "if",
    "elseif",
    "while",
    "then",
    "end",
    "def",
    "enddef",
  };
  if (isNumber(word)) {
    token->OP_TYPE = OP_INT;
    token->value = atoi(word);
  } else if (isString(word)) {
    token->OP_TYPE = OP_STR;
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
    int parsingString = 0; /* Different behaviour when parsing strings. */
    while ((lengthOfLine = getline(&line, &len, source)) != -1) {
      int wordIndex = 0;
      memset(word, 0, sizeof(word));
      int lineIndex;
      for (lineIndex = 0; lineIndex < lengthOfLine; lineIndex++) {
        char c = line[lineIndex];
        /* Catch comments and ignore the rest (by exiting for loop). */
        if (parsingString == 1) {
          if (c == '"') {
            word[wordIndex++] = c;
            parsingString = 0;
            Token *token = makeToken(row, lineIndex - wordIndex, word);
            pushQueue(instructions, token);
            wordIndex = 0;
            memset(word, 0, sizeof(word));
          } else {
            assert(wordIndex < MAX_WORD_SIZE, "Word is too long!");
            word[wordIndex++] = c;
          }
        } else if (c == '"') {
          word[wordIndex++] = c;
          parsingString = 1;
        } else if (c == '-' && lineIndex < lengthOfLine - 1 && line[lineIndex+1] == '-') {
          break;
        } else if (c == ' ' || c == '\n') {
          if (wordIndex == 0) {
            /* Skipping multiple spaces. */
            continue;
          }
          Token *token = makeToken(row, lineIndex - wordIndex, word);
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

  while (!isEmptyQueue(instructions)) {
    parseQueue(stack, instructions, definitions, pollQueue(instructions));
  }

  return 0;
}
