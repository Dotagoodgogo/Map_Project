/*
* GlGeomSphere.cpp - Version 1.1 - November 13, 2020
*
* C++ class for rendering spheres in Modern OpenGL.
*   A GlGeomSphere object encapsulates a VAO, VBO, and VEO,
*   which can be used to render a sphere.
*   The number of slices and stacks can be varied.
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

#include "LinearR3.h"
#include "MathMisc.h"
#include "assert.h"

#include "GlGeomSphere.h"

void GlGeomSphere::Remesh(int slices, int stacks)
{
    if (slices == numSlices && stacks == numStacks) {
        return;
    }

    numSlices = ClampRange(slices, 3, 255);
    numStacks = ClampRange(stacks, 3, 255);

    VboEboLoaded = false;
}

// Create the VBO and EBO data for the sphere.
// See GlGeomBase.h for more information.
// This routine could be adapted for stand-alone use, as is.
void GlGeomSphere::CalcVboAndEbo(float* VBOdataBuffer, unsigned int* EBOdataBuffer,
    int vertPosOffset, int vertNormalOffset, int vertTexCoordsOffset, unsigned int stride)
{
    assert(vertPosOffset >= 0 && stride>0);
    bool calcNormals = (vertNormalOffset >= 0);       // Should normals be calculated?
    bool calcTexCoords = (vertTexCoordsOffset >= 0);  // Should texture coordinates be calculated?

     for (int i = 0; i <= numSlices; i++) {
        // Handle a slice of vertices.
        // theta measures from the (negative-z)-axis, going counterclockwise viewed from above.
        float theta = ((float)(i%numSlices))*(float)PI2 / (float)(numSlices);
        float sTexCd = ((float)i) / (float)numSlices;     // s texture coordinate
        float costheta = cos(theta);
        float sintheta = sinf(theta);
        for (int j = 0; j <= numStacks; j++) {
            unsigned int vertNumber;
            if (!GetVertexNumber(i, j, calcTexCoords, &vertNumber)) {
                continue;       // North or South pole -- duplicate not needed
            }
            // phi measures from the (postive-y)-axis
            float tTexCd = ((float)j) / (float)(numStacks); // t texture coordinate
            float phi = tTexCd * (float)PI;
            float cosphi = cosf(phi);
            float sinphi = (j < numStacks) ? sinf(phi) : 0.0f;
            float x = -sintheta*sinphi;       // Position, x coordinate            
            float y = -cosphi;                // Position, y coordinate
            float z = -costheta*sinphi;       // Position, z coordinate
            float* basePtr = VBOdataBuffer + stride*vertNumber;
            float* vPtr = basePtr + vertPosOffset;
            *vPtr = x;
            *(vPtr + 1) = y;
            *(vPtr + 2) = z;
            if (calcNormals) {
                float* nPtr = basePtr + vertNormalOffset;
                *nPtr = x;
                *(nPtr + 1) = y;
                *(nPtr + 2) = z;
            }
            if (calcTexCoords) {
                float* tcPtr = basePtr + vertTexCoordsOffset;
                *tcPtr = (j != 0 && j != numStacks) ? sTexCd : 0.5f;  // s=0.5 at the poles
                *(tcPtr + 1) = tTexCd;
            }
        }
     }
     
     // Calculate elements (vertex indices) suitable for putting into an EBO
     //      in GL_TRIANGLES mode.
     unsigned int* toEbo = EBOdataBuffer;
     for (int i = 0; i < numSlices; i++) {
         // Handle a slice of vertices.
         unsigned int leftIdxOld, rightIdxOld;
         GetVertexNumber(i, 0, calcTexCoords, &leftIdxOld);
         GetVertexNumber(i + 1, 1, calcTexCoords, &rightIdxOld);
         for (int j = 0; j < numStacks-1; j++) {
             unsigned int leftIdxNew, rightIdxNew;
             GetVertexNumber(i, j + 1, calcTexCoords, &leftIdxNew);
             GetVertexNumber(i + 1, j + 2, calcTexCoords, &rightIdxNew);
             *(toEbo++) = leftIdxOld;
             *(toEbo++) = rightIdxOld;
             *(toEbo++) = leftIdxNew;

             *(toEbo++) = leftIdxNew;
             *(toEbo++) = rightIdxOld;
             *(toEbo++) = rightIdxNew;

             leftIdxOld = leftIdxNew;
             rightIdxOld = rightIdxNew;
         }
     }
     assert(toEbo - EBOdataBuffer == GetNumElements());
}

// Calculate the vertex number for the vertex on slice i and stack j.
// Returns false if this is a duplicate of the south or north pole.
bool GlGeomSphere::GetVertexNumber(int i, int j, bool calcTexCoords, unsigned int* retVertNum)
{
    if (j == 0) {
        *retVertNum = 0;    // South pole
        return (i == 0);
    }
    if (j == numStacks) {
        *retVertNum = 1;    // North pole
        return (i == 0);
    }
    int ii = calcTexCoords ? i : (i%numSlices);
    *retVertNum = (numStacks - 1)*ii + j + 1;
    return true;
}


void GlGeomSphere::InitializeAttribLocations(
	unsigned int pos_loc, unsigned int normal_loc, unsigned int texcoords_loc)
{
    // The call to GlGeomBase::InitializeAttribLocations will further call
    //   GlGeomSphere::CalcVboAndEbo()

    GlGeomBase::InitializeAttribLocations(pos_loc, normal_loc, texcoords_loc);
    VboEboLoaded = true;
}

void GlGeomSphere::PreRender() {
    GlGeomBase::PreRender();
    if (!VboEboLoaded) {
        ReInitializeAttribLocations();
    }
}

// **********************************************
// This routine does the rendering.
// If the sphere's VBO and EBO data need to be calculated, it does this first.
// **********************************************
void GlGeomSphere::Render()
{
    PreRender();
    GlGeomBase::Render();
}

// **********************************************
// This routine renders the i-th slice.
// If the sphere's VBO and EBO data need to be calculated, it does this first.
// **********************************************
void GlGeomSphere::RenderSlice(int i)
{
    assert(i >= 0 && i < numSlices);
    PreRender();

    int sliceLen = GetNumElementsInSlice();
    GlGeomBase::RenderEBO(GL_TRIANGLES, sliceLen, i*sliceLen);
}

// **********************************************
// This routine renders a single horizontal stack as a triangle strip.
// If the sphere's VBO and EBO data need to be calculated, it does this first.
//   Not efficient for reuse: Recalculates the EBO data every time.
//   Generates a new EBO everytime
//  j can range from 0 to numStacks. At the two extremes the bottom and top
//     fans are rendered as triangle strips with degenerate triangles.
// **********************************************
void GlGeomSphere::RenderStack(int j)
{
    assert(j >= 0 && j < numStacks);
    PreRender();

    // Create the EBO (element buffer data) for the i-th slice as a triangle strip.
    unsigned int* stackElts = new unsigned int[(numSlices + 1) * 2];
    unsigned int* toElt = stackElts;
    for (int i = 0; i <= numSlices; i++) {
        GetVertexNumber(i, j+1, UseTexCoords(), toElt++);
        GetVertexNumber(i, j, UseTexCoords(), toElt++);
    }

    // Render the triangle strip
    GlGeomBase::RenderElements(GL_TRIANGLE_STRIP, (numSlices + 1) * 2, stackElts);
    delete[] stackElts;

}

// **********************************************
// This routine renders the triangle fan around the North Pole.
// If the sphere's VBO and EBO data need to be calculated, it does this first.
//   Not efficient for reuse: Recalculates the EBO data every time.
//   Generates a new EBO everytime
// **********************************************
void GlGeomSphere::RenderNorthPoleFan() {
    PreRender();

    // Create the EBO (element buffer data) for the north pole as a triangle fan
    unsigned int* poleElts = new unsigned int[numSlices + 2];
    unsigned int* toElt = poleElts;
    GetVertexNumber( 0, numStacks, UseTexCoords(), toElt++ ); // North pole is the center of the triangle fan
    for (int i = 0; i <= numSlices; i++) {
        GetVertexNumber(i, numStacks - 1, UseTexCoords(), toElt++);
    }

    // Render the triangle fan
    GlGeomBase::RenderElements(GL_TRIANGLE_FAN, numSlices + 2, poleElts);
}






