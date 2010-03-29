/* evd_test.c
 * SRE, Wed Nov 12 11:17:27 1997 [St. Louis]
 *
 * Test driver for EVD distribution support in histogram.c
 * Generates random EVD samples; fits them; checks fitted mu, lambda
 * against parametric mu, lambda. If they differ badly, calls Die().
 * If OK, returns EXIT_SUCCESS.
 *
 * CVS $Id: evd_test.c,v 1.7 2003/04/14 16:00:17 eddy Exp $
 */

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <float.h>
#include <math.h>
#include <string.h>
#include <stdarg.h>
#include <sys/stat.h>
#include <ctype.h>
#include <time.h>
#include "histogram.h"

/* Function: HMMERBanner()
 * Date:     SRE, Sun Jun 20 17:19:41 1999 [Graeme's kitchen]
 *
 * Purpose:  Print a package version and copyright banner.
 *           Used by all the main()'s.
 *
 *    Expects to be able to pick up defined preprocessor variables:
 *    variable          example
 *    --------           --------------
 *    PACKAGE_NAME      "HMMER"
 *    PACKAGE_VERSION   "2.0.42"
 *    PACKAGE_DATE      "April 1999"
 *    PACKAGE_COPYRIGHT "Copyright (C) 1992-1999 Washington University School of Medicine"
 *    PACKAGE_LICENSE   "Freely distributed under the GNU General Public License (GPL)."
 *
 *    This gives us a general mechanism to update release information
 *    without changing multiple points in the code.
 *
 * Args:     fp     - where to print it
 *           banner - one-line program description, e.g.:
 *                    "foobar - make bars from foo with elan"
 * Returns:  (void)
 */
void
HMMERBanner(FILE *fp, char *banner)
{
  fprintf(fp, "%s\n", banner);
  fprintf(fp, "- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -\n");
}

//#include "structs.h"
//#include "funcs.h"
//#include "globals.h"
//#include "squid.h"

/* Function: Gaussrandom()
 *
 * Pick a Gaussian-distributed random variable
 * with some mean and standard deviation, and
 * return it.
 *
 * Based on RANLIB.c public domain implementation.
 * Thanks to the authors, Barry W. Brown and James Lovato,
 * University of Texas, M.D. Anderson Cancer Center, Houston TX.
 * Their implementation is from Ahrens and Dieter, "Extensions
 * of Forsythe's method for random sampling from the normal
 * distribution", Math. Comput. 27:927-937 (1973).
 *
 * Impenetrability of the code is to be blamed on its FORTRAN/f2c lineage.
 *
 */
double
Gaussrandom(double mean, double stddev)
{
  static double a[32] = {
    0.0,3.917609E-2,7.841241E-2,0.11777,0.1573107,0.1970991,0.2372021,0.2776904,    0.3186394,0.36013,0.4022501,0.4450965,0.4887764,0.5334097,0.5791322,
    0.626099,0.6744898,0.7245144,0.7764218,0.8305109,0.8871466,0.9467818,
    1.00999,1.077516,1.150349,1.229859,1.318011,1.417797,1.534121,1.67594,
    1.862732,2.153875
  };
  static double d[31] = {
    0.0,0.0,0.0,0.0,0.0,0.2636843,0.2425085,0.2255674,0.2116342,0.1999243,
    0.1899108,0.1812252,0.1736014,0.1668419,0.1607967,0.1553497,0.1504094,
    0.1459026,0.14177,0.1379632,0.1344418,0.1311722,0.128126,0.1252791,
    0.1226109,0.1201036,0.1177417,0.1155119,0.1134023,0.1114027,0.1095039
  };
  static double t[31] = {
    7.673828E-4,2.30687E-3,3.860618E-3,5.438454E-3,7.0507E-3,8.708396E-3,
    1.042357E-2,1.220953E-2,1.408125E-2,1.605579E-2,1.81529E-2,2.039573E-2,
    2.281177E-2,2.543407E-2,2.830296E-2,3.146822E-2,3.499233E-2,3.895483E-2,
    4.345878E-2,4.864035E-2,5.468334E-2,6.184222E-2,7.047983E-2,8.113195E-2,
    9.462444E-2,0.1123001,0.136498,0.1716886,0.2276241,0.330498,0.5847031
  };
  static double h[31] = {
    3.920617E-2,3.932705E-2,3.951E-2,3.975703E-2,4.007093E-2,4.045533E-2,
    4.091481E-2,4.145507E-2,4.208311E-2,4.280748E-2,4.363863E-2,4.458932E-2,
    4.567523E-2,4.691571E-2,4.833487E-2,4.996298E-2,5.183859E-2,5.401138E-2,
    5.654656E-2,5.95313E-2,6.308489E-2,6.737503E-2,7.264544E-2,7.926471E-2,
    8.781922E-2,9.930398E-2,0.11556,0.1404344,0.1836142,0.2790016,0.7010474
  };
  static long i;
  static double snorm,u,s,ustar,aa,w,y,tt;

  u = sre_random();
  s = 0.0;
  if(u > 0.5) s = 1.0;
  u += (u-s);
  u = 32.0*u;
  i = (long) (u);
  if(i == 32) i = 31;
  if(i == 0) goto S100;
  /*
   * START CENTER
   */
  ustar = u-(double)i;
  aa = *(a+i-1);
S40:
  if(ustar <= *(t+i-1)) goto S60;
  w = (ustar-*(t+i-1))**(h+i-1);
S50:
  /*
   * EXIT   (BOTH CASES)
   */
  y = aa+w;
  snorm = y;
  if(s == 1.0) snorm = -y;
  return (stddev*snorm + mean);
S60:
  /*
   * CENTER CONTINUED
   */
  u = sre_random();
  w = u*(*(a+i)-aa);
  tt = (0.5*w+aa)*w;
  goto S80;
S70:
  tt = u;
  ustar = sre_random();
S80:
  if(ustar > tt) goto S50;
  u = sre_random();
  if(ustar >= u) goto S70;
  ustar = sre_random();
  goto S40;
S100:
  /*
   * START TAIL
   */
  i = 6;
  aa = *(a+31);
  goto S120;
S110:
  aa += *(d+i-1);
  i += 1;
S120:
  u += u;
  if(u < 1.0) goto S110;
  u -= 1.0;
S140:
  w = u**(d+i-1);
  tt = (0.5*w+aa)*w;
  goto S160;
S150:
  tt = u;
S160:
  ustar = sre_random();
  if(ustar > tt) goto S50;
  u = sre_random();
  if(ustar >= u) goto S150;
  u = sre_random();
  goto S140;
}


/* Function: Getopt()
 *
 * Purpose:  Portable command line option parsing with abbreviated
 *           option switches. Replaces UNIX getopt(). Using UNIX getopt()
 *           hinders portability to non-UNIX platforms, and getopt()
 *           is also limited to single letter options.
 *
 *           Getopt() implements a superset of UNIX getopt().
 *           All of getopt()'s single-character switch behavior
 *           is emulated, and "--" by itself terminates the options.
 *           Additionally, Getopt() provides extended switches
 *           like "--youroptionhere", and Getopt() type checks
 *           arguments.
 *
 *           Extended options must start with "--", as in "--option1".
 *           Normal options must start with "-", as in "-o".
 *           Normal options may be concatenated, as in "-a -b" == "-ab".
 *
 *           See bottom of this .c file after #fdef GETOPT_TESTDRIVER
 *           for an example of calling Getopt().
 *
 * Args:     argc  - from main(). number of elems in argv.
 *           argv  - from main(). argv[0] is the name of the command.
 *           opt   - array of opt_s structures, defining option switches
 *           nopts - number of switches in opt
 *           usage - a (possibly long) string to print if usage error.
 *           ret_optind - RETURN: the index in argv[] of the next
 *                        valid command-line token.
 *           ret_optname- RETURN: ptr to the name of option switch
 *                        seen, or NULL if no option was seen.
 *           ret_optarg - RETURN: ptr to the optional argument, if any;
 *                        NULL if option takes no argument.
 *
 * Return:   1 if a valid option was parsed.
 *           0 if no option was found, and command-line parsing is complete.
 *           Die()'s here if an error is detected.
 */
int
Getopt(int argc, char **argv, struct opt_s *opt, int nopts, char *usage,
       int *ret_optind, char **ret_optname, char **ret_optarg)
{
  int i;
  int arglen;
  int nmatch;
  static int optind   = 1;        /* init to 1 on first call  */
  static char *optptr = NULL;     /* ptr to next valid switch */
  int opti = 0;			  /* init only to silence gcc uninit warnings */

  /* Check to see if we've run out of options.
   * A '-' by itself is an argument (e.g. "read from stdin")
   * not an option.
   */
  if (optind >= argc || argv[optind][0] != '-' || strcmp(argv[optind], "-") == 0)
    {
      *ret_optind  = optind;
      *ret_optarg  = NULL;
      *ret_optname = NULL;
      return 0;
    }

  /* Check to see if we're being told that this is the end
   * of the options with the special "--" flag.
   */
  if (strcmp(argv[optind], "--") == 0)
    {
      optind++;
      *ret_optind  = optind;
      *ret_optname = NULL;
      *ret_optarg  = NULL;
      return 0;
    }

  /* We have a real option. Find which one it is.
   * We handle single letter switches "-o" separately
   * from full switches "--option", based on the "-" vs. "--"
   * prefix -- single letter switches can be concatenated
   * as long as they don't have arguments.
   */
				/* full option */
  if (optptr == NULL && strncmp(argv[optind], "--", 2) == 0)
    {
      /* Use optptr to parse argument in options of form "--foo=666"
       */
      if ((optptr = strchr(argv[optind], '=')) != NULL)
	{ *optptr = '\0'; optptr++; }

      arglen = strlen(argv[optind]);
      nmatch = 0;
      for (i = 0; i < nopts; i++)
	if (opt[i].single == FALSE &&
	    strncmp(opt[i].name, argv[optind], arglen) == 0)
	  {
	    nmatch++;
	    opti = i;
	    if (arglen == (int)strlen(opt[i].name)) break; /* exact match, stop now */
	  }
      if (nmatch > 1 && arglen != (int)strlen(opt[i].name))
	Die("Option \"%s\" is ambiguous; please be more specific.\n%s",
	    argv[optind], usage);
      if (nmatch == 0)
	Die("No such option \"%s\".\n%s", argv[optind], usage);

      *ret_optname = opt[opti].name;

      /* Set the argument, if there is one
       */
      if (opt[opti].argtype != sqdARG_NONE)
	{
	  if (optptr != NULL)
	    {			/* --foo=666 style */
	      *ret_optarg = optptr;
	      optptr = NULL;
	      optind++;
	    }
	  else if (optind+1 >= argc)
	    Die("Option %s requires an argument\n%s", opt[opti].name, usage);
	  else			/* "--foo 666" style */
	    {
	      *ret_optarg = argv[optind+1];
	      optind+=2;
	    }
	}
      else  /* sqdARG_NONE */
	{
	  if (optptr != NULL)
	    Die("Option %s does not take an argument\n%s", opt[opti].name, usage);
	  *ret_optarg = NULL;
	  optind++;
	}
    }
  else				/* else, a single letter option "-o" */
    {
				/* find the option */
      if (optptr == NULL)
	optptr = argv[optind]+1;
      for (opti = -1, i = 0; i < nopts; i++)
	if (opt[i].single == TRUE && *optptr == opt[i].name[1])
	  { opti = i; break; }
      if (opti == -1)
	Die("No such option \"%c\".\n%s", *optptr, usage);
      *ret_optname = opt[opti].name;

				/* set the argument, if there is one */
      if (opt[opti].argtype != sqdARG_NONE)
	{
	  if (*(optptr+1) != '\0')   /* attached argument */
	    {
	      *ret_optarg = optptr+1;
	      optind++;
	    }
	  else if (optind+1 < argc) /* unattached argument */
	    {
	      *ret_optarg = argv[optind+1];
	      optind+=2;
	    }
	  else Die("Option %s requires an argument\n%s", opt[opti].name, usage);

	  optptr = NULL;	/* can't concatenate after an argument */
	}
      else  /* sqdARG_NONE */
	{
	  *ret_optarg = NULL;
	  if (*(optptr+1) != '\0')   /* concatenation */
	    optptr++;
	  else
	    {
	      optind++;                /* move to next field */
	      optptr = NULL;
	    }
	}

    }

  /* Type check the argument, if there is one
   */
  if (opt[opti].argtype != sqdARG_NONE)
    {
      if (opt[opti].argtype == sqdARG_INT && ! IsInt(*ret_optarg))
	Die("Option %s requires an integer argument\n%s",
	    opt[opti].name, usage);
      else if (opt[opti].argtype == sqdARG_FLOAT && ! IsReal(*ret_optarg))
	Die("Option %s requires a numerical argument\n%s",
	    opt[opti].name, usage);
      else if (opt[opti].argtype == sqdARG_CHAR && strlen(*ret_optarg) != 1)
	Die("Option %s requires a single-character argument\n%s",
	    opt[opti].name, usage);
      /* sqdARG_STRING is always ok, no type check necessary */
    }

  *ret_optind = optind;
  return 1;
}

static char banner[] = "\
evd_test : testing of EVD code in histogram.c";

static char usage[] = "\
Usage: testdriver [-options]\n\
  Available options are:\n\
  -h              : help; display this usage info\n\
  -c <x>          : censor data below <x>\n\
  -e <n>          : sample <n> times from EVD\n\
  -g <n>          : add <n> Gaussian samples of \"noise\"\n\
  -n <n>          : set number of trials to <n>\n\
  -s <n>          : set random seed to <n>\n\
  -v              : be verbose (default is to simply exit with status 1 or 0)\n\
";

static char experts[] = "\
  --xmgr <file>   : save graphical data to <file>\n\
  --hist          : fit to histogram instead of raw samples\n\
  --loglog <file> : save log log regression line to <file>\n\
  --regress       : do old-style linear regression fit, not ML\n\
  --mu <x>        : set EVD mu to <x>\n\
  --lambda <x>    : set EVD lambda to <x>\n\
  --mean <x>      : set Gaussian mean to <x>\n\
  --sd   <x>      : set Gaussian std. dev. to <x>\n\
\n";

static struct opt_s OPTIONS[] = {
  { "-h",       TRUE,  sqdARG_NONE  },
  { "-c",       TRUE,  sqdARG_FLOAT },
  { "-e",       TRUE,  sqdARG_INT },
  { "-g",       TRUE,  sqdARG_INT },
  { "-n",       TRUE,  sqdARG_INT },
  { "-s",       TRUE,  sqdARG_INT   },
  { "-v",       TRUE,  sqdARG_NONE  },
  { "--xmgr",   FALSE, sqdARG_STRING},
  { "--hist",   FALSE, sqdARG_NONE},
  { "--loglog", FALSE, sqdARG_STRING},
  { "--regress",FALSE, sqdARG_NONE},
  { "--mu",     FALSE, sqdARG_FLOAT},
  { "--lambda", FALSE, sqdARG_FLOAT},
  { "--mean",   FALSE, sqdARG_FLOAT},
  { "--sd",     FALSE, sqdARG_FLOAT},
};
#define NOPTIONS (sizeof(OPTIONS) / sizeof(struct opt_s))

int
main(int argc, char **argv)
{
  struct histogram_s *h;        /* histogram structure          */
  int ntrials;			/* number of different fits     */
  int be_verbose;               /* option: TRUE to show output  */
  int seed;                     /* option: random number seed   */
  int   nevd;                   /* # of samples from EVD        */
  float mu;			/* EVD mu parameter             */
  float lambda;			/* EVD lambda parameter         */
  int   ngauss;			/* # of samples from Gaussian   */
  float mean;			/* Gaussian "noise" mean        */
  float sd;			/* Gaussian "noise" std. dev.   */
  float x;			/* a random sample              */
  int   i, idx;
  float *val;			/* array of samples             */
  float mlmu;			/* estimate of mu               */
  float mllambda;		/* estimate of lambda           */

  char *xmgrfile;               /* output file for XMGR graph data */
  char *logfile;                /* output file for regression line */
  FILE *xmgrfp;                 /* open output file                */
  FILE *logfp;                  /* open log log file               */
  int   do_ml;			/* TRUE to do a max likelihood fit */
  int   fit_hist;		/* TRUE to fit histogram instead of samples */
  int   censoring;		/* TRUE to left-censor the data    */
  float censorlevel;		/* value to censor at              */

  char *optname;                /* name of option found by Getopt()         */
  char *optarg;                 /* argument found by Getopt()               */
  int   optind;                 /* index in argv[]                          */


  /***********************************************
   * Parse command line
   ***********************************************/
  be_verbose = FALSE;
  seed       = (int) time ((time_t *) NULL);
  ntrials    = 1;
  nevd       = 1000;
  mu         = -20.0;
  lambda     = 0.4;
  ngauss     = 0;
  mean       = 20.;
  sd         = 20.;
  xmgrfile   = NULL;
  logfile    = NULL;
  xmgrfp     = NULL;
  logfp      = NULL;
  do_ml      = TRUE;
  censoring  = FALSE;
  censorlevel= 0.;
  fit_hist   = FALSE;

  while (Getopt(argc, argv, OPTIONS, NOPTIONS, usage,
                &optind, &optname, &optarg))  {
    if      (strcmp(optname, "-e")       == 0) { nevd       = atoi(optarg); }
    else if (strcmp(optname, "-c")       == 0) { censoring  = TRUE;
                                                 censorlevel= atof(optarg); }
    else if (strcmp(optname, "-g")       == 0) { ngauss     = atoi(optarg); }
    else if (strcmp(optname, "-n")       == 0) { ntrials    = atoi(optarg); }
    else if (strcmp(optname, "-s")       == 0) { seed       = atoi(optarg); }
    else if (strcmp(optname, "-v")       == 0) { be_verbose = TRUE;         }
    else if (strcmp(optname, "--xmgr")   == 0) { xmgrfile   = optarg; }
    else if (strcmp(optname, "--hist")   == 0) { fit_hist   = TRUE; }
    else if (strcmp(optname, "--loglog") == 0) { logfile    = optarg; }
    else if (strcmp(optname, "--regress")== 0) { do_ml      = FALSE; }
    else if (strcmp(optname, "--mu")     == 0) { mu         = atof(optarg); }
    else if (strcmp(optname, "--lambda") == 0) { lambda     = atof(optarg); }
    else if (strcmp(optname, "--mean")   == 0) { mean       = atof(optarg); }
    else if (strcmp(optname, "--sd")     == 0) { sd         = atof(optarg); }
    else if (strcmp(optname, "-h")       == 0) {
      HMMERBanner(stdout, banner);
      puts(usage);
      puts(experts);
      exit(0);
    }
  }
  if (argc - optind != 0)
    Die("Incorrect number of arguments.\n%s\n", usage);

  sre_srandom(seed);

  /****************************************************************
   * Print options
   ****************************************************************/

  if (be_verbose)
    {
      puts("--------------------------------------------------------");
      printf("EVD samples    = %d\n", nevd);
      printf("mu, lambda     = %f, %f\n", mu, lambda);
      if (ngauss > 0) {
	printf("Gaussian noise = %d\n", ngauss);
	printf("mean, sd       = %f, %f\n", mean, sd);
      }
      if (censoring) printf("pre-censoring  = ON, at %f\n", censorlevel);
      printf("total trials   = %d\n", ntrials);
      printf("random seed    = %d\n", seed);
      printf("fit method     = %s\n", do_ml ? "ML" : "linear regression");
      printf("fit is to      = %s\n", fit_hist ? "histogram" : "list");
      puts("--------------------------------------------------------");
    }

  if (xmgrfile != NULL)
    if ((xmgrfp = fopen(xmgrfile, "w")) == NULL)
      Die("Failed to open output file %s", xmgrfile);
  if (logfile != NULL)
    if ((logfp = fopen(logfile, "w")) == NULL)
      Die("Failed to open output file %s", logfile);

  /* Generate random EVD "signal" (and Gaussian "noise")
   * samples and put them in the histogram
   */
  while (ntrials--)
    {
      val = MallocOrDie(sizeof(double) * (nevd+ngauss));
      h   = AllocHistogram(-20, 20, 10);

				/* EVD signal */
      idx = 0;
      for (i = 0; i < nevd; i++)
	{
	  x = EVDrandom(mu, lambda);
	  if (! censoring || x > censorlevel)
	    {
	      AddToHistogram(h, x);
	      val[idx] = x;
	      idx++;
	    }
	}
				/* Gaussian noise */
      for (; i < nevd + ngauss; i++)
	{
	  x = Gaussrandom(mean, sd);
	  if (! censoring || x > censorlevel)
	    {
	      AddToHistogram(h, x);
	      val[idx] = x;
	      idx++;
	    }
	}

      if (do_ml)
	{

	  if (censoring)
	    {
	      if (be_verbose)
		printf("I have censored the data at %f: %d observed, %d censored\n", censorlevel, idx, (nevd+ngauss)-idx);

	      EVDCensoredFit(val, NULL, idx,
			     (nevd+ngauss)-idx, censorlevel,
			     &mlmu, &mllambda);
	      ExtremeValueSetHistogram(h, (float) mlmu, (float) mllambda,
				       censorlevel, h->highscore, 1);
	    }
	  else
	    {
	      if (fit_hist)
		{
		  ExtremeValueFitHistogram(h, TRUE, 20.);
		}
	      else
		{
		  EVDMaxLikelyFit(val, NULL, idx, &mlmu, &mllambda);
		  ExtremeValueSetHistogram(h, (float) mlmu, (float) mllambda,
					   h->lowscore, h->highscore, 2);
		}
	    }
	}
      else
	EVDBasicFit(h);

      if (be_verbose) {
	printf("%f\tmu\n",     h->param[EVD_MU]);
	printf("%f\tlambda\n", h->param[EVD_LAMBDA]);
	printf("%f\t%% error on mu\n",
	       fabs(100. * (h->param[EVD_MU] - mu) / mu));
	printf("%f\t%% error on lambda\n",
	       fabs(100. * (h->param[EVD_LAMBDA] - lambda) / lambda));
	printf("%f\tchi-squared P value\n", h->chip);
      }
      if (xmgrfp != NULL) PrintXMGRHistogram(xmgrfp, h);
      /*      if (xmgrfp != NULL) PrintXMGRDistribution(xmgrfp, h); */
      if (logfp  != NULL) PrintXMGRRegressionLine(logfp, h);

      /* Generate the expected lines: sets 5,7 of xmgrfile (manually delete 4,6)
       *                              set 3 of loglogfile  (manually delete 2)
       */
      ExtremeValueSetHistogram(h, mu, lambda, h->lowscore, h->highscore, 0);
      if (xmgrfp != NULL) PrintXMGRHistogram(xmgrfp, h);
      /*      if (xmgrfp != NULL) PrintXMGRDistribution(xmgrfp, h); */
      if (logfp  != NULL) PrintXMGRRegressionLine(logfp, h);

      /* Do the internal test.
       * Criterion: on a 1000 sample EVD of u = -40 and lambda = 0.4,
       * estimate u to within +/- 2 and lambda to within +/- 0.05.
       */
      if (fabs(h->param[EVD_MU] - mu) > 2.)
	Die("evd_test: tolerance to mu exceeded (%f)",
	    fabs(h->param[EVD_MU] - mu));
      if (fabs(h->param[EVD_LAMBDA] - lambda) > 0.05)
	Die("evd_test: tolerance to lambda exceeded (%f)",
	    fabs(h->param[EVD_LAMBDA] - lambda));

      FreeHistogram(h);
      free(val);
    }

  if (xmgrfp != NULL) fclose(xmgrfp);
  if (logfp != NULL)  fclose(logfp);
  return EXIT_SUCCESS;
}
