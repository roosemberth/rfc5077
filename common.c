#include <unistd.h>
#include "common.h"

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>

static char current[2048];
static int running = 0;

#define BEGIN "\r\033[2K"
#define CHECK "\033[1;32m✔\033[0m"
#define BALLOT "\033[1;31m✘\033[0m"
#define WARN "\033[1;33m‼\033[0m"

static void
display(const char *sign, const char *format, va_list ap) {
  if (running) {
    running = 0;
    fprintf(stderr, BEGIN "[%s] %s%c\n", sign, current, format?':':'.');
    fflush(stderr);
  }

  if (format) {
    /* We indent the message */
    int   n = 0;
    int   size    = 0;
    char *message = NULL;
    char *cur;
    while (n >= size) {
      if ((message = realloc(message, size + 8192)) == NULL) return;
      size = size + 8192;
      if ((n = vsnprintf(message, size, format, ap)) == -1) return;
    }
    cur = message;
    fprintf(stderr, "    │ ");
    while (*cur) {
      if (*cur == '\n')
	fprintf(stderr, "\n    │ ");
      else
	fprintf(stderr, "%c", *cur);
      cur++;
    }
    fprintf(stderr, "\n");
    fflush(stderr);
    free(message);
  }
}

void
start(const char *format, ...) {
  va_list ap;
  int     n;
  if (running) end(NULL);

  /* Save the current message */
  va_start(ap, format);
  n = vsnprintf(current, sizeof(current), format, ap);
  va_end(ap);
  if (n == -1 || n >= sizeof(current))
    exit(EXIT_FAILURE);

  /* Display */
  fprintf(stderr, "[ ] %s ...", current);
  fflush(stderr);
  running = 1;
}

void
end(const char *format, ...) {
  va_list ap;
  va_start(ap, format);
  display(CHECK, format, ap);
  va_end(ap);
}

void
fail(const char *format, ...) {
  va_list ap;
  va_start(ap, format);
  display(BALLOT, format, ap);
  va_end(ap);
  exit(EXIT_FAILURE);
}

void
warn(const char *format, ...) {
  va_list ap;
  va_start(ap, format);
  display(WARN, format, ap);
  va_end(ap);
}

int
requestFromFile(FILE *fp, size_t *offset, char* headerBuff, char* bodyBuff){
	if (fp==NULL)
		return -1;
	ssize_t readLineLenght = 0;

	readLineLenght = getline(&headerBuff, offset, fp);
//	end("--- %d Bytes readed from file: %s", (int)readLineLenght, headerBuff);
	// EOF
	if (readLineLenght==-1)
		return 1;
	// White line, start over
	if (readLineLenght==0)
		return requestFromFile(fp, offset, headerBuff, bodyBuff);

	headerBuff[strcspn(headerBuff, "\r\n")] = 0;

	readLineLenght = getline(&bodyBuff, offset, fp);
//	end("--- %d Bytes readed from file: %s", (int)readLineLenght, bodyBuff);
	// EOF
	if (readLineLenght==-1)
		return 1;
	// White line, start over
	if (readLineLenght==0)
		return requestFromFile(fp, offset, headerBuff, bodyBuff);

	bodyBuff[strcspn(bodyBuff, "\r\n")] = 0;
	//start("Sending request %d from source file", (int)(sourcedLines%2+1));
	return 0;
}
