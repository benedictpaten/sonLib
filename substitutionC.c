#include <math.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>

#include "substitutionC.h"
#include "fastCMaths.h"
#include "commonC.h"

//This is the max alphabet size
#define MAX_ALPHABET_SIZE 100

struct SubModel *constructSubModel(float *forward,
                                   float *backward,
                                   float *stationaryDistribution,
                                   int32_t alphabetSize) {
    struct SubModel *subModel;

    subModel = (struct SubModel *)mallocLocal(sizeof(struct SubModel));
    subModel->forward = forward;
    subModel->backward = backward;
    subModel->stationaryDistribution = stationaryDistribution;
    subModel->alphabetSize = alphabetSize;
    return subModel;
}

void destructSubModel(struct SubModel *subModel) {
    free(subModel->forward);
    free(subModel->backward);
    free(subModel->stationaryDistribution);
    free(subModel);
}

/////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////
//library functions for dealing with wV
/////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////

void transformWVByDistance(float *wV, float *subMatrix, float *result, int32_t alphabetSize) {
    //transform wV by given substitution matrix
    int32_t i;
    float j;
    float *k;
    int32_t l;
    static float nc[MAX_ALPHABET_SIZE];

    for(i=0; i<alphabetSize; i++) {
        nc[i] = 0.0f;
    }
    // *nc = [fastMaths.ZERO_PROB]*ALPHABET_SIZE
    k = subMatrix;
    for(i=0; i<alphabetSize; i++) { // in xrange(0, ALPHABET_SIZE):
        j = wV[i];
        for(l=0; l< alphabetSize; l++) {
            nc[l] += j * k[l];
        }
        k += alphabetSize;
    }
    memcpy(result, &nc, sizeof(float)*alphabetSize);
}

void copyWV(float *wVX, float *wVY, int32_t alphabetSize) {
    int32_t i;

    for(i=0; i<alphabetSize; i++) {
        wVY[i] = wVX[i];
    }
}

void multiplyWV(float *wVX, float *wVY, float *result, int32_t alphabetSize) {
    int32_t i;

    for(i=0; i<alphabetSize; i++) {
        result[i] = wVX[i] * wVY[i];
    }
}

void normaliseWV(float *wV, float *result, int32_t alphabetSize) {
    normaliseWV_GiveFac(wV, result, 1.0f, alphabetSize);
}

void normaliseWV_GiveFac(float *wV, float *result, float normFac, int32_t alphabetSize) {
     //make char probs divisible by one
    int32_t i;
    float j;

    j = wV[0];
    for(i=1; i<alphabetSize; i++) {
        j += wV[i];
    }
    j /= normFac;
    for(i=0; i<alphabetSize; i++) {
        result[i] = wV[i] / j;
    }
    //f = sum(wV)
    //return [ i/f for i in wV ]
}

float combineWV(float *wVX, float *wVY, int32_t alphabetSize) {
    int32_t i;
    float j;

    j = wVX[0] * wVY[0];;
    for(i=1; i<alphabetSize; i++) {
        j += wVX[i] * wVY[i];
    }
    return j;
}

float sumWV(float *wV, int32_t alphabetSize) {
    int32_t i;
    float j;

    j = wV[0];
    for(i=1; i<alphabetSize; i++) {
        j += wV[i];
    }
    return j;
}

void addWV(float *wVX, float *wVY, float *result, int32_t alphabetSize) {
    int32_t i;

    for(i=0; i<alphabetSize; i++) {
        result[i] = wVX[i] + wVY[i];
    }
}

float * dNAMap_IUPACToWVFn(char i) {
    static float j[4];
    int32_t k;

    for(k=0; k<4; k++) {
        j[k] = 0.0f;
    }
    switch(i) {
        case 'A':
        case 'a':
            j[0] = 1.0f;
            break;
        case 'C':
        case 'c':
            j[1] = 1.0f;
            break;
        case 'G':
        case 'g':
            j[2] = 1.0f;
            break;
        case 'T':
        case 't':
            j[3] = 1.0f;
            break;
        default:
            for(k=0; k<4; k++) {
                j[k] = 0.25f;
            }
            break;
   }
   assert(sumWV(j, 4) < 1.01);
   assert(sumWV(j, 4) > 0.99);
   return j;
}

int32_t subMatCo(int32_t i, int32_t j, int32_t alphabetSize) {
	return i *alphabetSize + j;
}

/////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////
//substitution matrix functions
/////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////

int32_t valuesAreClose(float a, float b, float tolerance) {
    return a <= b + tolerance && a >= b - tolerance;
}

void checkMatrix(float *freqVec, float *matrix, int32_t alphabetSize) {
    int32_t i;
    int32_t j;

    float v1;
    float v2;

    for(i=0; i<alphabetSize; i++) {
        for(j=0; j<alphabetSize; j++) {
            //printf("hello %f %f \n", matrix[i*ALPHABET_SIZE + j], matrix[j*ALPHABET_SIZE + i]);
            v1 = freqVec[i] * matrix[i*alphabetSize + j];
            v2 = freqVec[j] * matrix[j*alphabetSize + i];
            assert(v1 < v2 + 0.0001);
            assert(v2 < v1 + 0.0001);
        }
    }
}

float *reverseSubMatrixInPlace(float *wV, int32_t alphabetSize) {
    int32_t i;
    int32_t j;
    float k;

    for(i=0; i<alphabetSize; i++) {
        for(j=i+1; j<alphabetSize; j++) {
            k = wV[i*alphabetSize + j];
            wV[i*alphabetSize + j] = wV[j*alphabetSize + i];
            wV[j*alphabetSize + i] = k;
        }
    }
    return wV;
}

float *tamuraNei(float distance, float freqA, float freqC, float freqG, float freqT,
                    float alphaPurine, float alphaPyrimidine, float beta) {
    int32_t i;
    int32_t j;
    int32_t k;
    float l;
    float *matrix;

    float freq[] = { freqA, freqC, freqG, freqT };
    float alpha[] = { alphaPurine, alphaPyrimidine, alphaPurine, alphaPyrimidine };

    matrix = (float *)mallocLocal(sizeof(float)*4*4);
    //see page 203 of Felsenstein's Inferring Phylogenies for explanation of calculations
    for(i=0; i<4; i++) { //long winded, totally unoptimised method for calculating matrix
        for(j=0; j<4; j++) {
            l = 0.0f;
            for(k=0; k<4; k++) {
                l += ((j % 2) == (k % 2) ? 1.0f : 0.0f) * freq[k]; //wat-kro function
            }
            matrix[i*4 + j] =
            exp(-(alpha[i] + beta) * distance) * (i == j ? 1.0f : 0.0f) //kroenicker delta
            + exp(-beta*distance) * (1.0f - exp(-alpha[i] * distance)) * (freq[j] * ((i % 2) == (j % 2) ? 1.0f : 0.0f) / l)
            + (1.0f - exp(-beta * distance)) * freq[j];
        }
    }
    checkMatrix(freq, matrix, 4);
    return matrix;
}

float *hKY(float distance, float freqA, float freqC, float freqG, float freqT,
                                float transitionTransversionRatio) {
   float alphaPurine;
   float alphaPyrimidine;
   float beta;

   float freqPurine; //working parameters
   float freqPyrimidine;
   float p;

   freqPurine = freqA + freqG;
   freqPyrimidine = freqC + freqT;
   p = freqPurine/freqPyrimidine; //makes like HKY
   //p = 1.0f; //ratio of purine / pyrimidine transition rate parameters -- set to 1.0 for now
   beta = 1.0f / (2.0f * freqPurine * freqPyrimidine * (1.0f + transitionTransversionRatio));
   alphaPyrimidine = ((freqPurine * freqPyrimidine * transitionTransversionRatio) - (freqA * freqG) - (freqC * freqT))
                     / (2.0f * (1.0f + transitionTransversionRatio) * (freqPyrimidine * freqA * freqG * p + freqPurine * freqC * freqT));
   alphaPurine = p * alphaPyrimidine;
   return tamuraNei(distance, freqA, freqC, freqG, freqT, alphaPurine, alphaPyrimidine, beta);
}

struct SubModel *constructHKYSubModel(float distance,
                                            float freqA, float freqC, float freqG, float freqT,
                                            float transitionTransversionRatio) {
    float *i;
    int32_t k;

    float freq[] = { freqA, freqC, freqG, freqT };

    k = sizeof(float)*4;
    i = (float *)memcpy(mallocLocal(k), freq, k);
    return constructSubModel(hKY(distance, freqA, freqC, freqG, freqT, transitionTransversionRatio),
                             reverseSubMatrixInPlace(hKY(distance, freqA, freqC, freqG, freqT, transitionTransversionRatio), 4), i, 4);
}

float *jukesCantor(float d) {
    float i;
    float j;
    float *k;
    int32_t l;

    i = 0.25 + 0.75*exp(-(4.0/3.0)*d);
    j = 0.25 - 0.25*exp(-(4.0/3.0)*d);
    k = (float *)mallocLocal(sizeof(float)*4*4);
    for(l=0; l<4*4; l++) {
        k[l] = j;
    }
    for(l=0; l<4*4; l+=4+1) {
        k[l] = i;
    }
    return k;
}

struct SubModel *constructJukesCantorSubModel(float distance) {
    float *i;
    int32_t k;

    i = (float *)mallocLocal(sizeof(float)*4);
    for(k=0; k<4; k++) {
        i[k] = 0.25f;
    }
    return constructSubModel(jukesCantor(distance),
                             jukesCantor(distance), i, 4);
}

/////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////
//misc functions
/////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////

float jukesCantorCorrection(float subsPerSite) {
	assert(subsPerSite >= 0.0);
	assert(subsPerSite <= 1.0);
	if(subsPerSite >= 0.749) { //Correction is not defined for distances greater than 0.75, where goes to infinity
		logInfo("Warning, distance has been artificially rounded to 5 subs per site given distance: " "%f" "\n", subsPerSite);
		return 5.0;
	}
	return -(3.0/4.0) * log(1.0 - ((4.0/3.0)*subsPerSite));
}

void kimuraCorrection(float transitionsPerSite, float transversionsPerSite,
					  float *correctedTransitionsPerSite, float *correctedTransversionsPerSite) {
	assert(transitionsPerSite >= 0.0);
	assert(transversionsPerSite >= 0.0);
	assert(transitionsPerSite + transversionsPerSite <= 1.0);

	if(transversionsPerSite >= 0.497 || transitionsPerSite + transversionsPerSite > 0.749) { //Correction is not defined for transversions per site greater than 0.5, where goes to infinity
		logInfo("Warning, distance has been artificially rounded to 5 subs per site given transitions and transversion observations: " "%f" " " "%f" "\n", transitionsPerSite, transversionsPerSite);
		*correctedTransitionsPerSite = 2.5;
		*correctedTransversionsPerSite = 2.5;
		return;
	}

	*correctedTransitionsPerSite = -(0.5 * log(1.0 - 2.0*transitionsPerSite - transversionsPerSite)) + (0.25 * log(1.0 - 2.0*transversionsPerSite));
	*correctedTransversionsPerSite = -0.5 * log(1.0 - 2.0 * transversionsPerSite);
}

