/*
 * $Id: sopt.c,v 1.1 1999/05/18 17:55:28 james Exp $
 * sopt.c
 * 
 * Implementation of coopt_sopt(), utility routine for coopt.
 * (c) Copyright James Aylett 1999
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "coopt.h"

/* In a different compilation unit (strstr.c) */
extern char *coopt_strstr(char const *, char const *);

#define available(x) (used+x<len)

void coopt_sopt(char *buffer, struct coopt_return *ret, int show_marker,
		struct coopt_state *state)
{
  if (ret==NULL || ret->marker==NULL)
    return; /* illegally called, or no option processed within */
  
  if (ret->result==COOPT_RESULT_END)
    return; /* no option to print */

  if (show_marker!=0)
    sprintf(buffer, ret->marker+1);

  switch (ret->result)
  {
   case COOPT_RESULT_END: /* no option to print */
    return;
    break;
   case COOPT_RESULT_BADOPTION: /* recover option from ret->param */
    switch (ret->marker[0])
    {
     case 'S':
      strncat(buffer, ret->param, 1);
      break;
     case 'L':
      if (state->flags.allow_long_eq_params)
      {
	/* Go up to state->long_eq within ret->param */
	char *eq = coopt_strstr(ret->param, state->long_eq);
	if (eq==NULL)
	  strcat(buffer, ret->param);
	else
	  strncat(buffer, ret->param, eq - ret->param);
      }
      else
      {
	/* Go to end of ret->param */
	strcat(buffer, ret->param);
      }
    }
    break;
   case COOPT_RESULT_ERROR: /* this *may* have a fairly full option in it */
   case COOPT_RESULT_AMBIGUOUSOPT: /* these will */
   case COOPT_RESULT_MULTIMIXED:
   case COOPT_RESULT_NOPARAM:
   case COOPT_RESULT_HADPARAM:
   case COOPT_RESULT_OKAY:
   case COOPT_RESULT_MISSINGPARAM: /* display ret->opt */
    switch (ret->marker[0])
    {
     case 'S':
      strncat(buffer, &(ret->opt->short_option), 1);
      break;
     case 'L':
      strcat(buffer, ret->opt->long_option);
      break;
    }
    break;
#ifdef COOPT_DEBUG
   default:
    fprintf(stderr, "coopt internal error: coopt_[sf]opt() passed unknown result code\n");
    break;
#endif
  }
}
