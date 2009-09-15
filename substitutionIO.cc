#include <stdlib.h>
#include "substitutionIO.h"
#include "xmlParser.h"
#include <assert.h>

extern "C" {
	#include "fastCMaths.h"
	#include "commonC.h"
};

/////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////
//substitution matrix io functions
/////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////

struct HKYModelParameters *readHKYSubModel(XMLNode xMainNode) {
	XMLNode xNode = xMainNode.getChildNode("hky");

	struct HKYModelParameters *hKY;

	hKY = (struct HKYModelParameters *)mallocLocal(sizeof(struct HKYModelParameters));

	sscanf(xNode.getChildNode("freqA").getAttribute("value"), "%f", &hKY->freqA);
	sscanf(xNode.getChildNode("freqC").getAttribute("value"), "%f", &hKY->freqC);
	sscanf(xNode.getChildNode("freqG").getAttribute("value"), "%f", &hKY->freqG);
	sscanf(xNode.getChildNode("freqT").getAttribute("value"), "%f", &hKY->freqT);
	sscanf(xNode.getChildNode("transitionTransversionRatio").getAttribute("value"), "%f", &hKY->transitionTransversionRatio);
	sscanf(xNode.getChildNode("distance").getAttribute("value"), "%f", &hKY->distance);

	checkHKY(hKY);
	return hKY;
}

void writeHKYSubModel(XMLNode xMainNode, struct HKYModelParameters *hKY) {
	char cA[100];

	XMLNode xNode = xMainNode.getChildNode("hky");
	if((&xNode) != (&xNode.emptyXMLNode)) {
		xNode.deleteNodeContent();
	}
	xNode = xMainNode.addChild("hky");

	sprintf(cA, "%f", hKY->freqA);
	xNode.addChild("freqA").addAttribute("value", cA);

	sprintf(cA, "%f", hKY->freqC);
	xNode.addChild("freqC").addAttribute("value", cA);

	sprintf(cA, "%f", hKY->freqG);
	xNode.addChild("freqG").addAttribute("value", cA);

	sprintf(cA, "%f", hKY->freqT);
	xNode.addChild("freqT").addAttribute("value", cA);

	sprintf(cA, "%f", hKY->transitionTransversionRatio);
	xNode.addChild("transitionTransversionRatio").addAttribute("value", cA);

	sprintf(cA, "%f", hKY->distance);
	xNode.addChild("distance").addAttribute("value", cA);
}

void logHKY(struct HKYModelParameters *hKY) {
	logInfo("HKY Model parameters are as follows: freq A: %f, freq C: %f, freq G: %f, freq T: %f, transition/transversion ratio: %f, distance: %f\n",
					hKY->freqA, hKY->freqC, hKY->freqG, hKY->freqT,
					hKY->transitionTransversionRatio, hKY->distance);
}

void checkHKY(struct HKYModelParameters *hKY) {
#ifdef BEN_DEBUG
	float i = hKY->freqA + hKY->freqC + hKY->freqG + hKY->freqT;

	assert(i < 1.001);
	assert(i > 0.999);
	assert(hKY->transitionTransversionRatio > 0.0);
	assert(hKY->distance >= 0.0);
#endif
}
