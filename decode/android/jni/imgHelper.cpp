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

#include <vector>
#include "platform.h"
#include "imgHelper.h"




struct TGAHeader {
   char  idlength;
   char  colourmaptype;
   char  datatypecode;
   short colourmaporigin;
   short colourmaplength;
   //char  colourmapdepth; //NOTE this is in the spec, but doesn't seem like photoshop outputs it?
   short x_origin;
   short y_origin;
   short width;
   short height;
   char  bitsperpixel;
   char  imagedescriptor;
} ;

bool loadTGAFile(const char* pFilename, ImageData& id,AAssetManager* gAndroidAM)
{
    memset(&id,0,sizeof(ImageData));

	//FILE* fi = fopen(pFilename,"rb");
	
	AAsset* fi = AAssetManager_open(gAndroidAM, pFilename, AASSET_MODE_STREAMING);

	//note the spec : http://www.paulbourke.net/dataformats/tga/

	if(fi == 0)
	{
		LOGE("Couldn't load file");
		return false;
	}
	// header
	TGAHeader header;
	//fread((char*)&header,sizeof(TGAHeader),1,fi);
	AAsset_read(fi, (char*)&header,sizeof(TGAHeader));
	
	if(header.datatypecode != 2 && header.datatypecode != 3)
	{
		LOGE("Sorry, this specific tga format not supported");
		AAsset_close(fi);
		return false;
	}


	//grab our with, height, and bits per pixel
	id.width  = header.width;
	id.height = header.height;
	const unsigned char outBPP	= header.bitsperpixel; // we expect RGBA?
	id.imgSizeInBytes = id.width * id.height * 4;

	// Compute the number of BYTES per pixel
	const unsigned int bytesPerPixel	= (outBPP / 8);
	// Compute the total amout ofmemory needed to store data
	const unsigned int imageSize		= (bytesPerPixel * id.width * id.height);

	//allocate space to hold the image data
	id.pImgData	= new char[imageSize];
	
	// Note that TGA files can have an alternate 'origin'
	// Top Left, or Bottom Left etc. So we need to read the bits, to determine if we should
	// invert the image nor nit.
	if((header.imagedescriptor & 0x20) == 0)
	{
		// The origin of our image is in the lower corner, which is opposite from what we expect, 
		// Read the file in backwards.

		// image stride represents one scan line of the image
		const unsigned int imgStride = bytesPerPixel * id.width;
		
		// we are going to read the file data, and place it into memory, all reverse like
		// so start our destination pointer at the last scanline in our image
		char* pDstCopy = id.pImgData + imageSize - imgStride;
		for(unsigned int i =0; i < id.height;i++)
		{
			// read a scanline from file
			//fread(,1,fi);
			AAsset_read(fi, (char*)pDstCopy,imgStride);

			//move our scanline destination backwards
			pDstCopy -=imgStride;
		}

	}
	else
	{
		//just read the whole file in one kick.
		AAsset_read(fi,id.pImgData,imageSize);
	}
	

	// Note that TGA often stores data as BGR instead of RGB (which is what GL expects)
	// so we need to swap the channels around
	if(bytesPerPixel > 2)
	{
		for(unsigned int cswap = 0; cswap < (int)imageSize; cswap += bytesPerPixel)
		{
			id.pImgData[cswap] ^= id.pImgData[cswap+2] ^= id.pImgData[cswap] ^= id.pImgData[cswap+2];
		}
	}

	AAsset_close(fi);
    
    // last thing, we expect RGBA output from this load function. Does the input texture conform to this?
    if(bytesPerPixel==3)
    {
        LOGE("Input TGA is RGB, converting to RGBA\n");
        char* rgba = new char[id.width * id.height * 4];
        //copy each RGB value to RGBA
        const int numPix = id.width * id.height;
        for(unsigned int i = 0; i < numPix; i++)
        {
            rgba[(i*4)+0] = id.pImgData[(i*3)+0];
            rgba[(i*4)+1] = id.pImgData[(i*3)+1];
            rgba[(i*4)+2] = id.pImgData[(i*3)+2];
            rgba[(i*4)+3] = 0xFF;
        }
        
        delete[] id.pImgData;
        id.pImgData = rgba;

    }

	return true;
}


