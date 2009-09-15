#include <cstdlib>
#include <cstdio>
#include <limits.h>
#include <assert.h>
#include <string.h>
#include <inttypes.h>
#include <float.h>

#include "eVD.h"
#include "XMLTools.h"
#include "xmlParser.h"
#include "histogram.h"

extern "C" {
#include "commonC.h"
#include "bioioC.h"
}

#include "Argument_helper.h"

int main(int argc, char *argv[]) {
	/*
	 * Script to fit an extreme value distribution to a set of scores generated from
	 * some null model. Uses the HMMER library os Sean Eddy, found in Histogram.c
	 */

	//////////////////////////////////////////////
	//Get inputs.
	//////////////////////////////////////////////

	FILE *fileHandle;
	std::string scoresFile = "None";
	std::string outputFile = "None";
	std::string logLevelString = "None";
	std::string attributeName = "logOddsProb";
	int32_t i;
	double minScore, maxScore;
	struct histogram_s *histogram;
	dsr::Argument_helper ah;
	struct EVDParameters *eVDParameters;
	double norm = 1.0;

	double minDesiredBinSize = 10.0;
	double maxDesiredBinSize = 50.0;
	double binSizeIncrement = 1.0;
	double desiredBinSize;
	double averageBinSize;
	double scale;

	bool xML = false;
	double d;

	ah.new_named_string('a', "scoresFile", "", "The score file as a list of int scores", scoresFile);
	ah.new_named_string('b', "outputFile", "", "The output file", outputFile);
	ah.new_named_double('c', "norm", "", "The normalisation factor", norm);
	ah.new_named_double('d', "minBinSize", "", "The min desired bin size", minDesiredBinSize);
	ah.new_named_double('e', "maxBinSize", "", "The max desired bin size", maxDesiredBinSize);
	ah.new_named_double('f', "binSizeIncrement", "", "The bin size increment", binSizeIncrement);
	ah.new_named_string('g', "logLevel", "", "Set the log level", logLevelString);
	ah.new_flag('h', "xml", "Input file is XML formatted", xML);
	ah.new_named_string('i', "attributeName", "", "The attribute name of the value for XML parsing", attributeName);

	ah.set_description("eVDScript");
	ah.set_author("Benedict Paten, benedict@soe.ucsc.edu");
	ah.set_version("0.1");

	ah.process(argc, argv);

	//////////////////////////////////////////////
	//Set up logging
	//////////////////////////////////////////////

	if(strcmp(logLevelString.c_str(), "INFO") == 0) {
		setLogLevel(LOGGING_INFO);
	}
	if(strcmp(logLevelString.c_str(), "DEBUG") == 0) {
		setLogLevel(LOGGING_DEBUG);
	}

	logInfo("Read parameters and starting EVD script.\n");

	//////////////////////////////////////////////
	//Parse score file
	//////////////////////////////////////////////

	minScore = INFINITY;
	maxScore = -INFINITY;
	eVDParameters = (struct EVDParameters *)mallocLocal(sizeof(struct EVDParameters));

	if(xML == TRUE) {
		XMLNode xMainNode = readXMLFile(scoresFile.c_str());
		eVDParameters->scoreNo = readScores(xMainNode, "scores", "logAlignmentProb", &eVDParameters->scores);
	}
	else {
		fileHandle = fopen(scoresFile.c_str(), "r");
		eVDParameters->scoreNo = 0;
		while(fscanf(fileHandle, "%lf", &d) == 1) {
			eVDParameters->scoreNo++;
		}
		fclose(fileHandle);

		fileHandle = fopen(scoresFile.c_str(), "r");
		eVDParameters->scores = (double *)mallocLocal(sizeof(double)*eVDParameters->scoreNo);
		i = 0;
		while(fscanf(fileHandle, "%lf", &d) == 1) {
			eVDParameters->scores[i++] = d;
		}
		fclose(fileHandle);
		assert(i == eVDParameters->scoreNo);
	}

	for(i=0; i<eVDParameters->scoreNo; i++) {
		if(eVDParameters->scores[i] < minScore) {
			minScore = eVDParameters->scores[i];
		}
		if(eVDParameters->scores[i] > maxScore) {
			maxScore = eVDParameters->scores[i];
		}
	}

	logInfo("Min score: %f, max score %f, score number: %i\n", minScore, maxScore, eVDParameters->scoreNo);

	//////////////////////////////////////////////
	//Do the evd fit
	//////////////////////////////////////////////

	//correct for when we have more than the minimum amounts of data for the fit.
	averageBinSize = eVDParameters->scoreNo/(maxScore - minScore);
	if(minDesiredBinSize < averageBinSize) {
		minDesiredBinSize = averageBinSize;
	}
	if(maxDesiredBinSize < minDesiredBinSize) {
		maxDesiredBinSize = minDesiredBinSize;
	}

	eVDParameters->pChiSq = -INFINITY;
	histogram = NULL;

	for(desiredBinSize=minDesiredBinSize; desiredBinSize<=maxDesiredBinSize; desiredBinSize+=binSizeIncrement) {
		if(histogram != NULL) {
			FreeHistogram(histogram);
		}

		scale = desiredBinSize/averageBinSize;
		logDebug("Average bin size: %f, desired bin size: %f, scale factor: %f\n", averageBinSize, desiredBinSize, scale);

		histogram = AllocHistogram(minScore/scale, maxScore/scale, (maxScore - minScore + 1)/scale);

		for(i=0; i<eVDParameters->scoreNo; i++) {
			AddToHistogram(histogram, eVDParameters->scores[i]/scale);
		}
		if(ExtremeValueFitHistogram(histogram, TRUE, INT_MAX) == 0) {
			fprintf(stderr, "Couldn't fit the histogram!\n");
		}
		else {
			if(eVDParameters->pChiSq < histogram->chip) {
				eVDParameters->lambda = histogram->param[EVD_LAMBDA]/scale;
				eVDParameters->mu = histogram->param[EVD_MU]*scale;
				eVDParameters->chisq = histogram->chisq;
				eVDParameters->pChiSq = histogram->chip;
				eVDParameters->norm = norm;
				eVDParameters->scale = scale;
				if(LOG_LEVEL == LOGGING_DEBUG) {
					PrintASCIIHistogram(stdout, histogram);
				}
				logEVDParameters(eVDParameters);
			}
		}
	}

	if(eVDParameters->pChiSq == -INFINITY) {
		fprintf(stderr, "Couldn't fit the histogram with any binning parameter!\n");
		return 1;
	}

	//////////////////////////////////////////////
	//Print to score file
	//////////////////////////////////////////////

	logEVDParameters(eVDParameters);

	if(strcmp(outputFile.c_str(), "None") != 0) {
		XMLNode xMainNode = readXMLFile(outputFile.c_str());
		writeEVDParameters(xMainNode, eVDParameters);
		writeXMLFile(outputFile.c_str(), xMainNode);

		logInfo("Written to output file: %s\n", outputFile.c_str());
	}
	else {
		logInfo("Not writing output file\n");
	}

	//////////////////////////////////////////////
	//Clean up
	//////////////////////////////////////////////

	FreeHistogram(histogram);
	destructEVDParameters(eVDParameters);

	return 0;
}
