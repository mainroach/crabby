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
#include "imgHelper.h"
#include "crabby.h"
// Forward decl, for sanity.
int loadPNGFile(const char* pFilename, ImageData& id);


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

bool loadTGAFile(const char* pFilename, ImageData& id)
{
    memset(&id,0,sizeof(ImageData));

	FILE* fi = fopen(pFilename,"rb");
	
	//note the spec : http://www.paulbourke.net/dataformats/tga/

	if(fi == 0)
	{
		printf("Couldn't load file");
		return false;
	}
	// header
	TGAHeader header;
	fread((char*)&header,sizeof(TGAHeader),1,fi);
	
	if(header.datatypecode != 2 && header.datatypecode != 3)
	{
		printf("Sorry, this specific tga format not supported");
		fclose(fi);
		return false;
	}


	//grab our with, height, and bits per pixel
	id.width  = header.width;
	id.height = header.height;
	id.bpp	= header.bitsperpixel; // we expect RGBA?
	id.imgSizeInBytes = id.width * id.height * 4;

	// Compute the number of BYTES per pixel
	const unsigned int bytesPerPixel	= (id.bpp / 8);
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
			fread((char*)pDstCopy,imgStride,1,fi);

			//move our scanline destination backwards
			pDstCopy -=imgStride;
		}

	}
	else
	{
		//just read the whole file in one kick.
		fread(id.pImgData,imageSize,1,fi);
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

	fclose(fi);
    
    // last thing, we expect RGBA output from this load function. Does the input texture conform to this?
    if(bytesPerPixel==3)
    {
        fprintf(stderr,"Input TGA is RGB, converting to RGBA\n");
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




void saveTGA(const char* pFilename,RGBAColor* pOutImage, const unsigned int outWidth,const unsigned int outHeight)
{
	TGAHeader hd;
	memset(&hd,0,sizeof(TGAHeader));
	hd.idlength = 0;
	hd.colourmaptype = 0;
	hd.datatypecode = 2;
	hd.colourmaporigin = 0;
	hd.colourmaplength = 0;
	hd.x_origin = 0;
   hd.y_origin = outHeight;
	hd.width = outWidth;
	hd.height = outHeight;
	hd.bitsperpixel = 32;
	hd.imagedescriptor = (1<<2) | (1<<5);
	
	
	

	FILE* f = fopen(pFilename,"wb");
	fwrite(&hd, 1, sizeof(TGAHeader), f);
	// Note that TGA prefers data in BGRA format. So swizzle.
	const unsigned int numPixels=outWidth*outHeight;
	for(unsigned int i=0; i < numPixels; i++)
	{
		fwrite(&(pOutImage[i].b), sizeof(char),1,  f);	
		fwrite(&(pOutImage[i].g), sizeof(char),1,  f);	
		fwrite(&(pOutImage[i].r), sizeof(char),1,  f);	
		fwrite(&(pOutImage[i].a), sizeof(char),1,  f);	
	}
	//fwrite(pOutImage, sizeof(RGBAColor)*outWidth*outHeight,1,  f);
	fclose(f);
}




int loadTexture(const char* pFilename, ImageData& id)
{
	const char* ext = strrchr(pFilename,'.');
	if(!strcmp(ext,".tga") || !strcmp(ext,".TGA"))
		return loadTGAFile( pFilename,  id);
	else if(!strcmp(ext,".png") || !strcmp(ext,".PNG"))
		return 0==loadPNGFile(pFilename,  id);

	fprintf(stderr,"\nSorry bro, unregonized image format not supported");
	return -1;
}