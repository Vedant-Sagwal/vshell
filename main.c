#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>

#define VSHELL_BUFSIZE 1024

char *vshell_read_line(void) {
  int buffersize = VSHELL_BUFSIZE;
  int position = 0;
  char *buffer = malloc(sizeof(char) * buffersize);
  int c;
  if (!buffer) {
    fprintf(stderr, "VSHELL: Allocation Error!!");
    exit(EXIT_FAILURE);
  }

  while (1) {
    c = getchar();
    if (c == EOF || c == '\n') {
      // add a delimiter
      buffer[position] = '\0';
      return buffer;
    } else {
      buffer[position] = c;
    }
    position++;

    // if buffersize exceeds
    if (position >= buffersize) {
      buffersize += VSHELL_BUFSIZE;
      buffer = realloc(buffer, buffersize);
      if (!buffer) {
        fprintf(stderr, "VSHELL : Allocation Error!!");
        exit(EXIT_FAILURE);
      }
    }
  }
}

#define VSHELL_TOK_BUFFSIZE 64
#define VSHELL_TOK_DELM " \t\r\n\a"

char **vshell_split_line(char *line) {
  // parsing of the line
  int buffersize = VSHELL_TOK_BUFFSIZE;
  char **tokens = malloc(buffersize * sizeof(char *));
  int position = 0;
  char *token;
  if (!tokens) {
    fprintf(stderr, "Vshell : Allocation Error!!");
    exit(EXIT_FAILURE);
  }
  token = strtok(line, VSHELL_TOK_DELM);
  while (token != NULL) {
    tokens[position] = token;
    position++;

    if (position >= buffersize) {
      buffersize += VSHELL_TOK_BUFFSIZE;
      tokens = realloc(tokens, sizeof(char *));
      if (!tokens) {
        fprintf(stderr, "VSHELL : Allocation Error");
      }
    }
    token = strtok(line, VSHELL_TOK_DELM);
  }
  tokens[position] = token;
  return tokens;
}

// execute the arguments
int vshell_launch(char **args) {
  pid_t pid, wpid;
  int status;

  pid = fork();
  if (pid == 0) {
    // child process
    if (execvp(args[0], args) == -1) {
      perror("vshell");
    }
    exit(EXIT_FAILURE);
  } else if (pid < 0) {
    // error in forking
    perror("vshell");
  } else {
    do {
      wpid = waitpid(pid, &status, WUNTRACED);
    } while (!WIFEXITED(status) && !WIFSIGNALED(status));
  }
  return 1;
}

// some builtin functions
int vshell_cd(char **args);
int vshell_help(char **args);
int vshell_exit(char **args);

char *builtin_str[] = {"cd", "help", "exit"};
int (*builtin_func[])(char **) = {&vshell_cd, &vshell_help, &vshell_exit};

int vshell_builtins_num() { return sizeof(builtin_str) / sizeof(char *); }

int vshell_cd(char **args) {
  if (args[1] == NULL) {
    fprintf(stderr, "vshell expected arguments to \"cd\"\n ");
  } else {
    if (chdir(args[1]) != 0) {
      perror("vshell");
    }
  }
  return 1;
}

int vshell_help(char **args) {
  int i;
  printf("VSHELL made by Vedant Sagwal");
  printf("Type Command and argument and hit enter and see the magic");
  printf("Built in options : ");
  for (int i = 0; i < vshell_builtins_num(); i++) {
    printf("%s\n ", builtin_str[i]);
  }
  printf("Use man command for information on other programs");
  return 1;
}

int vshell_exit(char **args) { return 0; }

int vshell_execute(char **args) {
  int i;
  if (args[0] == NULL) {
    return 1;
  }
  for (i = 0; i < vshell_builtins_num(); i++) {
    if (strcmp(args[0], builtin_str[i]) == 0) {
      return (*builtin_func[i])(args);
    }
  }
  return vshell_launch(args);
}

void vshell_loop(void) {
  char *line; // input
  char **args;
  int status;

  do {
    printf(">");
    line = vshell_read_line();
    args = vshell_split_line(line);
    status = vshell_execute(args);

    free(line);
    free(args);
  } while (status);
}
