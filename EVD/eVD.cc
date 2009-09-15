//#include <stdlib.h>
//#include <stdio.h>
//#include <string.h>

#include <assert.h>
#include <iostream>
#include <sstream>

#include "eVD.h"
#include "xmlParser.h"
#include "XMLTools.h"

extern "C" {
	#include "bioioC.h"
	#include "commonC.h"
};

////////////////////////////////////////////////
////////////////////////////////////////////////
////////////////////////////////////////////////
//EVD stuff
////////////////////////////////////////////////
////////////////////////////////////////////////
////////////////////////////////////////////////

struct EVDParameters *readEVDParameters(XMLNode xMainNode) {
	struct EVDParameters *eVDParameters;

	eVDParameters = (struct EVDParameters *)malloc(sizeof(struct EVDParameters));

	XMLNode xNode = xMainNode.getChildNode("evd");

	assert(sscanf(xNode.getChildNode("mu").getAttribute("value"), "%lf", &eVDParameters->mu) == 1);
	assert(sscanf(xNode.getChildNode("lambda").getAttribute("value"), "%lf", &eVDParameters->lambda) == 1);
	assert(sscanf(xNode.getChildNode("norm").getAttribute("value"), "%lf", &eVDParameters->norm) == 1);
	assert(sscanf(xNode.getChildNode("chisq").getAttribute("value"), "%lf", &eVDParameters->chisq) == 1);
	assert(sscanf(xNode.getChildNode("pChiSq").getAttribute("value"), "%lf", &eVDParameters->pChiSq) == 1);
	assert(sscanf(xNode.getChildNode("scoreNo").getAttribute("value"), "%i", &eVDParameters->scoreNo) == 1);
	eVDParameters->scores = (double *)malloc(sizeof(double)*(eVDParameters->scoreNo + 1));
	if(eVDParameters->scoreNo > 0) {
		readDoubles(xNode.getText(), eVDParameters->scoreNo, eVDParameters->scores);
	}

	checkEVDParameters(eVDParameters);
	return eVDParameters;
}

void writeEVDParameters(XMLNode xMainNode, struct EVDParameters *eVDParameters) {
	int i;
	char cA[100];
	using std::ostringstream; // stream insertion operators
	ostringstream cA2;

	XMLNode xNode = xMainNode.getChildNode("evd");
	if((&xNode) != (&xNode.emptyXMLNode)) {
		xNode.deleteNodeContent();
	}
	xNode = xMainNode.addChild("evd");

	sprintf(cA, "%f", eVDParameters->mu);
	xNode.addChild("mu").addAttribute("value", cA);

	sprintf(cA, "%f", eVDParameters->lambda);
	xNode.addChild("lambda").addAttribute("value", cA);

	sprintf(cA, "%f", eVDParameters->norm);
	xNode.addChild("norm").addAttribute("value", cA);

	sprintf(cA, "%f", eVDParameters->chisq);
	xNode.addChild("chisq").addAttribute("value", cA);

	sprintf(cA, "%f", eVDParameters->pChiSq);
	xNode.addChild("pChiSq").addAttribute("value", cA);

	sprintf(cA, "%i", eVDParameters->scoreNo);
	xNode.addChild("scoreNo").addAttribute("value", cA);

	for(i=0; i<eVDParameters->scoreNo; i++) {
		cA2 << eVDParameters->scores[i] << " ";
	}
	xNode.updateText(cA2.str().c_str(), xNode.getText());
}

void destructEVDParameters(struct EVDParameters *eVDParameters) {
	free(eVDParameters->scores);
	free(eVDParameters);
}

void checkEVDParameters(struct EVDParameters *eVDParameters) {
	//pass!
}

void logEVDParameters(struct EVDParameters *eVDParameters) {
	logInfo("Model parameters are as follows: mu: %f, lambda: %f, nm normalising factor: %f, chi-squared: %f, p(chi-squared): %f, scale: %f\n",
			eVDParameters->mu, eVDParameters->lambda, eVDParameters->norm, eVDParameters->chisq, eVDParameters->pChiSq, eVDParameters->scale);
}

double calculateEVDProbFromScore(struct EVDParameters *eP, double score, double normSize) {
	/*
	 * Calculates the extreme value distribution probability of a given log odds score.
	 */
	double pSGTX = 1.0 - exp(-exp(-eP->lambda * (score - eP->mu)));
	double pXN = 1.0 - exp(-(normSize/eP->norm) * pSGTX);
	return pXN;
}

int readScores(XMLNode xMainNode, char *childName, char *attributeName, double **scores) {
	/*
	 * Reads scores from an XML file.
	 */
	int i, j, k;

	i=xMainNode.nChildNode(childName);
	*scores = (double *)mallocLocal(sizeof(double)*i);

	k = 0;
	for(j=0; j<i; j++) {
		XMLNode xNode = xMainNode.getChildNode(childName, &k);
		assert(sscanf(xNode.getChildNode(attributeName).getAttribute("value"), "%lf", &((*scores)[i])) == 1);
	}

	return i;
}

