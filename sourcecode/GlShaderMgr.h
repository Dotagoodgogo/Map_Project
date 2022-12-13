// 
// GlShaderMgr.h - Version 1.1, December 20, 2019
//
// Author: Sam Buss
//
// Software accompanying POSSIBLE SECOND EDITION TO the book
// 3D Computer Graphics : A Mathematical Introduction with OpenGL,
// by S.Buss, Cambridge University Press, 2003.
//
// Software is "as-is" and carries no warranty.It may be used without
// restriction, but if you modify it, please change the filenames to
// prevent confusion between different versions.
// Bug reports : Sam Buss, sbuss@ucsd.edu.
// Web page : http://math.ucsd.edu/~sbuss/MathCG2
//

//
// Routines for 
//   A. Reading shader source code from files.
//   B. Compiling and linking shader programs
//

#ifndef GL_SHADER_MGR_H
#define GL_SHADER_MGR_H

#define GLEW_STATIC
#include <GL/glew.h> 
#include <GLFW/glfw3.h> 

#include <string>
#include <vector>

class GlShaderMgr {

public:
    // ***** 
    // Routines to load source code for shaders 
    // *****

    // Load shader source code from a file.
    //   Shader names (shader code block names) and the shader types
    //   are read from the file.
    // The files must use the #beginglsl ...  #endglsl convention.
    static bool LoadShaderSource(const char* filename);
 
    // Load shader source code from multiple files.
    static bool LoadShaderSource(int numFiles, const char* filenamePtr[]);

    // These are more "legacy" kind of routines.
    // They load a single shader source code block from either
    //    a file or a string
    static bool LoadSingleShaderFile(const char* filename, const char* shaderType, const char* shaderCodeName );
    static bool LoadSingleShaderString(const char* shaderSource, const char* shaderType, const char* shaderCodeName);

    // ***** 
    // Routines to compile shaders, and link shader programs.
    // *****
 
    // Compile and link multiple shaders.
    // These are convenience methods that is useful mostly if shaders
    //     are not re-used in multiple shader programs, and if
    //     there are no codeblock's
    // If no arguments are given, all loaded shaders are compiled
    //     into the shader program.
    // These three routines cannot be used with codeblock's:
    //     For those, compile the shaders separately.
    static unsigned int CompileAndLinkProgram(const char* shaderName1, const char* shaderName2);
    static unsigned int CompileAndLinkProgram(const char* shaderName1, const char* shaderName2, const char* shaderName3);
    static unsigned int CompileAndLinkAll();

    // Create a shader program by linking a list of already compiled shaders.
    // The already compiled shaders are specified by their OpenGL handles,
    //     as returned by CompileShader().
    static unsigned int LinkShaderProgram(int numShaders, const unsigned int shaderList[]);

    // Compile a shader from a single block of shader code
    //    Returns the OpenGL handle (name) for the shader.
    //    If the shader is already compiled, it will not cause an error
    //    The ID is >0 if the compilation was successful.
    //    If any error occurs, "0" is returned.
    static unsigned int CompileShader(const char* shaderCodeName);

    // Compile a shader from a two or three block of shader code
    //    One of the shaders must give the type of the shader,
    //        the other shadesr must be of type "codeblock".
    //    Returns the OpenGL handle (name) for the shader.
    //    The ID is >0 if the compilation was successful.
    //    If any error occurs, "0" is returned.
    static unsigned int CompileShader(const char* shaderCodeName1, const char* shaderCodeName2);
    static unsigned int CompileShader(const char* shaderCodeName1, const char* shaderCodeName2, const char* shaderCodeName3);

    // Compile a shader formed by concatenating multiple blocks of code.
    //    One of the shaders must give the type of the shader,
    //        the other shaders must be of type "codeblock".
    //    Returns the OpenGL handle (name) for the shader.
    //    The ID is >0 if the compilation was successful.
    //    If any error occurs, "0" is returned.
    static unsigned int CompileShader(int numcodeBlocks, const char* shaderCodeNames[]);
    
    // Clean up all intermediate results from compiling shaders
    //    Removes source code, and deletes no-longer needed shaders
    static void FinalizeCompileAndLink();

    // ****
    // Routines for error reporting. 
    // ****

    static unsigned int check_compilation_shader(unsigned int shader);
    static unsigned int check_link_status(unsigned int program);
    static unsigned int check_ok_to_link(int numShaders, const unsigned int shaderList[]);

protected:
    enum ShaderType { vertex_shader, fragment_shader, geometry_shader, code_block };
    static std::vector<std::string> shaderTypeName;
    static std::vector<unsigned int> openGLtypes;

    // The vector shdrInfo holds information about shaders.
    // Some shaders may lack names and source, namely if they were compiled from multiple code blocks.
    // Some shaders lack OpenGL handle, as they are only components of other shaders.
    //   shaderType - the type of the shader, vertex, fragment, etc., or "code block"
    //                a "code block" is a piece of a larger shader.
    //   shaderCodeName - name of the shader code (if any)
    //   shaderCodeArray - source code for the shader (if any)
    //   shaderOpenGLhandle - as generated during compilation
    typedef struct {
        ShaderType shaderType;
        std::string shaderCodeName;
        std::string shaderCodeArray;
        unsigned int shaderOpenGLhandle;
    } ShaderInfo;
    static std::vector<ShaderInfo> shdrInfo;

    static std::vector<ShaderInfo>::iterator findCodeName(const std::string theName);
    static std::vector<ShaderInfo>::iterator findOpenGLhandle(unsigned int theHandle);
    static bool AllocateShdrInfo(std::string& shaderType, const std::string& shaderCodeName);

    // The vector shdrPrograms contains the OpenGL handles for all linked shader programs.
    static std::vector<unsigned int> shdrPrograms;
};


#endif // GL_SHADER_MGR_H



#pragma once
