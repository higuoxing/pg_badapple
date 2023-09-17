#include "postgres.h"

#include "fmgr.h"
#include "libpq/libpq.h"
#include "miscadmin.h"
#include "storage/backendid.h"
#include "utils/elog.h"

PG_MODULE_MAGIC;

PG_FUNCTION_INFO_V1(play_badapple);

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

Datum play_badapple(PG_FUNCTION_ARGS) {

  char sharepath[MAXPGPATH];
  char badapple_txt[MAXPGPATH];
  char **badapple_lines;
  int numframes = 0;
  int numlines = 0;
  char badapple_frame[4000];

  get_share_path(my_exec_path, sharepath);
  snprintf(badapple_txt, MAXPGPATH, "%s/extension/badapple.txt", sharepath);

  badapple_lines = readfile(badapple_txt);
  for (int i = 0; badapple_lines[i]; i++)
    numlines++;
  numframes = numlines / 30;

  for (int frame = 0; frame < numframes; frame++) {
    int len = 0;
    MemSet(badapple_frame, 0, sizeof(badapple_frame));
    /* 30 lines per frame. */
    for (int row = 0; row < 30; ++row) {
      memcpy(&badapple_frame[len], badapple_lines[frame * 30 + row],
             strlen(badapple_lines[frame * 30 + row]));
      len += strlen(badapple_lines[frame * 30 + row]);
    }

    ereport(NOTICE, (errmsg("\033c" /* Reset device. */
                            "%s",
                            badapple_frame)));
    pg_usleep(20000);
    CHECK_FOR_INTERRUPTS();
  }

  PG_RETURN_VOID();
}
