/*
 * $Id: serror.c,v 1.1 1999/08/10 15:28:42 james Exp $
 * serror.c
 * 
 * Implementation of coopt_serror(), utility routine for coopt.
 * (c) Copyright James Aylett 1999
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "coopt.h"

/*
 * Do we show markers in error strings?
 *  0 if we don't, 1 if we do
 */
#define SHOW_MARKERS 1

#define available(x) (written+x<bufsize-1) /* -1 to allow space for terminator */

#define writestr(x) \
	if (available(strlen(x))) \
	  written+=sprintf(buffer+written, x)
#define writeopt() \
        written+=coopt_sopt(buffer+written, bufsize-written, \
                            ret, SHOW_MARKERS, state)

/* Some strings that will be used below ... */
#define str_ERROR "Internal error while processing "
#define str_HADPARAM "Parameter given to "
#define str_MULTIMIXED "More than one parameter required in a block of short options"
#define str_AMBIGUOUSOPT "Ambiguous abbreviation "
#define str_BADOPTION "Unknown option "
#define str_NOPARAM "Required parameter ommited for "
#define str_MISSINGPARAM str_NOPARAM
#define str_OKAYARG "Argument "
#define str_OKAYOPT "Option "
#define str_END "End of options"

size_t coopt_serror(char *buffer, size_t bufsize, struct coopt_return *ret,
		    struct coopt_state *state)
{
  size_t written=0;
  buffer[0]=0;
  
  if (ret==NULL || state==NULL)
    return written; /* illegally called */
  
  switch (ret->result)
  {
   case COOPT_RESULT_ERROR:
    writestr(str_ERROR);
    writeopt();
    break;
   case COOPT_RESULT_HADPARAM:
    writestr(str_HADPARAM);
    writeopt();
    break;
   case COOPT_RESULT_MULTIMIXED:
    writestr(str_MULTIMIXED);
    break;
   case COOPT_RESULT_AMBIGUOUSOPT:
    writestr(str_AMBIGUOUSOPT);
    writeopt();
    break;
   case COOPT_RESULT_BADOPTION:
    writestr(str_BADOPTION);
    writeopt();
    break;
   case COOPT_RESULT_NOPARAM:
    writestr(str_NOPARAM);
    writeopt();
    break;
   case COOPT_RESULT_OKAY:
    if (ret->opt==NULL)
    {
      writestr(str_OKAYARG);
      writestr(ret->param);
    }
    else
    {
      writestr(str_OKAYOPT);
      writeopt();
    }
    break;
   case COOPT_RESULT_MISSINGPARAM:
    writestr_str(str_MISSINGPARAM);
    writeopt();
    break;
   case COOPT_RESULT_END:
    writestr(str_END);
    break;
#ifdef COOPT_DEBUG
   default:
    fprintf(stderr, "coopt internal error: coopt_serror() passed unknown result code %i\n", ret->result);
    break;
#endif
  }
  return written;
}
