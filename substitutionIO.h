#ifndef _substitutionIO_h_
#define _substitutionIO_h_

#include "xmlParser.h"

extern "C" {
	#include "fastCMaths.h"
};

/////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////
//substitution matrix io functions
/////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////

struct HKYModelParameters {
	float freqA;
	float freqC;
	float freqG;
	float freqT;
	float transitionTransversionRatio;
	float distance;
};

struct HKYModelParameters *readHKYSubModel(XMLNode xMainNode);

void writeHKYSubModel(XMLNode xMainNode, struct HKYModelParameters *hKY);

void logHKY(struct HKYModelParameters *hKY);

void checkHKY(struct HKYModelParameters *hKY);

#endif
