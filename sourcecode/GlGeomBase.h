/*
* GlGeomBase.h - Version 1.1 - November 13, 2020
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

#pragma once
#ifndef GLGEOM_BASE_H
#define GLGEOM_BASE_H

#include <limits.h>
#include <assert.h>

// GlGeomBase
//     Handles all the OpenGL rendering for the GlGeomShape classes.
// Supports the following:
//    (1) Allocating a VAO, VBO, and EBO
//    (2) Doing the rendering with OpenGL

class GlGeomBase
{
public:
    GlGeomBase() {}
    ~GlGeomBase();

    // Disable all copy and assignment operators for a GlGeomBase object.
    //     If you need to pass it to/from a function, use references or pointers
    //     and be sure that there are no implicit copy or assignment operations!
    GlGeomBase(const GlGeomBase&) = delete;
    GlGeomBase& operator=(const GlGeomBase&) = delete;
    GlGeomBase(GlGeomBase&&) = delete;
    GlGeomBase& operator=(GlGeomBase&&) = delete;

    // These must be implemented in each GlGeomShape class.
    //   GetNumElements() returns the number of elements in the EBO for rendering
    //   Alternately, GetNumElementsMax() and GetNumElementsRender() can be defined.
    //   GetNumElementsMax() returns an upper bound on the number of elements in the EBO (for allocation)
    //   GetNumElementsRender() returns the actual number of elements in the EBO (for rendering)
    //   GetNumVerticesTexCoords() returns the number of vertices when there are texture coordinates
    //   GetNumVerticesNoTexCoords() returns the number of vertices when no texture coordinates
    virtual int GetNumElements() const { assert(false); return -1; };
    virtual int GetNumElementsMax() const { return GetNumElements(); }
    virtual int GetNumElementsRender() const { return GetNumElements(); }
    virtual int GetNumVerticesTexCoords() const = 0;
    virtual int GetNumVerticesNoTexCoords() const = 0;

    unsigned int GetVAO() const { return theVAO; }
    unsigned int GetVBO() const { return theVBO; }
    unsigned int GetEBO() const { return theEBO; }

    // The routine CalcVboAndEbo must be implemented for all GlGeomShape classes, 
    //    but is meant for internal use, and is not usually called by the user.
    // It is called from the constructor or a ReMesh() or Render() method
    //         via a call to InitializeAttribLocations. It takes as input:
    //    * Pointers to the VBO buffer and EBO buffer. Typically these
    //      are allocated earlier by InitializeAttribLocations
    //    * Layout of data in the VBO:  offsets for the vertex position,
    //          the normal and the texture coordinates (if used),
    //          and the stride value.
    // Inputs:
    //   VBOdataBuffer - pointer to the VBO buffer (mapped to memory)
    //   EBOdataBuffer - pointer to the EBO buffer (mapped to memory)
    //       - The VBO and EBO buffersare filled with the vertex info and elements for GL_TRIANGLES drawing
    //   vertPosOffset and stride control where the vertex positions are placed.
    //   vertNormalOffset and stride control where the vertex normals are placed.
    //   vertTexCoordsOffset and stride control where the texture coordinates are placed.
    //   Offset and stride values are **integers** (not bytes), measuring offsets in terms of floats.
    //   Use "-1" for the offset for any value which should be omitted.
    //   For the (unit) sphere, the normals are always exactly equal to the positions.
    // Output: 
    //   Data VBO and EBO data is calculated and loaded into the two buffers VBOdataBuffer and EBOdataBuffer.
    // Typical usages are:
    //   CalcVboAndEbo( vboPtr, eboPtr, 0, -1, -1, 3); // positions only, tightly packed
    //   CalcVboAndEbo( vboPtr, eboPtr, 0, -1, 3, 5); // positions, then (s,t) texture coords, tightly packed
    //   CalcVboAndEbo( vboPtr, eboPtr, 0, 3, 6, 8); // positions, normals, then (s,t) texture coords, tightly packed
    virtual void CalcVboAndEbo(float* VBOdataBuffer, unsigned int* EBOdataBuffer,
            int vertPosOffset, int vertNormalOffset, int vertTexCoordsOffset,
            unsigned int stride) = 0;

protected:
    // Allocate the VAO, VBO, and EBO.
    // Set up info about the Vertex Attribute Locations
    // This must be called before render is first called.
    // First parameter is the location for the vertex position vector in the shader program.
    // Second parameter is the location for the vertex normal vector in the shader program.
    // Third parameter is the location for the vertex 2D texture coordinates in the shader program.
    // The second and third parameters are optional.
    virtual void InitializeAttribLocations(
        unsigned int pos_loc, unsigned int normal_loc = UINT_MAX, unsigned int texcoords_loc = UINT_MAX);
    void ReInitializeAttribLocations();
    void CalcVBOandEBO_Base();

    void PreRender();
    void Render(); 
    void RenderElements(unsigned int drawMode, int numRenderElements, const unsigned int *elementsData);
    void RenderEBO(unsigned int drawMode, int numRenderElements, int EBOstart);

private:
    unsigned int theVAO = 0;        // Vertex Array Object
    unsigned int theVBO = 0;        // Vertex Buffer Object
    unsigned int theEBO = 0;        // Element Buffer Object;

    unsigned int posLoc;            // location of vertex position x,y,z data in the shader program
    unsigned int normalLoc;         // location of vertex normal data in the shader program
    unsigned int texcoordsLoc;      // location of s,t texture coordinates in the shader program.

public:
    // Stride value, and offset values for the data in the VBO
    // These take into account whether normals and texture coordinates are used.
    bool UseNormals() const { return normalLoc != UINT_MAX; }
    bool UseTexCoords() const { return texcoordsLoc != UINT_MAX; }
    int StrideVal() const {
        return 3 + (UseNormals() ? 3 : 0) + (UseTexCoords() ? 2 : 0);
    }
    int NormalOffset() const { return 3; }
    int TexOffset() const { return 3 + (UseNormals() ? 3 : 0); }
};

#endif  // GLGEOM_BASE_H
