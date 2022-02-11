#include <dirent.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define IN_EXT ".stc"
#define OUT_EXT ".o"
#define printUsage fprintf(stderr, "Usage: `%s [-duv] [directory]` or `%s [-uv] [files...]\n", thisName, thisName)

/* Run tests on all files. */
static int testDirectory = 0;
/* Updates all the output files. */
static int forceUpdate = 0;
/* Logs standard output. */
static int verboseOutput = 0;

static char *thisName;

int compareFiles(FILE *, FILE *);
int chopEnd(char *, char *);
int runTest(char*);

int compareFiles(FILE *program, FILE *expected) {
  char c = 1, d = 1;
  int result = 1;

  do {
    if (c != EOF) {
      c = fgetc(program);
    }
    if (d != EOF) {
      d = fgetc(expected);
    }
    if (c != EOF && verboseOutput != 0) {
      fputc(c, stdout);
    }
    if (c != d) {
      result = 0;
    }
  } while (c != EOF || d != EOF);

  return result && c == EOF && d == EOF;
}

int runTest(char* fileName) {
  if (verboseOutput != 0) {
    fprintf(stdout, "\n[%s] Testing %s:\n", thisName, fileName);
  }

  FILE *program, *expected;
  char *command, *programFile;
  asprintf(&programFile, "%s%s", fileName, IN_EXT);
  if (access(programFile, R_OK) != 0) {
    /* Checks for read permission for programFile. */
    fprintf(stderr, "StackC Program File `%s%s` not found.\n", fileName, IN_EXT);
    return 0;
  }
  asprintf(&command, "./stackc %s 2>&1", programFile);
  program = popen(command, "r");
  if (program == NULL) {
    fprintf(stderr, "Something went wrong executing StackC Program File `%s%s`.\n", fileName, IN_EXT);
    return 0;
  }

  char *expectedFile;
  asprintf(&expectedFile, "%s%s", fileName, OUT_EXT);
  expected = fopen(expectedFile, "r");
  if (expected == NULL) {
    fprintf(stderr, "Expected Output File `%s` not found.\n", expectedFile);
    return 0;
  }

  int result = compareFiles(program, expected);

  int status = pclose(program);
  /* int errcode = WEXITSTATUS(status); */
  if (status == -1) {
    fprintf(stderr, "Error closing file with `pclose`.\n");
  }

  return result;
}

/* Returns 0 if ending not found. Will not be chopped. */
int chopEnd(char *string, char *ending) {
  size_t size = 0;
  char *c = string, *d = ending;
  while (1) {
    if (*d == '\0') {
      *(c - size) = '\0';
      return *c == '\0';
    } else if (*c == '\0') {
      return 0;
    } else if (*c == *d) {
      c++;
      d++;
      size++;
    } else {
      c++;
      d = ending;
      size = 0;
    }
  }
}

int main(int argc, char* argv[]) {
  thisName = argv[0];
  int opt;
  while ((opt = getopt(argc, argv, "duv")) != -1) {
    switch (opt) {
      case 'd': testDirectory = 1; break;
      case 'u': forceUpdate = 1; break;
      case 'v': verboseOutput = 1; break;
      default:
        printUsage;
        exit(1);
    }
  }

  if (verboseOutput != 0) {
    printf("testDirectory: %d forceUpdate: %d verboseOutput: %d\n", testDirectory, forceUpdate, verboseOutput);
  }

  int tests = 0;
  int passed = 0;

  if (testDirectory == 0) {
    int i;
    for (i = 1; i < argc; i++) {
      char *arg = argv[i];
      if (arg[0] == '-') {
        continue;
      }
      tests++;
      if (runTest(arg)) {
        passed++;
      }
    }
    if (tests == 0) {
      printUsage;
      return 1;
    }
  } else {
    char *testDir = NULL;
    int i;
    for (i = 1; i < argc; i++) {
      char *arg = argv[i];
      if (arg[0] != '-') {
        testDir = arg;
        break;
      }
    }
    if (testDir == NULL) {
      printUsage;
      return 1;
    }
    /* Add a `/` to the end of testDir. */
    asprintf(&testDir, "%s/", testDir);
    DIR *d = opendir(testDir);
    struct dirent *dir;
    if (d == NULL) {
      fprintf(stderr, "Directory %s not found.\n", testDir);
      return 1;
    }

    char *fileName;
    while((dir = readdir(d)) != NULL) {
      fileName = dir->d_name;
      if (chopEnd(fileName, IN_EXT)) {
        char *fileWithDir;
        asprintf(&fileWithDir, "%s%s", testDir, fileName);
        tests++;
        if (runTest(fileWithDir)) {
          passed++;
        }
      }
    }

    closedir(d);
  }

  fprintf(stdout, "Tests: %d/%d\n", passed, tests);
  if (passed == tests) {
    fprintf(stdout, "\n[%s] All tests passed! 🎉\n", thisName);
  }

  return 0;
}
