/*
 * $Id: coopt.c,v 1.4 2000/02/13 21:06:35 james Exp $
 * coopt.c
 *
 * Implementation file for coopt, the Tartarus option parsing library
 * (c) Copyright James Aylett 1999-2000. All Rights Reserved.
 *
 *   Redistribution and use in source and binary forms, with or without
 *   modification, are permitted provided that the following conditions
 *   are met:
 *
 *   1.  Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *
 *   2.  Redistributions in binary form must reproduce the above
 *       copyright notice, this list of conditions and the following
 *       disclaimer in the documentation and/or other materials provided
 *       with the distribution.
 *
 *   3.  Neither name of coopt nor the names of its contributors may be
 *       used to endorse or promote products derived from this software
 *       without specific prior written permission.
 *
 *   THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 *   ``AS IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 *   LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 *   A PARTICULAR PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL THE
 *   CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 *   EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 *   PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 *   PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
 *   LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
 *   NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 *   SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 * Additional design and assistance: Simon Tatham, Ben Harris, Owen Dunn,
 * Chris Emerson, Richard Boulton.
 *
 * Problems: mixed short parameters and missing parameter won't work with
 * defaults (so don't allow it!). Need to note this in the manual. FIXME:
 * I'm no longer positive what this means ... ?
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "coopt.h"

/* Some utility routines we'll use later */
static struct coopt_return coopt_shortopt(struct coopt_state *);
static char *coopt_strstarts(char const *, char const *);
static char *coopt_strnstarts(char const *, char const *, size_t);

#ifndef HAVE_STRSTR
char *strstr(char const *, char const *);
#endif

/*
 * Initialise the coopt_state structure to (a) the user setup, and
 * (b) starting position with default options.
 */
void coopt_init(struct coopt_state *state,
		struct coopt_option const * options,
		unsigned int num_options,
		int argc, char const * const * argv)
{
  char const **markers;
  state->options = options;
  state->num_options = num_options;

  state->argc = argc;
  state->argv = argv;
  state->char_within_arg = 0;
  state->skip_next_arg = 0;
  state->last_marker = NULL;

  state->flags.allow_mix_short_params = 0;
  state->flags.allow_long_eq_params = 1;
  state->flags.allow_long_sep_params = 1;
  state->flags.allow_long_opts_breved = 0;
  state->separator = "--";
  state->long_eq = "=";

  markers=(char const **)malloc(3*sizeof(char *));
  if (markers!=NULL)
  {
    markers[0]="L--";
    markers[1]="S-";
    markers[2]=NULL;
  }
  state->markers = markers;
}

/*
 * Process the next option
 */

struct coopt_return coopt(struct coopt_state * state)
{
  struct coopt_return result;
  result.result=COOPT_RESULT_OKAY; /* Look mummy! Optimistic code! */
  result.ambigresult=COOPT_RESULT_OKAY; /* Look mummy! Optimistic code! */
  result.opt=NULL;
  result.param=NULL;
  result.marker=NULL;

  if (state==NULL || state->argv==NULL)
  {
    result.result=COOPT_RESULT_ERROR;
    return result;
  }

/*  printf("[coopt:entered with argc=%i, argv[0]=%p, char_within_arg=%i]\n",
	 state->argc, state->argv[0], state->char_within_arg);*/
/*  printf("[coopt:state->markers[0]=%p='%s']\n", state->markers[0],
	 state->markers[0]);*/

  if (state->argc<=0)
  {
    result.result = COOPT_RESULT_END;
    return result;
  }

  if (state->char_within_arg < 0)
  {
/*    printf("[coopt:automatic argument]\n");*/
    state->argc--; /* fewer left */
    result.param=state->argv++[0];
    return result;
  }

  if (state->char_within_arg == 0)
  {
    int marker;
    char const *m=NULL;

/*    printf("[coopt:first char]\n");*/
    /* first, are we supposed to skip this arg? */
    if (state->skip_next_arg)
    {
/*      printf("[coopt:skipping arg]\n");*/
      state->skip_next_arg=0; /* don't do it again! */
      state->argc--;
      state->argv++;
      return coopt(state);
    }
    /* start of a new option - we need to (a) find out if this is
     * the separator, and then (b) find out which marker we're using
     * then we can worry about what option it is
     */
    if (state->separator!=NULL && strcmp(state->argv[0], state->separator) == 0)
    {
/*      printf("[coopt:skipping separator]\n");*/
      state->argc--; /* skip over this separator, which */
      state->argv++; /* isn't return to the caller */
      state->char_within_arg = -1; /* will return the argument quickly */
      return coopt(state);
    }

    /* let's find out which marker is involved */
    marker=0;
    while (state->markers[marker]!=NULL && m==NULL)
    {
      /* returns NULL or pointer to the character after the end of the
       * second argument, found at the start of the first.
       */
      m = coopt_strstarts(state->argv[0], state->markers[marker]+1);
      if (m==NULL)
	marker++;
    }

    /* if we didn't find a marker, or we found a marker with no option
     * after it, we consider it to be an argument not an option.
     */
    if (state->markers[marker]==NULL || m[0]==0)
    {
      /* didn't find a marker - must be an argument */
      state->argc--;
      result.param=state->argv++[0];
      return result;
    }

    switch (state->markers[marker][0])
    {
      /* Put it here ... it's ick, but it'll do for the moment */
      int length_to_test;
      int i;
      struct coopt_option const *opt;
      int ambiguous;

     case 'S':
      state->last_marker = state->markers[marker];
      /* so subsequent short options have this set up correctly */
      state->char_within_arg = (m-state->argv[0]); /* skip marker */
      return coopt_shortopt(state);
      break;

     case 'L':
      /*
       * if we get a long option, we need to worry about allow_long_eq_params
       * and allow_long_opts_breved, both of which affect finding which
       * option we're talking about, and allow_long_sep_params, which affects
       * locating parameters.
       */
      opt=NULL;
      ambiguous=0;

      if (state->flags.allow_long_eq_params && state->long_eq!=NULL)
      {
	char const *r = strstr(m, state->long_eq);
	if (r!=NULL)
	{
/*	  printf("[coopt:found eq]\n");*/
	  length_to_test = r - m;
	}
	else
	{
/*	  printf("[coopt:no eq]\n");*/
	  length_to_test = strlen(m);
	}
      }
      else
      {
/*	printf("[coopt:eqs off]\n");*/
	length_to_test = strlen(m);
      }

      /* opt==NULL - so stop after we've found one
       * || state->allow_long_opts_breved - so don't actually stop if
       * we're allowing abbreviated options, because we want to fault
       * ambiguous abbreviations
       */
      for (i=0; i<state->num_options &&
	        (opt==NULL || state->flags.allow_long_opts_breved); i++)
      {
	if (state->options[i].long_option!=NULL)
	{
	  if (state->flags.allow_long_opts_breved)
	  {
/*	    printf("[coopt:length_to_test=%i]\n", length_to_test);*/
	    if (coopt_strnstarts(state->options[i].long_option,
				 m,length_to_test)!=NULL)
	    {
	      if (opt==NULL)
	      {
/*		printf("[coopt:breved opt]\n");*/
		/* Pointer arithmetic ... */
		opt=state->options + i;
	      }
	      else
	      {
/*		printf("[coopt:ambiguous opt]\n");*/
		ambiguous++;
	      }
	    }
	  }
	  else
	  {
	    /* We only want to test the section before the long_eq instance,
	     * if any. So the length of the option we're looking at must be
	     * the same as the space we're testing against.
	     * This could be done faster in our own testing routine ...
	     */
	    if (strlen(state->options[i].long_option)==length_to_test &&
		strncmp(m, state->options[i].long_option,length_to_test)==0)
	    {
/*	      printf("[coopt:long opt]\n");*/
	      /* Bleurgh, pointer arithmetic ... */
	      opt=state->options +i;
	    }
	  }
        }
      }

      /* Do this now because it's applicable to all subsequent */
      state->argc--;
      state->argv++;
      result.marker=state->markers[marker];

      if (opt==NULL)
      {
	/* Couldn't find it ... */
	result.result=COOPT_RESULT_BADOPTION;
	result.param=m;
	return result;
      }
      else
      {
	result.opt = opt;
	result.param = NULL;
	result.result = (ambiguous==0)?(COOPT_RESULT_OKAY):
	                (COOPT_RESULT_AMBIGUOUSOPT);

	/* parse the parameter whether or not the option wants it */
	if (m[length_to_test]!=0) /* so long_eq follows the long option */
	{
/*	  printf("[coopt: inline param]\n");*/
	  if (state->flags.allow_long_eq_params && state->long_eq!=NULL)
	  {
	    result.param = m+length_to_test;
	    /* Now skip the long_eq
	     * Note that checking we don't overrun the buffer is
	     * unnecessary because we used a strstr()-alike earlier to
	     * calculate length_to_test, so the whole of long_eq *must*
	     * be present at m+length_to_test
	     */
	    result.param += strlen(state->long_eq);
	  }
	  else
	  {
	    /* Something went wrong!
	     * This really shouldn't happen, because
	     * length_to_test=strlen(m) by definition if
	     * state->allow_long_eq_params is turned off!
	     */
	    result.result = COOPT_RESULT_ERROR;
	    return result;
	  }
	}

/*	printf("[coopt:parsing params]\n");*/
        /* Found it - let's process any parameter
         * note that we only do this if everything's fine, because otherwise
         * we might accidentally overwrite the *real* error code ...
         */
	if (opt->has_param == COOPT_REQUIRED_PARAM)
	{
/*	  printf("[coopt: param requested]\n");*/
	  if (m[length_to_test]==0) /* param follows in subsequent argument */
	  {
/*	    printf("[coopt: param follows]\n");*/
	    if (state->flags.allow_long_sep_params)
	    {
	      if (state->argc<=0) /* none to have ... */
	      {
/*	        printf("[coopt: none to have]\n");*/
	        if (result.result==COOPT_RESULT_OKAY)
	          result.result = COOPT_RESULT_MISSINGPARAM;
	        else
	          result.ambigresult = COOPT_RESULT_MISSINGPARAM;
	      }
	      else
	      {
/*	        printf("[coopt: got it]\n");*/
	        result.param = state->argv[0];
	        state->argc--;
	        state->argv++;
	      }
	    }
	    else
	    {
/*	      printf("[coopt: following params forbidden]\n");*/
              /* long_eq wasn't present, and the next argument isn't allowed
	       * to be a parameter. So long_eq and the parameter were missing.
	       */
	      if (result.result==COOPT_RESULT_OKAY)
	        result.result = COOPT_RESULT_NOPARAM;
	      else
	        result.ambigresult = COOPT_RESULT_NOPARAM;
	    }
	  }
	}
	else
	{
/*	  printf("[coopt:no param requested]\n");*/
	  if (m[length_to_test]!=0) /* so long_eq follows the long option */
	  {
/*            printf("[coopt:but there was one!]\n");*/
	    if (result.result==COOPT_RESULT_OKAY)
	      result.result = COOPT_RESULT_HADPARAM;
	    else
	      result.ambigresult = COOPT_RESULT_HADPARAM;
	  }
	  /* No param */
	}
	return result;
      }
      break;

     default:
      /* marker block was wrong ... */
      result.result = COOPT_RESULT_ERROR;
      return result;
      break;
    }
  }
  else
  {
    /* must be part of a run of short options. In other words we know
     * that state->argv[0][state->char_within_arg] should be an option
     * character.
     */
/*    printf("[coopt:mid-short run]\n");*/
    return coopt_shortopt(state);
  }
}

/*
 * if we get a short option, we need to worry about
 * allow_mix_short_params or its alternative, ie -cfv has "v" as param
 * to -f. The former can set skip_next_arg. If this is already set,
 * use COOPT_RESULT_MULTIMIXED.
 */
static struct coopt_return coopt_shortopt(struct coopt_state *state)
{
  int opt;
  struct coopt_return result;
  result.result=COOPT_RESULT_OKAY;
  result.ambigresult=COOPT_RESULT_OKAY;
  result.opt=NULL;
  result.param=NULL;
  result.marker=state->last_marker; /* always gets used */

  if (state->argv[0][state->char_within_arg]==0)
  {
    /* no more options here */
    state->argv++;
    state->argc--;
    state->char_within_arg=0;
    state->last_marker=NULL;
    return coopt(state);
  }

  for (opt=0; opt<state->num_options; opt++)
  {
    /* if short_option==0, it isn't a valid short option ... */
    if (state->options[opt].short_option!=0 && state->options[opt].short_option==state->argv[0][state->char_within_arg])
    {
      state->char_within_arg++;
      /* Bleurgh, pointer arithmetic ... */
      result.opt=state->options + opt; /* Always from now on in this block */
      /* Found it! Hooray! */
      if (state->options[opt].has_param == COOPT_REQUIRED_PARAM)
      {
/*	printf("[coopt:req param]\n");*/
	/* First case: this is the last short option in this argument, so
	 * its parameter *must* be the next argument. Second case: mixed
	 * short parameters are on, so the parameter again has to be in the
	 * next argument.
	 */
	if (state->argv[0][state->char_within_arg]==0
	    || state->flags.allow_mix_short_params)
	{
	  if (state->argc<=1) /* run out of arguments */
	  {
	    result.result=COOPT_RESULT_MISSINGPARAM;
	    return result;
	  }
	  if (state->skip_next_arg>0) /* already had this once! death! */
	  {
	    result.result = COOPT_RESULT_MULTIMIXED;
	    return result;
	  }
	  state->skip_next_arg=1;
	  result.param=state->argv[1];
	  return result;
	}
	else
	{
	  /* Parameter is inline as part of the current argument.
	   * state->char_within_arg is already right for this ...
	   */
	  result.param = state->argv[0] + state->char_within_arg;
	  state->argc--;
	  state->argv++;
	  state->char_within_arg=0;
	  state->last_marker=NULL;
	  return result;
	}
      }
      else
      {
	/* No parameter - just return (we've already skipped this one) */
	return result;
      }
    }
  }

  /* Didn't find one. Oh dear ... */
  result.result = COOPT_RESULT_BADOPTION;
  result.param = state->argv[0] + state->char_within_arg;
  state->char_within_arg++; /* skip the one we had trouble with */
  return result;
}

/*
 * returns NULL, or pointer to the character after the end of (start),
 * found at the start of (target). If (target) is shorter than (start),
 * this will return NULL.
 */
static char *coopt_strstarts(char const *target, char const *start)
{
  char a,b;
  do
  {
    a = target++[0];
    b = start++[0];
/*    printf("[[coopt: '%c' '%c']]\n", a, b);*/
    if (b!=0 && a!=b)
    {
/*      printf("[[coopt:char mismatch]]\n");*/
      return (char *)(NULL);
    }
  } while (a!=0 && b!=0);

  if (b!=0)
  {
/*    printf("[[coopt:start didn't terminate]]\n");*/
    return (char *)(NULL);
  }
  else
    return (char *)(target-1);
}

/*
 * returns NULL, or pointer to the character after the end of (start),
 * found at the start of (target). If (target) is shorter than (start),
 * this will return NULL.
 *
 * With the modification that we don't test more than max characters.
 */
static char *coopt_strnstarts(char const *target, char const *start,
			      size_t max)
{
  char a,b;
  if (max==0) /* easiest to special case like this */
    return (char *)target;
/*  printf("[[coopt:target='%s' start='%s' max=%i]]\n", target, start, max);
  fflush(stdout);*/
  do
  {
    a = target++[0];
    b = start++[0];
/*    printf("[[coopt: '%c' '%c']]\n", a, b);
    fflush(stdout);*/
    if (b!=0 && a!=b)
      return (char *)(NULL);
    max--;
  } while (a!=0 && b!=0 && max>0);

  if (b!=0 && max>0) /* didn't terminate in either way */
    return (char *)(NULL);
  else
    return (char *)(target-1);
}
