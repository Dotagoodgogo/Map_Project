/*
* GlGeomCylinder.cpp - Version 1.1 - November 13, 2020.
*
* C++ class for rendering cylinders in Modern OpenGL.
*   A GlGeomCylinder object encapsulates a VAO, a VBO, and an EBO,
*   which can be used to render a cylinder.
*   The number of slices and stacks and rings can be varied.
*
* Author: Sam Buss
*
* Software accompanying POSSIBLE SECOND EDITION TO the book
*		3D Computer Graphics: A Mathematical Introduction with OpenGL,
*		by S. Buss, Cambridge University Press, 2003.
*
* Software is "as-is" and carries no warranty.  It may be used without
*   restriction, but if you modify it, please change the filenames to
*   prevent confusion between different versions.
* Bug reports: Sam Buss, sbuss@ucsd.edu.
* Web page: http://math.ucsd.edu/~sbuss/MathCG2
*/

// Use the static library (so glew32.dll is not needed):
#define GLEW_STATIC
#include <GL/glew.h> 
#include <GLFW/glfw3.h>

#include "GlGeomCylinder.h"
#include "MathMisc.h"
#include "assert.h"


void GlGeomCylinder::Remesh(int slices, int stacks, int rings)
{
    if (slices == numSlices && stacks == numStacks && rings == numRings) {
        return;
    }
    numSlices = ClampRange(slices, 3, 255);
    numStacks = ClampRange(stacks, 1, 255);
    numRings = ClampRange(rings, 1, 255);

    VboEboLoaded = false;
}


void GlGeomCylinder::CalcVboAndEbo(float* VBOdataBuffer, unsigned int* EBOdataBuffer,
    int vertPosOffset, int vertNormalOffset, int vertTexCoordsOffset, unsigned int stride)
{
    assert(vertPosOffset >= 0 && stride > 0);
    bool calcNormals = (vertNormalOffset >= 0);       // Should normals be calculated?
    bool calcTexCoords = (vertTexCoordsOffset >= 0);  // Should texture coordinates be calculated?

    // VBO Data is laid out: top face vertices, then bottom face vertices, then side vertices

    // Set top and bottom center vertices
    SetDiscVerts(0.0, 0.0, 0, 0, VBOdataBuffer, vertPosOffset, vertNormalOffset, vertTexCoordsOffset, stride);
    int stopSlices = calcTexCoords ? numSlices : numSlices - 1;
    for (int i = 0; i <= stopSlices; i++) {
        // Handle a slice of vertices.
        // theta measures from the negative z-axis, counterclockwise viewed from above.
        float theta = ((float)(i%numSlices))*(float)PI2 / (float)(numSlices);
        float c = -cosf(theta);      // Negated values (start at negative z-axis)
        float s = -sinf(theta);
        if (i < numSlices) {
            // Top & bottom face vertices, positions and normals and texture coordinates
            for (int j = 1; j <= numRings; j++) {
                float radius = (float)j / (float)numRings;
                SetDiscVerts(s * radius, c * radius, i, j, VBOdataBuffer, vertPosOffset, vertNormalOffset, vertTexCoordsOffset, stride);
            }
        }
        float* basePtr = VBOdataBuffer + (2*GetNumVerticesDisk()+ i*(numStacks + 1))*stride;
        float sCoord = ((float)i) / (float)(numSlices);
        // Side vertices, positions and normals and texture coordinates
        for (int j = 0; j <= numStacks; j++, basePtr+=stride) {
            float* vPtr = basePtr + vertPosOffset;
            float tCoord = (float)j / (float)numStacks;
            *(vPtr++) = s;
            *(vPtr++) = -1.0f + 2.0f*tCoord;
            *vPtr = c;
            if (calcNormals) {
                float* nPtr = basePtr + vertNormalOffset;
                *(nPtr++) = s;
                *(nPtr++) = 0.0f;
                *nPtr = c;
            }
            if (calcTexCoords) {
                float* tcPtr = basePtr + vertTexCoordsOffset;
                *(tcPtr++) = sCoord;
                *tcPtr = tCoord;
            }
        }
    }

    // EBO data is also laid out as base, the top, then sides
    unsigned int* eboPtr = EBOdataBuffer;
    // Bottom 
    for (int i = 0; i < numSlices; i++) {
        int r = i*numRings + 1;
        int rightR = ((i+1)%numSlices)*numRings + 1;
        *(eboPtr++) = 0;
        *(eboPtr++) = rightR;
        *(eboPtr++) = r;
        for (int j = 0; j < numRings-1; j++) {
            *(eboPtr++) = r + j;
            *(eboPtr++) = rightR + j;
            *(eboPtr++) = rightR + j + 1;
 
            *(eboPtr++) = r + j;
            *(eboPtr++) = rightR + j + 1;
            *(eboPtr++) = r + j + 1;
        }
    }
    // Top 
    int delta = GetNumVerticesDisk();
    for (int i = 0; i < numSlices; i++) {
        int r = delta + i*numRings + 1;
        int leftR = delta + ((i + 1) % numSlices)*numRings + 1;
        *(eboPtr++) = delta;
        *(eboPtr++) = r;
        *(eboPtr++) = leftR;
        for (int j = 0; j < numRings-1; j++) {
            *(eboPtr++) = leftR + j;
            *(eboPtr++) = r + j;
            *(eboPtr++) = r + j + 1;

            *(eboPtr++) = leftR + j;
            *(eboPtr++) = r + j + 1;
            *(eboPtr++) = leftR + j + 1;
        }
    }
    // Side
    for (int i = 0; i < numSlices; i++) {
        int r = i*(numStacks + 1) + 2*delta;
        int ii = calcTexCoords ? (i + 1) : (i + 1) % numSlices;
        int rightR = ii*(numStacks + 1) + 2*delta;
        for (int j = 0; j < numStacks; j++) {
            *(eboPtr++) = rightR + j;
            *(eboPtr++) = r + j + 1;
            *(eboPtr++) = r + j;

            *(eboPtr++) = rightR + j;
            *(eboPtr++) = rightR + j + 1;
            *(eboPtr++) = r + j + 1;
        }
    }
}

void GlGeomCylinder::SetDiscVerts(float x, float z, int i, int j, float* VBOdataBuffer,
    int vertPosOffset, int vertNormalOffset, int vertTexCoordsOffset, int stride)
{
    // i is the slice number, j is the ring number.
    // j==0 means the center point.  In this case, i must equal 0. (Not checked)
    float* basePtrBottom = VBOdataBuffer + stride*(i*numRings + j);
    int delta = GetNumVerticesDisk()*stride;
    float* vPtrBottom = basePtrBottom + vertPosOffset;
    float* vPtrTop = vPtrBottom + delta;
    *(vPtrBottom++) = x;
    *(vPtrBottom++) = -1.0;
    *vPtrBottom = z;
    *(vPtrTop++) = x;
    *(vPtrTop++) = 1.0;
    *vPtrTop = z;
    if (vertNormalOffset>=0) {
        float* nPtrBottom = basePtrBottom + vertNormalOffset;
        float* nPtrTop = nPtrBottom + delta;
        *(nPtrBottom++) = 0.0;
        *(nPtrBottom++) = -1.0;
        *nPtrBottom = 0.0;
        *(nPtrTop++) = 0.0;
        *(nPtrTop++) = 1.0;
        *nPtrTop = 0.0;
    }
    if (vertTexCoordsOffset>=0) {
        float sCoord = 0.5f*(x + 1.0f);
        float tCoord = 0.5f*(-z + 1.0f);
        float* tcPtrBottom = basePtrBottom + vertTexCoordsOffset;
        float* tcPtrTop = tcPtrBottom + delta;
        *(tcPtrBottom++) = 1.0f - sCoord;
        *tcPtrBottom = tCoord;
        *(tcPtrTop++) = sCoord;
        *tcPtrTop = tCoord;
    }
}


void GlGeomCylinder::InitializeAttribLocations(
    unsigned int pos_loc, unsigned int normal_loc, unsigned int texcoords_loc)
{
    // The call to GlGeomBase::InitializeAttribLocations will further call
    //   GlGeomSphere::CalcVboAndEbo()

    GlGeomBase::InitializeAttribLocations(pos_loc, normal_loc, texcoords_loc);
    VboEboLoaded = true;
}


// **********************************************
// These routines do the rendering.
// If the cylinder's VAO, VBO, EBO need to be loaded, it does this first.
// **********************************************

void GlGeomCylinder::PreRender()
{
    GlGeomBase::PreRender();

    if (!VboEboLoaded) {
        ReInitializeAttribLocations();
    }
}

void GlGeomCylinder::Render()
{
    PreRender();
    GlGeomBase::Render();
}

void GlGeomCylinder::RenderTop()
{
    PreRender();

    GlGeomBase::RenderEBO(GL_TRIANGLES, GetNumElementsDisk(), 0);
}

void GlGeomCylinder::RenderBase()
{
    PreRender();

    int n = GetNumElementsDisk();
    GlGeomBase::RenderEBO(GL_TRIANGLES, n, n);
}

void GlGeomCylinder::RenderSide()
{
    PreRender();

    GlGeomBase::RenderEBO(GL_TRIANGLES, GetNumElementsSide(), 2 * GetNumElementsDisk());
}



