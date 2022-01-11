#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int assert(int truth, char *message) {
  if (truth == 0) {
    printf("ERROR: %s\n", message);
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
  OP_PEEK,
  OP_POP,
  OP_EMIT,
  OP_SIZE,
  OP_DUP,
  OP_DROP,
  OP_SWAP,
  OP_OVER,
  OPS_COUNT // size of enum OPS
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

/* Initialise a new stack. */
struct Stack* newStack(void) {
  struct Stack *stack;
  stack = (struct Stack*) malloc(sizeof(struct Stack));
  stack->size = 0;
  stack->root = NULL;
  return stack;
}

/* Checking if a stack is empty. */
int isEmpty(struct Stack* stack) {
  return stack->size == 0;
}

/* Push an integer onto a stack. */
int push(struct Stack* stack, int value) {
  struct Node* newNode;
  newNode = (struct Node*) malloc(sizeof(struct Node));
  newNode->value = value;
  newNode->next = stack->root;
  stack->root = newNode;
  stack->size++;
  return 1;
}

/* Peek at the first element of the stack. */
int peek(struct Stack* stack) {
  if (isEmpty(stack)) {
    assert(0, "Stack underflow.\n");
  }
  return stack->root->value;
}

/* Pops the first element of the stack. */
int pop(struct Stack* stack) {
  if (isEmpty(stack)) {
    assert(0, "Stack underflow.\n");
  }
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
void parse(struct Stack* stack, struct Token* token) {
  assert(OPS_COUNT == 14, "Update control flow in parse().");
  if (token->OP_TYPE == OP_INT) {
    push(stack, token->value);
  } else if (token->OP_TYPE == OP_ADD) {
    int a = pop(stack);
    int b = pop(stack);
    push(stack, b + a);
  } else if (token->OP_TYPE == OP_SUB) {
    int a = pop(stack);
    int b = pop(stack);
    push(stack, b - a);
  } else if (token->OP_TYPE == OP_MUL) {
    int a = pop(stack);
    int b = pop(stack);
    push(stack, b * a);
  } else if (token->OP_TYPE == OP_DIV) {
    int a = pop(stack);
    int b = pop(stack);
    push(stack, b / a);
  } else if (token->OP_TYPE == OP_PEEK) {
    int top = peek(stack);
    printf("%d ", top);
  } else if (token->OP_TYPE == OP_POP) {
    int top = pop(stack);
    printf("%d ", top);
  } else if (token->OP_TYPE == OP_EMIT) {
    int top = pop(stack);
    printf("%c", top);
  } else if (token->OP_TYPE == OP_SIZE) {
    int size = stack->size;
    printf("%d ", size);
  } else if (token->OP_TYPE == OP_DUP) {
    int top = peek(stack);
    push(stack, top);
  } else if (token->OP_TYPE == OP_DROP) {
    pop(stack);
  } else if (token->OP_TYPE == OP_SWAP) {
    int a = pop(stack);
    int b = pop(stack);
    push(stack, a);
    push(stack, b);
  } else if (token->OP_TYPE == OP_OVER) {
    int a = pop(stack);
    int second = peek(stack);
    push(stack, a);
    push(stack, second);
  } else {
    assert(0, "Unreachable code.");
  }
}

struct Token* makeToken(int pos, char *word) {
  struct Token* token;
  token = (struct Token*) malloc(sizeof(struct Token));
  token->pos = pos;
  token->value = 0;
  assert(OPS_COUNT == 14, "Update control flow in makeToken().");
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
  } else {
    token->OP_TYPE = OP_UNKNOWN;
    printf("ERROR: Word %s not implemented yet.\n", word);
    exit(1);
  }
  return token;
}

/* Main Function */
int main(int argc, char* argv[]) {
  if (argc < 2) {
    printf("ERROR: No file given.\n");
    printf("    Please use `./forth.out filename`\n");
    exit(1);
  }
  char* filename = argv[1];
  FILE *sourceptr;
  sourceptr = fopen(filename, "r");

  if (sourceptr == NULL) {
    printf("ERROR: File %s not found\n", filename);
    exit(1);
  }

  struct Stack* stack = newStack();
  // word size is limited to 256
  char word[256];
  int pos = 0;
  while(fscanf(sourceptr, "%s", word) != EOF) {
    struct Token *token = makeToken(pos, word);
    parse(stack, token);
    pos = ftell(sourceptr);
  }
  fclose(sourceptr);
  return 0;
}
