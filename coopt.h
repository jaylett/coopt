/*
 * $Id: coopt.h,v 1.1 1999/05/18 17:55:28 james Exp $
 * coopt.h
 *
 * Interface header file for coopt, the Tartarus option parsing library
 * (c) Copyright James Aylett 1999
 */

#ifndef COOPT_H
#define COOPT_H

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
  unsigned int has_param; /* COOPT_NO_PARAM or COOPT_REQUIRED_PARAM only.
  			   * Optional parameters are horrible.
  			   */
  char const * long_option; /* full text of long option */
  void * private; /* can leave out completely in initialiser;
  		   * this is private to the user - coopt won't touch it
  		   */
};

#define COOPT_NO_PARAM		(0)
#define COOPT_REQUIRED_PARAM	(1)

/*
 * No, we don't like these defines living completely outside the COOPT
 * namespace. If you really want this, you can uncomment this section,
 * but there's not really much point. Also, we call them params not
 * arguments for clarity.
#define no_argument		(0)
#define required_argument	(1)
 */

/*
 * Note that unlike GNU getopt, we don't allow optional_argument.
 * This is because we believe it to be more confusing than it's worth.
 * (It would make the coopt() code itself more complex, since we'd have
 * to worry about what is an argument and what isn't, and we get into
 * trouble with --file -fish where "-fish" is the *optional* argument
 * to --file and there's also a short option "-f".)
 * We advocate having a separate option which gives the behaviour with
 * a default parameter.
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
 * that aren't valid in context
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
  int result; /* negative for error, zero for found something, positive for
  	       * termination cases
  	       */
  struct coopt_option const * opt; /* NULL on _END and on argument
  				    * (param -> argument)
  				    */
  char const * param; /* pointer to the parameter for this option (or NULL) */
  char const * marker; /* pointer to the marker definition (eg: "L--") that
  			* was used for this option (or NULL)
  			*/
};

/* Returns come in four sorts: fatal error, non-fatal error, okay, and
 * termination. Errors are < 0, okay ==0, termination > 0. This is a
 * defined part of the interface. The fatal/non-fatal error boundary isn't
 * defined.
 * There are some macros to make sure you aren't dependent on these
 * boundaries at all.
 */
#define coopt_is_error(x) (x<0)
#define coopt_is_okay(x) (x==0)
#define coopt_is_termination(x) (x>0)
#define coopt_is_fatal(x) ((x&1)==1)
#define coopt_is_nonfatal(x) ((x&1)==0)
  
/* These are fatal errors */
/* Unspecified error - typically coopt() has been called wrongly
 * In some cases, it may be because coopt() went wrong - in which case you
 * may get most of the options processing having been completed successfully.
 * However coopt() probably won't work safely on next invocation, so it's
 * better to report to the user and then abort.
 */
#define COOPT_RESULT_ERROR		(-1)

/* These are non-fatal errors */
/* A long option that shouldn't have had a parameter turned up with one
 * This will set up all fields of the result properly; if the option had
 * been specified with a parameter, you'd get the same situation but with
 * COOPT_RESULT_OKAY.
 */
#define COOPT_RESULT_HADPARAM		(-10)
/* More than one option in mixed short params had a parameter */
#define COOPT_RESULT_MULTIMIXED		(-8)
/* An abbreviated long option was ambiguous. opt will contain the first
 * one found that matched, with this option fully processed - ie param will
 * be right as well. This allows a client to allow ambiguous abbreviations
 * with the preference defined by the order of long options in the option
 * array.
 */
#define COOPT_RESULT_AMBIGUOUSOPT	(-6)
/* Found an option which wasn't in the list. param -> option string that
 * we couldn't parse. Remember to check marker[0] to see whether we're
 * talking about a long or short option.
 */
#define COOPT_RESULT_BADOPTION		(-4)
/* --long with =-style parameter only. This is non-fatal, so you can
 * use this to get optional arguments if you really need them.
 */
#define COOPT_RESULT_NOPARAM		(-2)
/*
 * For arguments, use _OKAY, set opt to NULL and
 * use param to point to the argument
 */
#define COOPT_RESULT_OKAY		(0)
/*
 * For _MISSINGPARAM, still return opt in coopt_return so user
 * can treat as default or whatever - can only happen when we
 * run out of argv while looking for a parameter. param will be
 * NULL in this case.
 */
#define COOPT_RESULT_MISSINGPARAM	(1)
/*
 * Ran out of argv
 */
#define COOPT_RESULT_END		(2)

/*
 * main coopt processing routine
 */
struct coopt_return coopt(struct coopt_state * /*state*/);

#define COOPT_GRATUITOUS_BADGERS 5

/*
 * Complete coopt state. This is supplied by the user.
 * This contains both the current state of processing - where in
 * the arguments coopt has got - and options associated with
 * processing. The latter may be changed by the user, the former
 * should never be.
 */
struct coopt_state
{
  /* You can change this having initialised the structure with coopt_init() */
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
  char const * long_eq; /* the '=' in "--value=this". Don't make this empty! */
  char const * const * markers; /* this will look like
  				 *   { "L--", "S-", NULL }
  				 * or similar; it must be ordered: no marker
				 * may be a prefix of a marker later in the
				 * list (or the later one will be ignored)
  				 */

  /* Ignore this if you're a user */
  int argc;
  char const * const * argv;
  int char_within_arg; /* in the current implementation, <0 =>
			* found the separator, and we're in the
			* argument-only list. However this should
			* not be relied on - see the manual for a
			* better way of handling this requirement.
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
 */
void coopt_sopt(char * /*buffer*/, struct coopt_return * /*ret*/,
		int /*show_marker*/, struct coopt_state */*state*/);
  
#ifdef __cplusplus
}
#endif
   
#endif /* COOPT_H */
