/*
 * $Id: test.c,v 1.4 2000/01/20 23:28:50 james Exp $
 * test.c
 *
 * coopt test rig
 * (c) Copyright James Aylett 1999-2000
 *
 * 1. short options
 * 2. long options
 * 3. unusual situation behaviours
 * 4. error conditions
 * 5. reconfiguration behaviours
 * 6. potential uses of the 'private' field
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "coopt.h"

/* dupstr() - strdup, only we write it so we don't depend on it */
char *dupstr(const char *in)
{
  char *out = malloc(strlen(in) + 1);
  if (out!=NULL)
    strcpy(out, in);
  return out;
}

/*
 * Internal functions that do the hard work in most of the basic tests.
 */
void init_test(struct coopt_state *, struct coopt_option *, unsigned int,
	     char *, char *);
/*void do_test(struct coopt_state *state);*/

int test;
char subtest;
int globalresult;
int tests;
int testspassed;
int verboseflag;

#define display_test(e) printf("%i%c. %s ...%c", test, subtest++, e, (verboseflag)?('\n'):(' '));

void init_test(struct coopt_state *state, struct coopt_option *option,
	     unsigned int num_options, char *explanation, char *arglist)
{
  int argc=0, space_in_argv;
  char **argv;
  char *t;
  display_test(explanation);

  globalresult=1;

  t = dupstr(arglist);

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

  coopt_init(state, option, num_options, argc, (char const * const *)argv);
}

#define test_out() printf("%s.\n", (globalresult)?("passed"):("failed")); tests++; testspassed+=globalresult;
#define test_string(a,b) ((a==NULL)?(b==NULL):((b==NULL)?(0):(!strcmp(a,b))))

#define test_display() \
  if (!globalresult || verboseflag) \
  { \
    char temp[256]; \
    coopt_sopt(temp, 256, &ret, 1, state); \
    printf("Result = %i; ", ret.result); \
    if (ret.ambigresult!=COOPT_RESULT_OKAY) \
      printf("ambigresult = %i; ", ret.ambigresult); \
    printf("option = '%s'", temp); \
    if (ret.param!=NULL) \
    printf("; param = '%s'", ret.param); \
    printf("\n"); \
  }

#define test_test(t) globalresult=globalresult*(((t))?(1):(0)); test_display();

void expect(struct coopt_state *state, int result)
{
  struct coopt_return ret;

  ret=coopt(state);
  test_test (ret.result==result);
}

void expect_opt(struct coopt_state *state, int result, struct coopt_option const *opt)
{
  struct coopt_return ret;

  ret=coopt(state);
  test_test (ret.result==result && ret.opt == opt);
}

void expect_opt_param(struct coopt_state *state, int result,
                         struct coopt_option const *opt,
                         char *param)
{
  struct coopt_return ret;

  ret=coopt(state);
  test_test (ret.result==result && ret.opt == opt &&
             test_string(ret.param, param));
}

void expect_opt_param_marker(struct coopt_state *state, int result,
                         struct coopt_option const *opt,
                         char *param, char *marker)
{
  struct coopt_return ret;

  ret=coopt(state);
  test_test (ret.result==result && ret.opt == opt &&
             test_string(ret.param, param) &&
             test_string(ret.marker, marker));
}

void expect_private_marker(struct coopt_state *state, int result,
			   void *data, char *marker)
{
  struct coopt_return ret;

  ret=coopt(state);
  test_test (ret.result==result && ret.opt->data==data &&
  	     test_string(ret.marker, marker));
}

void expect_function_marker(struct coopt_state *state, int result,
                            char *fnresult, char *marker)
{
  struct coopt_return ret;
  char *r;
  char *(*noparam)();

  ret=coopt(state);
  noparam=(char *(*)())ret.opt->data;
  r = (*noparam)();
  test_test (ret.result==result && test_string(r, fnresult) &&
  	     test_string(ret.marker, marker));
}

void expect_function_param_marker(struct coopt_state *state, int result,
                            char *fnresult, char *param, char *marker)
{
  struct coopt_return ret;
  char *r;
  char *(*hasparam)(char const *);

  ret=coopt(state);
  hasparam=(char *(*)(char const *))ret.opt->data;
  r = (*hasparam)(ret.param);
  test_test (ret.result==result && test_string(r, fnresult) &&
             test_string(ret.param, param) &&
             test_string(ret.marker, marker));
}

char *verbose()
{
  return "verbose";
}

char *file(char const *in)
{
  if (strcmp(in,"<param>")==0)
    return "file";
  else if (strcmp(in,"<param2>")==0)
    return "file2";
  else
    return "";
}

char *silent()
{
  return "silent";
}

char *taciturn()
{
  return "taciturn";
}

void expect_private_param_marker(struct coopt_state *state, int result,
				 void *data, char *param, char *marker)
{
  struct coopt_return ret;

  ret=coopt(state);
  test_test (ret.result==result && ret.opt->data==data &&
             test_string(ret.param, param) &&
  	     test_string(ret.marker, marker));
}

void expect_opt_ambig_param_marker(struct coopt_state *state, int result, int ambig,
                         struct coopt_option const *opt,
                         char *param, char *marker)
{
  struct coopt_return ret;

  ret=coopt(state);
  test_test (ret.result==result && ret.ambigresult==ambig &&
             ret.opt == opt &&
             test_string(ret.param, param) &&
             test_string(ret.marker, marker));
}

int main(int argc, char const * const * argv)
{
  struct coopt_option option[6];
  struct coopt_state state;
  struct coopt_return ret;

  verboseflag=0;

  printf("coopt test rig.\n");
/*  printf("available options will be:\n");
  printf("\t-v, --verbose\n");
  printf("\t-f, --file\t<param>\n");
  printf("\t-s, --silent\n");
  printf("\t-g\n");
  printf("\t--visual\n\n");*/

  option[0].short_option='v';
  option[0].has_param=COOPT_NO_PARAM;
  option[0].long_option="verbose";
  option[0].data=0;

  coopt_init(&state, option, 1, argc-1, argv+1);
  do
  {
    ret = coopt(&state);
    if (!coopt_is_error(ret.result))
    {
      if (ret.opt==option)
        verboseflag=1;
    }
  } while (coopt_is_okay(ret.result));

  if (coopt_is_error(ret.result))
  {
    fprintf(stderr, "error in options processing\n");
    exit(1);
  }

  option[1].short_option='f';
  option[1].has_param=COOPT_REQUIRED_PARAM;
  option[1].long_option="file";
  option[1].data=0;

  option[2].short_option='s';
  option[2].has_param=COOPT_NO_PARAM;
  option[2].long_option="silent";
  option[2].data=0;

  option[3].short_option='g';
  option[3].has_param=COOPT_NO_PARAM;
  option[3].long_option=NULL;
  option[3].data=0;

  option[4].short_option=0;
  option[4].has_param=COOPT_NO_PARAM;
  option[4].long_option="visual";
  option[4].data=0;

  /* option[5] is used by some tests */

#define init(a,b) init_test(&state,option,5,a,b);

  tests=0;
  testspassed=0;

  printf("\n1. short options\n");
  test=1;
  subtest='a';

  init("short option recognition",
       "arg0 -v arg1 -s -- arg2 -arg3");
  expect_opt_param(&state,COOPT_RESULT_OKAY, NULL, "arg0");
  expect_opt_param_marker(&state,COOPT_RESULT_OKAY, option, NULL, "S-");
  expect_opt_param(&state,COOPT_RESULT_OKAY, NULL, "arg1");
  expect_opt_param_marker(&state,COOPT_RESULT_OKAY, option+2, NULL, "S-");
  expect_opt_param(&state,COOPT_RESULT_OKAY, NULL, "arg2");
  expect_opt_param(&state,COOPT_RESULT_OKAY, NULL, "-arg3");
  expect(&state,COOPT_RESULT_END);
  test_out();

  init("short options with params (-f <param>)",
       "arg0 -f <param> arg1 -s -- arg2 -arg3");
  expect_opt_param(&state,COOPT_RESULT_OKAY, NULL, "arg0");
  expect_opt_param_marker(&state,COOPT_RESULT_OKAY, option+1, "<param>", "S-");
  expect_opt_param(&state,COOPT_RESULT_OKAY, NULL, "arg1");
  expect_opt_param_marker(&state,COOPT_RESULT_OKAY, option+2, NULL, "S-");
  expect_opt_param(&state,COOPT_RESULT_OKAY, NULL, "arg2");
  expect_opt_param(&state,COOPT_RESULT_OKAY, NULL, "-arg3");
  expect(&state,COOPT_RESULT_END);
  test_out();

  init("short options with following params (-f<param>)",
       "arg0 -f<param> arg1 -s -- arg2 -arg3");
  expect_opt_param(&state,COOPT_RESULT_OKAY, NULL, "arg0");
  expect_opt_param_marker(&state,COOPT_RESULT_OKAY, option+1, "<param>", "S-");
  expect_opt_param(&state,COOPT_RESULT_OKAY, NULL, "arg1");
  expect_opt_param_marker(&state,COOPT_RESULT_OKAY, option+2, NULL, "S-");
  expect_opt_param(&state,COOPT_RESULT_OKAY, NULL, "arg2");
  expect_opt_param(&state,COOPT_RESULT_OKAY, NULL, "-arg3");
  expect(&state,COOPT_RESULT_END);
  test_out();

  init("multiple short option (-vs)",
       "arg0 -vs arg1 -g -- arg2 -arg3");
  expect_opt_param(&state,COOPT_RESULT_OKAY, NULL, "arg0");
  expect_opt_param_marker(&state,COOPT_RESULT_OKAY, option, NULL, "S-");
  expect_opt_param_marker(&state,COOPT_RESULT_OKAY, option+2, NULL, "S-");
  expect_opt_param(&state,COOPT_RESULT_OKAY, NULL, "arg1");
  expect_opt_param_marker(&state,COOPT_RESULT_OKAY, option+3, NULL, "S-");
  expect_opt_param(&state,COOPT_RESULT_OKAY, NULL, "arg2");
  expect_opt_param(&state,COOPT_RESULT_OKAY, NULL, "-arg3");
  expect(&state,COOPT_RESULT_END);
  test_out();

  init("multiple short options with params (-vsf <param>)",
       "arg0 -vsf <param> arg1 -- arg2 -arg3");
  expect_opt_param(&state,COOPT_RESULT_OKAY, NULL, "arg0");
  expect_opt_param_marker(&state,COOPT_RESULT_OKAY, option, NULL, "S-");
  expect_opt_param_marker(&state,COOPT_RESULT_OKAY, option+2, NULL, "S-");
  expect_opt_param_marker(&state,COOPT_RESULT_OKAY, option+1, "<param>", "S-");
  expect_opt_param(&state,COOPT_RESULT_OKAY, NULL, "arg1");
  expect_opt_param(&state,COOPT_RESULT_OKAY, NULL, "arg2");
  expect_opt_param(&state,COOPT_RESULT_OKAY, NULL, "-arg3");
  expect(&state,COOPT_RESULT_END);
  test_out();

  init("multiple short options with following param (-vsf<param>)",
       "arg0 -vsf<param> arg1 -- arg2 -arg3");
  expect_opt_param(&state,COOPT_RESULT_OKAY, NULL, "arg0");
  expect_opt_param_marker(&state,COOPT_RESULT_OKAY, option, NULL, "S-");
  expect_opt_param_marker(&state,COOPT_RESULT_OKAY, option+2, NULL, "S-");
  expect_opt_param_marker(&state,COOPT_RESULT_OKAY, option+1, "<param>", "S-");
  expect_opt_param(&state,COOPT_RESULT_OKAY, NULL, "arg1");
  expect_opt_param(&state,COOPT_RESULT_OKAY, NULL, "arg2");
  expect_opt_param(&state,COOPT_RESULT_OKAY, NULL, "-arg3");
  expect(&state,COOPT_RESULT_END);
  test_out();

  init("multiple short options with mix params (-fvs <param>)",
       "arg0 -fvs <param> arg1 -- arg2 -arg3");
  state.flags.allow_mix_short_params = 1;
  expect_opt_param(&state,COOPT_RESULT_OKAY, NULL, "arg0");
  expect_opt_param_marker(&state,COOPT_RESULT_OKAY, option+1, "<param>", "S-");
  expect_opt_param_marker(&state,COOPT_RESULT_OKAY, option, NULL, "S-");
  expect_opt_param_marker(&state,COOPT_RESULT_OKAY, option+2, NULL, "S-");
  expect_opt_param(&state,COOPT_RESULT_OKAY, NULL, "arg1");
  expect_opt_param(&state,COOPT_RESULT_OKAY, NULL, "arg2");
  expect_opt_param(&state,COOPT_RESULT_OKAY, NULL, "-arg3");
  expect(&state,COOPT_RESULT_END);
  test_out();

  init("multiple short options with mix params off",
       "arg0 -f<vs> param arg1 -- arg2 -arg3");
  state.flags.allow_mix_short_params = 0;
  expect_opt_param(&state,COOPT_RESULT_OKAY, NULL, "arg0");
  expect_opt_param_marker(&state,COOPT_RESULT_OKAY, option+1, "<vs>", "S-");
  expect_opt_param(&state,COOPT_RESULT_OKAY, NULL, "param");
  expect_opt_param(&state,COOPT_RESULT_OKAY, NULL, "arg1");
  expect_opt_param(&state,COOPT_RESULT_OKAY, NULL, "arg2");
  expect_opt_param(&state,COOPT_RESULT_OKAY, NULL, "-arg3");
  expect(&state,COOPT_RESULT_END);
  test_out();

  init("multiple short options with invalid mix params (-ffs <param>)",
       "arg0 -ffs <param> arg1 -- arg2 -arg3");
  state.flags.allow_mix_short_params = 1;
  expect_opt_param(&state,COOPT_RESULT_OKAY, NULL, "arg0");
  expect_opt_param_marker(&state,COOPT_RESULT_OKAY, option+1, "<param>", "S-");
  expect_opt_param_marker(&state,COOPT_RESULT_MULTIMIXED, option+1, NULL, "S-");
  expect_opt_param_marker(&state,COOPT_RESULT_OKAY, option+2, NULL, "S-");
  expect_opt_param(&state,COOPT_RESULT_OKAY, NULL, "arg1");
  expect_opt_param(&state,COOPT_RESULT_OKAY, NULL, "arg2");
  expect_opt_param(&state,COOPT_RESULT_OKAY, NULL, "-arg3");
  expect(&state,COOPT_RESULT_END);
  test_out();

  init("unrecognised short options",
       "arg0 -z arg1 -- arg2 -arg3");
  expect_opt_param(&state,COOPT_RESULT_OKAY, NULL, "arg0");
  expect_opt_param_marker(&state,COOPT_RESULT_BADOPTION, NULL, "z", "S-");
  expect_opt_param(&state,COOPT_RESULT_OKAY, NULL, "arg1");
  expect_opt_param(&state,COOPT_RESULT_OKAY, NULL, "arg2");
  expect_opt_param(&state,COOPT_RESULT_OKAY, NULL, "-arg3");
  expect(&state,COOPT_RESULT_END);
  test_out();

  init("unrecognised short options (in multiple)",
       "arg0 -zv arg1 -- arg2 -arg3");
  expect_opt_param(&state,COOPT_RESULT_OKAY, NULL, "arg0");
  expect_opt_param_marker(&state,COOPT_RESULT_BADOPTION, NULL, "zv", "S-");
  expect_opt_param_marker(&state,COOPT_RESULT_OKAY, option, NULL, "S-");
  expect_opt_param(&state,COOPT_RESULT_OKAY, NULL, "arg1");
  expect_opt_param(&state,COOPT_RESULT_OKAY, NULL, "arg2");
  expect_opt_param(&state,COOPT_RESULT_OKAY, NULL, "-arg3");
  expect(&state,COOPT_RESULT_END);
  test_out();

  init("missing parameter (-f)",
       "arg0 -vg arg1 -f");
  expect_opt_param(&state,COOPT_RESULT_OKAY, NULL, "arg0");
  expect_opt_param_marker(&state,COOPT_RESULT_OKAY, option, NULL, "S-");
  expect_opt_param_marker(&state,COOPT_RESULT_OKAY, option+3, NULL, "S-");
  expect_opt_param(&state,COOPT_RESULT_OKAY, NULL, "arg1");
  expect_opt_param_marker(&state,COOPT_RESULT_MISSINGPARAM, option+1, NULL, "S-");
  test_out();

  printf("\n2. long options\n");
  test=2;
  subtest='a';

  init("long option recognition (--verbose)",
       "arg0 --verbose arg1 -- arg2 --arg3");
  expect_opt_param(&state,COOPT_RESULT_OKAY, NULL, "arg0");
  expect_opt_param_marker(&state,COOPT_RESULT_OKAY, option, NULL, "L--");
  expect_opt_param(&state,COOPT_RESULT_OKAY, NULL, "arg1");
  expect_opt_param(&state,COOPT_RESULT_OKAY, NULL, "arg2");
  expect_opt_param(&state,COOPT_RESULT_OKAY, NULL, "--arg3");
  test_out();

  init("long option abbreviation (--verb)",
       "arg0 --verb arg1 -- arg2 --arg3");
  state.flags.allow_long_opts_breved=1;
  expect_opt_param(&state,COOPT_RESULT_OKAY, NULL, "arg0");
  expect_opt_param_marker(&state,COOPT_RESULT_OKAY, option, NULL, "L--");
  expect_opt_param(&state,COOPT_RESULT_OKAY, NULL, "arg1");
  expect_opt_param(&state,COOPT_RESULT_OKAY, NULL, "arg2");
  expect_opt_param(&state,COOPT_RESULT_OKAY, NULL, "--arg3");
  test_out();

  init("long option abbreviation (--verb) and breved opts turned off",
       "arg0 --verb arg1 -- arg2 --arg3");
  state.flags.allow_long_opts_breved=0;
  expect_opt_param(&state,COOPT_RESULT_OKAY, NULL, "arg0");
  expect_opt_param_marker(&state,COOPT_RESULT_BADOPTION, NULL, "verb", "L--");
  expect_opt_param(&state,COOPT_RESULT_OKAY, NULL, "arg1");
  expect_opt_param(&state,COOPT_RESULT_OKAY, NULL, "arg2");
  expect_opt_param(&state,COOPT_RESULT_OKAY, NULL, "--arg3");
  test_out();

  init("long option ambiguous abbreviation (--v)",
       "arg0 --v arg1 -- arg2 --arg3");
  state.flags.allow_long_opts_breved=1;
  expect_opt_param(&state,COOPT_RESULT_OKAY, NULL, "arg0");
  expect_opt_ambig_param_marker(&state,COOPT_RESULT_AMBIGUOUSOPT, COOPT_RESULT_OKAY,option, NULL, "L--");
  expect_opt_param(&state,COOPT_RESULT_OKAY, NULL, "arg1");
  expect_opt_param(&state,COOPT_RESULT_OKAY, NULL, "arg2");
  expect_opt_param(&state,COOPT_RESULT_OKAY, NULL, "--arg3");
  test_out();

  init("long option with params (--file <param>)",
       "arg0 --file <param> arg1 -- arg2 --arg3");
  expect_opt_param(&state,COOPT_RESULT_OKAY, NULL, "arg0");
  expect_opt_param_marker(&state,COOPT_RESULT_OKAY, option+1, "<param>", "L--");
  expect_opt_param(&state,COOPT_RESULT_OKAY, NULL, "arg1");
  expect_opt_param(&state,COOPT_RESULT_OKAY, NULL, "arg2");
  expect_opt_param(&state,COOPT_RESULT_OKAY, NULL, "--arg3");
  test_out();

  init("long option with following params (--file=<param>)",
       "arg0 --file=<param> arg1 -- arg2 --arg3");
  expect_opt_param(&state,COOPT_RESULT_OKAY, NULL, "arg0");
  expect_opt_param_marker(&state,COOPT_RESULT_OKAY, option+1, "<param>", "L--");
  expect_opt_param(&state,COOPT_RESULT_OKAY, NULL, "arg1");
  expect_opt_param(&state,COOPT_RESULT_OKAY, NULL, "arg2");
  expect_opt_param(&state,COOPT_RESULT_OKAY, NULL, "--arg3");
  test_out();

  init("unrecognised long options",
       "arg0 --fish arg1 -- arg2 --arg3");
  expect_opt_param(&state,COOPT_RESULT_OKAY, NULL, "arg0");
  expect_opt_param_marker(&state,COOPT_RESULT_BADOPTION, NULL, "fish", "L--");
  expect_opt_param(&state,COOPT_RESULT_OKAY, NULL, "arg1");
  expect_opt_param(&state,COOPT_RESULT_OKAY, NULL, "arg2");
  expect_opt_param(&state,COOPT_RESULT_OKAY, NULL, "--arg3");
  test_out();

  init("missing parameter (--file)",
       "arg0 --file");
  expect_opt_param(&state,COOPT_RESULT_OKAY, NULL, "arg0");
  expect_opt_param_marker(&state,COOPT_RESULT_MISSINGPARAM, option+1, NULL, "L--");
  test_out();

  init("missing parameter (with sep_param turned off)",
       "arg0 --file arg1 -- arg2 --arg3");
  state.flags.allow_long_sep_params=0;
  expect_opt_param(&state,COOPT_RESULT_OKAY, NULL, "arg0");
  expect_opt_param_marker(&state,COOPT_RESULT_NOPARAM, option+1, NULL, "L--");
  expect_opt_param(&state,COOPT_RESULT_OKAY, NULL, "arg1");
  expect_opt_param(&state,COOPT_RESULT_OKAY, NULL, "arg2");
  expect_opt_param(&state,COOPT_RESULT_OKAY, NULL, "--arg3");
  test_out();

  init("parameter when none requested (eq variant only)",
       "arg0 --verbose=no arg1 -- arg2 --arg3");
  expect_opt_param(&state,COOPT_RESULT_OKAY, NULL, "arg0");
  expect_opt_param_marker(&state,COOPT_RESULT_HADPARAM, option, "no", "L--");
  expect_opt_param(&state,COOPT_RESULT_OKAY, NULL, "arg1");
  expect_opt_param(&state,COOPT_RESULT_OKAY, NULL, "arg2");
  expect_opt_param(&state,COOPT_RESULT_OKAY, NULL, "--arg3");
  test_out();

  init("eq parameter with eq params turned off",
       "arg0 --file=no arg1 -- arg2 --arg3");
  state.flags.allow_long_eq_params=0;
  expect_opt_param(&state,COOPT_RESULT_OKAY, NULL, "arg0");
  expect_opt_param_marker(&state,COOPT_RESULT_BADOPTION, NULL, "file=no", "L--");
  expect_opt_param(&state,COOPT_RESULT_OKAY, NULL, "arg1");
  expect_opt_param(&state,COOPT_RESULT_OKAY, NULL, "arg2");
  expect_opt_param(&state,COOPT_RESULT_OKAY, NULL, "--arg3");
  test_out();

  printf("\n3. rare cases\n");
  test=3;
  subtest='a';

  option[5].short_option='v';
  option[5].has_param=COOPT_NO_PARAM;
  option[5].long_option="verbose";
  option[5].data=0;
  init_test(&state,option,6,"two identical long options",
       "arg0 --verbose arg1 --invalid -- arg2 --arg3");
  expect_opt_param(&state,COOPT_RESULT_OKAY, NULL, "arg0");
  expect_opt_param_marker(&state,COOPT_RESULT_OKAY, option, NULL, "L--");
  expect_opt_param(&state,COOPT_RESULT_OKAY, NULL, "arg1");
  expect_opt_param_marker(&state,COOPT_RESULT_BADOPTION, NULL, "invalid", "L--");
  expect_opt_param(&state,COOPT_RESULT_OKAY, NULL, "arg2");
  expect_opt_param(&state,COOPT_RESULT_OKAY, NULL, "--arg3");
  test_out();

  option[3].short_option=0;

  init("skipping options", "arg0 -g arg1 --visual -- arg2 --arg3");
  expect_opt_param(&state,COOPT_RESULT_OKAY, NULL, "arg0");
  expect_opt_param_marker(&state,COOPT_RESULT_BADOPTION, NULL, "g", "S-");
  expect_opt_param(&state,COOPT_RESULT_OKAY, NULL, "arg1");
  expect_opt_param_marker(&state,COOPT_RESULT_OKAY, option+4, NULL, "L--");
  expect_opt_param(&state,COOPT_RESULT_OKAY, NULL, "arg2");
  expect_opt_param(&state,COOPT_RESULT_OKAY, NULL, "--arg3");
  test_out();

  option[3].short_option='g'; /* reset */

  /* This tests where we have multiple long options, and one is the empty string.
   * We've also set long_eq to the empty string, so "--option" is that option with
   * "option" as its parameter (think about it). However this in general won't
   * actually happen ...
   */
  option[3].long_option="";
  option[3].has_param=COOPT_REQUIRED_PARAM;
  init("empty string as long option and long_eq",
       "arg0 --verbose arg1 --invalid --visual -- arg2 --arg3");
  state.long_eq="";
  state.flags.allow_long_opts_breved = 1;
  expect_opt_param(&state,COOPT_RESULT_OKAY, NULL, "arg0");
  expect_opt_ambig_param_marker(&state,COOPT_RESULT_AMBIGUOUSOPT, COOPT_RESULT_HADPARAM, option, "verbose", "L--");
  expect_opt_param(&state,COOPT_RESULT_OKAY, NULL, "arg1");
  expect_opt_ambig_param_marker(&state,COOPT_RESULT_AMBIGUOUSOPT, COOPT_RESULT_HADPARAM, option, "invalid", "L--");
  expect_opt_ambig_param_marker(&state,COOPT_RESULT_AMBIGUOUSOPT, COOPT_RESULT_HADPARAM, option, "visual", "L--");
  expect_opt_param(&state,COOPT_RESULT_OKAY, NULL, "arg2");
  expect_opt_param(&state,COOPT_RESULT_OKAY, NULL, "--arg3");
  test_out();

  init("empty string as long option and long_eq (no breve)",
       "arg0 --verbose arg1 --invalid --visual -- arg2 --arg3");
  state.long_eq="";
  expect_opt_param(&state,COOPT_RESULT_OKAY, NULL, "arg0");
  expect_opt_param_marker(&state,COOPT_RESULT_OKAY, option+3, "verbose", "L--");
  expect_opt_param(&state,COOPT_RESULT_OKAY, NULL, "arg1");
  expect_opt_param_marker(&state,COOPT_RESULT_OKAY, option+3, "invalid", "L--");
  expect_opt_param_marker(&state,COOPT_RESULT_OKAY, option+3, "visual", "L--");
  expect_opt_param(&state,COOPT_RESULT_OKAY, NULL, "arg2");
  expect_opt_param(&state,COOPT_RESULT_OKAY, NULL, "--arg3");
  test_out();

  /* Another one, this time with only that one long option */
  init_test(&state,option+3,1,"empty string as long option and long_eq",
       "arg0 --verbose arg1 --invalid --visual -- arg2 --arg3");
  state.long_eq="";
  expect_opt_param(&state,COOPT_RESULT_OKAY, NULL, "arg0");
  expect_opt_param_marker(&state,COOPT_RESULT_OKAY, option+3, "verbose", "L--");
  expect_opt_param(&state,COOPT_RESULT_OKAY, NULL, "arg1");
  expect_opt_param_marker(&state,COOPT_RESULT_OKAY, option+3, "invalid", "L--");
  expect_opt_param_marker(&state,COOPT_RESULT_OKAY, option+3, "visual", "L--");
  expect_opt_param(&state,COOPT_RESULT_OKAY, NULL, "arg2");
  expect_opt_param(&state,COOPT_RESULT_OKAY, NULL, "--arg3");
  test_out();

  option[3].long_option=NULL; /* reset */
  option[3].has_param=COOPT_NO_PARAM;

  init_test(&state,option+1,1,"single maximally abbreviated long opt",
            "arg0 --=foo arg1 --= -- arg2 --arg3");
  state.flags.allow_long_opts_breved = 1;
  expect_opt_param(&state,COOPT_RESULT_OKAY, NULL, "arg0");
  expect_opt_param_marker(&state,COOPT_RESULT_OKAY, option+1, "foo", "L--");
  expect_opt_param(&state,COOPT_RESULT_OKAY, NULL, "arg1");
  expect_opt_param_marker(&state,COOPT_RESULT_OKAY, option+1, "", "L--");
  expect_opt_param(&state,COOPT_RESULT_OKAY, NULL, "arg2");
  expect_opt_param(&state,COOPT_RESULT_OKAY, NULL, "--arg3");
  test_out();

  init("multiple maximally abbreviated long opt (no param)",
       "arg0 --=foo arg1 --= -- arg2 --arg3");
  state.flags.allow_long_opts_breved = 1;
  expect_opt_param(&state,COOPT_RESULT_OKAY, NULL, "arg0");
  expect_opt_ambig_param_marker(&state,COOPT_RESULT_AMBIGUOUSOPT, COOPT_RESULT_HADPARAM, option, "foo", "L--");
  expect_opt_param(&state,COOPT_RESULT_OKAY, NULL, "arg1");
  expect_opt_ambig_param_marker(&state,COOPT_RESULT_AMBIGUOUSOPT, COOPT_RESULT_HADPARAM, option, "", "L--");
  expect_opt_param(&state,COOPT_RESULT_OKAY, NULL, "arg2");
  expect_opt_param(&state,COOPT_RESULT_OKAY, NULL, "--arg3");
  test_out();

  init_test(&state,option+1,4,"multiple maximally abbreviated long opt (param)",
       "arg0 --=foo arg1 --= -- arg2 --arg3");
  state.flags.allow_long_opts_breved = 1;
  expect_opt_param(&state,COOPT_RESULT_OKAY, NULL, "arg0");
  expect_opt_ambig_param_marker(&state,COOPT_RESULT_AMBIGUOUSOPT, COOPT_RESULT_OKAY, option+1, "foo", "L--");
  expect_opt_param(&state,COOPT_RESULT_OKAY, NULL, "arg1");
  expect_opt_ambig_param_marker(&state,COOPT_RESULT_AMBIGUOUSOPT, COOPT_RESULT_OKAY, option+1, "", "L--");
  expect_opt_param(&state,COOPT_RESULT_OKAY, NULL, "arg2");
  expect_opt_param(&state,COOPT_RESULT_OKAY, NULL, "--arg3");
  test_out();

  /*
   * This one involves knowledge of the internal data structures ...
   * It also involves writing to a read-only location (because argv is fairly const)
   * I've cast that const-ness away, so compilers shouldn't complain ...
   *
   * Note that this covertly tests long and short options together. If we ever have
   * problems with the divide, I'll add an explicit test ...
   */
  init("empty arguments",
       "arg0 -f splat arg1 --file splat -- arg2 -arg3");
  {
    char **ping;
    ping = (char **)state.argv;
    ping[2] = "";
    ping = (char **)state.argv;
    ping[5] = "";
  }
  expect_opt_param(&state,COOPT_RESULT_OKAY, NULL, "arg0");
  expect_opt_param_marker(&state,COOPT_RESULT_OKAY, option+1, "", "S-");
  expect_opt_param(&state,COOPT_RESULT_OKAY, NULL, "arg1");
  expect_opt_param_marker(&state,COOPT_RESULT_OKAY, option+1, "", "L--");
  expect_opt_param(&state,COOPT_RESULT_OKAY, NULL, "arg2");
  expect_opt_param(&state,COOPT_RESULT_OKAY, NULL, "-arg3");
  expect(&state,COOPT_RESULT_END);
  test_out();

  /* Similar sorts of notes apply to this ... */
  init("separator is the empty string",
       "arg0 -v arg1 --file <param> -- -- arg2 -arg3");
  state.separator="";
  {
    char **ping;
    ping = (char **)state.argv;
    ping[6] = "";
  }
  expect_opt_param(&state,COOPT_RESULT_OKAY, NULL, "arg0");
  expect_opt_param_marker(&state,COOPT_RESULT_OKAY, option, NULL, "S-");
  expect_opt_param(&state,COOPT_RESULT_OKAY, NULL, "arg1");
  expect_opt_param_marker(&state,COOPT_RESULT_OKAY, option+1, "<param>", "L--");
  expect_opt_param(&state,COOPT_RESULT_OKAY, NULL, "--");
  expect_opt_param(&state,COOPT_RESULT_OKAY, NULL, "arg2");
  expect_opt_param(&state,COOPT_RESULT_OKAY, NULL, "-arg3");
  expect(&state,COOPT_RESULT_END);
  test_out();

  init("no markers in list",
       "arg0 -f <param> arg1 --file <param> -- arg2 -arg3");
  {
    char const **markers;
    markers = (char const **)malloc(1*sizeof(char *));
    if (markers!=NULL)
    {
      markers[0]=NULL;
    }
    state.markers=markers;
  }
  expect_opt_param(&state,COOPT_RESULT_OKAY, NULL, "arg0");
  expect_opt_param(&state,COOPT_RESULT_OKAY, NULL, "-f");
  expect_opt_param(&state,COOPT_RESULT_OKAY, NULL, "<param>");
  expect_opt_param(&state,COOPT_RESULT_OKAY, NULL, "arg1");
  expect_opt_param(&state,COOPT_RESULT_OKAY, NULL, "--file");
  expect_opt_param(&state,COOPT_RESULT_OKAY, NULL, "<param>");
  expect_opt_param(&state,COOPT_RESULT_OKAY, NULL, "arg2");
  expect_opt_param(&state,COOPT_RESULT_OKAY, NULL, "-arg3");
  expect(&state,COOPT_RESULT_END);
  test_out();

  init_test(&state,option,0,"no options in list",
       "arg0 --verbose arg1 --file=<param> -- arg2 --arg3");
  state.flags.allow_long_opts_breved = 1;
  expect_opt_param(&state,COOPT_RESULT_OKAY, NULL, "arg0");
  expect_opt_param(&state,COOPT_RESULT_BADOPTION, NULL, "verbose");
  expect_opt_param(&state,COOPT_RESULT_OKAY, NULL, "arg1");
  expect_opt_param(&state,COOPT_RESULT_BADOPTION, NULL, "file=<param>");
  expect_opt_param(&state,COOPT_RESULT_OKAY, NULL, "arg2");
  expect_opt_param(&state,COOPT_RESULT_OKAY, NULL, "--arg3");
  expect(&state,COOPT_RESULT_END);
  test_out();

  option[3].long_option="fridge";
  init("ambiguous opts with sep params",
       "arg0 --f <param> arg1 -- arg2 --arg3");
  state.flags.allow_long_opts_breved = 1;
  expect_opt_param(&state,COOPT_RESULT_OKAY, NULL, "arg0");
  expect_opt_ambig_param_marker(&state,COOPT_RESULT_AMBIGUOUSOPT, COOPT_RESULT_OKAY, option+1, "<param>", "L--");
  expect_opt_param(&state,COOPT_RESULT_OKAY, NULL, "arg1");
  expect_opt_param(&state,COOPT_RESULT_OKAY, NULL, "arg2");
  expect_opt_param(&state,COOPT_RESULT_OKAY, NULL, "--arg3");
  expect(&state,COOPT_RESULT_END);
  test_out();

  init("ambiguous opts with eq params",
       "arg0 --f=<param> arg1 -- arg2 --arg3");
  state.flags.allow_long_opts_breved = 1;
  expect_opt_param(&state,COOPT_RESULT_OKAY, NULL, "arg0");
  expect_opt_ambig_param_marker(&state,COOPT_RESULT_AMBIGUOUSOPT, COOPT_RESULT_OKAY, option+1, "<param>", "L--");
  expect_opt_param(&state,COOPT_RESULT_OKAY, NULL, "arg1");
  expect_opt_param(&state,COOPT_RESULT_OKAY, NULL, "arg2");
  expect_opt_param(&state,COOPT_RESULT_OKAY, NULL, "--arg3");
  expect(&state,COOPT_RESULT_END);
  test_out();

  init("ambiguous opts with sep params (missing param)",
       "arg0 --f");
  state.flags.allow_long_opts_breved = 1;
  expect_opt_param(&state,COOPT_RESULT_OKAY, NULL, "arg0");
  expect_opt_ambig_param_marker(&state,COOPT_RESULT_AMBIGUOUSOPT, COOPT_RESULT_MISSINGPARAM, option+1, NULL, "L--");
  test_out();

  init("ambiguous opts with eq params (no param)",
       "arg0 --f arg1");
  state.flags.allow_long_opts_breved = 1;
  state.flags.allow_long_sep_params = 0;
  expect_opt_param(&state,COOPT_RESULT_OKAY, NULL, "arg0");
  expect_opt_ambig_param_marker(&state,COOPT_RESULT_AMBIGUOUSOPT, COOPT_RESULT_NOPARAM, option+1, NULL, "L--");
  expect_opt_param(&state,COOPT_RESULT_OKAY, NULL, "arg1");
  expect(&state,COOPT_RESULT_END);
  test_out();

  option[1].has_param=COOPT_NO_PARAM;
  init("ambiguous opts with sep params (had param)",
       "arg0 --f <param>");
  state.flags.allow_long_opts_breved = 1;
  expect_opt_param(&state,COOPT_RESULT_OKAY, NULL, "arg0");
  expect_opt_ambig_param_marker(&state,COOPT_RESULT_AMBIGUOUSOPT, COOPT_RESULT_OKAY, option+1, NULL, "L--");
  expect_opt_param(&state,COOPT_RESULT_OKAY, NULL, "<param>");
  expect(&state,COOPT_RESULT_END);
  test_out();

  init("ambiguous opts with eq params (had param)",
       "arg0 --f=<param>");
  state.flags.allow_long_opts_breved = 1;
  expect_opt_param(&state,COOPT_RESULT_OKAY, NULL, "arg0");
  expect_opt_ambig_param_marker(&state,COOPT_RESULT_AMBIGUOUSOPT, COOPT_RESULT_HADPARAM, option+1, "<param>", "L--");
  test_out();

  option[3].long_option=NULL; /* reset */
  option[1].has_param=COOPT_REQUIRED_PARAM;

  printf("\n4. error cases\n");
  test=4;
  subtest='a';

  init("long_eq==NULL", "arg0 --file=<param> arg1 --verbose -- arg2 --arg3");
  state.long_eq=NULL;
  expect_opt_param(&state,COOPT_RESULT_OKAY, NULL, "arg0");
  expect_opt_param_marker(&state,COOPT_RESULT_BADOPTION, NULL, "file=<param>", "L--");
  expect_opt_param(&state,COOPT_RESULT_OKAY, NULL, "arg1");
  expect_opt_param_marker(&state,COOPT_RESULT_OKAY, option, NULL, "L--");
  expect_opt_param(&state,COOPT_RESULT_OKAY, NULL, "arg2");
  expect_opt_param(&state,COOPT_RESULT_OKAY, NULL, "--arg3");
  test_out();

  display_test("state==NULL");
  expect(NULL,COOPT_RESULT_ERROR);
  test_out();

  init("argv==NULL", "arg0");
  state.argv=NULL;
  expect(&state,COOPT_RESULT_ERROR);
  test_out();

  init("illegal marker block", "arg0 --verbose");
  {
    char const **markers;
    markers = (char const **)malloc(2*sizeof(char *));
    if (markers!=NULL)
    {
      markers[0]="R--";
      markers[1]=NULL;
    }
    state.markers=markers;
  }
  expect_opt_param(&state,COOPT_RESULT_OKAY,NULL,"arg0");
  expect(&state,COOPT_RESULT_ERROR);
  test_out();

  init("calling after MISSINGPARAM", "arg0 --file");
  expect_opt_param(&state,COOPT_RESULT_OKAY,NULL,"arg0");
  expect_opt_param_marker(&state,COOPT_RESULT_MISSINGPARAM,option+1,NULL,"L--");
  expect(&state,COOPT_RESULT_END);
  expect(&state,COOPT_RESULT_END);
  expect(&state,COOPT_RESULT_END);
  expect(&state,COOPT_RESULT_END);
  test_out();

  init("calling after END", "arg0");
  expect_opt_param(&state,COOPT_RESULT_OKAY,NULL,"arg0");
  expect(&state,COOPT_RESULT_END);
  expect(&state,COOPT_RESULT_END);
  expect(&state,COOPT_RESULT_END);
  expect(&state,COOPT_RESULT_END);
  test_out();

  init("argc==0", "");
  expect(&state,COOPT_RESULT_END);
  test_out();

  printf("\n5. reconfiguration cases\n");
  test=5;
  subtest='a';

  init("separator", "arg0 --verbose -g arg1 sep arg2 -arg3");
  state.separator="sep";
  expect_opt_param(&state, COOPT_RESULT_OKAY, NULL, "arg0");
  expect_opt_param_marker(&state, COOPT_RESULT_OKAY, option, NULL, "L--");
  expect_opt_param_marker(&state, COOPT_RESULT_OKAY, option+3, NULL, "S-");
  expect_opt_param(&state, COOPT_RESULT_OKAY, NULL, "arg1");
  expect_opt_param(&state, COOPT_RESULT_OKAY, NULL, "arg2");
  expect_opt_param(&state, COOPT_RESULT_OKAY, NULL, "-arg3");
  expect(&state,COOPT_RESULT_END);
  test_out();

  init("null separator", "arg0 --verbose -g arg1 sep arg2 -arg3");
  state.separator=NULL;
  /* rely on knowledge of internal structure ... */
  {
    char **ping;
    ping = (char **)state.argv;
    ping[4] = "";
  }
  expect_opt_param(&state, COOPT_RESULT_OKAY, NULL, "arg0");
  expect_opt_param_marker(&state, COOPT_RESULT_OKAY, option, NULL, "L--");
  expect_opt_param_marker(&state, COOPT_RESULT_OKAY, option+3, NULL, "S-");
  expect_opt_param(&state, COOPT_RESULT_OKAY, NULL, "arg1");
  expect_opt_param(&state, COOPT_RESULT_OKAY, NULL, "");
  expect_opt_param(&state, COOPT_RESULT_OKAY, NULL, "arg2");
  expect_opt_param_marker(&state, COOPT_RESULT_BADOPTION, NULL, "arg3", "S-");
  expect_opt_param_marker(&state, COOPT_RESULT_BADOPTION, NULL, "rg3", "S-");
  expect_opt_param_marker(&state, COOPT_RESULT_OKAY, option+3, NULL, "S-");
  expect_opt_param_marker(&state, COOPT_RESULT_BADOPTION, NULL, "3", "S-");
  expect(&state,COOPT_RESULT_END);
  test_out();

  init("long_eq", "arg0 --file:<param> --file=<param> arg1 -- arg2 --arg3");
  state.long_eq=":";
  expect_opt_param(&state, COOPT_RESULT_OKAY, NULL, "arg0");
  expect_opt_param_marker(&state, COOPT_RESULT_OKAY, option+1, "<param>", "L--");
  expect_opt_param_marker(&state, COOPT_RESULT_BADOPTION, NULL, "file=<param>", "L--");
  expect_opt_param(&state, COOPT_RESULT_OKAY, NULL, "arg1");
  expect_opt_param(&state, COOPT_RESULT_OKAY, NULL, "arg2");
  expect_opt_param(&state, COOPT_RESULT_OKAY, NULL, "--arg3");
  expect(&state,COOPT_RESULT_END);
  test_out();

  init("markers", "arg0 -file=<param> --v arg1 -- arg2 --arg3");
  {
    char const **markers;
    markers = (char const **)malloc(3*sizeof(char *));
    if (markers!=NULL)
    {
      markers[0]="S--";
      markers[1]="L-";
      markers[2]=NULL;
    }
    state.markers=markers;
  }
  expect_opt_param(&state, COOPT_RESULT_OKAY, NULL, "arg0");
  expect_opt_param_marker(&state, COOPT_RESULT_OKAY, option+1, "<param>", "L-");
  expect_opt_param_marker(&state, COOPT_RESULT_OKAY, option, NULL, "S--");
  expect_opt_param(&state, COOPT_RESULT_OKAY, NULL, "arg1");
  expect_opt_param(&state, COOPT_RESULT_OKAY, NULL, "arg2");
  expect_opt_param(&state, COOPT_RESULT_OKAY, NULL, "--arg3");
  expect(&state,COOPT_RESULT_END);
  test_out();

  init("markers (complex)", "arg0 -l-file=<param> -v arg1 +g +++visual=film -- arg2 --arg3");
  {
    char const **markers;
    markers = (char const **)malloc(5*sizeof(char *));
    if (markers!=NULL)
    {
      markers[0]="L+++";
      markers[1]="L-l-";
      markers[2]="S-";
      markers[3]="S+";
      markers[4]=NULL;
    }
    state.markers=markers;
  }
  expect_opt_param(&state, COOPT_RESULT_OKAY, NULL, "arg0");
  expect_opt_param_marker(&state, COOPT_RESULT_OKAY, option+1, "<param>", "L-l-");
  expect_opt_param_marker(&state, COOPT_RESULT_OKAY, option, NULL, "S-");
  expect_opt_param(&state, COOPT_RESULT_OKAY, NULL, "arg1");
  expect_opt_param_marker(&state, COOPT_RESULT_OKAY, option+3, NULL, "S+");
  expect_opt_param_marker(&state, COOPT_RESULT_HADPARAM, option+4, "film", "L+++");
  expect_opt_param(&state, COOPT_RESULT_OKAY, NULL, "arg2");
  expect_opt_param(&state, COOPT_RESULT_OKAY, NULL, "--arg3");
  expect(&state,COOPT_RESULT_END);
  test_out();

  printf("\n6. 'private' field test\n");
  test=6;
  subtest='a';

  option[0].short_option=0;
  option[0].has_param=COOPT_NO_PARAM;
  option[0].long_option="verbose";
  option[0].data=(void *)1;

  option[1].short_option=0;
  option[1].has_param=COOPT_REQUIRED_PARAM;
  option[1].long_option="file";
  option[1].data=(void *)2;

  option[2].short_option=0;
  option[2].has_param=COOPT_NO_PARAM;
  option[2].long_option="silent";
  option[2].data=(void *)3;

  option[3].short_option=0;
  option[3].has_param=COOPT_NO_PARAM;
  option[3].long_option="taciturn";
  option[3].data=(void *)3;

  option[4].short_option=0;
  option[4].has_param=COOPT_REQUIRED_PARAM;
  option[4].long_option="output";
  option[4].data=(void *)2;

  init("cast int", "--verbose --silent --file <param> --output=<param2> --taciturn");
  expect_private_marker(&state,COOPT_RESULT_OKAY,(void *)1,"L--");
  expect_private_marker(&state,COOPT_RESULT_OKAY,(void *)3,"L--");
  expect_private_param_marker(&state,COOPT_RESULT_OKAY,(void *)2,"<param>","L--");
  expect_private_param_marker(&state,COOPT_RESULT_OKAY,(void *)2,"<param2>","L--");
  expect_private_marker(&state,COOPT_RESULT_OKAY,(void *)3,"L--");
  expect(&state,COOPT_RESULT_END);
  test_out();

  option[0].data=(void *)verbose;
  option[1].data=(void *)file;
  option[2].data=(void *)silent;
  option[3].data=(void *)taciturn;
  option[4].data=(void *)file;

  init("function pointer", "--verbose --silent --file <param> --output=<param2> --taciturn");
  expect_function_marker(&state,COOPT_RESULT_OKAY,"verbose","L--");
  expect_function_marker(&state,COOPT_RESULT_OKAY,"silent","L--");
  expect_function_param_marker(&state,COOPT_RESULT_OKAY,"file","<param>","L--");
  expect_function_param_marker(&state,COOPT_RESULT_OKAY,"file2","<param2>","L--");
  expect_function_marker(&state,COOPT_RESULT_OKAY,"taciturn","L--");
  expect(&state,COOPT_RESULT_END);
  test_out();

  printf("\nRan %i tests, passed %i.\n", tests, testspassed);

  return (tests-testspassed);
}
