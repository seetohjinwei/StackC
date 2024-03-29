#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define DEF_SIZE 64
#define MAX_WORD_SIZE 1024
#define PARSE_FUNC_TYPE Stack* stack, Queue* instructions, Definitions* definitions, Token* token

static char *thisName;

typedef enum TYPE {
  TYPE_INT,
  TYPE_CHAR,
  TYPE_STR,
  TYPE_COUNT,
} TYPE;

typedef enum OPS {
  OP_UNKNOWN,
  OP_INT,
  OP_CHAR,
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
  OP_SIZE,
  OP_PSTACK,
  OP_DUP,
  OP_DROP,
  OP_SWAP,
  OP_OVER,
  OP_ROT,
  OP_IF,
  OP_ELSEIF,
  OP_WHILE,
  OP_THEN,
  OP_DEF,
  OP_END,
  OP_CAST_INT,
  OP_CAST_CHAR,
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
  fprintf(stderr, "-- [%s] Token --\n", thisName);
  fprintf(stderr, "Position: %d %d\n", token->row, token->col);
  fprintf(stderr, "OP_TYPE: %d\n", token->OP_TYPE);
  fprintf(stderr, "Value: %d\n", token->value);
  fprintf(stderr, "Word: %s\n", token->word);
}

/* Assert with Token. */
int assertWithToken(int truth, char *message, Token* token) {
  if (truth == 0) {
    fprintf(stderr, "[%s] Assertion Error: %s\n", thisName, message);
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
int peekStack(Stack* stack, Token* token) {
  assertWithToken(!isEmptyStack(stack), "Stack underflow while peeking stack.\n", token);
  return stack->root->value;
}

/* Pops the first element of the stack. */
int popStack(Stack* stack, Token* token) {
  assertWithToken(!isEmptyStack(stack), "Stack underflow while popping stack.\n", token);
  Node* poppedNode = stack->root;
  stack->root = poppedNode->next;
  stack->size--;
  return poppedNode->value;
}

/* Prints contents of a stack. */
void printStack(Stack* stack) {
  Node *node = stack->root;
  fprintf(stderr, "-- [%s] Stack (size: %d) --\n", thisName, stack->size);
  while (node != NULL) {
    fprintf(stderr, "%d ", node->value);
    node = node->next;
  }
  fprintf(stderr, "EOS\n");
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
  pushStack(stack, TYPE_INT);
}

void parseCHAR(PARSE_FUNC_TYPE) {
  pushStack(stack, token->value);
  pushStack(stack, TYPE_CHAR);
}

void parseSTR(PARSE_FUNC_TYPE) {
  /* Parses a string. */
  /* The string size will be at the top of the stack, followed by `n` characters in ascii and then the terminating NULL character. */
  char *word = token->word;
  Stack *chars = newStack();
  int hasEnd = 0, size = 0;
  char current;
  int i;
  for (i = 0; i < MAX_WORD_SIZE; i++) {
    current = word[i];
    if (current == '\0') {
      pushStack(stack, 0);
      hasEnd = 1;
      break;
    } else {
      pushStack(chars, current);
      size++;
    }
  }
  assertWithToken(hasEnd == 1, "String has no NULL terminating character.", token);
  while (!isEmptyStack(chars)) {
    current = popStack(chars, token);
    pushStack(stack, current);
  }
  pushStack(stack, size);
  pushStack(stack, TYPE_STR);
}

/* If both a or b are int, the result will be a int. Else, it will be a char. */
void parseADD(PARSE_FUNC_TYPE) {
  int a_type = popStack(stack, token);
  int a = popStack(stack, token);
  assertWithToken(a_type == TYPE_INT || a_type == TYPE_CHAR, "+ is only defined for int and char.", token);
  int b_type = popStack(stack, token);
  int b = popStack(stack, token);
  assertWithToken(b_type == TYPE_INT || b_type == TYPE_CHAR, "+ is only defined for int and char.", token);
  assertWithToken(a_type != TYPE_CHAR || b_type != TYPE_CHAR, "char char + not supported", token);
  pushStack(stack, b + a);
  if (a_type == TYPE_INT && b_type == TYPE_INT) {
    pushStack(stack, TYPE_INT);
  } else {
    pushStack(stack, TYPE_CHAR);
  }
}

void parseSUB(PARSE_FUNC_TYPE) {
  int a_type = popStack(stack, token);
  int a = popStack(stack, token);
  assertWithToken(a_type == TYPE_INT || a_type == TYPE_CHAR, "- is only defined for int and char.", token);
  int b_type = popStack(stack, token);
  int b = popStack(stack, token);
  assertWithToken(b_type == TYPE_INT || b_type == TYPE_CHAR, "- is only defined for int and char.", token);
  pushStack(stack, b - a);
  if (a_type == TYPE_INT && b_type == TYPE_INT) {
    pushStack(stack, TYPE_INT);
  } else if (a_type == TYPE_INT && b_type == TYPE_CHAR) {
    pushStack(stack, TYPE_CHAR);
  } else {
    assertWithToken(0, "- is only defined for int int - and char int -", token);
  }
}

void parseMUL(PARSE_FUNC_TYPE) {
  int a_type = popStack(stack, token);
  int a = popStack(stack, token);
  int b_type = popStack(stack, token);
  int b = popStack(stack, token);
  assertWithToken(a_type == TYPE_INT && b_type == TYPE_INT, "* is only defined for int", token);
  pushStack(stack, b * a);
  pushStack(stack, TYPE_INT);
}

void parseDIV(PARSE_FUNC_TYPE) {
  int a_type = popStack(stack, token);
  int a = popStack(stack, token);
  int b_type = popStack(stack, token);
  int b = popStack(stack, token);
  assertWithToken(a_type == TYPE_INT && b_type == TYPE_INT, "/ is only defined for int", token);
  pushStack(stack, b / a);
  pushStack(stack, TYPE_INT);
}

void parseREM(PARSE_FUNC_TYPE) {
  int a_type = popStack(stack, token);
  int a = popStack(stack, token);
  int b_type = popStack(stack, token);
  int b = popStack(stack, token);
  assertWithToken(a_type == TYPE_INT && b_type == TYPE_INT, "% is only defined for int", token);
  pushStack(stack, b % a);
  pushStack(stack, TYPE_INT);
}

int checkEquality(Stack *stack, Token *token) {
  int a_type = popStack(stack, token);
  if (a_type == TYPE_STR) {
    int sizeA = popStack(stack, token);
    char wordA[sizeA + 1];
    int i;
    for (i = 0; i < sizeA + 1; i++) {
      wordA[i] = popStack(stack, token);
    }
    int b_type = popStack(stack, token);
    assertWithToken(b_type == TYPE_STR, "Can only compare strings with each other (=)", token);
    int sizeB = popStack(stack, token);
    int result = sizeA == sizeB;
    char wordB[sizeB + 1];
    for (i = 0; i < sizeB + 1; i++) {
      wordB[i] = popStack(stack, token);
      if (result != 0 && wordA[i] != wordB[i]) {
        result = 0;
      }
    }
    return result;
  } else {
    int a = popStack(stack, token);
    int b_type = popStack(stack, token);
    int b = popStack(stack, token);
    assertWithToken((a_type == TYPE_INT || a_type == TYPE_CHAR) && (b_type == TYPE_INT || b_type == TYPE_CHAR), "Invalid types for =", token);
    return a == b;
  }
}

void parseEQU(PARSE_FUNC_TYPE) {
  int result = checkEquality(stack, token);
  pushStack(stack, result);
  pushStack(stack, TYPE_INT);
}

void parseNEQU(PARSE_FUNC_TYPE) {
  int result = !checkEquality(stack, token);
  pushStack(stack, result);
  pushStack(stack, TYPE_INT);
}

int checkLessThan(Stack *stack, Token *token, int swap) {
  int b_type = popStack(stack, token);
  int b = popStack(stack, token);
  int a_type = popStack(stack, token);
  int a = popStack(stack, token);
  assertWithToken((a_type == TYPE_INT || a_type == TYPE_CHAR) && (b_type == TYPE_INT || b_type == TYPE_CHAR), "Invalid types for inequalities", token);
  if (swap == 0) {
    return a < b;
  } else {
    return b < a;
  }
}

void parseGTE(PARSE_FUNC_TYPE) {
  /* !(a < b) == b <= a == a <= b */
  int result = !checkLessThan(stack, token, 0);
  pushStack(stack, result);
  pushStack(stack, TYPE_INT);
}

void parseLTE(PARSE_FUNC_TYPE) {
  /* !(b < a) == a <= b */
  int result = !checkLessThan(stack, token, 1);
  pushStack(stack, result);
  pushStack(stack, TYPE_INT);
}

void parseGT(PARSE_FUNC_TYPE) {
  /* b < a == a > b */
  int result = checkLessThan(stack, token, 1);
  pushStack(stack, result);
  pushStack(stack, TYPE_INT);
}

void parseLT(PARSE_FUNC_TYPE) {
  /* a < b */
  int result = checkLessThan(stack, token, 0);
  pushStack(stack, result);
  pushStack(stack, TYPE_INT);
}

void parsePOP(PARSE_FUNC_TYPE) {
  int type = popStack(stack, token);
  if (type == TYPE_INT) {
    int value = popStack(stack, token);
    printf("%d", value);
  } else if (type == TYPE_CHAR) {
    int value = popStack(stack, token);
    printf("%c", value);
  } else if (type == TYPE_STR) {
    int size = popStack(stack, token);
    int i;
    for (i = 0; i < size; i++) {
      fputc(popStack(stack, token), stdout);
    }
    int null = popStack(stack, token);
    assertWithToken(null == '\0', "String must have a null character at the end", token);
  } else {
    fprintf(stderr, "Invalid Type Code: %d\n", type);
    assertWithToken(0, "Invalid type code (.)", token);
  }
}

void parseSIZE(PARSE_FUNC_TYPE) {
  int size = stack->size;
  printf("%d", size);
}

void parsePSTACK(PARSE_FUNC_TYPE) {
  printStack(stack);
}

/* ABC (n == 2) -> ABCBC */
void copyNElements(Stack *stack, Node *node, int n) {
  if (n == 0) {
    return;
  }
  copyNElements(stack, node->next, n - 1);
  pushStack(stack, node->value);
}

void parseDUP(PARSE_FUNC_TYPE) {
  int top_type = peekStack(stack, token);
  if (top_type == TYPE_INT) {
    copyNElements(stack, stack->root, 2);
  } else if (top_type == TYPE_CHAR) {
    copyNElements(stack, stack->root, 2);
  } else if (top_type == TYPE_STR) {
    int size = stack->root->next->value;
    Node *node = stack->root;
    copyNElements(stack, node, size + 3);
  } else {
    fprintf(stderr, "Invalid Type Code: %d\n", top_type);
    assertWithToken(0, "Invalid type code (dup)", token);
  }
}

void parseDROP(PARSE_FUNC_TYPE) {
  int type = popStack(stack, token);
  if (type == TYPE_INT) {
    popStack(stack, token);
  } else if (type == TYPE_CHAR) {
    popStack(stack, token);
  } else if (type == TYPE_STR) {
    int size = popStack(stack, token);
    int i;
    for (i = 0; i < size; i++) {
      popStack(stack, token);
    }
    int null = popStack(stack, token);
    assertWithToken(null == '\0', "String must have a null character at the end", token);
  }
}

void parseSWAP(PARSE_FUNC_TYPE) {
  Node *a_type = stack->root;
  assertWithToken(a_type != NULL, "Not enough elements to swap", token);
  int numberToIterate = 0;
  /* numberToIterate from a_type to the node before b_type */
  if (a_type->value == TYPE_INT) {
    numberToIterate = 1;
  } else if (a_type->value == TYPE_CHAR) {
    numberToIterate = 1;
  } else if (a_type->value == TYPE_STR) {
    numberToIterate = a_type->next->value + 2;
  } else {
    fprintf(stderr, "Invalid Type Code: %d\n", a_type->value);
    assertWithToken(0, "Invalid type code (swap)", token);
  }
  Node *before_b_type = a_type;
  while (numberToIterate-- > 0) {
    assertWithToken(before_b_type != NULL, "Not enough elements to swap", token);
    before_b_type = before_b_type->next;
  }
  /* numberToIterate from b_type to the node before after */
  Node *b_type = before_b_type->next;
  assertWithToken(b_type != NULL, "Not enough elements to swap", token);
  if (b_type->value == TYPE_INT) {
    numberToIterate = 2;
  } else if (b_type->value == TYPE_CHAR) {
    numberToIterate = 2;
  } else if (b_type->value == TYPE_STR) {
    numberToIterate = b_type->next->value + 3;
  } else {
    fprintf(stderr, "Invalid Type Code: %d\n", b_type->value);
    assertWithToken(0, "Invalid type code (swap)", token);
  }
  Node *before_after = before_b_type;
  while (numberToIterate-- > 0) {
    assertWithToken(before_after != NULL, "Not enough elements to swap", token);
    before_after = before_after->next;
  }
  stack->root = before_b_type->next;
  before_b_type->next = before_after->next;
  before_after->next = a_type;
}

void parseOVER(PARSE_FUNC_TYPE) {
  int top_type = peekStack(stack, token);
  int numberToIterate = 0;
  if (top_type == TYPE_INT) {
    numberToIterate = 2;
  } else if (top_type == TYPE_CHAR) {
    numberToIterate = 2;
  } else if (top_type == TYPE_STR) {
    numberToIterate = stack->root->next->value + 3;
  } else {
    fprintf(stderr, "Invalid Type Code: %d\n", top_type);
    assertWithToken(0, "Invalid type code (over)", token);
  }
  Node *node = stack->root;
  while (numberToIterate-- > 0) {
    node = node->next;
    assertWithToken(node != NULL, "Not enough elements to over", token);
  }
  int second_type = node->value;
  if (second_type == TYPE_INT) {
    copyNElements(stack, node, 2);
  } else if (second_type == TYPE_CHAR) {
    copyNElements(stack, node, 2);
  } else if (second_type == TYPE_STR) {
    int size = node->next->value;
    copyNElements(stack, node, size + 3);
  } else {
    fprintf(stderr, "Invalid Type Code: %d\n", second_type);
    assertWithToken(0, "Invalid type code (over)", token);
  }
}

void parseROT(PARSE_FUNC_TYPE) {
  int a_type = peekStack(stack, token);
  Node *root = stack->root;
  int numberToIterate = 0;
  if (a_type == TYPE_INT) {
    numberToIterate = 2;
  } else if (a_type == TYPE_CHAR) {
    numberToIterate = 2;
  } else if (a_type == TYPE_STR) {
    int size = root->next->value;
    numberToIterate = size + 3;
  } else {
    fprintf(stderr, "Invalid Type Code: %d\n", a_type);
    assertWithToken(0, "Invalid type code (rot)", token);
  }
  Node *node = root;
  while (numberToIterate-- > 0) {
    node = node->next;
    assertWithToken(node != NULL, "Not enough elements to rot", token);
  }
  int b_type = node->value;
  if (b_type == TYPE_INT) {
    numberToIterate = 1;
  } else if (b_type == TYPE_CHAR) {
    numberToIterate = 1;
  } else if (b_type == TYPE_STR) {
    int size = node->next->value;
    numberToIterate = size + 2;
  } else {
    fprintf(stderr, "Invalid Type Code: %d\n", b_type);
    assertWithToken(0, "Invalid type code (rot)", token);
  }
  while (numberToIterate-- > 0) {
    node = node->next;
    assertWithToken(node != NULL, "Not enough elements to rot", token);
  }
  /* node is now before c-type. */
  assertWithToken(node->next != NULL, "Not enough elements to rot", token);
  int c_type = node->next->value;
  if (c_type == TYPE_INT) {
    int count = 2;
    numberToIterate = count;
    stack->size -= count;
    copyNElements(stack, node->next, count);
  } else if (c_type == TYPE_CHAR) {
    int count = 2;
    numberToIterate = count;
    stack->size -= count;
    copyNElements(stack, node->next, count);
  } else if (c_type == TYPE_STR) {
    int size = node->next->next->value;
    int count = size + 3;
    numberToIterate = count;
    stack->size -= count;
    copyNElements(stack, node->next, count);
  } else {
    fprintf(stderr, "Invalid Type Code: %d\n", c_type);
    assertWithToken(0, "Invalid type code (rot)", token);
  }
  Node *after = node->next;
  while (numberToIterate-- > 0) {
    assertWithToken(after != NULL, "Not enough elements to rot", token);
    after = after->next;
  }
  node->next = after;
}

void parseIF(PARSE_FUNC_TYPE) {
  int startType = token->OP_TYPE;
  if (startType == OP_END) {
    return;
  }
  assertWithToken(startType == OP_IF || startType == OP_ELSEIF, "controlIf must be called with `if` or `elseif` token", token);
  /* Evaluate until `then` keyword */
  int hasThen = 0, type;
  while (!isEmptyQueue(instructions)) {
    QueueElem *queueElem = pollQueue(instructions);
    Token *current = queueElem->token;
    type = current->OP_TYPE;
    assertWithToken(type != OP_DEF, "No `def` in if", token);
    if (type == OP_THEN) {
      hasThen = 1;
      break;
    }
    parseQueue(stack, instructions, definitions, queueElem);
  }
  assertWithToken(hasThen, "`then` not found after `if` or `elseif`", token);
  int truth_type = popStack(stack, token);
  assertWithToken(truth_type == TYPE_INT, "`then` must pop an integer/boolean.", token);
  int truth = popStack(stack, token);
  if (truth == 0) {
    /* Jump to next block (elseif or end) for evaluation. */
    int ends = 1, jumpType;
    QueueElem *jumpElem;
    Token *jumpToken;
    while (ends > 0 && !isEmptyQueue(instructions)) {
      jumpElem = pollQueue(instructions);
      jumpToken = jumpElem->token;
      jumpType = jumpToken->OP_TYPE;
      assertWithToken(jumpType != OP_DEF, "No `def` in if", jumpToken);
      if (jumpType == OP_IF || jumpType == OP_WHILE) {
        ends++;
      } else if (jumpType == OP_END) {
        ends--;
      }
      if (jumpType == OP_ELSEIF || ends == 0) {
        /* Jumps to either elseif block or end of if. */
        break;
      }
    }
    parseIF(stack, instructions, definitions, jumpToken);
  } else {
    int hasEnd = 0, type;
    while (!isEmptyQueue(instructions)) {
      QueueElem *queueElem = pollQueue(instructions);
      Token *current = queueElem->token;
      type = current->OP_TYPE;
      assertWithToken(type != OP_DEF, "No `def` in if", token);
      if (type == OP_ELSEIF) {
        /* Jump to end. */
        int ends = 1, jumpType;
        QueueElem *jumpElem;
        Token *jumpToken;
        while (ends > 0 && !isEmptyQueue(instructions)) {
          jumpElem = pollQueue(instructions);
          jumpToken = jumpElem->token;
          jumpType = jumpToken->OP_TYPE;
          assertWithToken(jumpType != OP_DEF, "No `def` in if", jumpToken);
          if (jumpType == OP_IF || jumpType == OP_WHILE) {
            ends++;
          } else if (jumpType == OP_END) {
            ends--;
          }
          if (ends == 0) {
            /* Jumps to end of if. */
            break;
          }
        }
        return;
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
    assertWithToken(type != OP_DEF, "No `def` in while loop", token);
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
  while (1) {
    /* Evaluate evalInstuctions. */
    Queue *evalQueue = copyQueue(evalInstructions);
    while (!isEmptyQueue(evalQueue)) {
      parseQueue(stack, evalQueue, definitions, pollQueue(evalQueue));
    }
    int truth_type = popStack(stack, token);
    assertWithToken(truth_type == TYPE_INT, "`then` must pop an integer/boolean.", token);
    int truth = popStack(stack, token);
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

void parseCASTINT(PARSE_FUNC_TYPE) {
  int type = popStack(stack, token);
  assertWithToken(type == TYPE_CHAR, "Only can cast char -> int.", token);
  pushStack(stack, TYPE_INT);
}

void parseCASTCHAR(PARSE_FUNC_TYPE) {
  int type = popStack(stack, token);
  assertWithToken(type == TYPE_INT, "Only can cast int -> char.", token);
  pushStack(stack, TYPE_CHAR);
}

int validateWordName(char *word) {
  char first = word[0];
  if ('0' <= first && first <= '9') {
    /* first character cannot be a number */
    return 0;
  }
  /* no character in the word can be any of these */
  #define invalidCharsSize 2
  char invalidChars[invalidCharsSize] = {'"', '\''};
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

void parseDEF(PARSE_FUNC_TYPE) {
  Token *wordNameToken = pollQueue(instructions)->token;
  assertWithToken(wordNameToken->OP_TYPE == OP_UNKNOWN, "Word must not be defined before.", wordNameToken);
  char *wordName = wordNameToken->word;
  assertWithToken(validateWordName(wordName) == 1, "Word name contains invalid characters.", wordNameToken);
  int ends = 1, hasEnd = 0, type;
  Queue *block = newQueue();
  Token *defToken;
  while (!isEmptyQueue(instructions)) {
    defToken = pollQueue(instructions)->token;
    type = defToken->OP_TYPE;
    assertWithToken(type != OP_DEF, "No nested `def`", defToken);
    if (type == OP_IF || type == OP_WHILE) {
      ends++;
    } else if (type == OP_END) {
      ends--;
    }
    if (ends == 0) {
      hasEnd = 1;
      break;
    }
    pushQueue(block, defToken);
  }
  assertWithToken(hasEnd, "`end` not found after `def`", token);
  addDefinition(definitions, wordName, block);
}

/* Parses a token. */
void parseQueue(Stack* stack, Queue* instructions, Definitions* definitions, QueueElem* queueElem) {
  assert(OPS_COUNT == 31, "Update control flow in parse().");
  Token *token = queueElem->token;
  void (*parsers[OPS_COUNT]) (PARSE_FUNC_TYPE) = {
    parseUNKNOWN,
    parseINT,
    parseCHAR,
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
    parseSIZE,
    parsePSTACK,
    parseDUP,
    parseDROP,
    parseSWAP,
    parseOVER,
    parseROT,
    parseIF,
    parseELSEIF,
    parseWHILE,
    parseTHEN,
    parseDEF,
    parseEND,
    parseCASTINT,
    parseCASTCHAR,
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
  assert(OPS_COUNT == 31, "Update control flow in makeToken().");
  /* control flow to decide type of operation */
  char *types[OPS_COUNT] = {
    "", /* UNKNOWN */
    "", /* INT */
    "", /* CHAR */
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
    ".s",
    ".stack",
    "dup",
    "drop",
    "swap",
    "over",
    "rot",
    "if",
    "elseif",
    "while",
    "then",
    "def",
    "end",
    "(int)",
    "(char)",
  };
  if (isNumber(word)) {
    token->OP_TYPE = OP_INT;
    token->value = atoi(word);
    /* isString and isCharacter are overridden anyways. */
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

int endsWith(char *string, char *ending) {
  char *c = string, *d = ending;
  while (1) {
    if (*d == '\0') {
      return *c == '\0';
    } else if (*c == '\0') {
      return 0;
    } else if (*c == *d) {
      c++;
      d++;
    } else {
      c++;
      d = ending;
    }
  }
}

/* Main Function */
int main(int argc, char* argv[]) {
  thisName = argv[0];
  assert(argc > 1, "Not enough arguments.\nUsage: `./stackc filename`");
  Queue *instructions = newQueue();
  Stack *stack = newStack();
  Definitions *definitions = newDefinitions();
  /* word size is limited to MAX_WORD_SIZE */
  char word[MAX_WORD_SIZE];
  memset(word, 0, sizeof(word));

  char* filename = argv[1];
  assert(endsWith(filename, ".stc"), "File must have \".stc\" extension.");

  if (access(filename, R_OK) != 0) {
    /* Checks for read permission for programFile. */
    fprintf(stderr, "[%s] StackC Program File `%s` not found.\n", thisName, filename);
    return 1;
  }
  FILE *source;
  source = fopen(filename, "r");

  if (source == NULL) {
    char *message;
    asprintf(&message, "[%s] StackC Program File %s not found.", thisName, filename);
    assert(source == NULL, message);
  }

  /* Parsing input line by line. */
  char *line = NULL;
  /* Signed integer, includes new line character. */
  int row = 0;
  ssize_t lengthOfLine;
  size_t len = 0;
  int parsingString = 0; /* Different behaviour when parsing strings. */
  int parsingChar = 0; /* Different behaviour when parsing chars. */
  while ((lengthOfLine = getline(&line, &len, source)) != -1) {
    int wordIndex = 0;
    memset(word, 0, sizeof(word));
    int lineIndex;
    for (lineIndex = 0; lineIndex < lengthOfLine; lineIndex++) {
      char c = line[lineIndex];
      /* Catch comments and ignore the rest (by exiting for loop). */
      if (c == '\'' && wordIndex == 0) {
        char next = line[++lineIndex];
        if (next == '\\') {
          char escape = line[++lineIndex];
          if (escape == '\\') {
            next = '\\';
          } else if (escape == 'n') {
            next = '\n';
          } else if (escape == 'r') {
            next = '\r';
          } else if (escape == 't') {
            next = '\t';
          } else if (escape == '"') {
            next = '"';
          } else if (escape == '\'') {
            next = '\'';
          } else {
            fprintf(stderr, "[%s] Ascii of: %d\n", thisName, escape);
            assert(0, "Unknown Escape Character");
          }
        }
        char *assertMessage;
        asprintf(&assertMessage, "Invalid character at %d %d", row + 1, lineIndex - wordIndex + 1);
        assert(line[++lineIndex] == '\'', assertMessage);
        word[0] = next;
        word[1] = '\0';
        /* C ensures that next is a valid ascii because it is typed as a char here. */
        Token *token = makeToken(row + 1, lineIndex - wordIndex + 1, word);
        pushQueue(instructions, token);
        /* We already know this is a character. */
        token->OP_TYPE = OP_CHAR;
        token->value = next;
        wordIndex = 0;
      } else if (parsingString == 1) {
        if (c == '\\') {
          /* Handle Escape Characters */
          char next = line[++lineIndex];
          if (next == '\\') {
            word[wordIndex++] = '\\';
          } else if (next == 'n') {
            word[wordIndex++] = '\n';
          } else if (next == 'r') {
            word[wordIndex++] = '\r';
          } else if (next == 't') {
            word[wordIndex++] = '\t';
          } else if (next == '"') {
            word[wordIndex++] = '"';
          } else if (next == '\'') {
            word[wordIndex++] = '\'';
          } else {
            fprintf(stderr, "[%s] Ascii of: %d\n", thisName, next);
            assert(0, "Unknown Escape Character");
          }
        } else if (c == '"') {
          word[wordIndex++] = '\0';
          parsingString = 0;
          Token *token = makeToken(row + 1, lineIndex - wordIndex + 1, word);
          pushQueue(instructions, token);
          /* We already know this is a string. */
          token->OP_TYPE = OP_STR;
          wordIndex = 0;
          memset(word, 0, sizeof(word));
        } else {
          assert(wordIndex < MAX_WORD_SIZE, "Word is too long!");
          word[wordIndex++] = c;
        }
      } else if (c == '"' && parsingChar == 0) {
        parsingString = 1;
      } else if (c == '/' && lineIndex < lengthOfLine - 1 && line[lineIndex+1] == '/') {
        /* Catch comments and stop parsing. */
        break;
      } else if (c == ' ' || c == '\n') {
        if (wordIndex == 0) {
          /* Skipping multiple spaces. */
          continue;
        }
        Token *token = makeToken(row + 1, lineIndex - wordIndex + 1, word);
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

  while (!isEmptyQueue(instructions)) {
    parseQueue(stack, instructions, definitions, pollQueue(instructions));
  }

  return 0;
}
