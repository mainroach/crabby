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

#pragma once
#include <string>
#include <assert.h>
#include <math.h>



struct ImageData
{
   std::string imgFileName;
	unsigned int uniqueID;
	unsigned int width;
	unsigned int height;
	char* pImgData;
	unsigned int imgSizeInBytes;
	unsigned char imgFormat;
};

struct RGBAColor
{
	unsigned char r;
	unsigned char g;
	unsigned char b;
	unsigned char a;
};

bool loadTGAFile(const char* pFilename, ImageData& id);
void saveTGA(const char* pFilename, RGBAColor* pOutImage, const unsigned int outWidth, const unsigned int outHeight);
