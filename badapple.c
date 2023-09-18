#include "postgres.h"

#include "fmgr.h"
#include "miscadmin.h"
#include "utils/elog.h"

#include <time.h>

PG_MODULE_MAGIC;

PG_FUNCTION_INFO_V1(play_badapple);
PG_FUNCTION_INFO_V1(play_basketball);

/*
 * get the lines from a text file
 */
static char **readfile(const char *path) {
  FILE *infile;
  int maxlength = 1, linelen = 0;
  int nlines = 0;
  int n;
  char **result;
  char *buffer;
  int c;

  if ((infile = fopen(path, "r")) == NULL) {
    ereport(ERROR,
            (errmsg("could not open file \"%s\" for reading: %m", path)));
  }

  /* pass over the file twice - the first time to size the result */
  while ((c = fgetc(infile)) != EOF) {
    linelen++;
    if (c == '\n') {
      nlines++;
      if (linelen > maxlength)
        maxlength = linelen;
      linelen = 0;
    }
  }

  /* handle last line without a terminating newline (yuck) */
  if (linelen)
    nlines++;
  if (linelen > maxlength)
    maxlength = linelen;

  /* set up the result and the line buffer */
  result = (char **)palloc((nlines + 1) * sizeof(char *));
  buffer = (char *)palloc(maxlength + 1);

  /* now reprocess the file and store the lines */
  rewind(infile);
  n = 0;
  while (fgets(buffer, maxlength + 1, infile) != NULL && n < nlines)
    result[n++] = pstrdup(buffer);

  fclose(infile);
  pfree(buffer);
  result[n] = NULL;

  return result;
}

static void play(const char *filename) {
  char sharepath[MAXPGPATH];
  char play_txt[MAXPGPATH];
  char **play_lines;
  int numframes = 0;
  int numlines = 0;
  char play_frame[4000];
  struct timespec start_tm;

  get_share_path(my_exec_path, sharepath);
  snprintf(play_txt, MAXPGPATH, "%s/extension/%s", sharepath, filename);

  play_lines = readfile(play_txt);
  for (int i = 0; play_lines[i]; i++)
    numlines++;
  numframes = numlines / 30;

  if (clock_gettime(CLOCK_MONOTONIC, &start_tm) != 0)
    ereport(ERROR, (errmsg("clock_gettime() failed due to: %m")));

  for (;;) {
    int frame;
    int len = 0;
    float elapsed;
    struct timespec current_tm;
    MemSet(play_frame, 0, sizeof(play_frame));

    if (clock_gettime(CLOCK_MONOTONIC, &current_tm) != 0)
      ereport(ERROR, (errmsg("clock_gettime() failed due to: %m")));

    elapsed = (current_tm.tv_sec - start_tm.tv_sec) +
              ((current_tm.tv_nsec - start_tm.tv_nsec) / 1000000000.0);
    frame = (int)(elapsed * 10);

    if (frame >= numframes)
      break;

    /* 30 lines per frame. */
    for (int row = 0; row < 30; ++row) {
      memcpy(&play_frame[len], play_lines[frame * 30 + row],
             strlen(play_lines[frame * 30 + row]));
      len += strlen(play_lines[frame * 30 + row]);
    }

    ereport(NOTICE, (errmsg("\033c" /* Reset device. */
                            "%s",
                            play_frame)));
    pg_usleep(50000);
    CHECK_FOR_INTERRUPTS();
  }
}

Datum play_badapple(PG_FUNCTION_ARGS) {
  play("badapple.txt");
  PG_RETURN_VOID();
}

Datum play_basketball(PG_FUNCTION_ARGS) {
  play("basketball.txt");
  PG_RETURN_VOID();
}
