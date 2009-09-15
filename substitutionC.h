#ifndef SUBSTITUTIONC_H_
#define SUBSTITUTIONC_H_

#include <math.h>
#include "fastCMaths.h"
#include "commonC.h"

//subtitution stuff
#define GAP 1000
//just dna right now

struct SubModel {
   float *forward;
   float *backward;
   float *stationaryDistribution;
   int32_t alphabetSize;
};

struct SubModel *constructSubModel(float *forward,
                                   float *backward,
                                   float *stationaryDistribution,
                                   int32_t alphabetSize);

void destructSubModel(struct SubModel *subModel);

/////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////
//library functions for dealing with wV
/////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////

void copyWV(float *wVX, float *wVY, int32_t alphabetSize);

void transformWVByDistance(float *wV, float *subMatrix, float *result, int32_t alphabetSize);

void multiplyWV(float *wVX, float *wVY, float *result, int32_t alphabetSize);

void normaliseWV_GiveFac(float *wV, float *result, float normFac, int32_t alphabetSize);

void normaliseWV(float *wV, float *result, int32_t alphabetSize);

float combineWV(float *wVX, float *wVY, int32_t alphabetSize);

float sumWV(float *wV, int32_t alphabetSize);

void addWV(float *wVX, float *wVY, float *result, int32_t alphabetSize);

float * dNAMap_IUPACToWVFn(char i);

int32_t subMatCo(int32_t i, int32_t j, int32_t alphabetSize);

/////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////
//substitution matrix functions
/////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////

float *hKY(float distance, float freqA, float freqC, float freqG, float freqT,
                                float transitionTransversionRatio);

struct SubModel *constructHKYSubModel(float distance, float freqA, float freqC, float freqG, float freqT,
                                            float transitionTransversionRatio);

float *jukesCantor(float d);

struct SubModel *constructJukesCantorSubModel(float distance);

float *reverseSubMatrixInPlace(float *wV, int32_t alphabetSize);

/////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////
//misc functions
/////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////

float jukesCantorCorrection(float distance);

void kimuraCorrection(float transitionsPerSite, float transversionsPerSite,
					  float *correctedTransitionsPerSite, float *correctedTransversionsPerSite);

#endif /*SUBSTITUTIONC_H_*/
