/** @file
 *
 *  Implementation of glprogram.hpp
 *
 *  @copyright 2016  Palm Studios
 *
 *  @date
 *
 *  @author Jesse Buhagiar
 */
#include <SH3/system/glprogram.hpp>
#include <SH3/system/log.hpp>

#include <fstream>
#include <vector>

static const GLuint BAD_SHADER = 0;

using namespace sh3_graphics;

void sh3_glprogram::load_error::set_error(load_result res)
{
    result = res;
}

std::string sh3_glprogram::load_error::message() const
{
    std::string error;
    switch(result)
    {
    case load_result::SUCCESS:
        error = "SUCCESS";
        break;
    case load_result::COMPILATION_FAILED:
        error = "COMPILATION_FAILED";
        break;
    case load_result::LINK_FAILED:
        error = "LINK_FAILED";
        break;
    case load_result::INVALID_VALUE:
        error = "GL_INVALID_VALUE";
        break;
    case load_result::INVALID_OPERATION:
        error = "GL_INVALID_OPERATION";
        break;
    }

    return error;
}

void sh3_glprogram::Load(const std::string& shader, load_error& err, const std::vector<std::string>& attribs)
{
    GLenum status = 0;

    programID = glCreateProgram();

    // Generate our vertex and fragment shader
    vertShader = (GLuint)Compile(GL_VERTEX_SHADER);
    fragShader = (GLuint)Compile(GL_FRAGMENT_SHADER);

    glAttachShader(programID, vertShader);
    glAttachShader(programID, fragShader);

    // Bind any attributes before we link the program
    for(int i = 0; i < attribs.size(); i++)
    {
        glBindAttribLocation(programID, i, reinterpret_cast<const GLchar*>(attribs[i].c_str()));

        status = glGetError();
        if(status == GL_INVALID_VALUE)
        {
            err.set_error(load_result::INVALID_VALUE);
            Log(LogLevel::ERROR, "glprogram::Load( ): Error binding attribute %s (index > GL_MAX_VERTEX_ATTRIBS or programID invalid!)", attribs[i].c_str());
        }
        else if(status == GL_INVALID_OPERATION)
        {
            err.set_error(load_result::INVALID_OPERATION);
            Log(LogLevel::ERROR, "glprogram::Load( ): Error binding attribute %s (perhaps attrib name is spelt wrong or starts with gl_!?!)", attribs[i].c_str());
        }
    }

    glLinkProgram(programID);

    // Make sure our program was linked without any errors
    glGetProgramiv(programID, GL_LINK_STATUS, (int*)&status);

    if(status == GL_FALSE)
    {
        GLint logLength = 0;
        std::vector<GLchar> errorLog;   // Error log
        glGetShaderiv(programID, GL_INFO_LOG_LENGTH, &logLength);

        errorLog.resize(logLength);
        glGetProgramInfoLog(programID, logLength, &logLength, &errorLog[0]);
        glDeleteShader(vertShader);
        glDeleteShader(fragShader);

        Log(LogLevel::ERROR, "%s\n%s", "glprogram::Load( ): Linking failed! Reason:", &errorLog[0]);
        err.set_error(load_result::LINK_FAILED);
        // TODO: Default shader fallback.
    }

    glDeleteShader(vertShader); // Blast these shaders into Oblivion!! They are no use to us anymore!
    glDeleteShader(fragShader);

    err.set_error(load_result::SUCCESS);
}

void sh3_glprogram::Bind()
{
    if(programID == 0)
    {
        Log(LogLevel::WARN, "sh3_glprogram::Bind( ): It seems this glprogram has not been compiled! It is impossible for it to be bound!");
        return;
    }

    glUseProgram(programID);
}

void sh3_glprogram::Unbind()
{
    glUseProgram(0); // The 'correct' way to unbind a shader, but apparently undefined?!?!
}

// TODO: Unfuck this function! It looks like a pile of shit (and acts like one too)..
GLuint sh3_glprogram::Compile(GLenum type)
{
    GLint   status; // Used to check compilation status
    GLuint  id;     // ID of this shader

    std::ifstream       source;     // Handle to the shader program source
    std::string         ssource;    // Temporary source string to read into
    std::string         fname;      // Name of the file we want to open
    std::string         line;
    std::vector<GLchar> errorLog;   // Error log

    if(type == GL_VERTEX_SHADER)
    {
        id = glCreateShader(GL_VERTEX_SHADER); // Generate a shader ID
        fname = "data/shaders/" + programName + ".vert";
    }
    else if(type == GL_FRAGMENT_SHADER)
    {
        id = glCreateShader(GL_FRAGMENT_SHADER); // Generate a shader ID
        fname = "data/shaders/" + programName + ".frag";
    }
    else
    {
        die("glprogram::Compile( ): Only GL_VERTEX and GL_FRAGMENT shader programs are supported!");
    }

    source.open(fname);

    if(!source.is_open())
    {
        //TODO: default shader fallback.
        Log(LogLevel::ERROR, "sh3_glprogram::Compile( ): Unable to open a handle to %s!", fname.c_str());
        return BAD_SHADER; // 0 is the 'bad shader' or rather one that is unbound.
    }

    while(!source.eof())
    {
        std::getline(source, line);
        ssource += line + "\n";
    }

    const GLchar* csource = ssource.c_str();
    glShaderSource(id, 1, &csource, 0);
    glCompileShader(id);

    glGetShaderiv(id, GL_COMPILE_STATUS, &status);
    if(status == GL_FALSE) // Ooops, our shader compilation failed! D:
    {
        GLint logLength = 0;
        std::string errmsg = "sh3_glprogram::Compile( ) error in " + fname;

        glGetShaderiv(id, GL_INFO_LOG_LENGTH, &logLength);

        errorLog.resize(logLength);
        glGetShaderInfoLog(id, logLength, &logLength, &errorLog[0]);
        glDeleteShader(id); // Probably pointless, considering we can't continue!

        Log(LogLevel::ERROR, "%s\n%s", errmsg.c_str(), &errorLog[0]); // TODO: Default shader fallback
    }

    return id;
}