#ifndef _histogram_h_
#define _histogram_h_

/* Structure: histogram_s
 *
 * Keep a score histogram.
 *
 * The main implementation issue here is that the range of
 * scores is unknown, and will go negative. histogram is
 * a 0..max-min array that represents the range min..max.
 * A given score is indexed in histogram array as score-min.
 * The AddToHistogram() function deals with dynamically
 * resizing the histogram array when necessary.
 */
struct histogram_s {
  int *histogram;		/* counts of hits                     */
  int  min;			/* elem 0 of histogram == min         */
  int  max;                     /* last elem of histogram == max      */
  int  highscore;		/* highest active elem has this score */
  int  lowscore;		/* lowest active elem has this score  */
  int  lumpsize;		/* when resizing, overalloc by this   */
  int  total;			/* total # of hits counted            */

  float *expect;		/* expected counts of hits            */
  int    fit_type;		/* flag indicating distribution type  */
  float  param[3];		/* parameters used for fits           */
  float  chisq;			/* chi-squared val for goodness of fit*/
  float  chip;			/* P value for chisquared             */
};
#define HISTFIT_NONE     0	/* no fit done yet               */
#define HISTFIT_EVD      1	/* fit type = extreme value dist */
#define HISTFIT_GAUSSIAN 2	/* fit type = Gaussian           */
#define EVD_MU		 0	/* EVD fit parameter mu          */
#define EVD_LAMBDA       1	/* EVD fit parameter lambda      */
#define EVD_WONKA        2      /* EVD fit fudge factor          */
#define GAUSS_MEAN       0	/* Gaussian parameter mean       */
#define GAUSS_SD         1	/* Gaussian parameter std. dev.  */

/* from histogram.c
 * accumulation of scores
 */
extern struct histogram_s *AllocHistogram(int min, int max, int lumpsize);
extern void FreeHistogram(struct histogram_s *h);
extern void UnfitHistogram(struct histogram_s *h);
extern void AddToHistogram(struct histogram_s *h, float sc);
extern void PrintASCIIHistogram(FILE *fp, struct histogram_s *h);
extern void PrintXMGRHistogram(FILE *fp, struct histogram_s *h);
extern void PrintXMGRDistribution(FILE *fp, struct histogram_s *h);
extern void PrintXMGRRegressionLine(FILE *fp, struct histogram_s *h);
extern void EVDBasicFit(struct histogram_s *h);
extern int  ExtremeValueFitHistogram(struct histogram_s *h, int censor,
				     float high_hint);
extern void ExtremeValueSetHistogram(struct histogram_s *h, float mu, float lambda,
				     float low, float high, int ndegrees);
extern int  GaussianFitHistogram(struct histogram_s *h, float high_hint);
extern void GaussianSetHistogram(struct histogram_s *h, float mean, float sd);
extern double EVDDensity(float x, float mu, float lambda);
extern double EVDDistribution(float x, float mu, float lambda);
extern double ExtremeValueP (float x, float mu, float lambda);
extern double ExtremeValueP2(float x, float mu, float lambda, int N);
extern double ExtremeValueE (float x, float mu, float lambda, int N);
extern float  EVDrandom(float mu, float lambda);
extern int    EVDMaxLikelyFit(float *x, int *y, int n,
			      float *ret_mu, float *ret_lambda);
extern int    EVDCensoredFit(float *x, int *y, int n, int z, float c,
			     float *ret_mu, float *ret_lambda);
extern void   Lawless416(float *x, int *y, int n, float lambda,
			 float *ret_f, float *ret_df);
extern void   Lawless422(float *x, int *y, int n, int z, float c,
			 float lambda, float *ret_f, float *ret_df);


/* from sre_string.c
 */
extern void *sre_malloc(char *file, int line, size_t size);
extern void *sre_realloc(char *file, int line, void *p, size_t size);

/*
 * from sqerror.c
 */
extern void Die(char *format, ...);

/* from sre_math.c
 */
extern double   IncompleteGamma(double a, double x);

extern int      Linefit(float *x, float *y, int N,
		        float *ret_a, float *ret_b, float *ret_r);

/*
 * from types.c
 */
extern int  IsInt(char *s);
extern int  IsReal(char *s);

/* sre_random.h
 * Header file for sre_random.c
 *
 * SRE, Tue Oct  1 15:24:29 2002
 * CVS $Id: sre_random.h,v 1.1 2002/10/09 14:26:09 eddy Exp $
 */

extern double sre_random(void);

extern void sre_srandom(int seed);

/* Malloc/realloc calls are wrapped
 */

#define MallocOrDie(x)     sre_malloc(__FILE__, __LINE__, (x))
#define ReallocOrDie(x,y)  sre_realloc(__FILE__, __LINE__, (x), (y))

/****************************************************
 * Miscellaneous macros and defines
 ****************************************************/

#ifndef MIN
#define MIN(a,b)         (((a)<(b))?(a):(b))
#endif
#ifndef MAX
#define MAX(a,b)         (((a)>(b))?(a):(b))
#endif

/****************************************************
 * Three levels of debugging printf's and assert's
 *      level 1: little impact on verbosity or performance
 *      level 2: moderate impact
 *      level 3: high impact
 * Example:
 *    SQD_DPRINTF3(("Matrix row %d col %d = %f\n", i, j, val));
 * Note the double parentheses; these are important.
 ****************************************************/

#ifndef DEBUGLEVEL
#define DEBUGLEVEL 0
#endif

#if (DEBUGLEVEL >= 1)
#define SQD_DPRINTF1(x)  printf x
#define SQD_DASSERT1(x)  assert x
#else
#define SQD_DPRINTF1(x)
#define SQD_DASSERT1(x)
#endif
#if (DEBUGLEVEL >= 2)
#define SQD_DPRINTF2(x)  printf x
#define SQD_DASSERT2(x)  assert x
#else
#define SQD_DPRINTF2(x)
#define SQD_DASSERT2(x)
#endif
#if (DEBUGLEVEL >= 3)
#define SQD_DPRINTF3(x)  printf x
#define SQD_DASSERT3(x)  assert x
#else
#define SQD_DPRINTF3(x)
#define SQD_DASSERT3(x)
#endif


/* For convenience and (one hopes) clarity in boolean tests:
 */
#ifndef TRUE
#define TRUE 1
#endif
#ifndef FALSE
#define FALSE 0
#endif

/****************************************************
 * Support for a portable, flexible Getopt()
 ****************************************************/

/* Structure: opt_s
 *
 * Structure for declaring options to a main().
 */
struct opt_s {
  char *name;			/* name of option, e.g. "--option1" or "-o" */
  int   single;			/* TRUE if a single letter option           */
  int   argtype;		/* for typechecking, e.g. sqdARG_INT        */
};
				/* acceptable argtype's...           */
#define sqdARG_NONE   0		/* no argument                       */
#define sqdARG_INT    1		/* something that atoi() can grok    */
#define sqdARG_FLOAT  2		/* something that atof() can grok    */
#define sqdARG_CHAR   3		/* require single character or digit */
#define sqdARG_STRING 4		/* anything goes                     */

#endif
