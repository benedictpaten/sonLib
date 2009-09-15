#ifndef _eVD_h_
#define _eVD_h_

#include "xmlParser.h"
#include "XMLTools.h"

////////////////////////////////////////////////
////////////////////////////////////////////////
////////////////////////////////////////////////
//EVD stuff
////////////////////////////////////////////////
////////////////////////////////////////////////
////////////////////////////////////////////////

struct EVDParameters {
	double mu;
	double lambda;
	double norm;
	double chisq;
	double pChiSq;
	double scale;
	int scoreNo;
	double *scores;
};

struct EVDParameters *readEVDParameters(XMLNode xMainNode);

void writeEVDParameters(XMLNode xMainNode, struct EVDParameters *mP);

void destructEVDParameters(struct EVDParameters *eVDParameters);

void checkEVDParameters(struct EVDParameters *eVDParameters);

void logEVDParameters(struct EVDParameters *eVDParameters);

double calculateEVDProbFromScore(struct EVDParameters *eP, double score, double normSize);

/*
 * Method for extracting scores from an input XML file.
 */
int readScores(XMLNode xMainNode, char *childName, char *attributeName, double **scores);

#endif
