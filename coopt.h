/*
 * $Id: coopt.h,v 1.8 2000/03/22 23:12:06 james Exp $
 * coopt.h
 *
 * Interface header file for coopt, the Tartarus option parsing library
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
 * Plus points over getopt:
 *   all memory handled by the user
 *   no global state, so can be threaded
 *   separator ("--") can be altered
 *   long options supported
 *   both sep (--file <file>) and eq (--file=<file>) long opt params
 *   sep (-f <file>) and inline (-f<file>) short opt params
 *   mix short params (-fv <file>)
 *   long and short option markers ("-", "--") can be altered
 *   options can be easily added and removed between calling coopt
 *   arguments may be interspersed with options
 *   'private' field per option, making sophisticated option dispatch easier
 *
 * Plus points over GNU getopt:
 *   argv really is const
 *   porting not required (hopefully) - some GNU getopt ports are incomplete
 *   BSD-style license, so re-licensable
 *
 * See the manual for more information, and detailed examples.
 */

#ifndef COOPT_H
#define COOPT_H

#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

/*
 * note that the error case of an invalid combination of option
 * and marker need to be dealt with by the user
 */

/*
 * definition of an option; if short_option==0 and long_option==NULL
 * then this is an invalid entry, and is skipped on processing
 */
struct coopt_option
{
  char short_option; /* 0 if no short equivalent */
  unsigned int has_param; /* COOPT_NO_PARAM or COOPT_REQUIRED_PARAM only. */
  char const * long_option; /* full text of long option */
  void * data; /* can leave out completely in initialiser;
		* this is private to the user - coopt won't touch it
		*/
};

#define COOPT_NO_PARAM		(0)
#define COOPT_REQUIRED_PARAM	(1)

/*
 * Note that unlike GNU getopt, we don't allow optional_argument.
 * This is because we believe it to be more confusing than it's worth.
 * We advocate having a separate option which gives the behaviour with
 * a default parameter.
 * If you really need an optional parameter, use COOPT_NO_PARAM and catch
 * COOPT_RESULT_HADPARAM. (Although this only works for long options with
 * eq params.)
 */

struct coopt_state; /* declare this to prevent any possible problems
		     * it is defined later on in this header file
		     */

/*
 * Call once to initialise the coopt_state structure, and to set
 * default options. The options can be changed directly by the user
 * later (see the structure definition, below).
 * 'options' may contain 'invalid' entries (see above); these are
 * skipped on processing, allowing you to quickly disable options
 * that aren't valid in context.
 * Note that argc and argv as passed to coopt_init() should be
 * argc-1 and argv+1 as passed to main().
 */
void coopt_init(struct coopt_state * /*state*/,
		struct coopt_option const * /*options*/,
		unsigned int /*num_options*/,
		int /*argc*/, char const * const * /*argv*/);

/*
 * structure returned by coopt() after each pass, indicating what was
 * found. In error cases, this will be filled out as much as is possible
 * (eg: missing param with --long=fish style parameters will still tell you
 * that "long" was the option used).
 */
struct coopt_return
{
  int result; /* see below */
  int ambigresult; /* if result==COOPT_RESULT_AMBIGUOUSOPT, this will contain
  		    * COOPT_RESULT_OKAY, or an error code relating to parameter
  		    * processing. We don't advise allowing ambiguous options
  		    * (abbreviated long options where more than one long option
  		    * matches what was given), but this allows you to do
  		    * complete processing on them if you really want.
  		    */
  struct coopt_option const * opt; /* NULL on _END and on argument
  				    * (param -> argument)
  				    * Also NULL for BADOPTION, in which case
  				    * param -> what looked like the option
  				    * (may include an eq-separated param; use
  				    * coopt_sopt() to extract just the 'option')
  				    */
  char const * param; /* pointer to the parameter for this option (or NULL) */
  char const * marker; /* pointer to the marker definition (eg: "L--") that
  			* was used for this option (or NULL)
  			*/
};

/*
 * These are fatal errors
 */

/* Unspecified error - typically coopt() has been called wrongly
 * In some cases, it may be because coopt() went wrong - in which case you
 * may get most of the options processing having been completed successfully.
 * However coopt() probably won't work safely on next invocation, so it's
 * better to report to the user and then abort.
 */
#define COOPT_RESULT_ERROR		(-1)

/*
 * These are non-fatal errors
 */

/* A long option that shouldn't have had a parameter turned up with one
 * This will set up all fields of the result properly; if the option had
 * been specified with a parameter, you'd get the same situation but with
 * COOPT_RESULT_OKAY.
 */
#define COOPT_RESULT_HADPARAM		(-10)

/* More than one option in mixed short params had a parameter
 * This will be called on all options with parameters after the first one.
 * eg: -fzt where all three options take parameters; 'f' will get _OKAY, but
 * 'z' and 't' will get _MULTIMIXED.
 * 'param' will be NULL in this case.
 */
#define COOPT_RESULT_MULTIMIXED		(-8)

/* An abbreviated long option was ambiguous. 'opt' will contain the first
 * one found that matched, with this option fully processed - ie 'param' will
 * be right as well. This allows a client to allow ambiguous abbreviations
 * with the preference defined by the order of long options in the option
 * array.
 * 'ambigresult' will contain either _OKAY, _HADPARAM or _NOPARAM depending
 * on any errors in parameter processing for the first matching option.
 */
#define COOPT_RESULT_AMBIGUOUSOPT	(-6)

/* Found an option which wasn't in the list. 'param' -> option string that
 * we couldn't parse. Remember to check marker[0] to see whether we're
 * talking about a long or short option.
 * (Note that because coopt won't touch the argv list you gave it, the
 * option string that 'param' points to may contain an eq-separated
 * parameter - you should use coopt_sopt() to extract just the option name
 * if you need it.)
 */
#define COOPT_RESULT_BADOPTION		(-4)

/* --long with =-style parameter only. The option will be fully processed
 * other than this. You shouldn't use this to get optional parameters in
 * general, because with sep-parameters turned on this will never happen
 * (lack of an eq-parameter implies a separated parameter, so
 *   --with-my-library --verbose
 *  would give --with-my-library, with "--verbose" as its parameter)
 */
#define COOPT_RESULT_NOPARAM		(-2)

/*
 * Either an option was parsed with no troubles, or an argument was found
 * (in which case 'opt' will be NULL, and 'param' will point to the
 * argument.
 */
#define COOPT_RESULT_OKAY		(0)

/*
 * Ran out of entries in argv while looking for a parameter. 'param' will
 * therefore be NULL.
 */
#define COOPT_RESULT_MISSINGPARAM	(1)

/*
 * Ran out of argv in normal processing (this isn't an error - it just
 * signals that coopt has processed all options and arguments).
 */
#define COOPT_RESULT_END		(2)

/* Returns come in four sorts: fatal error, non-fatal error, okay, and
 * termination. Errors are < 0, okay ==0, termination > 0. This is a
 * defined part of the interface. The fatal/non-fatal error boundary isn't
 * defined.
 * There are some macros to make sure you aren't dependent on these
 * boundaries at all. Probably the best way to call coopt is to use something
 * like:
 *
 * struct coopt_return ret;
 * do
 * {
 *   ret = coopt(&state);
 *   if (!coopt_is_error(ret.result))
 *   {
 *     // process ret.opt
 *   }
 * } while (coopt_is_okay(ret.result));
 *
 * And then tidy up afterwards, switching on the individual errors and
 * termination cases.
 */
#define coopt_is_error(x) (x<0 || x==COOPT_RESULT_MISSINGPARAM)
#define coopt_is_okay(x) (x==0)
#define coopt_is_termination(x) (x>0)
#define coopt_is_fatal(x) (x<0 && (x&1)==1)
#define coopt_is_nonfatal(x) ((x<0 && (x&1)==0) || x==COOPT_RESULT_MISSINGPARAM)

/*
 * main coopt processing routine. Just call this repeatedly to cycle
 * through all options and arguments.
 */
struct coopt_return coopt(struct coopt_state * /*state*/);

/*
 * The number of badgers acts as a version indicator for the internal
 * implementation of coopt. This allows people to write clever things
 * using knowledge of the internals, and not have it compile and then
 * break in inexplicable ways in future - it simply won't compile.
 *
 * The badgers themselves are gratuitous.
 */
#define COOPT_GRATUITOUS_BADGERS 5

/*
 * Complete coopt state. This is supplied by the user, but should be set
 * up by calling coopt_init().
 * This contains both the current state of processing - where in
 * the arguments coopt has got - and options associated with
 * processing. The latter may be changed by the user, the former
 * should never be.
 */
struct coopt_state
{
  /* You can change these having initialised the structure with coopt_init() */
  struct coopt_option const * options;
  unsigned int num_options;
  struct coopt_flags
  {
    unsigned int allow_mix_short_params	: 1; /* eg: -cfv <filename>
    					      * <filename> binds to f
    					      */
    unsigned int allow_long_eq_params	: 1; /* ie: "--value=this" style */
    unsigned int allow_long_sep_params	: 1; /* ie: "--value this" style */
    unsigned int allow_long_opts_breved	: 1; /* eg: --long[-opt]; non-unique
					      * abbreviations will cause an
					      * error
					      */
  } flags;

  /* Only change these if you're really sure about the consequences ... */
  char const * separator; /* to disable, set to NULL */
  char const * long_eq; /* the '=' in "--value=this".
                         * Don't make this empty unless you *really* mean it.
                         * If you make this NULL it will disable long_eq_params,
                         * but please use the flag above instead ...
                         */
  char const * const * markers; /* this will look like
  				 *   { "L--", "S-", NULL }
  				 * or similar; it must be ordered: no marker
				 * may be a prefix of a marker later in the
				 * list (or the later one will be ignored)
				 * (So switching the first two members of the
				 * above list would cause "S-" to be used
				 * even when the option started "--".)
  				 */

  /* Ignore this if you're a user */
  int argc;
  char const * const * argv;
  int char_within_arg; /* in the current implementation, <0 =>
			* found the separator, and we're in the
			* argument-only list.
			*/
  unsigned int skip_next_arg; /* non-zero to skip the next one on start of
			       * processing */
  char const * last_marker; /* used with multiple short options in one
			     * argument
			     */
};

/* And some support routines, which may make life easier on you */

/*
 * Fill the buffer with the fully-qualified option string ONLY.
 * This will take account of the possible error types in the return
 * structure to reassemble something in cases where ret->opt hasn't
 * been set up - eg: unrecognised option, where ret->param contains the
 * text of the unparsable option.
 * If show_marker is non-zero, the marker that was used will be prefixed
 * to the option text.
 * Returns: number of characters successfully written into the buffer.
 * This may not be the extent of the buffer, even if there wasn't space to
 * print the entire option string into the buffer, because it only does, eg,
 * a sprintf() if it can fit the entire string in.
 * Buffer is always NUL-terminated on exit.
 */
size_t coopt_sopt(char * /*buffer*/, size_t /*bufsize*/,
                  struct coopt_return * /*ret*/, int /*show_marker*/,
                  struct coopt_state * /*state*/);

/*
 * Fill the buffer with a string describing the current state.
 * Typically this is only used on error, but arguments and normal
 * options will be described as well (although slightly tersely).
 * Currently this hasn't even been considered in terms of localisation - but
 * that's okay, because I'm not convinced that gettext is really a sensible
 * long-term solution to locality issues anyway. We'll see.
 * Returns: number of characters successfully written into the buffer.
 * This may not be the extent of the buffer, even if there wasn't space to
 * print the entire error string into the buffer, because it only does, eg,
 * a sprintf() if it can fit the entire string in.
 * Buffer is always NUL-terminated on exit.
 */
size_t coopt_serror(char * /*buffer*/, size_t /*bufsize*/,
		    struct coopt_return * /*ret*/,
		    struct coopt_state * /*state*/);

#ifdef __cplusplus
}
#endif

#endif /* COOPT_H */
