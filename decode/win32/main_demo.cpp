
#include <stdlib.h>
#include <stdio.h>
#include <sys/stat.h>
#include <vector>

#include "platform.h"

//-----------------------------------------------------------------------------
unsigned char numSteps = 1; //54*54 == ~~3000 draws / frame
void demo_incDrawCount()
{
	numSteps +=1;
}
void demo_decDrawCount()
{
	if (numSteps >1)
		numSteps-=1;
}
//-----------------------------------------------------------------------------

GLuint g_programObj;
GLuint g_vertexShader;
GLuint g_fragmentShader;



GLuint mvpLoc;
GLuint atlasTextureLoc;
GLuint framesTextureLoc;
GLuint shaderParamsLoc;
GLuint constantsVec4Loc;
GLuint frameDataVec4Loc;
GLuint atlasDataVec4Loc;

//-----------------------------------------------------------------------------

// GL_T2F_C3F_V3F a vertex format we'll use
struct Vertex
{
	float tu, tv;
	float r, g, b;
	float x, y, z;
};



Vertex gFullscreenQuad[] =
{
	// tu,  tv     r    g    b       x     y     z
	{ 0.0f,0.0f,  1.0f,1.0f,0.0f, -1.0f,-1.0f, 0.0f },
	{ 1.0f,0.0f,  1.0f,0.0f,0.0f,  1.0f,-1.0f, 0.0f },
	{ 1.0f,1.0f,  0.0f,1.0f,0.0f,  1.0f, 1.0f, 0.0f },
	{ 0.0f,1.0f,  0.0f,0.0f,1.0f, -1.0f, 1.0f, 0.0f },
};
GLubyte g_Indices[] = { 0, 1, 2, 0, 2, 3 };

//-----------------------------------------------------------------------------
// per frame, here's our control data
struct Frame
{
	char srcImageName[128];
	unsigned int frameWidth;
	unsigned int frameHeight;
	float uvOffsetX;
	float uvOffsetY;
	float rcpWidth;
	float rcpHeight;
};
//-----------------------------------------------------------------------------
struct flipbookObj
{
	unsigned char compressionMode;
	int palleteWidth;
	int palleteHeight;
	int numFrames;
	int controlWidth;
	int controlHeight;
	int frameWidth;
	int frameHeight;


	GLuint atlasTextureHandle;
	GLuint framesTextureHandle;

	std::vector<Frame> frames;
	int currentFrame;
};

FILE* openFile(const char* pFileName)
{
		char fullFileName[512];
	sprintf(fullFileName,"%s",pFileName);

	
	FILE* file = fopen(fullFileName,"rb");

	LOGE("OPEN %s",fullFileName);

	if(file == 0)
	{
		LOGE("DID NOT FIND FILE %s",fullFileName);
		return 0;
	}

	return file;
}
//-----------------------------------------------------------------------------
unsigned char* readWholeFile(const char* pFileName, unsigned int& fileSize)
{
	// read the whole file from disk
	FILE* file = openFile(pFileName);
	fseek(file, 0, SEEK_END);
	fileSize = ftell(file);
	fseek(file, 0, SEEK_SET);
	

	unsigned char* pBlockAtlas = new unsigned char[fileSize+1];
	fread(pBlockAtlas,1,fileSize,file);
	fclose(file);
	pBlockAtlas[fileSize]=0;

	return pBlockAtlas;
}


flipbookObj gFlipObj; //Gloab is bad practices. But this is a demo!

//-----------------------------------------------------------------------------
void checkGLError(const char* pMsg = 0)
{
	int val = glGetError();
	while(val != GL_NO_ERROR)
	{
		LOGE("  %s,%i",pMsg,val);
		val = glGetError();
	};
}
//-----------------------------------------------------------------------------
bool checkGLStatus( GLuint obj )
{

   GLint status = GL_FALSE, len = 10;
   if( glIsShader(obj) )   glGetShaderiv( obj, GL_COMPILE_STATUS, &status );
   if( glIsProgram(obj) ) glGetProgramiv( obj, GL_LINK_STATUS, &status );
	 
	if( status ) return true;

	LOGE("Found GL status problem\n");
	if( glIsShader(obj) )   glGetShaderiv( obj, GL_INFO_LOG_LENGTH, &len );
	if( glIsProgram(obj) )  glGetProgramiv( obj, GL_INFO_LOG_LENGTH, &len );

	//std::vector< char > log( len, 'X' );
	char log[2048];
	if( glIsShader(obj) )   glGetShaderInfoLog( obj, len, NULL, log );
	if( glIsProgram(obj) )  glGetProgramInfoLog( obj, len, NULL, log );

	LOGE("%s",log);

	return false;


}
//-----------------------------------------------------------------------------
void demo_loadFlipbook(flipbookObj* pObj,std::string fileprefix)
{
	char fname[1024];

	unsigned int cBlockSize=4;
	unsigned int cMaxAtlasWidth = 256;

	
	//load metadata
	sprintf(fname,"%s_metadata.dat",fileprefix.c_str());
	FILE* file = openFile(&fname[0]);
	if(file==0)
		return;
	

	//load our values needed in decompression
	fread(&pObj->compressionMode,1,1,file);
	fread(&cBlockSize,1,1,file);
	fread(&pObj->palleteWidth,4,1,file);
   fread(&pObj->palleteHeight,4,1,file);
   fread(&pObj->numFrames,4,1,file);
	fread(&pObj->controlWidth,4,1,file);
	fread(&pObj->controlHeight,4,1,file);
	pObj->frames.resize(pObj->numFrames);
	

	//load our per-rame data
	for(int i =0; i < pObj->numFrames;i++)
	{
		fread(pObj->frames[i].srcImageName,64,1,file);
		fread(&pObj->frames[i].frameWidth,4,1,file);
		fread(&pObj->frames[i].frameHeight,4,1,file);
		fread(&pObj->frames[i].uvOffsetX,4,1,file);
		fread(&pObj->frames[i].uvOffsetY,4,1,file);
		fread(&pObj->frames[i].rcpWidth,4,1,file);
		fread(&pObj->frames[i].rcpHeight,4,1,file);
	}

	fclose(file);
	pObj->currentFrame = 0;


	LOGE("%i,%i,%i,%i,%i",pObj->palleteWidth,pObj->palleteHeight,pObj->numFrames,pObj->frameWidth,pObj->frameHeight);




	//load blockatlas
	int glErrorCode=  0;
	unsigned int fileSize = 0;
	sprintf(fname,"%s_blockatlas.raw",fileprefix.c_str());
	unsigned char* pBlockAtlas =readWholeFile(&fname[0],fileSize);
	
	 glGenTextures( 1, &pObj->atlasTextureHandle);
    glBindTexture( GL_TEXTURE_2D, pObj->atlasTextureHandle);
     
	 //note you have to set the min/mag filter here. otherwise nothing works.
    glTexParameteri( GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER, GL_NEAREST  );
    glTexParameteri( GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER, GL_NEAREST );
	 //REQUIRED IN GLES2 FOR NPOT TEXTURES
	 glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	 glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	     
    glTexImage2D( GL_TEXTURE_2D, 0, GL_RGBA,pObj->palleteWidth, pObj->palleteHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, pBlockAtlas);

	 free(pBlockAtlas);

	 checkGLError("Error creating pallete texture");
	 checkGLError("rest");
	   




	//load frames
	 sprintf(fname,"%s_frames.raw",fileprefix.c_str());
	 unsigned char* pFrames =readWholeFile(&fname[0],fileSize);
	
	glGenTextures( 1, &pObj->framesTextureHandle);
	glBindTexture( GL_TEXTURE_2D, pObj->framesTextureHandle );

    //note you have to set the min/mag filter here. otherwise nothing works.
	glTexParameteri( GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER, GL_NEAREST );
   glTexParameteri( GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER, GL_NEAREST );
	//REQUIRED IN GLES2 FOR NPOT TEXTURES
	 glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	 glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

     //if compression mode is 0, then we're 16bpp for the frames texture
	 if(pObj->compressionMode==0)
		glTexImage2D( GL_TEXTURE_2D, 0, GL_LUMINANCE16,pObj->controlWidth,pObj->controlHeight, 0, GL_LUMINANCE, GL_UNSIGNED_SHORT, pFrames);

	 

	 free(pFrames);

	 checkGLError("Error creating frames texture");


}


//-----------------------------------------------------------------------------

void demo_initShader( void )
{
	// this is really gnarly code that just sets up the shaders, binds them to eachother, and moves along.
    
    const char *vertexShaderStrings[1];
    const char *fragmentShaderStrings[1];
    unsigned int fileSize=0;
    
	const char* cPathToVS = "shaders/opengl/decode.vs";
	const char* cPathToPSA = "shaders/opengl/decode_modea.ps";


	g_vertexShader = glCreateShader( GL_VERTEX_SHADER );
   vertexShaderStrings[0] = (char*)readWholeFile(cPathToVS,fileSize);
   glShaderSource( g_vertexShader, 1, vertexShaderStrings, NULL );
   glCompileShader( g_vertexShader);
	if(!checkGLStatus(g_vertexShader))return;
    
    
   g_fragmentShader = glCreateShader( GL_FRAGMENT_SHADER );
    
	if(gFlipObj.compressionMode==0) //modeb
		fragmentShaderStrings[0] = (char*)readWholeFile(cPathToPSA,fileSize);
	

   glShaderSource( g_fragmentShader, 1, fragmentShaderStrings, NULL );
   glCompileShader( g_fragmentShader );
   if(!checkGLStatus(g_fragmentShader))return;
    
    
	g_programObj = glCreateProgram();
	glAttachShader( g_programObj, g_vertexShader );
	glAttachShader( g_programObj, g_fragmentShader );
	glLinkProgram( g_programObj );
	if(!checkGLStatus(g_programObj))return;
	 
   
    
	//
	// Locate some parameters by name so we can set them later...
	//
    
	atlasTextureLoc = glGetUniformLocation( g_programObj, "atlasTexture" );
	framesTextureLoc = glGetUniformLocation( g_programObj, "framesTexture" );
	shaderParamsLoc = glGetUniformLocation( g_programObj, "params" );


	constantsVec4Loc = glGetUniformLocation( g_programObj, "constants" );
	frameDataVec4Loc = glGetUniformLocation( g_programObj, "frameData" );
	atlasDataVec4Loc = glGetUniformLocation( g_programObj, "atlasData" );

		
	mvpLoc = glGetUniformLocation( g_programObj, "a_MVP" );



}

//Some basic math that allows us to set up view matricies
float gMVP[16];
//-----------------------------------------------------------------------------
void mulMat4(float* destMat, float* srcMat)
{
	float tmpSrc[16];
	unsigned char ctr=0;
	for ( int i=0; i<4; i++ )
	{
		for ( int j=0; j<4; j++ )
		{
			float	tmp = 0.f;
			for ( int k=0; k<4; k++ )
				tmp += destMat[i*4+k] * srcMat[k*4+j];
			
			tmpSrc[ctr] = tmp;
			ctr+=1;
		}
	}
	memcpy(destMat,tmpSrc,sizeof(float)*16);
}
//-----------------------------------------------------------------------------
void identityMVP()
{
	memset(&gMVP[0],0,sizeof(float)*16);
	gMVP[0] = 1.0f;
	gMVP[5] = 1.0f;
	gMVP[10] = 1.0f;
	gMVP[15] = 1.0f;
}
//-----------------------------------------------------------------------------
void scaleMVP(const float sx,const float sy,const float sz)
{
	float scaleMat[16];
	memset(&scaleMat[0],0,sizeof(float)*16);
	scaleMat[0] = sx;
	scaleMat[5] = sy;
	scaleMat[10] = sz;
	scaleMat[15] = 1.0f;

	mulMat4(&gMVP[0],&scaleMat[0]);
	
}
//-----------------------------------------------------------------------------
void translateMVP(const float tx, const float ty, const float tz)
{
	float trans[16]={1,0,0,0,0,1,0,0,0,0,1,0,0,0,0,1};
	
	trans[12] = tx;
	trans[13] = ty;
	trans[14] = tz;
	trans[15] = 1;

	mulMat4(&gMVP[0],&trans[0]);
}


//-----------------------------------------------------------------------------
void drawQuad()
{
	//draw a single quad. This is stupidly complex for ogl

		glInterleavedArrays( GL_T2F_C3F_V3F, 0, gFullscreenQuad );
		glDrawArrays( GL_QUADS, 0, 4 );


}

//-----------------------------------------------------------------------------
void demo_render( void )
{
	// Clear the screen and the depth buffer
    glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
	     
	 //set what program we're using
   glUseProgram( g_programObj );
    
	//atlas texture
	glActiveTexture(GL_TEXTURE0 + 0);	
	glBindTexture(GL_TEXTURE_2D, gFlipObj.atlasTextureHandle);
	glUniform1i( atlasTextureLoc, 0 );
	

	//frames texture
	glActiveTexture(GL_TEXTURE0 + 1);	
	glBindTexture(GL_TEXTURE_2D, gFlipObj.framesTextureHandle);
	glUniform1i( framesTextureLoc, 1 );
	
	

	//set the params
	
	const unsigned int frameCounter = gFlipObj.currentFrame;
	const unsigned char cBlockSize = 4;
	glUniform4f(constantsVec4Loc,cBlockSize,256/cBlockSize,0,0);
	glUniform4f(frameDataVec4Loc,	gFlipObj.frames[frameCounter].uvOffsetX,
												gFlipObj.frames[frameCounter].uvOffsetY,
												gFlipObj.frames[frameCounter].rcpWidth,
												gFlipObj.frames[frameCounter].rcpHeight);
	
	glUniform4f(atlasDataVec4Loc,	gFlipObj.frames[frameCounter].frameWidth,
												gFlipObj.frames[frameCounter].frameHeight,
												gFlipObj.palleteWidth,
												gFlipObj.palleteHeight);

	

	//now, draw MxN quads on the screen with this technique, to show off how fast it is.
	const float modifier = 2.0 / numSteps;
	const float startOffs = -1+(modifier*0.5);
	for(int y = 0; y < numSteps; y++)
	{
		for(int x = 0; x < numSteps; x++)
		{
			identityMVP();
			scaleMVP(1.0/numSteps,1.0/numSteps,1);
			//scaleMVP(0.25,0.25,1);
			translateMVP( startOffs + (x*modifier), startOffs + (y*modifier), 0.0f );
			
			glUniformMatrix4fv( mvpLoc, 1, GL_FALSE, (GLfloat*) &gMVP[0] );

			drawQuad();
		}
	}
		

	//increase our frame. In reality, we'd want to scale this by whatever the framerate is for this animation
	gFlipObj.currentFrame = (gFlipObj.currentFrame+1) % gFlipObj.numFrames;
    

}


//-----------------------------------------------------------------------------
void demo_init( std::string filenameprefix )
{

	glEnable( GL_TEXTURE_2D );
	glEnable( GL_DEPTH_TEST );
    
	glClearColor( 0.0f, 0.0f, 0.0f, 1.0f );
    
	checkGLError("resetting Errors");
    

	LOGE("Loading Flipbook");
	demo_loadFlipbook(&gFlipObj,filenameprefix);

	LOGE("Loading Shader");
	demo_initShader();
	LOGE("init complete");
}

//-----------------------------------------------------------------------------
void demo_deinit( void )
{
    glDeleteTextures( 1, &gFlipObj.atlasTextureHandle );
	 glDeleteTextures( 1, &gFlipObj.framesTextureHandle);
    
    glDeleteObject( g_vertexShader );
    glDeleteObject( g_fragmentShader );
    glDeleteObject( g_programObj );

    

}




