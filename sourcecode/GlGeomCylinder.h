/*
 * GlGeomCylinder.h - Version 1.1 - November 13, 2020
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

#ifndef GLGEOM_CYLINDER_H
#define GLGEOM_CYLINDER_H

#include "GlGeomBase.h"
#include <limits.h>

// GlGeomCylinder
//     Generates vertices, normals, and texture coodinates for a cylinder.
//     Cylinder formed of "slices" and "stacks" and "rings"
//     Cylinder has radius 1, height 2 and is centered at the origin.
//     The central axis is the y-axis. Texture coord (0.5,0.5) is on the z-axis.
// Supports:
//    (1) Allocating and loading a VAO, VBO, and EBO
//    (2) Rendering the cylinder with OpenGL.
// How to use:
//          First call either the constructor or Remesh() to set the numbers 
//          of slices, stacks and rings.
//          These values can be changed later by calling Remesh()
//    * Then call InitializeAttribLocations() to
//            gives locations in the VBO buffer for the shader program.
//            It loads all the vertex data into the VBO and EBO.
//    * Call Render() - to render the cylinder.  This gives the glDrawElements 
//            commands for the cylinder using the VAO, VBO and EBO.


class GlGeomCylinder : public GlGeomBase
{
public:
    GlGeomCylinder() : GlGeomCylinder(3, 1, 1) {}
    GlGeomCylinder(int slices, int stacks=1, int rings=1);

	// Remesh() - Re-mesh to change the number slices and stacks and rings.
    // Can be called either before or after InitializeAttribLocations(), but it is
    //    more efficient if Remesh() is called first, or if the constructor sets the mesh resolution.
    void Remesh(int slices, int stacks, int rings);

	// Allocate the VAO, VBO, and EBO.
	// Set up info about the Vertex Attribute Locations
	// This must be called before Render() is first called.
    // First parameter is the location for the vertex position vector in the shader program.
    // Second parameter is the location for the vertex normal vector in the shader program.
    // Third parameter is the location for the vertex 2D texture coordinates in the shader program.
    // The second and third parameters are optional.
    void InitializeAttribLocations(
		unsigned int pos_loc, unsigned int normal_loc = UINT_MAX, unsigned int texcoords_loc = UINT_MAX);

    void Render();          // Render: renders entire cylinder
    void RenderTop();
    void RenderBase();
    void RenderSide();

    int GetNumSlices() const { return numSlices; }
    int GetNumStacks() const { return numStacks; }
    int GetNumRings() const { return numRings; }
    
    // Use GetNumElements() and GetNumVerticesTexCoords() and GetNumVerticesNoTexCoords()
    //    to determine the amount of data that will returned by CalcVboAndEbo.
    //    Numbers are different since texture coordinates must be assigned differently
    //        to some vertices depending on which triangle they appear in.
    int GetNumElements() const { return 2 * GetNumElementsDisk() + GetNumElementsSide(); }
    int GetNumVerticesTexCoords() const { return 2 * GetNumVerticesDisk() + GetNumVerticesSideTexCoords(); }
    int GetNumVerticesNoTexCoords() const { return 2 * GetNumVerticesDisk() + GetNumVerticesSideNoTexCoords(); }

    // "Disk" methods are for the bottom or top circular face.  "Side" for the cylinder's side
    int GetNumElementsDisk() const { return 3 * (2 * numRings - 1)*numSlices; }
    int GetNumVerticesDisk() const { return 1 + numRings * numSlices; }
    int GetNumElementsSide() const { return 6 * numStacks*numSlices; }
    int GetNumVerticesSideTexCoords() const { return (numStacks + 1)*(numSlices + 1); }
    int GetNumVerticesSideNoTexCoords() const { return (numStacks + 1)*numSlices; }

    // CalcVboAndEbo- return all VBO vertex information, and EBO elements for GL_TRIANGLES drawing.
    // See GlGeomBase.h for additional information
    void CalcVboAndEbo(float* VBOdataBuffer, unsigned int* EBOdataBuffer,
        int vertPosOffset, int vertNormalOffset, int vertTexCoordsOffset,
        unsigned int stride);

private: 

    // Disable all copy and assignment operators.
    // A GlGeomCylinder can be allocated as a global or static variable, or with new.
    //     If you need to pass it to/from a function, use references or pointers
    //     and be sure that there are no implicit copy or assignment operations!
    GlGeomCylinder(const GlGeomCylinder&) = delete;
    GlGeomCylinder& operator=(const GlGeomCylinder&) = delete;
    GlGeomCylinder(GlGeomCylinder&&) = delete;
    GlGeomCylinder& operator=(GlGeomCylinder&&) = delete;


private:
    int numSlices;          // Number of radial slices (like cake slices
    int numStacks;          // Number of stacks between the two end faces
    int numRings;           // Number of concentric rings on two end faces


private: 
    bool VboEboLoaded = false;

    void PreRender();

    void SetDiscVerts(float x, float z, int i, int j, float* VBOdataBuffer,
        int vertPosOffset, int vertNormalOffset, int vertTexCoordsOffset, int stride);
 };

// Constructor
inline GlGeomCylinder::GlGeomCylinder(int slices, int stacks, int rings)
{
	numSlices = slices;
	numStacks = stacks;
    numRings = rings;
}

#endif  // GLGEOM_CYLINDER_H
