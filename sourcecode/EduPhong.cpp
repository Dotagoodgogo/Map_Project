
// *******************************
// EduPhong.cpp - Version 1.0 - April 14, 2020
//
// EduPhong.cpp and EduPhong.h code gives C++ classes
//     assisting in demonstrating Phong lighting for 
//     educational purposes.
//
// Version 0.3: 
//       Added support for the halfway vector
//       Added support for the Fresnel factor
//       Now reads shader code from a .glsl file.
//
// Author: Sam Buss
//
// Software is "as-is" and carries no warranty. It may be used without
//  restriction, but if you modify it, please change the filenames to
//  prevent confusion between different versions.
// Bug reports: Sam Buss, sbuss@ucsd.edu
// *******************************

#include <stdio.h>
#include <string.h>

#include "EduPhong.h"
#include "GlShaderMgr.h"

#include <GL/glew.h> 
#include <GLFW/glfw3.h>

bool check_for_opengl_errors();

// Special constants for loading booleans into shader program
unsigned int trueGLbool = 0xffffffff, falseGLbool = 0;



// *********************************
// Interface data for the shader programs.
unsigned int phShaderPhongPhong;
unsigned int phShaderPhongGouraud;
const unsigned int phVertPos_loc = 0;                  // Corresponds to "location = 0" in the vertex shader definition
const unsigned int phVertNormal_loc = 1;               // Corresponds to "location = 1" in the vertex shader definition
const unsigned int phEmissiveColor_loc = 3;            // Corresponds to "location = 3" in the vertex shader definition
const unsigned int phAmbientColor_loc = 4;             // Corresponds to "location = 4" in the vertex shader definition
const unsigned int phDiffuseColor_loc = 5;             // Corresponds to "location = 5" in the vertex shader definition
const unsigned int phSpecularColor_loc = 6;            // Corresponds to "location = 6" in the vertex shader definition
const unsigned int phSpecularExponent_loc = 7;         // Corresponds to "location = 7" in the vertex shader definition
const unsigned int phUseFresnel_loc = 8;               // Corresponds to "location = 7" in the vertex shader definition

/* *** 
 * Functions for uniform variable locations
 * *** */

unsigned int phGetProjMatLoc(unsigned int programID) {
    return glGetUniformLocation(programID, phProjMatName);
}
unsigned int phGetModelviewMatLoc(unsigned int programID) {
    return glGetUniformLocation(programID, phModelviewMatName);
}
unsigned int phGetApplyTextureLoc(unsigned int programID) {
    return glGetUniformLocation(programID, phApplyTextureName);
}

const char* globallightBlockName= "phGlobal";       // Name of the global light uniform block
const char* lightsBlockName = "phLightArray";       // Name of the light array uniform block


/*
 * Build and compile two shader programs
 *  One is for Phong lighting with Phong shading
 *  The other is for Phong lighting with Gouraud shading
 */
unsigned int phongUBO;              // Uniform Buffer Object for Phong lighting information
const int numGlobal = 8;            // Number of entries in the phGlobal structure
const int numLightData = 14;        // Number of entries in the phLight structure
const char* globalNames[numGlobal] = {
	"GlobalAmbientColor", "NumLights", "LocalViewer",
	"EnableEmissive", "EnableDiffuse", "EnableAmbient", "EnableSpecular",
	"UseHalfwayVector"
};
const char* lightNames[numLightData+1] = {
	"Lights[0].IsEnabled", "Lights[0].IsAttenuated", "Lights[0].IsSpotLight", "Lights[0].IsDirectional",
	"Lights[0].Position", "Lights[0].AmbientColor", "Lights[0].DiffuseColor", "Lights[0].SpecularColor", "Lights[0].SpotDirection",
	"Lights[0].SpotCosCutoff", "Lights[0].SpotExponent", 
	"Lights[0].ConstantAttenuation", "Lights[0].LinearAttenuation", "Lights[0].QuadraticAttenuation",
	"Lights[1].IsEnabled"
};

bool shaderLayoutInfoKnown = false; // Has a shader program already been analyzed?
GLint offsetsGlobal[numGlobal];     // Offsets into the UBO data for phGlobal data items.
GLint offsetsLight[numLightData+1]; // Offsets into the UBO data for phLight data items.
int globallightBlockSize;           // Size of globallight buffer in bytes
int lightsBlockSize;                // Size of data for a single light
int lightsBlockOffset;              // Offset for the light block in the uniform buffer object
int lightStride;                    // Stride between light blocks in the shader.

/*
* Build and compile two shader programs
*  One is for Phong lighting with Phong shading
*  The other is for Phong lighting with Gouraud shading
*/
void setup_phong_shaders() {
    GlShaderMgr::LoadShaderSource("EduPhong.glsl");

    unsigned int shader_VPG = GlShaderMgr::CompileShader("vertexShader_PhongGouraud", "calcPhongLighting");
    unsigned int shader_FPG = GlShaderMgr::CompileShader("fragmentShader_PhongGouraud", "applyTextureMap");
    unsigned int shaders_PG[2] = { shader_VPG, shader_FPG };
    phShaderPhongGouraud = GlShaderMgr::LinkShaderProgram(2, shaders_PG);
    phRegisterShaderProgram(phShaderPhongGouraud);

    unsigned int shader_VPP = GlShaderMgr::CompileShader("vertexShader_PhongPhong");
    unsigned int shader_FPP = GlShaderMgr::CompileShader("fragmentShader_PhongPhong", "calcPhongLighting", "applyTextureMap");
    unsigned int shaders_PP[2] = { shader_VPP, shader_FPP };
    phShaderPhongPhong = GlShaderMgr::LinkShaderProgram(2, shaders_PP);
    phRegisterShaderProgram(phShaderPhongPhong);
}

// **** 
// Must call once for each shader program before first use.
//    The programID is the OpenGL handle for the shader (as returned by GlShaderMgr::LinkShaderProgram, say)
//    The shader program must have the standard uniform blocks and variables for an EduPhong shader program,
//       containing exactly the same variables in exactly the same order.
// ****
bool phRegisterShaderProgram(unsigned int programID)
{

    unsigned int globallightBlockIndex = glGetUniformBlockIndex(programID, globallightBlockName);
    unsigned int lightsBlockIndex = glGetUniformBlockIndex(programID, lightsBlockName);
    if (globallightBlockIndex==GL_INVALID_INDEX || lightsBlockIndex==GL_INVALID_INDEX) {
        fprintf(stderr, "phRegisterShaderProgram: Required uniform block is missing!\n");
        return false;
    }
    glUniformBlockBinding(programID, globallightBlockIndex, 0);      // Buffer binding 0 for global lights
    glUniformBlockBinding(programID, lightsBlockIndex, 1);           // Buffer binding 1 for lights

    glUseProgram(programID);
    unsigned int applyTextureLocation = phGetApplyTextureLoc(programID);
    glUniform1i(applyTextureLocation, 0); // Default is to  not apply the texture

    if (shaderLayoutInfoKnown) {
        return true;
    }
    shaderLayoutInfoKnown = true;

    glGetActiveUniformBlockiv(programID, globallightBlockIndex, GL_UNIFORM_BLOCK_DATA_SIZE, &globallightBlockSize);
    glGetActiveUniformBlockiv(programID, lightsBlockIndex, GL_UNIFORM_BLOCK_DATA_SIZE, &lightsBlockSize);
    glGenBuffers(1, &phongUBO);
    glBindBuffer(GL_UNIFORM_BUFFER, phongUBO);
    int uboAlign;
    glGetIntegerv(GL_UNIFORM_BUFFER_OFFSET_ALIGNMENT, &uboAlign);
    lightsBlockOffset = uboAlign * (1 + (globallightBlockSize - 1) / uboAlign );
    int totalSize = lightsBlockOffset + lightsBlockSize;
    glBufferData(GL_UNIFORM_BUFFER, totalSize, 0, GL_STATIC_DRAW);
    glBindBufferRange(GL_UNIFORM_BUFFER, 0, phongUBO, 0, globallightBlockSize);
    glBindBufferRange(GL_UNIFORM_BUFFER, 1, phongUBO, lightsBlockOffset, lightsBlockSize);

    // Query locations in the global lights block
    GLuint indicesGlobal[numGlobal];
    // GLint sizesGlobal[numGlobal];
    // GLint typesGlobal[numGlobal];
    glGetUniformIndices(programID, numGlobal, globalNames, indicesGlobal);
    glGetActiveUniformsiv(programID, numGlobal, indicesGlobal, GL_UNIFORM_OFFSET, offsetsGlobal);
    // glGetActiveUniformsiv(programID, numGlobal, indicesGlobal, GL_UNIFORM_SIZE, sizesGlobal);
    // glGetActiveUniformsiv(programID, numGlobal, indicesGlobal, GL_UNIFORM_TYPE, typesGlobal);

    // Query locations in the individual lights block
    GLuint indicesLight[numLightData+1];
    //GLint sizesLight[numLightData];
    //GLint typesLight[numLightData];
    glGetUniformIndices(programID, numLightData+1, lightNames, indicesLight);
    glGetActiveUniformsiv(programID, numLightData+1, indicesLight, GL_UNIFORM_OFFSET, offsetsLight);
    //glGetActiveUniformsiv(programID, numLightData, indicesLight, GL_UNIFORM_SIZE, sizesLight);
    //glGetActiveUniformsiv(programID, numLightData, indicesLight, GL_UNIFORM_TYPE, typesLight);
    lightStride = offsetsLight[numLightData] - offsetsLight[0];
    if (phMaxNumLights*lightStride != lightsBlockSize) {
        fprintf(stderr, "EduPhong: Likely error in layout with shaders.\n");
        return false;
    }
    return true;
}

void phMaterial::LoadIntoShaders()
{
    float vecEntries[3];
    EmissiveColor.Dump(vecEntries);
    glVertexAttrib3fv(phEmissiveColor_loc, vecEntries);		// Emissive
    AmbientColor.Dump(vecEntries);
    glVertexAttrib3fv(phAmbientColor_loc, vecEntries);		// Ambient
    DiffuseColor.Dump(vecEntries);
    glVertexAttrib3fv(phDiffuseColor_loc, vecEntries);		// Diffuse
    SpecularColor.Dump(vecEntries);
    glVertexAttrib3fv(phSpecularColor_loc, vecEntries);		// Specular

    glVertexAttrib1f(phSpecularExponent_loc, SpecularExponent);	// Specular exponent
    glVertexAttrib1f(phUseFresnel_loc, UseFresnel ? 1.0f : 0.0f);	   // Load Use_Fresnel flag as a float 1.0 or 0.0.
}

void phGlobal::LoadIntoShaders()
{
    char* buffer = new char[globallightBlockSize];
    GlobalAmbientColor.Dump((float*)(buffer + offsetsGlobal[0]));
    memcpy(buffer + offsetsGlobal[1], &NumLights, sizeof(unsigned int));
    memcpy(buffer + offsetsGlobal[2], LocalViewer ? &trueGLbool : &falseGLbool, 4);      // Note the obscure way of loading a bool as a 4 byte integer
    memcpy(buffer + offsetsGlobal[3], EnableEmissive ? &trueGLbool : &falseGLbool, 4);
    memcpy(buffer + offsetsGlobal[4], EnableDiffuse ? &trueGLbool : &falseGLbool, 4);
    memcpy(buffer + offsetsGlobal[5], EnableAmbient ? &trueGLbool : &falseGLbool, 4);
    memcpy(buffer + offsetsGlobal[6], EnableSpecular ? &trueGLbool : &falseGLbool, 4);
    memcpy(buffer + offsetsGlobal[7], UseHalfwayVector ? &trueGLbool : &falseGLbool, 4);   
    glBindBuffer(GL_UNIFORM_BUFFER, phongUBO);
    glBufferSubData(GL_UNIFORM_BUFFER, 0, globallightBlockSize, buffer);

    delete[] buffer;
}

void phLight::LoadIntoShaders(int lightNumber) {
    assert(0<=lightNumber && lightNumber < phMaxNumLights);
    char* buffer = new char[lightStride];       // Allocate enough space to hold data for one light
    memcpy(buffer + offsetsLight[0], IsEnabled ? &trueGLbool : &falseGLbool, 4);      // Note: load a bool as a 4 byte integer
    memcpy(buffer + offsetsLight[1], IsAttenuated ? &trueGLbool : &falseGLbool, 4);
    memcpy(buffer + offsetsLight[2], IsSpotLight ? &trueGLbool : &falseGLbool, 4);
    memcpy(buffer + offsetsLight[3], IsDirectional ? &trueGLbool : &falseGLbool, 4);
    PosOrDir.Dump((float*)(buffer + offsetsLight[4]));
    AmbientColor.Dump((float*)(buffer + offsetsLight[5]));
    DiffuseColor.Dump((float*)(buffer + offsetsLight[6]));
    SpecularColor.Dump((float*)(buffer + offsetsLight[7]));
    SpotDirection.Dump((float*)(buffer + offsetsLight[8]));
    memcpy(buffer + offsetsLight[9], &SpotCosCutoff, sizeof(float));
    memcpy(buffer + offsetsLight[10], &SpotExponent, sizeof(float));
    memcpy(buffer + offsetsLight[11], &ConstantAttenuation, sizeof(float));
    memcpy(buffer + offsetsLight[12], &LinearAttenuation, sizeof(float));
    memcpy(buffer + offsetsLight[13], &QuadraticAttenuation, sizeof(float));
    glBindBuffer(GL_UNIFORM_BUFFER, phongUBO);
    int startLoc = lightsBlockOffset + lightNumber * lightStride;
    glBufferSubData(GL_UNIFORM_BUFFER, startLoc, lightStride, buffer);

}


bool phLight::CheckCorrectness()
{
    // If it is directional,  the position is instead the direction, and should be a unit vector.
    if (IsDirectional) {
        float sqNorm =
            (float)(PosOrDir[0] * PosOrDir[0]
                + PosOrDir[1] * PosOrDir[1]
                + PosOrDir[2] * PosOrDir[2]);
        if (sqNorm<0.000001) {
            fprintf(stdout, "phLight error: Directional light needs directon from light in 'Position'.\n");
            return false;
        }
    }
    // If it is a spot light, the spot direction should be a unit vector.
    if (IsSpotLight) {
        float sqNorm =
            (float)(SpotDirection[0] * SpotDirection[0]
                + SpotDirection[1] * SpotDirection[1]
                + SpotDirection[2] * SpotDirection[2]);
        if (IsDirectional || sqNorm<0.999f || sqNorm>1.001f) {
            fprintf(stdout, "phLight error: Spot light misconfigured.\n");
            return false;
        }
    }
    return true;
}

bool phGlobal::CheckCorrectness() 
{
    if (NumLights > phMaxNumLights) {
        fprintf(stdout, "pgGlobal error: Too many lights.\n");
        return false;
    }
    return true;
}

