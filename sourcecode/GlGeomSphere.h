/*
* GlGeomSphere.h - Version 1.1 - November 13, 2020
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

#pragma once
#ifndef GLGEOM_SPHERE_H
#define GLGEOM_SPHERE_H

#include "GlGeomBase.h"

// GlGeomSphere
//     Generates vertices, normals, and texture coordinates for a sphere.
//     Sphere formed of "slices" and "stacks"
//     "Slices" means the number of vertical wedges.
//     "Stacks" means the number of horizontal pieces.
// Supports:
//    (1) Allocating and loading a VAO, VBO, and EBO
//    (2) Rendering sphere with OpenGL.
//    (3) Specialty routines for rendering a single slice or single wedge
// How to use:
//     * First call either the constructor GlGeomSphere() or ReMesh()
//             to set the numbers of slices and stacks.
//             These numbers can be changed by calling ReMesh().
//     * Then call InitializeAttribLocations() to specify whether to use
//          normals and texture coordinates and to specify
//          the locations in the VBO buffer for the shader program.
//          The also allocates and loads the VAO, VBO and EBO.
//     * Call Render() to render the sphere. Render() issues the 
//          the glDrawElements commands for the sphere using the VAO, VBO and EBO.

class GlGeomSphere : public GlGeomBase
{
public:
    GlGeomSphere() : GlGeomSphere(6, 6) {}
    GlGeomSphere(int slices, int stacks);
 
    // Remesh: re-mesh to change the number slices and stacks.
    // Can be called either before or after InitializeAttribLocations(), but it is
    //    more efficient if Remesh() is called first, or if the constructor sets the mesh resolution.
    void Remesh(int slices, int stacks);

    // Allocate the VAO, VBO, and EBO.
    // Set up info about the Vertex Attribute Locations
    // This must be called before render is first called.
    // First parameter is the location for the vertex position vector in the shader program.
    // Second parameter is the location for the vertex normal vector in the shader program.
    // Third parameter is the location for the vertex 2D texture coordinates in the shader program.
    // The second and third parameters are optional.
    void InitializeAttribLocations(
        unsigned int pos_loc, unsigned int normal_loc = UINT_MAX, unsigned int texcoords_loc = UINT_MAX);

    // Render the sphere.  Must call InitializeAttribLocations first.
    void Render();

    // Some specialized render routines for rendering portions of the sphere
    // Selectively render a slice or a stack or a north pole triangle fan
    // Slice numbers i rangle from 0 to numSlices-1.
    // Stack numbers j are allowed to range from 1 to numStacks-2.
    void RenderSlice(int i);    // Renders the i-th slice as triangles
    void RenderStack(int j);    // Renders the j-th stack as a triangle strip
    void RenderNorthPoleFan();  // Renders the north pole stack as a triangle fan.

    int GetNumSlices() const { return numSlices; }
    int GetNumStacks() const { return numStacks; }

    // Use GetNumElements() and GetNumVerticesTexCoords() and GetNumVerticesNoTexCoords()
    //    to determine the amount of data that will returned by CalcVboAndEbo.
    //    Numbers are different since texture coordinates must be assigned differently
    //        to some vertices depending on which triangle they appear in.
    int GetNumElements() const { return 6 * numSlices*(numStacks - 1); }
    int GetNumVerticesTexCoords() const { return (numSlices + 1)*(numStacks - 1) + 2; }
    int GetNumVerticesNoTexCoords() const { return numSlices * (numStacks - 1) + 2; }

    int GetNumElementsInSlice() const { return 6 * (numStacks - 1); }
    int GetNumTrianglesInSlice() const { return 2 * (numStacks - 1); }
    int GetNumTrianglesInStack() const { return 2 * numSlices; }
    int GetNumTriangles() const { return 2 * numSlices*(numStacks - 1); }

    // CalcVboAndEbo- return all VBO vertex information, and EBO elements for GL_TRIANGLES drawing.
    // See GlGeomBase.h for additional information
    void CalcVboAndEbo(float* VBOdataBuffer, unsigned int* EBOdataBuffer,
        int vertPosOffset, int vertNormalOffset, int vertTexCoordsOffset,
        unsigned int stride);

private:

	// Disable all copy and assignment operators.
	// A GlGeomSphere can be allocated as a global or static variable, or with new.
    //     If you need to pass it to/from a function, use references or pointers
    //     and be sure that there are no implicit copy or assignment operations!
    GlGeomSphere(const GlGeomSphere&) = delete;
	GlGeomSphere& operator=(const GlGeomSphere&) = delete;
	GlGeomSphere(GlGeomSphere&&) = delete;
	GlGeomSphere& operator=(GlGeomSphere&&) = delete;

private:
    int numSlices;              // Number of radial slices
    int numStacks;              // Number of levels separating the north pole from the south pole.

    bool VboEboLoaded = false;

private:
    bool GetVertexNumber(int i, int j, bool calcTexCoords, unsigned int* retVertNum);
    void PreRender();
};

// Constructor
inline GlGeomSphere::GlGeomSphere(int slices, int stacks)
{
	numSlices = slices;
	numStacks = stacks;
}

#endif  // GLGEOM_SPHERE_H
