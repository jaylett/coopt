/*
 * $Id: test.c,v 1.1 1999/05/18 17:55:28 james Exp $
 * test.c
 *
 * coopt test rig
 * (c) Copyright James Aylett 1999
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "coopt.h"

/*
 * 1. short options
 * 2. long options
 * 3. unusual situation behaviours
 * 	3a. two long options with identical strings defined in the option list
 * 	3b. long option of the empty string, long_eq empty string and a param
 * 	    every arg non-trivially starting with a long opt marker gets
 *	     passed as a param to this
 * 	3c. single long option, abbreviated: "--=foo", foo is parameter and
 * 	    this is valid. Check also that with multiple long options, we get
 * 	    the ambiguity error and that the first long option was parsed for
 * 	    this situation (assuming it took an argument - otherwise as 2h.)
 * 	3d. empty arguments (in bash: splat --fish "")
 * 4. error conditions
 * 	4a. passing state==NULL
 * 	4b. passing argv==NULL
 * 	4c. illegal marker block (ie: something starting other than [LS])
 * 5. reconfiguration behaviours
 * 6. potential uses of the 'private' field
 */

/*
 * Internal functions that do the hard work in most of the basic tests.
 */
void init_test(struct coopt_state *, struct coopt_option *, unsigned int,
	     char *, char *);
void do_test(struct coopt_state *state);

int test;
char subtest;

void init_test(struct coopt_state *state, struct coopt_option *option,
	     unsigned int num_options, char *explanation, char *arglist)
{
  int argc=0, space_in_argv;
  char **argv;
  char *t;
  printf("\t%i%c. %s\n", test, subtest++, explanation);
  printf("\targlist = %s\n\n", arglist);

  t = strdup(arglist);

  /* Get argc, argv */
  space_in_argv = 10;
  argv = (char **)malloc(space_in_argv * sizeof(char *));
  do
  {
    if (argv==NULL)
    {
      fprintf(stderr, "Couldn't allocate space for [argv]\n");
      exit(1);
    }
    argv[argc++] = strtok(t, " ");
    t=NULL;
    if (argc==space_in_argv)
    {
      space_in_argv*=2;
      argv = (char **)realloc(argv, space_in_argv * sizeof(char *));
    }
  }
  while (argv!=NULL && argv[argc-1]!=NULL);
  argc--; /* correct for the one that failed */

  coopt_init(state, option, 5, argc, (char const * const *)argv);
}

void do_test(struct coopt_state *state)
{
  char buf[256];
  struct coopt_return ret;

  do
  {
    ret=coopt(state);
    buf[0]=0;
    switch (ret.result)
    {
     case COOPT_RESULT_OKAY:
      if (ret.opt==NULL)
      {
	/* argument */
	printf("argument = %s\n", ret.param);
      }
      else
      {
	coopt_sopt(buf, &ret, 1, state);
	printf("option = %s", buf);
	if (ret.param!=NULL)
	  printf("; param = %s", ret.param);
	printf("\n");
      }
      break;
     case COOPT_RESULT_END:
      printf("finished list\n");
      break;
     default:
      coopt_sopt(buf, &ret, 1, state);
      printf("error number = %i; option = %s",ret.result, buf);
      if (ret.param!=NULL)
	printf("; param = %s", ret.param);
      printf("\n");
      break;
    }
  } while (!coopt_is_fatal(ret.result) && !coopt_is_termination(ret.result));
  printf("\n");
}

int main(int argc, char **argv)
{
  struct coopt_option option[5];
  struct coopt_state state;

  printf("coopt test rig.\n");
  printf("available options will be:\n");
  printf("\t-v, --verbose\n");
  printf("\t-f, --file\t<param>\n");
  printf("\t-s, --silent\n");
  printf("\t-g\n");
  printf("\t--visual\n\n");

  option[0].short_option='v';
  option[0].has_param=COOPT_NO_PARAM;
  option[0].long_option="verbose";
  option[0].private=0;
  
  option[1].short_option='f';
  option[1].has_param=COOPT_REQUIRED_PARAM;
  option[1].long_option="file";
  option[1].private=0;
  
  option[2].short_option='s';
  option[2].has_param=COOPT_NO_PARAM;
  option[2].long_option="silent";
  option[2].private=0;
  
  option[3].short_option='g';
  option[3].has_param=COOPT_NO_PARAM;
  option[3].long_option=NULL;
  option[3].private=0;
  
  option[4].short_option=0;
  option[4].has_param=COOPT_NO_PARAM;
  option[4].long_option="visual";
  option[4].private=0;

#define test(a,b) init_test(&state,option,5,a,b); do_test(&state);
#define stest(a,b,c) init_test(&state,option,5,a,b); c; do_test(&state);

  printf("1. short options\n\n");
  test=1;
  subtest='a';

  test("short option recognition",
       "arg0 -v arg1 -s -- arg2 -arg3");
  test("short options with params (-f <param>)",
       "arg0 -f <param> arg1 -s -- arg2 -arg3");
  test("short options with following params (-f<param>)",
       "arg0 -f<param> arg1 -s -- arg2 -arg3");
  test("multiple short option (-vs)",
       "arg0 -vs arg1 -g -- arg2 -arg3");
  test("multiple short options with params (-vsf <param>)",
       "arg0 -vsf <param> arg1 -- arg2 -arg3");
  test("multiple short options with following param (-vsf<param>)",
       "arg0 -vsf<param> arg1 -- arg2 -arg3");
  stest("multiple short options with mix params (-fvs <param>)",
       "arg0 -fvs <param> arg1 -- arg2 -arg3",
       state.flags.allow_mix_short_params = 1);
  stest("multiple short options with mix params off",
       "arg0 -f<vs> param arg1 -- arg2 -arg3",
       state.flags.allow_mix_short_params = 0);
  stest("multiple short options with invalid mix params (-ffs <param>)",
       "arg0 -ffs <param> arg1 -- arg2 -arg3",
       state.flags.allow_mix_short_params = 1);
  test("unrecognised short options",
       "arg0 -z arg1 -- arg2 -arg3");
  test("unrecognised short options (in multiple)",
       "arg0 -zv arg1 -- arg2 -arg3");
  test("missing parameter (-f)",
       "arg0 -vg arg1 -f");

  printf("2. long options\n\n");
  test=2;
  subtest='a';

  test("long option recognition (--verbose)",
       "arg0 --verbose arg1 -- arg2 --arg3");
  stest("long option abbreviation (--verb)",
       "arg0 --verb arg1 -- arg2 --arg3",
       state.flags.allow_long_opts_breved=1);
  stest("long option abbreviation (--verb) and breved opts turned off",
       "arg0 --verb arg1 -- arg2 --arg3",
	state.flags.allow_long_opts_breved=0);
  stest("long option ambiguous abbreviation (--v)",
       "arg0 --v arg1 -- arg2 --arg3",
       state.flags.allow_long_opts_breved=1);
  test("long option with params (--file <param>)",
       "arg0 --file <param> arg1 -- arg2 --arg3");
  test("long option with following params (--file=<param>)",
       "arg0 --file=<param> arg1 -- arg2 --arg3");
  test("unrecognised long options",
       "arg0 --fish arg1 -- arg2 --arg3");
  test("missing parameter (--file)",
       "arg0 --file");
  stest("missing parameter (with sep_param turned off)",
       "arg0 --file arg1 -- arg2 arg3",
       state.flags.allow_long_sep_params=0);
  test("parameter when none requested (eq variant only)",
       "arg0 --verbose=no arg1 -- arg2 --arg3");
  stest("eq parameter with eq params turned off",
       "arg0 --file=no arg1 -- arg2 --arg3",
       state.flags.allow_long_eq_params=0);
  
  return 0;
}
