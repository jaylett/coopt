/*
 * $Id: sopt.c,v 1.4 2000/01/02 23:05:01 james Exp $
 * sopt.c
 *
 * Implementation of coopt_sopt(), utility routine for coopt.
 * (c) Copyright James Aylett 1999-2000
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "coopt.h"

#define available(x) (written+x<bufsize-1) /* -1 to allow space for terminator */

#ifndef HAVE_STRSTR
char *strstr(char const *, char const *);
#endif

size_t coopt_sopt(char *buffer, size_t bufsize, struct coopt_return *ret,
                  int show_marker, struct coopt_state *state)
{
  size_t written=0;
  buffer[0]=0;

  if (ret==NULL || ret->marker==NULL)
    return written; /* illegally called, or no option processed within */

  if (ret->result==COOPT_RESULT_END)
    return written; /* no option to print */

  if (show_marker!=0)
  {
    if (available(strlen(ret->marker+1)))
      written+=sprintf(buffer+written, ret->marker+1);
    else
      return written;
  }

  switch (ret->result)
  {
   case COOPT_RESULT_END: /* no option to print */
    return written;
    break;
   case COOPT_RESULT_BADOPTION: /* recover option from ret->param */
    switch (ret->marker[0])
    {
      case 'S':
        if (available(1))
        {
          buffer[written++]=ret->param[0];
          buffer[written]=0;
        }
        else
          return written;
        break;
      case 'L':
        if (state->flags.allow_long_eq_params && state->long_eq!=NULL)
        {
	  /* Go up to state->long_eq within ret->param */
	  char *eq = strstr(ret->param, state->long_eq);
	  if (eq==NULL)
	  {
	    if (available(strlen(ret->param)))
	      written+=sprintf(buffer+written, ret->param);
	    else
	      return written;
	  }
	  else
	  {
	    if (available(eq - ret->param))
	    {
	      strncat(buffer, ret->param, eq - ret->param);
	      written=strlen(buffer);
	    }
	    else
	      return written;
	  }
        }
        else
        {
	  /* Go to end of ret->param */
	  if (available(strlen(ret->param)))
	    written+=sprintf(buffer+written, ret->param);
	  else
	    return written;
        }
    }
    break;
   case COOPT_RESULT_ERROR: /* this *may* have a fairly full option in it */
     if (ret->opt==NULL)
       return written;
     /* explicit fall-through */
   case COOPT_RESULT_AMBIGUOUSOPT: /* these will */
   case COOPT_RESULT_MULTIMIXED:
   case COOPT_RESULT_NOPARAM:
   case COOPT_RESULT_HADPARAM:
   case COOPT_RESULT_OKAY:
   case COOPT_RESULT_MISSINGPARAM: /* display ret->opt */
     switch (ret->marker[0])
     {
       case 'S':
         if (available(1))
         {
           buffer[written++]=ret->opt->short_option;
           buffer[written]=0;
         }
         else
           return written;
         break;
       case 'L':
         if (available(strlen(ret->opt->long_option)))
           written+=sprintf(buffer+written, ret->opt->long_option);
         else
           return written;
         break;
     }
     break;
#ifdef COOPT_DEBUG
   default:
     fprintf(stderr, "coopt internal error: coopt_sopt() passed unknown result code %i\n", ret->result);
     break;
#endif
  }

  return written;
}
