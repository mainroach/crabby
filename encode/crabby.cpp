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
#include <assert.h>
#include <stdarg.h>
#include <algorithm>
#include <math.h>
#include "imgHelper.h"
#include "binPacking.h"
#include "crabby.h"



const uint32 cBlockSize=4;
const uint32 cMaxAtlasWidth = 256;


struct ControlFrame
{
    char srcImageName[128];

    uint32 srcImgWidth;
    uint32 srcImgHeight;
    std::vector<uint16> blockIdx;

    uint32 atlasLocX;
    uint32 atlasLocY;
};

//-----------------------------------
void pixelsIntoBlock(ImageData& img, const uint32 x,const uint32 y,const uint32 size, Block& blk)
{
	// This function will grab an MxM group of pixels, at [X,Y] location, and push it to the block object.

    int pxlCt=0;
    for(uint32 pxlY = y; pxlY < y+size; pxlY++)
    {
        for(uint32 pxlX = x; pxlX < x+size; pxlX++)
        {
            const uint32 pxlIDX = (pxlY * img.width + pxlX) * 4;
            blk.pixels[pxlCt].r = img.pImgData[pxlIDX+0];
            blk.pixels[pxlCt].g = img.pImgData[pxlIDX+1];
            blk.pixels[pxlCt].b = img.pImgData[pxlIDX+2];
            blk.pixels[pxlCt].a = img.pImgData[pxlIDX+3];

            pxlCt+=1;
        }
    }
}
//-----------------------------------
Block* imgToBlocks(ImageData& img)
{
	// Translate an image into a linear grouping of block objects in raster order

	// lets ensure that the image is exact mulitple of block size
    assert(img.width%cBlockSize==0 &&img.height%cBlockSize==0); //for now..

	 // some basic math to determine the number of blocks we'll be generating.
    const uint32 numXBlocks = img.width / cBlockSize;
    const uint32 numYBlocks = img.height / cBlockSize;
    const uint32 outNumBlocks = numXBlocks * numYBlocks;

	 //NOTE the caller object is now in charge of deleting this data
    Block* pBlocks = new Block[outNumBlocks];
    

	// Loop through all the blocks, and extract them into the block object.
    uint32 blockIndex = 0;
    for(uint32 blockY = 0; blockY < numYBlocks; blockY++)
    {
        for(uint32 blockX = 0; blockX < numXBlocks; blockX++)
        {
			  //find our block locations
            const uint32 blockpxlY = blockY * cBlockSize;
            const uint32 blockpxlX = blockX * cBlockSize;
            
            pixelsIntoBlock(img,blockpxlX,blockpxlY,cBlockSize,pBlocks[blockIndex]);

            blockIndex++;
        }
    }

    return pBlocks;
}




//----------------------------------------------
int roundUp(int numToRound, int multiple) 
{ 
	// given a mulitple, round a value to the next mutliple
    if(multiple == 0) 
    { 
        return numToRound; 
    } 

    int remainder = numToRound % multiple;
    if (remainder == 0)
        return numToRound;
    return numToRound + multiple - remainder;
} 

//----------------------------------------------
void generateControlAtlasUVs(std::vector<ControlFrame>& frames, const uint32 maxWidth, const uint32 maxHeight, uint32& outAtlasWidth, uint32& outAtlasHeight)
{

    //Since our input textures can be of different sizes, we need to pack them into our image texture
	// using some form of atlas packing. 
	// Go ahead and convert each of the input rectangle data to a Rect2D object that we can pass to the packer.
    std::vector<Rect2D> atlasImgData;
    atlasImgData.resize(frames.size());

	 //for each frame, copy over a unique ID, width and height in block size
    for(uint32 i =0; i < frames.size(); i++)
    {
        atlasImgData[i].width = frames[i].srcImgWidth / cBlockSize;
        atlasImgData[i].height = frames[i].srcImgHeight/ cBlockSize;
        atlasImgData[i].id = i;
    }

	//go ahead and pack the rectangles.
    packRectangles(atlasImgData,cSort_MaxSide);

	 //now that packing is complete, copy out the packing data.
    outAtlasWidth = 0;
    outAtlasHeight = 0;
    for(uint32 i =0; i < atlasImgData.size(); i++)
    {
		 const uint32 bottom = atlasImgData[i].bottom(); 
		 const uint32 right = atlasImgData[i].right(); 
       const uint32 id = atlasImgData[i].id;

       frames[id].atlasLocX = atlasImgData[i].left;
       frames[id].atlasLocY = atlasImgData[i].top;

		 // keep track of what our max boundries are
       if(bottom > outAtlasHeight)
           outAtlasHeight = bottom;
       if(right > outAtlasWidth)
           outAtlasWidth = right;
    }

    
    //scale the size of the atlas up to the next multiple of 16
	 // this is super important for OPENGLES2 stride restrictions!
    outAtlasWidth = roundUp(outAtlasWidth,4);
    outAtlasHeight = roundUp(outAtlasHeight,4);
    
};
//----------------------------------------------
void writeControlAtlas(std::string outPrefix,std::vector<ControlFrame>& frames, uint32& atlasWidth,uint32& atlasHeight)
{
    // first, generate the UV locations where the frames will be written to the file.
    generateControlAtlasUVs(frames,4096,4096,atlasWidth,atlasHeight);

    
   //for each of our control frames, we need to write the data into the larger atlas object
	 // go ahead and allocate the object. NOTE we're using mode A here, so everything is 16 bit indexes :(
    uint16* pAtlas = new uint16[atlasWidth * atlasHeight];
    memset(pAtlas,0xFF,atlasWidth*atlasHeight*sizeof(uint16));
    uint32 xOffset = 0;
    uint32 yOffset = 0;
    for(uint32 i = 0; i < frames.size();i++)
    {
        xOffset = frames[i].atlasLocX;
        yOffset = frames[i].atlasLocY;
        const uint32 numXBlocks = frames[i].srcImgWidth / cBlockSize;
        const uint32 numYBlocks = frames[i].srcImgHeight / cBlockSize;

        //copy in this block at this location
        for(uint8 q=0; q < numYBlocks; q++)
        {
            const uint32 dstIdx = (((yOffset+q) * atlasWidth) + xOffset);
            memcpy(&pAtlas[dstIdx],&(frames[i].blockIdx[q * numXBlocks]), numXBlocks * sizeof(uint16));
        }
    }
    
    //write to disk
	 char outPath[1024];
	 sprintf(outPath,"%s_frames.raw",outPrefix.c_str());
    FILE* pOut = fopen(&outPath[0],"wb");
    fwrite(pAtlas,sizeof(uint16)*atlasWidth*atlasHeight,1,pOut);
    fclose(pOut);
    
	 //just do some output to keep things sane.
    fprintf(stderr,"\nframes-size:%i,%i", atlasWidth, atlasHeight);
    fprintf(stderr,"\nframes.raw:%i",sizeof(uint16)*atlasWidth*atlasHeight);
    delete[] pAtlas;
}
//----------------------------------------------
void writeBlockPallete(std::string outPrefix, std::vector<Block>& blockPool,uint32& numBlocks, uint32& imgDeltaWidth, uint32& imgDeltaHeight)
{
    // This function will write our our RGBA blocks in a pallet form.
    
    const uint32 cMaxBlocksWidth = cMaxAtlasWidth / cBlockSize;
    
	 // some simple math here to determine what the X/Y of our out pallete will  be, given our block inputs
    uint32 outHeight = (uint32)(cBlockSize* ceil(blockPool.size() / (float)cMaxBlocksWidth));
    uint32 outWidth = cBlockSize* (cMaxBlocksWidth> blockPool.size() ? blockPool.size() : cMaxBlocksWidth);
    
    //allocate the output location
    RGBAColor* pAtlas = new RGBAColor[outWidth * outHeight];
    uint32 xOffset = 0;
    uint32 yOffset = 0;
    for(uint32 i = 0; i < blockPool.size();i++)
    {
        //copy in this block at this location
        for(uint8 q=0; q < cBlockSize; q++)
        {
            const uint32 dstIdx = (((yOffset+q) * outWidth) + xOffset);
            memcpy(&pAtlas[dstIdx],&(blockPool[i].pixels[q * cBlockSize]), cBlockSize * sizeof(RGBAColor));
        }
        
        //update our pointers
        xOffset += cBlockSize;
		  // if we run off the side, simply wrap around.
        if(xOffset >= cMaxAtlasWidth)
        {
            xOffset =0;
            yOffset += cBlockSize;
        }
    }
    

    //write to disk
	 char outPath[1024];
	 sprintf(outPath,"%s_blockatlas.raw",outPrefix.c_str());
	 //saveTGA(&outPath[0],pAtlas,outWidth,outHeight);
	 FILE* pOut = fopen(&outPath[0],"wb");
    fwrite(pAtlas,sizeof(RGBAColor)*outWidth*outHeight,1,pOut);
    fclose(pOut);
    
    
    //print out data, just to be sane.
    fprintf(stderr,"\natlas-size:%i,%i", outWidth, outHeight);
    fprintf(stderr,"\nblockatlas.raw:%i",sizeof(RGBAColor)*outWidth*outHeight);
    
	 //set the return values that will be used later
    imgDeltaWidth = outWidth;
    imgDeltaHeight = outHeight;
    numBlocks = blockPool.size();

	 delete[] pAtlas;
    
}
//----------------------------------------------
void writeMetaDataBinary(std::string outPrefix,  const eCompressionMode compressionMode, std::vector<ControlFrame>& frames,const uint32 palleteWidth, const uint32 palleteHeight, 
                                 const uint32 frameTexWidth, const uint32 frameTexHeight)
{
    
	char outPath[1024];
	 sprintf(outPath,"%s_metadata.dat",outPrefix.c_str());
    FILE* pFile = fopen(&outPath[0],"wb");

    //compression mode
    fwrite(&compressionMode,1,1,pFile);

    //pallete data
    fwrite(&cBlockSize,1,1,pFile);
    fwrite(&palleteWidth,4,1,pFile);
    fwrite(&palleteHeight,4,1,pFile);


    const uint32 numFrames = frames.size();
    fwrite(&numFrames,4,1,pFile);
    fwrite(&frameTexWidth,4,1,pFile);
    fwrite(&frameTexHeight,4,1,pFile);

    // per frame, write out the data from the atlas packer
    for(uint32 i =0; i < frames.size();i++)
    {
        fwrite(frames[i].srcImageName,64,1,pFile);

        //CALCULATE WHAT OUR UV COORDINATES WILL BE!
        const float xOffset = frames[i].atlasLocX / (float)frameTexWidth;
        const float yOffset = frames[i].atlasLocY / (float)frameTexHeight;
        const float widthScale = (frames[i].srcImgWidth / cBlockSize) / (float)frameTexWidth;
        const float heightScale = (frames[i].srcImgHeight / cBlockSize) / (float)frameTexHeight;
        
		  //write out our frame data to the file
        fwrite(&(frames[i].srcImgWidth),4,1,pFile);
        fwrite(&(frames[i].srcImgHeight),4,1,pFile);
        fwrite(&xOffset,4,1,pFile);
        fwrite(&yOffset,4,1,pFile);
        fwrite(&widthScale,4,1,pFile);
        fwrite(&heightScale,4,1,pFile);
    }
    

    fclose(pFile);

}

//----------------------------------------------
int addBlockToPool(std::vector<Block>& blockPool,Block& block)
{
    //is there an identitical block of this type somewhere?
    for(uint32 i =0; i < blockPool.size();i++)
    {
        RGBAColor* pxA = &blockPool[i].pixels[0];
        RGBAColor* pxB = &block.pixels[0];
		  //do a memcmp to determine if this is the same, exact data
        const int cmpVal = memcmp(pxA,pxB, sizeof(RGBAColor)*16);
        if(cmpVal ==0)
            return i ;
    }
    
    blockPool.push_back(block);
    //NOTE this has to be offset by 1!
    return blockPool.size()-1;
}

//----------------------------------------------
void paddImageCanvas(ImageData& imgDat)
{
	// Some of our input images will not be a multiple of our block size.
	// as such, padd our image data in a way to conform to our block sizes
	
   if(imgDat.width % cBlockSize == 0 && imgDat.height % cBlockSize == 0) return;

	// with some math, determine the new witdth by adding the remainder from mod(blocksize)
	int newWidth = imgDat.width + 4 - (imgDat.width % cBlockSize);
	int newHeight = imgDat.height + 4 - (imgDat.height % cBlockSize);

	// sanity check, are we still g2g?
	assert(newWidth % cBlockSize == 0 && newHeight % cBlockSize == 0);

	// allocate a new image at the right size
	char* pNewPixels = new char[newWidth * newHeight * 4];

	//set it to to whatever the default bg pixel is (which typically is the upper left corner pixel
	memset(pNewPixels, imgDat.pImgData[0], sizeof(int) * newWidth * newHeight);

	// copy in the existing data into the new, padded image by doing a few memcpy's per row.
	for(uint32 y=0; y < imgDat.height; y++)
	{
		char* pSrcPtr = imgDat.pImgData + (y * imgDat.width * 4);
		char* pDstPtr = pNewPixels + (y * newWidth * 4);
		memcpy(pDstPtr, pSrcPtr, imgDat.width*4);
	}

	// delete the old image
	delete[] imgDat.pImgData;

	//assign the new stats
	imgDat.pImgData = pNewPixels;
	imgDat.width = newWidth;
	imgDat.height = newHeight;

}
//----------------------------------------------
int compressFramesModeA(std::vector<std::string>& filenames, std::vector<Block>& blockPool, std::vector<ControlFrame>& frames,uint32& numBlocks, uint32& imgDeltaWidth)
{
    //generate each control frame
    for(uint32 i =0; i < filenames.size();i++)
    {
		// Load our image data into memory, getting an RGBA texture
      ImageData imgA;
		if(-1 != filenames[i].find(".tga"))
		{
			if(!loadTGAFile(filenames[i].c_str(), imgA))
			{
				fprintf(stderr,"\n ERROR could not load image %s\n\n",filenames[i].c_str());
				return -1;
			}
		}
		else
		{
			fprintf(stderr,"\n Sorry, this build can only load .tga files\n\n");
			return -1;
		}



        fprintf(stderr,"compiling - %s\n",filenames[i].c_str());

		  // Since the textures we recieve may not be mul4 aligned, go ahead and padd it by a few pixels.
		  paddImageCanvas(imgA);
        
		  //determine the number of blocks we'll need
        const uint32 numXBlocks = imgA.width / cBlockSize;
        const uint32 numYBlocks = imgA.height / cBlockSize;
        uint32 numBlocks = numXBlocks * numYBlocks;

			//copy in our filename, widht and height into the frame data.
			strcpy(frames[i].srcImageName,filenames[i].c_str());
        frames[i].srcImgWidth = imgA.width;
        frames[i].srcImgHeight = imgA.height;

			// convert our image into 4x4 blocks
		  Block* pABlocks = imgToBlocks(imgA);

        
		  // this process will loop through each block, and return the block index in the global pool
		  frames[i].blockIdx.resize(numBlocks);        
        for(uint32 q = 0; q < numBlocks; q++)
        {
            const uint32 blockIDX = addBlockToPool(blockPool, pABlocks[q]);
            frames[i].blockIdx[q] = blockIDX;
        }

			//free the data back to the heap.
			delete[] pABlocks;
    }

    
	 return 0;
}

//----------------------------------------------
void compressFlipbook(std::vector<std::string>& filenames, std::string outPrefix, const eCompressionMode compressionMode)
{
	uint32 numBlocks=0;
	uint32 imgDeltaWidth=0;

	std::vector<Block> globalBlockPool;
   std::vector<ControlFrame> frames;
   
	// we're only supporting modeA for now. Modes B,C,D are all in research phases.
   if(compressionMode == cMode_A)
	{	
		frames.resize(filenames.size());
      if( -1 == compressFramesModeA(filenames, globalBlockPool, frames, numBlocks, imgDeltaWidth))
			return;
	}

    
   uint32 controlAtlasWidth=0;
	uint32 controlAtlasHeight=0;
   uint32 imgDeltaHeight=0;
	// The control atlas: Per frame, per block in-frame, writeout the index of the desired block.
   writeControlAtlas(outPrefix, frames,controlAtlasWidth,controlAtlasHeight);
	// Block pallet is the unique set of blocks used by this image, stored in RGBA8
   writeBlockPallete(outPrefix, globalBlockPool,numBlocks,imgDeltaWidth,imgDeltaHeight);
	// The metadata binary contains information on where the frames are, and how to render them
	// Note, this could be JSON, XML, or whatever. I chose binary because it was the easiest to code, and produced the smallest output footprint.
   writeMetaDataBinary(outPrefix, compressionMode, frames, imgDeltaWidth, imgDeltaHeight, controlAtlasWidth,controlAtlasHeight);
}
