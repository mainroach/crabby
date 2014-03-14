

#define STRICT
#define WIN32_LEAN_AND_MEAN

#include "platform.h"

#pragma comment(lib,"opengl32.lib")
#pragma comment(lib,"glu32.lib")
PFNGLCREATEPROGRAMOBJECTARBPROC  glCreateProgram  = NULL;
PFNGLDELETEOBJECTARBPROC         glDeleteObject         = NULL;
PFNGLUSEPROGRAMOBJECTARBPROC     glUseProgram     = NULL;
PFNGLCREATESHADEROBJECTARBPROC   glCreateShader   = NULL;
PFNGLSHADERSOURCEARBPROC         glShaderSource         = NULL;
PFNGLCOMPILESHADERARBPROC        glCompileShader        = NULL;
//PFNGLGETOBJECTPARAMETERIVARBPROC glGetShaderiv = NULL;
PFNGLATTACHOBJECTARBPROC         glAttachShader         = NULL;
PFNGLGETINFOLOGARBPROC           glGetInfoLog           = NULL;
PFNGLLINKPROGRAMARBPROC          glLinkProgram          = NULL;
PFNGLGETUNIFORMLOCATIONARBPROC   glGetUniformLocation   = NULL;
PFNGLUNIFORM4FARBPROC            glUniform4f            = NULL;
PFNGLUNIFORM1IARBPROC            glUniform1i            = NULL;
PFNGLUNIFORMMATRIX4FVARBPROC				 glUniformMatrix4fv= NULL;
PFNGLACTIVETEXTUREARBPROC			glActiveTexture  = NULL;
PFNGLUNIFORM1FVPROC glUniform1fv=NULL;

PFNGLISSHADERPROC					glIsShader = NULL;
PFNGLISPROGRAMPROC					glIsProgram = NULL;
PFNGLGETSHADERIVPROC							glGetShaderiv=NULL;
PFNGLGETPROGRAMIVPROC				glGetProgramiv=NULL;
PFNGLGETPROGRAMINFOLOGPROC		glGetProgramInfoLog=NULL;





HDC g_hDC = NULL;
HGLRC g_hRC = NULL;
HWND g_hWnd = NULL;
HINSTANCE g_hInstance = NULL;

int WINAPI WinMain(HINSTANCE g_hInstance,HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow);
LRESULT CALLBACK WindowProc(HWND g_hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

void demo_deinit( void );
void demo_init( std::string );
void demo_render( void );
void demo_incDrawCount( void );
void demo_decDrawCount( void );


//-----------------------------------------------------------------------------
void init_OGL( void )
{
	GLuint PixelFormat;
    

	PIXELFORMATDESCRIPTOR pfd;
	memset(&pfd, 0, sizeof(PIXELFORMATDESCRIPTOR));
    
    pfd.nSize      = sizeof(PIXELFORMATDESCRIPTOR);
    pfd.nVersion   = 1;
    pfd.dwFlags    = PFD_DRAW_TO_WINDOW |PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER;
    pfd.iPixelType = PFD_TYPE_RGBA;
    pfd.cColorBits = 16;
    pfd.cDepthBits = 16;
	
	g_hDC = GetDC( g_hWnd );
	PixelFormat = ChoosePixelFormat( g_hDC, &pfd );
	SetPixelFormat( g_hDC, PixelFormat, &pfd);
	g_hRC = wglCreateContext( g_hDC );
	wglMakeCurrent( g_hDC, g_hRC );
	

	char *ext = (char*)glGetString( GL_EXTENSIONS );
    
    if( strstr( ext, "GL_ARB_shading_language_100" ) == NULL )
    {
        //This extension string indicates that the OpenGL Shading Language,
        // version 1.00, is supported.
        MessageBox(NULL,"GL_ARB_shading_language_100 extension was not found",
                   "ERROR",MB_OK|MB_ICONEXCLAMATION);
        return;
    }
    
    if( strstr( ext, "GL_ARB_shader_objects" ) == NULL )
    {
        MessageBox(NULL,"GL_ARB_shader_objects extension was not found",
                   "ERROR",MB_OK|MB_ICONEXCLAMATION);
        return;
    }
    else
    {
        glCreateProgram  = (PFNGLCREATEPROGRAMOBJECTARBPROC)wglGetProcAddress("glCreateProgramObjectARB");
        glDeleteObject         = (PFNGLDELETEOBJECTARBPROC)wglGetProcAddress("glDeleteObjectARB");
        glUseProgram     = (PFNGLUSEPROGRAMOBJECTARBPROC)wglGetProcAddress("glUseProgramObjectARB");
        glCreateShader   = (PFNGLCREATESHADEROBJECTARBPROC)wglGetProcAddress("glCreateShaderObjectARB");
        glShaderSource         = (PFNGLSHADERSOURCEARBPROC)wglGetProcAddress("glShaderSourceARB");
        glCompileShader        = (PFNGLCOMPILESHADERARBPROC)wglGetProcAddress("glCompileShaderARB");
        //glGetShaderiv = (PFNGLGETOBJECTPARAMETERIVARBPROC)wglGetProcAddress("glGetObjectParameterivARB");
        glAttachShader         = (PFNGLATTACHOBJECTARBPROC)wglGetProcAddress("glAttachObjectARB");
        glGetInfoLog           = (PFNGLGETINFOLOGARBPROC)wglGetProcAddress("glGetInfoLogARB");
        glLinkProgram          = (PFNGLLINKPROGRAMARBPROC)wglGetProcAddress("glLinkProgramARB");
        glGetUniformLocation   = (PFNGLGETUNIFORMLOCATIONARBPROC)wglGetProcAddress("glGetUniformLocationARB");
        glUniform4f            = (PFNGLUNIFORM4FARBPROC)wglGetProcAddress("glUniform4fARB");
		  glUniformMatrix4fv            = (PFNGLUNIFORMMATRIX4FVARBPROC)wglGetProcAddress("glUniformMatrix4fvARB");
		glUniform1i            = (PFNGLUNIFORM1IARBPROC)wglGetProcAddress("glUniform1iARB");
		 glActiveTexture  = (PFNGLACTIVETEXTUREARBPROC)wglGetProcAddress("glActiveTextureARB");
		 glUniform1fv  = (PFNGLUNIFORM1FVPROC)wglGetProcAddress("glUniform1fv");
		  

		 glIsShader  = (PFNGLISSHADERPROC)wglGetProcAddress("glIsShader");
		 glIsProgram  = (PFNGLISPROGRAMPROC)wglGetProcAddress("glIsProgram");
		 glGetShaderiv  = (PFNGLGETSHADERIVPROC)wglGetProcAddress("glGetShaderiv");
		 glGetProgramiv  = (PFNGLGETPROGRAMIVPROC)wglGetProcAddress("glGetProgramiv");
		 glGetProgramInfoLog  = (PFNGLGETPROGRAMINFOLOGPROC)wglGetProcAddress("glGetProgramInfoLog");

        if( !glCreateProgram || !glDeleteObject || !glUseProgram ||
           !glCreateShader || !glCreateShader || !glCompileShader ||
           !glGetShaderiv || !glAttachShader || !glGetInfoLog ||
           !glLinkProgram || !glGetUniformLocation || !glUniform4f || !glUniform1fv ||
           !glUniform1i || !glActiveTexture || !glIsShader || !glIsProgram || !glGetShaderiv || !glGetProgramiv || !glGetProgramInfoLog)
        {
            MessageBox(NULL,"One or more GL_ARB_shader_objects functions were not found",
                       "ERROR",MB_OK|MB_ICONEXCLAMATION);
            return;
        }
    }   
}



//-----------------------------------------------------------------------------
void shutDown( void )
{
    
	demo_deinit();

	if( g_hRC != NULL )
	{
		wglMakeCurrent( NULL, NULL );
		wglDeleteContext( g_hRC );

		g_hRC = NULL;
	}
    
	if( g_hDC != NULL )
	{
		ReleaseDC( g_hWnd, g_hDC );
		g_hDC = NULL;
	}

}



//-----------------------------------------------------------------------------
int WINAPI WinMain(	HINSTANCE hInstance,
                   HINSTANCE hPrevInstance,
                   LPSTR     lpCmdLine,
                   int       nCmdShow )
{
	WNDCLASSEX winClass;
	MSG        uMsg;
    
    memset(&uMsg,0,sizeof(uMsg));
    
	winClass.lpszClassName = "MY_WINDOWS_CLASS";
	winClass.cbSize        = sizeof(WNDCLASSEX);
	winClass.style         = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
	winClass.lpfnWndProc   = WindowProc;
	winClass.hInstance     = hInstance;
    winClass.hIcon	       = LoadIcon(hInstance, (LPCTSTR)IDI_WARNING);
    winClass.hIconSm	   = LoadIcon(hInstance, (LPCTSTR)IDI_WARNING);
	winClass.hCursor       = LoadCursor(NULL, IDC_ARROW);
	winClass.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);
	winClass.lpszMenuName  = NULL;
	winClass.cbClsExtra    = 0;
	winClass.cbWndExtra    = 0;
	
	if( !RegisterClassEx(&winClass) )
		return E_FAIL;
    
	const int scalar = 8;
	const int resX = 96*scalar;
	const int resY = 136*scalar;
	g_hWnd = CreateWindowEx( NULL,"MY_WINDOWS_CLASS",
						    "CRABBY WINDOWS",
							WS_OVERLAPPEDWINDOW,
					 	    0,0, resX,resY, NULL, NULL, g_hInstance, NULL );
    
	if( g_hWnd == NULL )
		return E_FAIL;
    
    ShowWindow( g_hWnd, nCmdShow );
    UpdateWindow( g_hWnd );
    
	if (__argc < 2)	
		return E_FAIL;
	

	char* inFile = __argv[1];	// The text file listing what loose assets to load
	

	init_OGL();

	demo_init(inFile);
	
    
	while( uMsg.message != WM_QUIT )
	{
		if( PeekMessage( &uMsg, NULL, 0, 0, PM_REMOVE ) )
		{
			TranslateMessage( &uMsg );
			DispatchMessage( &uMsg );
		}
      else
		{
		   demo_render();
			SwapBuffers( g_hDC );

		}
	}
    
	shutDown();
    
    UnregisterClass( "MY_WINDOWS_CLASS", g_hInstance );
    
	return uMsg.wParam;
}

//-----------------------------------------------------------------------------
LRESULT CALLBACK WindowProc( HWND   g_hWnd,
                            UINT   msg,
                            WPARAM wParam,
                            LPARAM lParam )
{
    static POINT ptLastMousePosit;
	static POINT ptCurrentMousePosit;
	static bool bMousing;
    
    switch( msg )
	{
        case WM_KEYDOWN:
		{
			switch( wParam )
			{
				case VK_ESCAPE:
					PostQuitMessage(0);
					break;
                    
				case VK_F1:
					
					break;
				case VK_UP:
					demo_incDrawCount();
					break;
				case VK_DOWN:
					demo_decDrawCount();
					break;
			}
		}
            break;
            
        case WM_LBUTTONDOWN:
		{
			ptLastMousePosit.x = ptCurrentMousePosit.x = LOWORD (lParam);
            ptLastMousePosit.y = ptCurrentMousePosit.y = HIWORD (lParam);
			bMousing = true;
		}
            break;
            
		case WM_LBUTTONUP:
		{
			bMousing = false;
		}
            break;
            
		case WM_MOUSEMOVE:
		{
			ptCurrentMousePosit.x = LOWORD (lParam);
			ptCurrentMousePosit.y = HIWORD (lParam);
            
			
			
			ptLastMousePosit.x = ptCurrentMousePosit.x;
            ptLastMousePosit.y = ptCurrentMousePosit.y;
		}
            break;
            
		case WM_SIZE:
		{
			int nWidth  = LOWORD(lParam);
			int nHeight = HIWORD(lParam);
			glViewport(0, 0, nWidth, nHeight);
            
		}
            break;
            
		case WM_CLOSE:
		{
			PostQuitMessage(0);	
		}
            
        case WM_DESTROY:
		{
            PostQuitMessage(0);
		}
            break;
            
		default:
		{
			return DefWindowProc( g_hWnd, msg, wParam, lParam );
		}
            break;
	}
    
	return 0;
}

