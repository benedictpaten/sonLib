#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <float.h>

#include "fastCMaths.h"
#include "commonC.h"
#include "ctype.h"
#include "bioioC.h"

/////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////
//integer reader / writer
/////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////

void readIntegers(FILE *file, int32_t intNumber, int32_t *iA) {
    int32_t i;

    for(i=0; i<intNumber; i++) {
        assert(fscanf(file, INT_STRING, iA + i) == 1);
    }
}

void writeIntegers(FILE *file, int32_t intNumber, int32_t *iA) {
    int32_t i;

    for(i=0; i<intNumber; i++) {
        fprintf(file, INT_STRING "\n", iA[i]);
    }
}

/////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////
//double reader / writer
/////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////

void readDoubles(const char *string, int32_t number, double *dA) {
	FILE *fileHandle;
	char *tempFile;
	int32_t i;

	tempFile = getTempFile(); //This is kind of a hack.
	fileHandle = fopen(tempFile, "w");
	fprintf(fileHandle, string);
	fclose(fileHandle);
	fileHandle = fopen(tempFile, "r");
	for(i=0; i<number; i++) {
		assert(fscanf(fileHandle, "%lf", &(dA[i])) == 1);
	}
	fclose(fileHandle);
	removeTempFile(tempFile);
}

/////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////
//fasta reader/writer
/////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////

char *fastaNormaliseHeader(const char *fastaHeader) {
    /*
     * Removes white space which is treated weirdly by many programs.
     */
	const char *c;
	int32_t i, j;
	char *c2;

	i = 0;
	c = fastaHeader;
	while(c[0] != ' ' && c[0] != '\t' && c[0] != '\n' && c[0] != '\0') {
		c++;
		i++;
	}
	c2 = st_malloc(sizeof(char)*(i+1));
	for(j=0; j<i; j++) {
		c2[j] = fastaHeader[j];
	}
	c2[i] = '\0';
	return c2;
}

struct List *fastaDecodeHeader(const char *fastaHeader) {
    /*
     * Decodes the fasta header
     */
	struct List *attributes = constructEmptyList(0, free);
	char *cA = stString_copy(fastaHeader);
	char *cA2 = strtok(cA, "|");
	while(cA2 != NULL) {
		listAppend(attributes, stString_copy(cA2));
		cA2 = strtok(NULL, "|");
	}
	free(cA);
	return attributes;
}

char *fastaEncodeHeader(struct List *attributes) {
    /*
     * Decodes the fasta header
     */
	return stString_join("|", (const char **)attributes->list, attributes->length);
}

void fastaWrite(char *sequence, char *header, FILE *file) {
	int32_t i, k;
	char j;

	fprintf(file, ">%s\n", header);
	k = (int)strlen(sequence);
	for(i=0; i<k; i++) {
		j = sequence[i];
		assert((j >= 'A' && j <= 'Z') || (j >= 'a' && j <= 'z')); //For safety and sanity I only allows roman alphabet characters in fasta sequences.
	}
	fprintf(file, "%s\n", sequence);
}

char *addSeqToList(char *seq, int32_t *length, int32_t *maxLength, char *fastaName, void (*addSeq)(const char *, const char *, int32_t)) {
    seq = arrayPrepareAppend(seq, maxLength, *length+1, sizeof(char));
    seq[*length] = '\0';
    addSeq(fastaName, seq, *length);
    (*length)  = 0;
    return seq;
}

void fastaReadToFunction(FILE *fastaFile, void (*addSeq)(const char *, const char *, int32_t)) {
    //reads in group of sequences INT_32o lists
    char j;
    int32_t k;
    static char *seq;
    static int32_t seqLength;
    char cA[STRING_ARRAY_SIZE];
    int32_t l;

    k=0;
    while((j = getc(fastaFile)) != EOF) { //initial terminating characters
        if(j == '>') { //fasta start
            fastaStart:
            cA[0] = '\0';
            l = 0;
            while(TRUE) {
                j = getc(fastaFile);
                if(j == EOF) {
                    seq = addSeqToList(seq, &k, &seqLength, cA, addSeq); //lax qualification for a sequence
                    return;
                }
                if(j == '\n')  {
                    break;
                }
                //add to name string
                cA[l++] = j;
                cA[l] = '\0';
            }
            while(TRUE) { //start of sequence
                j = getc(fastaFile);
                if(j == EOF) {
                    seq = addSeqToList(seq, &k, &seqLength, cA, addSeq);
                    return;
                }
                if(j != '\n' && j != ' ' && j != '\t') {
                    if(j == '>') {
                        //end of seq
                        seq = addSeqToList(seq, &k, &seqLength, cA, addSeq);
                        goto fastaStart;
                    }
                    else { //valid char
                        assert(isalpha(j) || j == '-'); //For safety and sanity I only allows roman alphabet characters and - in fasta sequences.
                        seq = arrayPrepareAppend(seq, &seqLength, k+1, sizeof(char));
                        seq[k++] = j;
                    }
                }
            }
        }
    }
}

struct List *fastaRead_fastaNames;
struct List *fastaRead_seqs;
struct List *fastaRead_seqLengths;

void fastaRead_function(const char *fastaHeader, const char *sequence, int32_t length) {
	listAppend(fastaRead_fastaNames, stString_copy(fastaHeader));
	listAppend(fastaRead_seqs, stString_copy(sequence));
	listAppend(fastaRead_seqLengths, constructInt(length));
}

void fastaRead(FILE *fastaFile, struct List *seqs, struct List *seqLengths, struct List *fastaNames) {
	fastaRead_fastaNames = fastaNames;
	fastaRead_seqs = seqs;
	fastaRead_seqLengths = seqLengths;
	fastaReadToFunction(fastaFile, fastaRead_function);
}

/////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////
//useful scanf functions
/////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////


char *eatWhiteSpace(char *string) {
    while(*string != '\0' && isspace(*string)) {
        string++;
    }
    return string;
}

int32_t parseInt(char **string, int32_t *j) {
	/*
	 * Parses a string from the input string, and moves up the pointer.
	 * Returns 1 for success, 0 for failures, no action on the pointer occurs if failed.
	 */
	int32_t i;

	i = sscanf(*string, INT_STRING, j);
	if(i == 1) {
		*string = eatWhiteSpace(*string);
		while(*string != '\0' && !isspace(**string)) {
			(*string)++;
		}
		*string = eatWhiteSpace(*string);
		return 1;
	}
	else {
		return 0;
	}
}

int32_t parseFloat(char **string, float *j) {
	/*
	 * Parses a string from the input string, and moves up the pointer.
	 * Returns 1 for success, 0 for failures, no action on the pointer occurs if failed.
	 */
	int32_t i;

	i = sscanf(*string, "%f", j);
	if(i == 1) {
		*string = eatWhiteSpace(*string);
		while(*string != '\0' && !isspace(**string)) {
			(*string)++;
		}
		*string = eatWhiteSpace(*string);
		return 1;
	}
	else {
		return 0;
	}
}

int32_t parseString(char **string, char *cA) {
	/*
	 * Parses a string from the input string, and moves up the pointer.
	 * Returns 1 for success, 0 for failures, no action on the pointer occurs if failed.
	 */
    int32_t i;

	i = sscanf(*string, "%s", cA);
	if(i == 1) {
		*string = eatWhiteSpace(*string);
		while(*string != '\0' && !isspace(**string)) {
			(*string)++;
		}
		*string = eatWhiteSpace(*string);
		return 1;
	}
	else {
		return 0;
	}
}

/////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////
//newick tree parser
/////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////


char* eatString(char *string, char **newString) {
    char *i;

    i = string;
    while(*string != '\0' && !isspace(*string)) {
           string++;
    }
    (*newString) = memcpy(st_malloc((string-i + 1)*sizeof(char)), i, (string-i)*sizeof(char));
    (*newString)[string - i] = '\0';
    return eatWhiteSpace(string);
}

char *replaceString(char *oldString, char old, char *new, int32_t newLength) {
    char *i;
    int32_t j;
    char *k;
    char *newString;
    j=0;
    k=oldString;
    for(i=oldString; *i != '\0'; i++) {
        if(*i == old) {
            j++;
        }
    }
    newString = st_malloc(sizeof(char)*(j*(newLength-1) + (i-k) + 1));
    k=newString;
    for(i=oldString; *i != '\0'; i++) {
        if(*i == old) {
            for(j=0; j<newLength; j++) {
                *k++ = new[j];
            }
        }
        else {
            *k++ = *i;
        }
    }
    *k = '\0';
    return newString;
}

char *replaceAndFreeString(char *oldString, char old, char *new, int32_t newLength) {
    char *i;

    i = replaceString(oldString, old, new, newLength);
    free(oldString);
    return i;
}


char *_parseFloat(char *string, float *f) {
    float f2;

    f2 = FLT_MIN;
    assert(sscanf(string, "%g", &f2) == 1);
    assert(f2 != FLT_MIN);
    *f = f2;
    while(*string != '\0' && !isspace(*string)) {
        string++;
    }
    return eatWhiteSpace(string);
}

char *newickTreeParser_fn(char *newickTreeString, float *distance) {
    if(*newickTreeString != '\0') {
        if (*newickTreeString == ':') {
            newickTreeString++;
            return _parseFloat(eatWhiteSpace(newickTreeString), distance);
        }
    }
    return newickTreeString;
}

static char *newickTreeParser_getLabel(char *newickTreeString, char **label) {
	if(*newickTreeString != ':' && *newickTreeString != ',' && *newickTreeString != ';' && *newickTreeString != ')' && *newickTreeString != '\0') {
	    return eatString(newickTreeString, label);
	}
	*label = stString_copy("");
	return newickTreeString;
}

char *newickTreeParser_fn2(char *newickTreeString, float defaultDistance, struct BinaryTree **binaryTree, int32_t unaryNodes) {
    struct BinaryTree *temp1;
    struct BinaryTree *temp2;
    float f;
    int32_t leaves;

    temp1 = NULL;
    temp2 = NULL;

    if(*newickTreeString == '(') {
        temp1 = NULL;
        newickTreeString = eatWhiteSpace(++newickTreeString);
        assert(*newickTreeString != ')');
        leaves = 0;
        while(1) {
        	leaves++;
            newickTreeString = newickTreeParser_fn2(newickTreeString, defaultDistance, &temp2, unaryNodes);
            if(temp1 != NULL) {
                //merge node
                temp1 = constructBinaryTree(0.0f, TRUE, "", temp1, temp2); //default to zero distance for nodes of
            }
            else {
                temp1 = temp2;
            }
            if(*newickTreeString == ',') {
                newickTreeString = eatWhiteSpace(++newickTreeString);
            }
            else {
            	assert(*newickTreeString == ')');
            	break;
            }
        }
        if(unaryNodes && leaves == 1) { //make a unary node
        	temp1 = constructBinaryTree(0.0f, TRUE, "", temp1, NULL);
        }
        else { //eat any label
        	char *cA;
        	newickTreeString = newickTreeParser_getLabel(newickTreeString, &cA);
        	free(cA);
        }
        newickTreeString = eatWhiteSpace(++newickTreeString);
    }
    else {
    	temp1 = constructBinaryTree(0.0f, FALSE, "", NULL, NULL);
    }
    free(temp1->label);
    newickTreeString = newickTreeParser_getLabel(newickTreeString, &temp1->label);
    f = defaultDistance;
    newickTreeString = newickTreeParser_fn(newickTreeString, &f);
    temp1->distance += f;
    (*binaryTree) = temp1;
    assert(*newickTreeString == ',' || *newickTreeString == ';' || *newickTreeString == ')' || *newickTreeString == '\0');

    return newickTreeString;
}

struct BinaryTree *newickTreeParser(char *newickTreeString, float defaultDistance, int32_t unaryNodes) {
    struct BinaryTree *binaryTree;
    char *i;
    //lax newick tree parser
    newickTreeString = replaceString(newickTreeString, '(', " ( ", 3);
    newickTreeString = replaceAndFreeString(newickTreeString, ')', " ) ", 3);
    newickTreeString = replaceAndFreeString(newickTreeString, ':', " : ", 3);
    newickTreeString = replaceAndFreeString(newickTreeString, ',', " , ", 3);
    newickTreeString = replaceAndFreeString(newickTreeString, ';', " ; ", 3);
    i = newickTreeString;
    newickTreeString = eatWhiteSpace(newickTreeString);
    newickTreeParser_fn2(newickTreeString, defaultDistance, &binaryTree, unaryNodes);
    free(i);

    return binaryTree;
}

/////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////
//read in fasta file, and turn into a column alignment
/////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////

struct CharColumnAlignment *multiFastaRead(char *fastaFile) {
    struct List *seqs;
    struct List *seqLengths;
    struct List *fastaNames;
    FILE *fileHandle;
    int32_t alignmentLength = 0;
    struct CharColumnAlignment *charColumnAlignment;
    int32_t i;
    int32_t j;
    int32_t k;

    seqs = constructEmptyList(0, free);
    seqLengths = constructEmptyList(0, free);
    fastaNames = constructEmptyList(0, free);
    fileHandle = fopen(fastaFile, "r");
    fastaRead(fileHandle, seqs, seqLengths, fastaNames);
    fclose(fileHandle);

    alignmentLength = 0;
    if(seqLengths->length != 0) {
        alignmentLength = listGetInt(seqLengths, 0);
    }
    for(i=0; i<seqLengths->length; i++) {
        assert(alignmentLength == listGetInt(seqLengths, 0));
    }
    charColumnAlignment = st_malloc(sizeof(struct CharColumnAlignment));
    charColumnAlignment->columnNo = alignmentLength;
    charColumnAlignment->seqNo = seqLengths->length;
    charColumnAlignment->columnAlignment = st_malloc(sizeof(char)*(charColumnAlignment->columnNo)*(charColumnAlignment->seqNo));
    k=0;
    for(i=0; i<alignmentLength; i++) {
        for(j=0; j<seqLengths->length; j++) {
            charColumnAlignment->columnAlignment[k++] = ((char *)seqs->list[j])[i];
        }
    }
    destructList(seqs);
    destructList(seqLengths);
    destructList(fastaNames);
    return charColumnAlignment;
}

char *charColumnAlignment_getColumn(struct CharColumnAlignment *charColumnAlignment, int32_t col) {
    return &charColumnAlignment->columnAlignment[col*charColumnAlignment->seqNo];
}

void destructCharColumnAlignment(struct CharColumnAlignment *charColumnAlignment) {
    free(charColumnAlignment->columnAlignment);
    free(charColumnAlignment);
}

/////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////
//Get line function, while getline is not standard.
/////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////

int32_t benLine(char **s, int32_t *n, FILE *f) {
	register int32_t nMinus1= ((*n)-1), i= 0;

	char *s2 = *s;
	while(TRUE) {
		register int32_t ch= (char)getc(f);

		if(ch == '\r') {
			ch= getc(f);
		}

		if(i == nMinus1) {
			*n = 2*(*n) + 1;
			*s = realloc(*s, (*n + 1)*sizeof(char));
			assert(*s != NULL);
			s2 = *s + i;
			nMinus1 = ((*n)-1);
		}

		if((ch == '\n') || (ch == EOF)) {
			*s2 = '\0';
			return(feof(f) ? -1 : i);
		}
		else {
			*s2 = ch;
			s2++;
		}
		++i;
	}
}
