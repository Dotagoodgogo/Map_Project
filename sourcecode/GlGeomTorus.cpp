/*
 * GlGeomTorus.cpp - Version 1.2 - Decmber 8, 2020 (Compatible with versions 1.1.
 *
 * C++ class for rendering tori in Modern OpenGL.
 *   A GlGeomTorus object encapsulates a VAO, a VBO, and an EBO,
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
 *
 * We thank David Clark for a correction to the generation
 * of EBO data (December 2020).
 */

// Use the static library (so glew32.dll is not needed):
#define GLEW_STATIC
#include <GL/glew.h> 
#include <GLFW/glfw3.h>

#include "GlGeomTorus.h"
#include "MathMisc.h"
#include "assert.h"


void GlGeomTorus::Remesh(int rings, int sides, float minorRadius)
{
    if (sides == numSides && rings == numRings && minorRadius == radius) {
        return;
    }
    numSides = ClampRange(sides, 3, 255);
    numRings = ClampRange(rings, 3, 255);
    radius = minorRadius;           // Should be between 0.0 and 1.0

    VboEboLoaded = false;
}


void GlGeomTorus::CalcVboAndEbo(float* VBOdataBuffer, unsigned int* EBOdataBuffer,
    int vertPosOffset, int vertNormalOffset, int vertTexCoordsOffset, unsigned int stride)
{
    assert(vertPosOffset >= 0 && stride > 0);
    bool calcNormals = (vertNormalOffset >= 0);       // Should normals be calculated?
    bool calcTexCoords = (vertTexCoordsOffset >= 0);  // Should texture coordinates be calculated?

    // VBO Data is laid out: Around each ring. Starting with ring at x==0 and z<0.
    //          Each ring starts at the innermost seam of the torus (nearest to the y-axis).
    float* toPtr = VBOdataBuffer;

    // Outermost loop over the rings
    int stopRings = calcTexCoords ? numRings : numRings-1;
    for (int i = 0; i <= stopRings; i++) {
        // Handle a ring of vertices.
        // theta measures from the negative z-axis, counterclockwise viewed from above.
        float sCoord = ((float)(i)) / (float)(numRings);
        float theta = (float)PI2 * ((float)(i % numRings)) / (float)(numRings);
        float c = -cosf(theta);      // Negated values (start at negative z-axis)
        float s = -sinf(theta);
        int stopSides = calcTexCoords ? numSides : numSides - 1;
        for (int j = 0; j <= stopSides; j++, toPtr += stride) {
            // phi measures from the inner seam, going under, around and over, back to the inner seam.
            float tCoord = ((float)(j)) / (float)(numSides);
            float phi = (float)PI2 * ((float)(j % numSides)) / (float)(numSides);
            float cphi = -cosf(phi);      // Negated value (start at inner seam)
            float sphi = -sinf(phi);       // Negated, start downward (-y)
            float* posPtr = toPtr;
            *(posPtr++) = s * (1.0f + radius * cphi);    // x coordinate
            *(posPtr++) = radius * sphi;                  // y coordinate
            *posPtr = c * (1.0f + radius * cphi);        // z coordinate
            if (calcNormals) {
                float* nPtr = toPtr + vertNormalOffset;
                *(nPtr++) = s * cphi;           // Normal in x direction
                *(nPtr++) = sphi;                  // Normal in y direction
                *nPtr = c * cphi;               // Normal in z direction
            }
            if (calcTexCoords) {
                float* tcPtr = toPtr + vertTexCoordsOffset;
                *(tcPtr++) = sCoord;
                *tcPtr = tCoord;
            }
        }
    }

    // EBO data is also laid out in the same order, for GL_TRIANGLES
    unsigned int* eboPtr = EBOdataBuffer;
    int ringDelta = calcTexCoords ? numSides + 1 : numSides;
    for (int ii = 0; ii < numRings; ii++) {
        int iii = calcTexCoords ? (ii + 1) : ((ii + 1) % numRings);
        int leftR = ii * ringDelta;
        int rightR = iii *ringDelta;
        for (int j = 0; j < numSides; j++) {
            int jj = calcTexCoords ? (j + 1) : ((j + 1) % numSides);
            *(eboPtr++) = rightR + j;
            *(eboPtr++) = leftR + jj;
            *(eboPtr++) = leftR+j;

            *(eboPtr++) = rightR + j;
            *(eboPtr++) = rightR + jj;
            *(eboPtr++) = leftR + jj;
        }
    }
}


void GlGeomTorus::InitializeAttribLocations(
    unsigned int pos_loc, unsigned int normal_loc, unsigned int texcoords_loc)
{
    // The call to GlGeomBase::InitializeAttribLocations will further call
    //   GlGeomTorus::CalcVboAndEbo()

    GlGeomBase::InitializeAttribLocations(pos_loc, normal_loc, texcoords_loc);
    VboEboLoaded = true;
}


// **********************************************
// These routines do the rendering.
// If the torus's VAO, VBO, EBO need to be loaded, it does this first.
// **********************************************

void GlGeomTorus::PreRender()
{
    GlGeomBase::PreRender();

    if (!VboEboLoaded) {
        ReInitializeAttribLocations();
    }
}

// Render entire torus as triangles
void GlGeomTorus::Render()
{
    PreRender();
    GlGeomBase::Render();
}

// Render one ring as triangles
void GlGeomTorus::RenderRing(int i)
{
    assert(i >= 0 && i < numRings);
    PreRender();

    int numElementsPerRing = GetNumElementsPerRing();
    GlGeomBase::RenderEBO(GL_TRIANGLES, numElementsPerRing, i*numElementsPerRing);
}

// Render one strip of sides as a triangle strip
//   Rebuilds an EBO every time it is called.
void GlGeomTorus::RenderSideStrip(int j)
{
    assert(j >= 0 && j < numSides);
    PreRender();

    // Create the EBO (element buffer data) for the i-th side (wedge) as a triangle strip.
    int numElts = 2 * (numRings + 1);
    unsigned int* sideElts = new unsigned int[numElts];
    int numEltsPerRing = UseTexCoords() ? numSides + 1 : numSides;
    int delta = UseTexCoords() ? 1 : (((j+1)%numRings) - j);
    unsigned int* toElt = sideElts;
    for (int i = 0; i <= numRings; i++) {
        int ii = UseTexCoords() ? i : (i%numRings);
        int eltA = ii * numEltsPerRing + j;
        *(toElt++) = eltA+delta;
        *(toElt++) = eltA;
    }

    // Render the triangle strip
    GlGeomBase::RenderElements(GL_TRIANGLE_STRIP, numElts, sideElts);
    delete[] sideElts;
}


