/*Copyright 2013 Google Inc. All Rights Reserved.

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

    http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
#limitations under the License.*/

/*
BLOCK-PALLETE ANIMATED TEXTURE COMPRESSION

*/

#include <vector>
#include <assert.h>
#include <stdarg.h>
#include "imgHelper.h"
#include "crabby.h"

static int print_usage()
{
   printf("Description: Crabby.\n");
   printf("Usage: encode.exe txtfilename outfileprefix <options>\n");
	printf("Options:\n");
	printf("\t\t -JSON \t\t Output metadata in .JSON format\n");
	printf("\t\t -DAT \t\t Output metadata in binary .DAT format\n");

	printf("\t\t -block_tga \t\t Output block palette in .tga format\n");
	printf("\t\t -block_raw \t\t Output block palette in .raw format\n");

   return EXIT_FAILURE;
}
//-----------------------------------
static int error(const char* pMsg, ...)
{
   va_list args;
   va_start(args, pMsg);
   char buf[512];
   vsprintf(buf, pMsg, args);
   va_end(args);
   fprintf(stderr,"%s", buf);
   return EXIT_FAILURE;
}



static bool readFileIntoMemory(const char* pFilename, unsigned int& dataSize, char** pData)
{
   //read the input file list into memory
   FILE* f = fopen(pFilename, "rb");

	if(!f)  
      return false;
	
		fseek( f, 0L, SEEK_END );
		dataSize = ftell( f );
		rewind ( f );

		*pData = new char[dataSize + 1];

		const unsigned int v = (unsigned int)fread(*pData, dataSize, 1, f);
		assert(v == 1);
		if(v != 1)
		{
			delete[] *pData;
			return false;
		}

		(*pData) [dataSize] = '\0'; //string must be null terminated for RAPIDXML to work..
		fclose( f );

      return true;
}

//-----------------------------------
int main(int argc, char *argv[])
{
	fprintf(stderr,"CRABBY - Version Built " __DATE__ ", " __TIME__ "\n");
	
	if (argc < 3)
	{
		print_usage();
		return -1;
	}

	char* inFile = argv[1];	// The text file listing what loose assets to load
	char* outFile = argv[2]; //the out filename prefix to use <blah>.tga, <blah>.frame

	eCompressionMode comprMode = cMode_A;
	eOutputMetaDataMode metaOutputMode =eMetaMode_Binary;
	eOutputImageMode outImageMode = eImageMode_Tga;

	if(argc >2)
	{
		for(int i =3; i < argc; i++)
		{
			if(!strcmp("-JSON",argv[i]))
			{
				metaOutputMode=eMetaMode_JSON;
			}
			else if(!strcmp("-block_tga",argv[i]))
			{
				outImageMode=eImageMode_Tga;
			}
			else if(!strcmp("-block_raw",argv[i]))
			{
				outImageMode=eImageMode_Raw;
			}
		}
	}
	

	//read the input file

	std::vector<std::string> images;
	
	char* pFileDat;
	unsigned int fileLen=0;
	if(!readFileIntoMemory(inFile,fileLen,&pFileDat))
	{
		error("Could not open %s",inFile);
		return -1;
	}

	//split the input content by returns
	char * pch;
	pch = strtok (pFileDat,"\n\r");
	while (pch != NULL)
	{
		std::string buffAsStdStr = pch;
		images.push_back(buffAsStdStr);

		pch = strtok (NULL, "\n\r");
	}

	// clean!
	delete[] pFileDat;

	
	compressFlipbook(images,outFile, comprMode, metaOutputMode, outImageMode);


	

	return 0;
}