/*
* GlGeomBase.cpp - Version 1.1 - November 13, 2020
*
* C++ class base class for GlGeomShape classes;
*       for rendering pre-designed geometric objects in Modern OpenGL.
*   A GlGeomBase object is pure virtual, and is a base class for
*      objects such as GlGeomSphere, GlGeomCylinder, GlGeomTorus,
*      GlGeomBezier, GlGeomTeapot, etc.
*   The GlGeomBase class handles all the interfaces with OpenGL
*      It encapsulates a VAO, VBO, and VEO,
*      And issues rendering commands to render triangles.
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


#include "GlGeomBase.h"
#include "assert.h"

// Use the static library (so glew32.dll is not needed):
#define GLEW_STATIC
#include <GL/glew.h> 
#include <GLFW/glfw3.h>

void GlGeomBase::ReInitializeAttribLocations()
{
    InitializeAttribLocations(posLoc, normalLoc, texcoordsLoc);
}

void GlGeomBase::InitializeAttribLocations(
    unsigned int pos_loc, unsigned int normal_loc, unsigned int texcoords_loc)
{
    posLoc = pos_loc;
    normalLoc = normal_loc;
    texcoordsLoc = texcoords_loc;

    // Generate Vertex Array Object and Buffer Objects, not already done.
    if (theVAO == 0) {
        glGenVertexArrays(1, &theVAO);
        glGenBuffers(1, &theVBO);
        glGenBuffers(1, &theEBO);
    }

    // Link the VBO and EBO to the VAO, and request OpenGL to
    //   allocate memory for them.
    glBindVertexArray(theVAO);
    glBindBuffer(GL_ARRAY_BUFFER, theVBO);
    int numVertices = UseTexCoords() ? GetNumVerticesTexCoords() : GetNumVerticesNoTexCoords();
    glBufferData(GL_ARRAY_BUFFER, StrideVal() * numVertices * sizeof(float), 0, GL_STATIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, theEBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, GetNumElementsMax() * sizeof(unsigned int), 0, GL_STATIC_DRAW);
    glVertexAttribPointer(posLoc, 3, GL_FLOAT, GL_FALSE, StrideVal() * sizeof(float), (void*)0);
    glEnableVertexAttribArray(posLoc);
    if (UseNormals()) {
        glVertexAttribPointer(normalLoc, 3, GL_FLOAT, GL_FALSE, StrideVal() * sizeof(float),
            (void*)(NormalOffset() * sizeof(float)));
        glEnableVertexAttribArray(normalLoc);
    }
    if (UseTexCoords()) {
         glVertexAttribPointer(texcoordsLoc, 2, GL_FLOAT, GL_FALSE, StrideVal() * sizeof(float),
            (void*)(TexOffset() * sizeof(float)));
        glEnableVertexAttribArray(texcoordsLoc);
    }

    CalcVBOandEBO_Base();
}

// Load the data into the VBO and EBO arrays.
// This invokes the appropriate CalVBOandEBO method
void GlGeomBase::CalcVBOandEBO_Base() {

	// Calculate the buffer data - map and the unmap the two buffers.
    glBindVertexArray(theVAO);
    glBindBuffer(GL_ARRAY_BUFFER, theVBO);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, theEBO);
    float* VBOdata = (float*)glMapBuffer(GL_ARRAY_BUFFER, GL_WRITE_ONLY);
    unsigned int* EBOdata = (unsigned int*)glMapBuffer(GL_ELEMENT_ARRAY_BUFFER, GL_WRITE_ONLY);
    int normalOffset = UseNormals() ? NormalOffset() : -1;
    int tcOffset = UseTexCoords() ? TexOffset() : -1;
    CalcVboAndEbo(VBOdata, EBOdata, 0, normalOffset, tcOffset, StrideVal());
    glUnmapBuffer(GL_ARRAY_BUFFER);
    glUnmapBuffer(GL_ELEMENT_ARRAY_BUFFER);
 
    // Good practice to unbind things: helps with debugging if nothing else
    glBindVertexArray(0); 
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}

void GlGeomBase::PreRender() {
    if (theVAO == 0) {
        assert(false && "InitializeAttribLocations must be called before rendering!");
    }
 }

// **********************************************
// This routine does the rendering.
//     The entire object is rendered (as loaded in the EBO).
// **********************************************
void GlGeomBase::Render()
{
    RenderEBO(GL_TRIANGLES, GetNumElementsRender(), 0);
}

// **********************************************
// This routine does the rendering of the specified EBO data
// The EBO has already been bound to the VAO.
// **********************************************
void GlGeomBase::RenderEBO(unsigned int drawMode, int numRenderElements, int EBOstart)
{
    if (theVAO == 0) {
        assert(false && "InitializeAttribLocations must be called before rendering!");
    }
    glBindVertexArray(theVAO);
    glDrawElements(drawMode, (GLsizei)numRenderElements, GL_UNSIGNED_INT, (void*)(EBOstart * sizeof(unsigned int)));
    glBindVertexArray(0);           // Good practice to unbind: helps with debugging if nothing else
}

// **********************************************
// This routine does the rendering of the specified elements
//    A temporary EBO is created for this purpose
//    (For this reason it is not really efficient for repeated use.)
// **********************************************
void GlGeomBase::RenderElements(unsigned int drawMode, int numRenderElements, const unsigned int *elementsData)
{
    unsigned int tempEBO;
    glGenBuffers(1, &tempEBO);
    glBindVertexArray(theVAO);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, tempEBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, numRenderElements * sizeof(unsigned int), elementsData, GL_STATIC_DRAW);

    glDrawElements(drawMode, numRenderElements, GL_UNSIGNED_INT, 0);
    
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, theEBO);  // Restore the main EBO (The VAO maintains its knowledge of this)
    glDeleteBuffers(1, &tempEBO);
    glBindVertexArray(0);

}

GlGeomBase::~GlGeomBase()
{
    glDeleteBuffers(3, &theVAO);  // The three buffer id's are contigous in memory!
}


