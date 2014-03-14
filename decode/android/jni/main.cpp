
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

#include <stdio.h>

#include <jni.h>
#include <errno.h>

#include <EGL/egl.h>
#include <GLES/gl.h>

#include <android/sensor.h>
#include <android/log.h>
#include <android_native_app_glue.h>
#include <android/asset_manager.h>

#include "platform.h"

void demo_deinit( void );
void demo_init( AAssetManager* mgr,const char* val );
void demo_render( void );
void demo_incDrawCount( void );
void demo_decDrawCount( void );


//-----------------------------------------------------
// Our saved state data; just dummy for now, cause we don't need it.
 
struct saved_state {
    int32_t dummy;
};
//-----------------------------------------------------
// Shared state for our app. We keep this around incease we loose context
 
struct engine {
    struct android_app* app;
    
    EGLDisplay display;
    EGLSurface surface;
    EGLContext context;
    int32_t width;
    int32_t height;
    struct saved_state state;
    AAssetManager* pAssetManager;
};

struct engine* gEngine;

//-----------------------------------------------------
static int init(struct engine* engine) 
{
    // initialize OpenGL ES and EGL

    // Here specify the attributes of the desired configuration.
    // Below, we select an EGLConfig with at least 8 bits per color
    // component compatible with on-screen windows
    //
    const EGLint attribs[] = {
            EGL_SURFACE_TYPE, EGL_WINDOW_BIT,
            EGL_BLUE_SIZE, 8,
            EGL_GREEN_SIZE, 8,
            EGL_RED_SIZE, 8,
            EGL_NONE
    };
    EGLint w, h, dummy, format;
    EGLint numConfigs;
    EGLConfig config;
    EGLSurface surface;
    EGLContext context;

    EGLDisplay display = eglGetDisplay(EGL_DEFAULT_DISPLAY);

    eglInitialize(display, 0, 0);

    // Here, the application chooses the configuration it desires. In this
    // sample, we have a very simplified selection process, where we pick
    // the first EGLConfig that matches our criteria 
    eglChooseConfig(display, attribs, &config, 1, &numConfigs);

    // EGL_NATIVE_VISUAL_ID is an attribute of the EGLConfig that is
    // guaranteed to be accepted by ANativeWindow_setBuffersGeometry().
    // As soon as we picked a EGLConfig, we can safely reconfigure the
    // ANativeWindow buffers to match, using EGL_NATIVE_VISUAL_ID. 
    eglGetConfigAttrib(display, config, EGL_NATIVE_VISUAL_ID, &format);

    ANativeWindow_setBuffersGeometry(engine->app->window, 0, 0, format);

    surface = eglCreateWindowSurface(display, config, engine->app->window, NULL);
    EGLint contextAttribs[] = { EGL_CONTEXT_CLIENT_VERSION, 2, EGL_NONE, EGL_NONE };
    context = eglCreateContext(display, config, EGL_NO_CONTEXT, contextAttribs);

    if (eglMakeCurrent(display, surface, surface, context) == EGL_FALSE) {
        LOGW("Unable to eglMakeCurrent");
        return -1;
    }

    eglQuerySurface(display, surface, EGL_WIDTH, &w);
    eglQuerySurface(display, surface, EGL_HEIGHT, &h);

    engine->display = display;
    engine->context = context;
    engine->surface = surface;
    engine->width = w;
    engine->height = h;

    // Initialize GL state.
    glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_FASTEST);
    glEnable(GL_CULL_FACE);
    
    glDisable(GL_DEPTH_TEST);

    //HAX
    demo_init(engine->pAssetManager,"crab");

    return 0;
}

//-----------------------------------------------------
static void deinit(struct engine* engine) 
{
    demo_deinit();
    //Tear down the EGL context currently associated with the display.
    if (engine->display != EGL_NO_DISPLAY) 
    {
        eglMakeCurrent(engine->display, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);
        if (engine->context != EGL_NO_CONTEXT) 
        {
            eglDestroyContext(engine->display, engine->context);
        }
        if (engine->surface != EGL_NO_SURFACE) {
            eglDestroySurface(engine->display, engine->surface);
        }
        eglTerminate(engine->display);
    }
    engine->display = EGL_NO_DISPLAY;
    engine->context = EGL_NO_CONTEXT;
    engine->surface = EGL_NO_SURFACE;

    engine->pAssetManager = 0;
}

//-----------------------------------------------------
static void render(struct engine* engine) 
{
    // Bail if we dont' have a display
    if (engine->display == NULL) 
        return;

    // Just fill the screen with a color.
    glClearColor(0.2,0.5,0.2,1);
    glClear(GL_COLOR_BUFFER_BIT);

    demo_render();

    // we swap the buffers at the end of the frame
    eglSwapBuffers(engine->display, engine->surface);
}



//-----------------------------------------------------
static int32_t onInput(struct android_app* app, AInputEvent* event) 
{
    //Process the next input event.
    
    //how this works: determinet he input type, given by AInputEvent_getType
    // then, depending on what value you get (AINPUT_EVENT_TYPE_MOTION for instance)
    // you use a seperate set of function calls to get information (AMotionEvent_getX for instance)
    // For a full list of available comamnds, please refer to the platform's INPUT.H file
    //  For example C:\android\ndk\platforms\android-19\arch-arm\usr\include\android\input.h

    struct engine* engine = (struct engine*)app->userData;

    const int  inputEvenType = AInputEvent_getType(event);

    // Our samples are really only interested in motion input events
    if (inputEvenType != AINPUT_EVENT_TYPE_MOTION) return 0;


    const int pointerID = AMotionEvent_getPointerId(event, 0);

    
    const int action = AMotionEvent_getAction(event);
    const unsigned int touchx = AMotionEvent_getX(event, 0);
    const unsigned int touchy = AMotionEvent_getY(event, 0);

    if(action ==0) //TOUCH BEGIN
    {
        
    }
    else if(action == 1) //TOUCH END
    {
        demo_incDrawCount();
    }
    else if(action == 2) //TOUCH MOVE
    {
        
    }

    return 1; 
}

//-----------------------------------------------------
static void onAppCommand(struct android_app* app, int32_t cmd) 
{
    // Process the next main command.
    struct engine* engine = (struct engine*)app->userData;
    switch (cmd) 
    {
        case APP_CMD_SAVE_STATE:
            // The system has asked us to save our current state.  Do so.
            // NOTE i haven't had this happen to me yet.. I think it's got more to do with
            //making the app become the foreground / background
            engine->app->savedState = malloc(sizeof(struct saved_state));
            *((struct saved_state*)engine->app->savedState) = engine->state;
            engine->app->savedStateSize = sizeof(struct saved_state);
            LOGI("SAVING STATE");
            break;

        case APP_CMD_INIT_WINDOW:
            // The window is being shown, get it ready.
            if (engine->app->window != NULL) 
            {
                init(engine);
                render(engine);
            }
            break;

        case APP_CMD_TERM_WINDOW:
            LOGI("TERMINATE");
            // The window is being hidden or closed, clean it up.
            deinit(engine);
            break;

        case APP_CMD_GAINED_FOCUS:
            // When our app gains focus.....
            
            break;

        case APP_CMD_LOST_FOCUS:
            LOGI("LOST FOCUS");
            // When our app loses focus.....
            // Also stop animating.
            render(engine);
            break;
    }
}
//-----------------------------------------------------
void android_main(struct android_app* state) 
{
    // This is the main entry point of a native application that is using
    // android_native_app_glue.  It runs in its own thread, with its own
    // event loop for receiving input events and doing other things.

    struct engine engine;

    // Make sure glue isn't stripped.
    app_dummy();

    memset(&engine, 0, sizeof(engine));
    state->userData = &engine;
    state->onAppCmd = onAppCommand;
    state->onInputEvent = onInput;
    
    engine.app = state;

    // We are starting with a previous saved state; restore from it.
    if (state->savedState != NULL)     
        engine.state = *(struct saved_state*)state->savedState;
    
    
    //let's test some fileIO processes
    //we need a pointer to the asset manager so we can read from the apk file
    engine.pAssetManager = state->activity->assetManager;
    
    
    LOGE("BOOTING UP");

    // loop waiting for stuff to do.
    while (1) 
    {
        // Read all pending events.
        int ident;
        int events;
        struct android_poll_source* source;

        // Loop until all events are read, then continue to draw the next frame of animation.
        while ((ident = ALooper_pollAll(0, NULL, &events, (void**)&source)) >= 0) 
        {
            // Process this event.
            if (source != NULL) 
                source->process(state, source); 

            // Check if we are exiting.
            if (state->destroyRequested != 0) 
            {
                deinit(&engine);
                return;
            }
        }

        //we're now done processing events, go ahead and render the screen    
        // Drawing is throttled to the screen update rate, so there
        // is no need to do timing here.
        render(&engine);

        
        
    }
}
