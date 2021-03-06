/*++

Copyright (c) 2016  Palm Studios

Module Name:
        sh3_glcontext.cpp

Abstract:
        Implementation of sh3_glcontext.hpp

Author:
        Jesse Buhagiar

Environment:

Notes:

Revision History:
        22-12-2016: File Created                                        [jbuhagiar]
--*/
#include <cassert>
#include <cstdio>
#include <cstdlib>
#include <limits>

#include "SH3/system/glcontext.hpp"
#include "SH3/system/log.hpp"
#include "SH3/system/window.hpp"
#include <GL/glew.h>
#include <GL/glu.h>
#include <GL/glext.h>

sh3_glcontext::sh3_glcontext(sh3_window& hwnd)
{
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);

    glContext.reset(SDL_GL_CreateContext(hwnd.hwnd.get()));

    if(glewInit() != GLEW_OK) // Initialise GLEW!
    {
        Log(LogLevel::FATAL, "sh3_glcontext::sh3_glcontext( ): GLEW Init failed!");
        std::exit(-1);
    }

    // Set the colour size for OpenGL!
    SDL_GL_SetAttribute(SDL_GL_RED_SIZE, 8);
    SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE, 8);
    SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE, 8);
    SDL_GL_SetAttribute(SDL_GL_ALPHA_SIZE, 8);
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
    SDL_GL_SetAttribute(SDL_GL_BUFFER_SIZE, 32);

    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
}

/*++

Routine Description:
        Gets the GLVENDOR String (i.e Graphics Card Manufacturer)

Arguments:
        None

Return Type:
        const char* - GLVENDOR String

--*/
const char* sh3_glcontext::GetVendor() const
{
    return GlGetString(GL_VENDOR);
}

/*++

Routine Description:
        Gets the GLVERSION String (minor.major version)

Arguments:
        None

Return Type:
        const char*

--*/
const char* sh3_glcontext::GetVersion() const
{
    return GlGetString(GL_VERSION);
}

/*++

Routine Description:
        Gets the name of the renderer (Graphics Card that created this context)

Arguments:
        None

Return Type:
        const char*

--*/
const char* sh3_glcontext::GetRenderer() const
{
    return GlGetString(GL_RENDERER);
}

/*++

Routine Description:
        Get a list of all the extensions supported by this Context (Supported by the actual GPU[???]).

Arguments:
        None

Return Type:
        None

--*/
void sh3_glcontext::GetExtensions()
{
    GLint numExts;
    GLint i;

    glGetIntegerv(GL_NUM_EXTENSIONS, &numExts); // Get the number of extensions the system supports
    static_assert(std::numeric_limits<std::size_t>::max() >= std::numeric_limits<GLint>::max(), "std::size_t must be able to represent all positive GLint");
    assert(numExts >= 0);
    extensions.reserve(static_cast<std::size_t>(numExts));

    // Iterate over each extension the graphics card supports and store it
    for(i = 0; i < numExts; i++)
    {
        extensions.push_back(reinterpret_cast<const char*>(glGetStringi(GL_EXTENSIONS, i)));
    }
}

/*++

Routine Description:
        Prints out information about the OpenGL Context we've created

Arguments:
        None

Return Type:
        None

--*/
void sh3_glcontext::PrintInfo() const
{
    Log(LogLevel::INFO, "GL_VENDOR:\t %s", GetVendor());
    Log(LogLevel::INFO, "GL_VERSION:\t %s", GetVersion());
    Log(LogLevel::INFO, "GL_RENDERER:\t %s", GetRenderer());

    std::printf("GL_VENDOR:\t %s\n", GetVendor());
    std::printf("GL_VERSION:\t %s\n", GetVersion());
    std::printf("GL_RENDERER:\t %s\n", GetRenderer());
}

/*++

Routine Description:
        Retrieve a C-String via glGetString

Arguments:
        name - the attribute to retrieve

Return Type:
        const char*

--*/
const char* sh3_glcontext::GlGetString(GLenum name)
{
    return reinterpret_cast<const char*>(glGetString(name));
}
