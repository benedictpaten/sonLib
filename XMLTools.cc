#include "xmlParser.h"
#include "XMLTools.h"

#include "string.h"
#include "assert.h"

extern "C" {
	#include "commonC.h"
};

void xmlStringFree(char *cA) {
	free(cA);
}


XMLNode readXMLFile(const char *file) {
	try {
		XMLNode xMainNode=XMLNode::openFileHelper(file, "data");
		return xMainNode;
	} catch(char *s)
	{
		logInfo("Got error message in parsing XML file\n");
	}

	XMLNode xMainNode=XMLNode::createXMLTopNode("xml",TRUE);
	XMLNode xNode=xMainNode.addChild("data");

	return xNode;
}

void writeXMLFile(const char *file, XMLNode xMainNode) {
	char *cA2;
	FILE *fileHandle;

	//Now write out the file.
	cA2 = xMainNode.createXMLString(false);
	fileHandle = fopen(file, "w");
	fprintf(fileHandle, "%s\n", cA2);
	xmlStringFree(cA2);
	fclose(fileHandle);
}
