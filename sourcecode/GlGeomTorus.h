/*
 * GlGeomTorus.h - Version 1.1 - November 13, 2020
 *
 * C++ class for rendering tori in Modern OpenGL.
 *   A GlGeomTorus object encapsulates a VAO, a VBO, and an EBO,
 *   which can be used to render a torus.
 *   The number of rings and sides can be varied.
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

#ifndef GLGEOM_TORUS_H
#define GLGEOM_TORUS_H

#include "GlGeomBase.h"
#include <limits.h>

// GlGeomTorus
//     Generates vertices, normals, and texture coodinates for a torus.
//     Torus is formed of "rings" and "sides"
//     Torus is centered at the origin, symmetrically around the y-axis.
//     The central axis is the y-axis. Texture coord (0.5,0.5) is on the z-axis.
// Supports:
//    (1) Allocating and loading a VAO, VBO, and EBO
//    (2) Rendering the torus with OpenGL.
// How to use:
//          First call either the constructor or Remesh() to set the numbers 
//          of sides and rings.
//          These values can be changed later by calling Remesh()
//    * Then call InitializeAttribLocations() to
//            gives locations in the VBO buffer for the shader program.
//            It loads all the vertex data into the VBO and EBO.
//    * Call Render() - to render the torus.  This gives the glDrawElements 
//            commands for the torus using the VAO, VBO and EBO.
// The ratio of the major and minor radii can be veried by user specification.
//         Value minorRatio is the radius of the circular tube.
//         Major radius is fixed at 1.0.
//         Minor radius should be less than 1.0.
// The number of rings = number of cuts at right angles to the inner circular path.
//         E.g. To share a doughtnut four ways, you would want to cut four rings.
// The number of sides = number of wedges around the inner circular path.


class GlGeomTorus : public GlGeomBase
{
public:
    GlGeomTorus() : GlGeomTorus(8, 8) {}
    GlGeomTorus(int rings, int sides, float minorRadius= 0.5);

	// Remesh(): Re-mesh to change the number of sides and rings.
    // Can be called either before or after InitAttribLocations(), but it is
    //    more efficient if Remesh() is called first, or if the constructor sets the mesh resolution.
    void Remesh(int rings, int sides) { Remesh(rings, sides, radius); }
    void Remesh(int rings, int sides, float minorRadius);

	// Allocate the VAO, VBO, and EBO.
	// Set up info about the Vertex Attribute Locations
	// This must be called before Render() is first called.
    // First parameter is the location for the vertex position vector in the shader program.
    // Second parameter is the location for the vertex normal vector in the shader program.
    // Third parameter is the location for the vertex 2D texture coordinates in the shader program.
    // The second and third parameters are optional.
    void InitializeAttribLocations(
		unsigned int pos_loc, unsigned int normal_loc = UINT_MAX, unsigned int texcoords_loc = UINT_MAX);

    void Render();          // Render(): renders entire torus

    // Some specialized render routines for rendering portions of the torus
    // Selectively render a ring or a strip of sides
    // Ring numbers i rangle from 0 to numRings-1.
    // Stack numbers j are allowed to range from 1 to numStacks-2.
    void RenderRing(int i);         // Renders the i-th ring as triangles
    void RenderSideStrip(int j);    // Renders the j-th side-strip as a triangle strip

    int GetNumSides() const { return numSides; }
    int GetNumRings() const { return numRings; }
    float GetMinorRadius() const { return radius; }
    float GetMajorRadius() const { return 1.0; }

    // Use GetNumElements() and GetNumVerticesTexCoords() and GetNumVerticesNoTexCoords()
    //    to determine the amount of data that will returned by CalcVboAndEbo.
    //    Numbers are different since texture coordinates must be assigned differently
    //        to some vertices depending on which triangle they appear in.
    int GetNumElements() const { return 6 * numRings * numSides; }
    int GetNumVerticesNoTexCoords() const { return numRings * numSides; }
    int GetNumVerticesTexCoords() const { return (numRings + 1) * (numSides + 1); }

    int GetNumElementsPerRing() const { return numSides * 6; }

    // CalcVboAndEbo- return all VBO vertex information, and EBO elements for GL_TRIANGLES drawing.
    // See GlGeomBase.h for additional information
    void CalcVboAndEbo(float* VBOdataBuffer, unsigned int* EBOdataBuffer,
        int vertPosOffset, int vertNormalOffset, int vertTexCoordsOffset,
        unsigned int stride);
 
private:

    // Disable all copy and assignment operators.
	// A GlGeomTorus can be allocated as a global or static variable, or with new.
	//     If you need to pass it to/from a function, use references or pointers
    //     and be sure that there are no implicit copy or assignment operations!
    GlGeomTorus(const GlGeomTorus&) = delete;
    GlGeomTorus& operator=(const GlGeomTorus&) = delete;
    GlGeomTorus(GlGeomTorus&&) = delete;
    GlGeomTorus& operator=(GlGeomTorus&&) = delete;

private:
    int numSides;           // Number sides going around the inner circular path
    int numRings;           // Number of ring-like pieces (perpindicular to the inner path)
    float radius;           // Minor radius (major radius is fixed equal to 1.0).

private: 
    bool VboEboLoaded = false;

    void PreRender();
};

inline GlGeomTorus::GlGeomTorus(int rings, int sides, float minorRadius)
{
	numSides = sides;
    numRings = rings;
    radius = minorRadius;
}

#endif  // GLGEOM_TORUS_H
