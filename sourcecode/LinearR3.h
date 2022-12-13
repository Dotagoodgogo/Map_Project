/*
 *
 * LinearR3.h, release 4.2.  September 12, 2020.
 *
 * Author: Samuel R. Buss
 *
 * Software accompanying the book
 *		3D Computer Graphics: A Mathematical Introduction with OpenGL,
 *		by S. Buss, Cambridge University Press, 2003.
 *
 * Software is "as-is" and carries no warranty.  It may be used without
 *   restriction, but if you modify it, please change the filenames to
 *   prevent confusion between different versions.  Please acknowledge
 *   all use of the software in any publications or products based on it.
 *
 * Bug reports: Sam Buss, sbuss@ucsd.edu.
 * Web page: http://math.ucsd.edu/~sbuss/MathCG
 *
 */

//
// Linear Algebra Classes over R3
//
//    VectorR3: a real column vector of length 3.
//
//    LinearMapR3 - arbitrary linear map; 3x3 real matrix
//
//	  See LinearR3bis.h for AffineMapR3, RotationMapR3, and RigidMapR3  
//

#ifndef LINEAR_R3_H
#define LINEAR_R3_H

#include <math.h>
#include <assert.h>
#include <iostream>
#include "MathMisc.h"
using namespace std;

class VectorR3;				// Space Vector (length 3)
class VectorR4;				// Space Vector (length 4)

class LinearMapR3;			// Linear Map (3x3 Matrix)

// Most for internal use:
class Matrix3x3;
class Matrix3x4;

class Quaternion;

// **************************************
// VectorR3 class                       *
// * * * * * * * * * * * * * * * * * * **

class VectorR3 {

public:
	double x, y, z;		// The x & y & z coordinates.

	static const VectorR3 Zero;
	// Deprecated due to unsafeness of global initialization
	//static const VectorR3 UnitX;
	//static const VectorR3 UnitY;
	//static const VectorR3 UnitZ;
	//static const VectorR3 NegUnitX;
	//static const VectorR3 NegUnitY;
	//static const VectorR3 NegUnitZ;

public:
	VectorR3( ) : x(0.0), y(0.0), z(0.0) {}
	VectorR3( double xVal, double yVal, double zVal )
		: x(xVal), y(yVal), z(zVal) {}

	VectorR3& Set( const Quaternion& );	// Convert quat to rotation vector
	VectorR3& Set( double xx, double yy, double zz ) 
				{ x=xx; y=yy; z=zz; return *this; }
	VectorR3& SetFromHg( const VectorR4& );	// Convert homogeneous VectorR4 to VectorR3
	VectorR3& SetZero() { x=0.0; y=0.0; z=0.0;  return *this;}
	VectorR3& SetUnitX() { x=1.0; y=0.0; z=0.0;  return *this;}
	VectorR3& SetUnitY() { x=0.0; y=1.0; z=0.0;  return *this;}
	VectorR3& SetUnitZ() { x=0.0; y=0.0; z=1.0;  return *this;}
	VectorR3& SetNegUnitX() { x=-1.0; y=0.0; z=0.0;  return *this;}
	VectorR3& SetNegUnitY() { x=0.0; y=-1.0; z=0.0;  return *this;}
	VectorR3& SetNegUnitZ() { x=0.0; y=0.0; z=-1.0;  return *this;}
	VectorR3& Load( const double* v );
	VectorR3& Load( const float* v );
	void Dump( double* v ) const;
	void Dump( float* v ) const;

	inline double operator[]( int i ) const;

	VectorR3& operator= ( const VectorR3& v ) 
		{ x=v.x; y=v.y; z=v.z; return(*this);}
	VectorR3& operator+= ( const VectorR3& v ) 
		{ x+=v.x; y+=v.y; z+=v.z; return(*this); } 
	VectorR3& operator-= ( const VectorR3& v ) 
		{ x-=v.x; y-=v.y; z-=v.z; return(*this); }
	VectorR3& operator*= ( double m ) 
		{ x*=m; y*=m; z*=m; return(*this); }
	VectorR3& operator/= ( double m ) 
			{ double mInv = 1.0/m; 
			  x*=mInv; y*=mInv; z*=mInv; 
			  return(*this); }
	VectorR3 operator- () const { return ( VectorR3(-x, -y, -z) ); }
	VectorR3& operator*= (const VectorR3& v);	// Cross Product
	VectorR3& CrossProductLeft (const VectorR3& v);	// Cross Product on left
	VectorR3& ArrayProd(const VectorR3&);		// Component-wise product

	VectorR3& AddScaled( const VectorR3& u, double s );
	VectorR3& SubtractFrom( const VectorR3& u );	
	VectorR3& AddCrossProduct( const VectorR3& u, const VectorR3& v );

	bool IsZero() const { return ( x==0.0 && y==0.0 && z==0.0 ); }
	double Norm() const { return ( (double)sqrt( x*x + y*y + z*z ) ); }
	double NormSq() const { return ( x*x + y*y + z*z ); }
	double MaxAbs() const;   // The L1 norm (maximum absolute value)
	double Dist( const VectorR3& u ) const;	// Distance from u
	double DistSq( const VectorR3& u ) const;	// Distance from u squared
	VectorR3& Negate() { x = -x; y = -y; z = -z; return *this;}	
	VectorR3& Normalize () { *this /= Norm(); return *this;}	// No error checking
	inline VectorR3& MakeUnit();		// Normalize() with error checking
	inline VectorR3& ReNormalize();
	bool IsUnit( ) const
		{ double norm = Norm();
		  return ( 1.000001>=norm && norm>=0.999999 ); }
	bool IsUnit( double tolerance ) const
		{ double norm = Norm();
		  return ( 1.0+tolerance>=norm && norm>=1.0-tolerance ); }
	bool NearZero(double tolerance) const { return( MaxAbs()<=tolerance );}
							// tolerance should be non-negative
	inline bool operator==(const VectorR3& u) const { return (x==u.x && y==u.y && z==u.z); }
	inline bool operator!=(const VectorR3& u) const { return (x!=u.x || y!=u.y || z!=u.z); }

	double YaxisDistSq() const { return (x*x+z*z); }
	double YaxisDist() const { return sqrt(x*x+z*z); }

	VectorR3& Rotate( double theta, const VectorR3& u); // rotate around u.
	VectorR3& RotateUnitInDirection ( const VectorR3& dir);	// rotate in direction dir
	VectorR3& Rotate( const Quaternion& );	// Rotate according to quaternion
											// Defined in Quaternion.cpp

	friend ostream& operator<< ( ostream& os, const VectorR3& u );

};

inline VectorR3 operator+( const VectorR3& u, const VectorR3& v );
inline VectorR3 operator-( const VectorR3& u, const VectorR3& v ); 
inline VectorR3 operator*( const VectorR3& u, double m); 
inline VectorR3 operator*( double m, const VectorR3& u); 
inline VectorR3 operator/( const VectorR3& u, double m); 

inline double operator^ (const VectorR3& u, const VectorR3& v ); // Dot Product
inline double InnerProduct(const VectorR3& u, const VectorR3& v ) { return (u^v); }
inline VectorR3 operator* (const VectorR3& u, const VectorR3& v);	 // Cross Product
inline VectorR3 ArrayProd ( const VectorR3& u, const VectorR3& v );

inline double Mag(const VectorR3& u) { return u.Norm(); }
inline double Dist(const VectorR3& u, const VectorR3& v) { return u.Dist(v); }
inline double DistSq(const VectorR3& u, const VectorR3& v) { return u.DistSq(v); }
inline double NormalizeError (const VectorR3& u);
 
// Deprecated due to unsafeness of global initialization
//extern const VectorR3 UnitVecIR3;
//extern const VectorR3 UnitVecJR3;
//extern const VectorR3 UnitVecKR3;

inline VectorR3 ToVectorR3( const Quaternion& q ) 
								{return VectorR3().Set(q);}


// 
// Advanced vector and position functions (prototypes)
//

VectorR3 Interpolate( const VectorR3& start, const VectorR3& end, double a);

// *****************************************
// Matrix3x3 class                         *
// * * * * * * * * * * * * * * * * * * * * *

class Matrix3x3 {
public:

	double m11, m21, m31, m12, m22, m32, m13, m23, m33;	
									
	// Implements a 3x3 matrix: m_i_j - row-i and column-j entry

	// Deprecated due to unsafeness of global initialization
	//static const Matrix3x3 Identity;

public:
	inline Matrix3x3();
	inline Matrix3x3(const VectorR3&, const VectorR3&, const VectorR3&); // Sets by columns!
	inline Matrix3x3(double, double, double, double, double, double,
					 double, double, double );	// Sets by columns

	inline void SetIdentity ();		// Set to the identity map
	inline void Set ( const Matrix3x3& );	// Set to the matrix.
	inline void Set3x3 ( const Matrix3x4& );	// Set to the 3x3 part of the matrix.
	inline void Set( const VectorR3&, const VectorR3&, const VectorR3& );
	inline void Set( double, double, double,
					 double, double, double,
					 double, double, double );
	inline void SetByRows( double, double, double, double, double, double,
							double, double, double );
	inline void SetByRows( const VectorR3&, const VectorR3&, const VectorR3& );
	inline void LoadByRows( const double* );
	
	inline void SetColumn1 ( double, double, double );
	inline void SetColumn2 ( double, double, double );
	inline void SetColumn3 ( double, double, double );
	inline void SetColumn1 ( const VectorR3& );
	inline void SetColumn2 ( const VectorR3& );
	inline void SetColumn3 ( const VectorR3& );
	inline VectorR3 Column1() const;
	inline VectorR3 Column2() const;
	inline VectorR3 Column3() const;

	inline void SetRow1 ( double, double, double );
	inline void SetRow2 ( double, double, double );
	inline void SetRow3 ( double, double, double );
	inline void SetRow1 ( const VectorR3& );
	inline void SetRow2 ( const VectorR3& );
	inline void SetRow3 ( const VectorR3& );
	inline VectorR3 Row1() const;
	inline VectorR3 Row2() const;
	inline VectorR3 Row3() const;

	inline void SetDiagonal( double, double, double );
	inline void SetDiagonal( const VectorR3& );
	inline double Diagonal( int ) const;

	// Set this so that  (this)v  =  u*v   where * is vector cross product
	inline void SetCrossProductMatrix( const VectorR3& u );

	// Set this = u * v^T.
	inline void SetOuterProduct( const VectorR3& u, const VectorR3& v );

	inline void MakeTranspose();					// Transposes it.
	Matrix3x3& ReNormalize();
	VectorR3 Solve(const VectorR3&) const;	// Returns solution

	inline void Transform( VectorR3* ) const;
	inline void Transform( const VectorR3& src, VectorR3* dest) const;
	inline void TransformTranspose( VectorR3* ) const;
	inline void TransformTranspose( const VectorR3& src, VectorR3* dest) const;

	double Trace() const { return m11+m22+m33; }
	double SumSquaresNorm() const;		// Returns sum of squares of entries

protected:
	void OperatorTimesEquals( const Matrix3x3& ); // Internal use only
	void RightMultiplyByTranspose( const Matrix3x3& );	  // Internal use only. Set this = this * M^T
	void LeftMultiplyBy( const Matrix3x3& );	  // Internal use only. Set this = M * this
	void LeftMultiplyByTranspose( const Matrix3x3& );	  // Internal use only.  Set this = M^T * this
	void SetZero ();							  // Set to the zero map

};

inline VectorR3 operator* ( const Matrix3x3&, const VectorR3& );

ostream& operator<< ( ostream& os, const Matrix3x3& A );


// *****************************************
// LinearMapR3 class                       *
// * * * * * * * * * * * * * * * * * * * * *

class LinearMapR3 : public Matrix3x3 {

public:

	LinearMapR3();
	LinearMapR3( const VectorR3&, const VectorR3&, const VectorR3& );
	LinearMapR3( double, double, double, double, double, double,
					 double, double, double );		// Sets by columns
	LinearMapR3 ( const Matrix3x3& );

	void SetZero ();			// Set to the zero map
	inline void Negate();

	inline LinearMapR3& operator+= (const Matrix3x3& );
	inline LinearMapR3& operator-= (const Matrix3x3& );
	inline LinearMapR3& operator*= (double);
	inline LinearMapR3& operator/= (double);
	inline void SubtractFrom( const Matrix3x3& m);	// Sets this = (m - this).
	LinearMapR3& operator*= (const Matrix3x3& );	// Matrix product
	void RightMultiplyByTranspose( const Matrix3x3& M ) { Matrix3x3::RightMultiplyByTranspose(M); }
	void LeftMultiplyBy( const Matrix3x3& M ) { Matrix3x3::LeftMultiplyBy(M); }
	void LeftMultiplyByTranspose( const Matrix3x3& M ) { Matrix3x3::LeftMultiplyByTranspose(M); }

	inline LinearMapR3 Transpose() const;	// Returns the transpose
	double Determinant () const;			// Returns the determinant
	LinearMapR3 Inverse() const;			// Returns inverse
	LinearMapR3& Invert();					// Converts into inverse.
	VectorR3 Solve(const VectorR3&) const;	// Returns solution
	LinearMapR3 InverseSym() const;			// Get inverse of symmetric matrix
	LinearMapR3 InversePosDef() const;		// Get inverse of symmetric positive definite matrix
	void InverseSym( LinearMapR3* inverse ) const;	// Get inverse of symmetric matrix
	void InversePosDef( LinearMapR3* inverse ) const;	// Get inverse of symmetric positive-definite matrix
	LinearMapR3& InvertSym();		// Converts to inverse of symmetric matrix
	LinearMapR3& InvertPosDef();		// Converts to inverse of symmetric positive-definite matrix
	LinearMapR3& InvertPosDefSafe();		// Converts to inverse of symmetric positive-definite matrix
	LinearMapR3 PseudoInverse() const;	// Returns pseudo-inverse TO DO
	VectorR3 PseudoSolve(const VectorR3&);	// Finds least squares solution TO DO

};
	
inline LinearMapR3 operator+ (const LinearMapR3&, const LinearMapR3&);
inline LinearMapR3 operator+ (const LinearMapR3&, const Matrix3x3&);
inline LinearMapR3 operator+ (const Matrix3x3&, const LinearMapR3&);
inline LinearMapR3 operator- (const LinearMapR3&);
inline LinearMapR3 operator- (const LinearMapR3&, const LinearMapR3&);
inline LinearMapR3 operator- (const LinearMapR3&, const Matrix3x3&);
inline LinearMapR3 operator- (const Matrix3x3&, const LinearMapR3&);
inline LinearMapR3 operator* ( const LinearMapR3&, double);
inline LinearMapR3 operator* ( double, const LinearMapR3& );
inline LinearMapR3 operator/ ( const LinearMapR3&, double );
LinearMapR3 operator* ( const LinearMapR3&, const LinearMapR3& ); 
								// Matrix product (composition)



// ***************************************************************
// * 3-space vector and matrix utilities (prototypes)			 *
// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *

// Returns the solid angle between vectors v and w.
inline double SolidAngle( const VectorR3& v, const VectorR3& w);

// Returns a righthanded orthonormal basis to complement unit vector x
void GetOrtho( const VectorR3& x,  VectorR3& y, VectorR3& z);
// Returns a vector v orthonormal to unit vector x
void GetOrtho( const VectorR3& x,  VectorR3& y );

// Projections

// The next three functions are templated below.
//inline VectorR3 ProjectToUnit ( const VectorR3& u, const VectorR3& v); // Project u onto v
//inline VectorR3 ProjectPerpUnit ( const VectorR3& u, const VectorR3 & v); // Project perp to v
//inline VectorR3 ProjectPerpUnitDiff ( const VectorR3& u, const VectorR3& v)
// v must be a unit vector.

// Projection maps (LinearMapR3s)

inline LinearMapR3 VectorProjectMap( const VectorR3& u );
inline LinearMapR3 PlaneProjectMap ( const VectorR3& w );
inline LinearMapR3 PlaneProjectMap ( const VectorR3& u, const VectorR3 &v );
// u,v,w - must be unit vector.  u and v must be orthonormal and
// specify the plane they are parallel to.  w specifies the plane
// it is orthogonal to.


// ***************************************************************
// * Stream Output Routines	(Prototypes)						 *
// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *

ostream& operator<< ( ostream& os, const VectorR3& u );


// *****************************************************
// * VectorR3 class - inlined functions				   *
// * * * * * * * * * * * * * * * * * * * * * * * * * * *

inline VectorR3& VectorR3::Load( const double* v ) 
{
	x = *v; 
	y = *(v+1);
	z = *(v+2);
	return *this;
}

inline VectorR3& VectorR3::Load( const float* v ) 
{
	x = *v; 
	y = *(v+1);
	z = *(v+2);
	return *this;
}

inline 	void VectorR3::Dump( double* v ) const
{
	*v = x; 
	*(v+1) = y;
	*(v+2) = z;
}

inline 	void VectorR3::Dump( float* v ) const
{
	*v = (float)x; 
	*(v+1) = (float)y;
	*(v+2) = (float)z;
}

inline double VectorR3::operator[]( int i ) const
{
	switch (i) {
	case 0:
		return x;
	case 1:
		return y;
	case 2:
		return z;
	default:
		assert(0);
		return 0.0;
	}
}

inline VectorR3& VectorR3::MakeUnit ()			// Convert to unit vector (or leave zero).
{
	double nSq = NormSq();
	if (nSq != 0.0) {
		*this /= sqrt(nSq);
	}
	return *this;
}

inline VectorR3 operator+( const VectorR3& u, const VectorR3& v ) 
{ 
	return VectorR3(u.x+v.x, u.y+v.y, u.z+v.z); 
}
inline VectorR3 operator-( const VectorR3& u, const VectorR3& v ) 
{ 
	return VectorR3(u.x-v.x, u.y-v.y, u.z-v.z); 
}
inline VectorR3 operator*( const VectorR3& u, register double m) 
{ 
	return VectorR3( u.x*m, u.y*m, u.z*m); 
}
inline VectorR3 operator*( register double m, const VectorR3& u) 
{ 
	return VectorR3( u.x*m, u.y*m, u.z*m); 
}
inline VectorR3 operator/( const VectorR3& u, double m) 
{ 
	register double mInv = 1.0/m;
	return VectorR3( u.x*mInv, u.y*mInv, u.z*mInv); 
}

inline double operator^ ( const VectorR3& u, const VectorR3& v ) // Dot Product
{ 
	return ( u.x*v.x + u.y*v.y + u.z*v.z ); 
}

inline VectorR3 operator* (const VectorR3& u, const VectorR3& v)	// Cross Product
{
	return (VectorR3(	u.y*v.z - u.z*v.y,
					u.z*v.x - u.x*v.z,
					u.x*v.y - u.y*v.x  ) );
}

inline VectorR3 ArrayProd ( const VectorR3& u, const VectorR3& v )
{
	return ( VectorR3( u.x*v.x, u.y*v.y, u.z*v.z ) );
}

inline VectorR3& VectorR3::operator*= (const VectorR3& v)		// Cross Product
{
	double tx=x, ty=y;
	x =  y*v.z -  z*v.y;
	y =  z*v.x - tx*v.z;
	z = tx*v.y - ty*v.x;
	return ( *this );
}

// Cross Product on left
//  Set   this := v*this;
inline VectorR3& VectorR3::CrossProductLeft (const VectorR3& v)
{
	double tx=x, ty=y;
	x =  z*v.y - y*v.z;
	y = tx*v.z - z*v.x;
	z = ty*v.x - tx*v.y;
	return ( *this );
}

// (*this) += u*v;    
inline VectorR3& VectorR3::AddCrossProduct( const VectorR3& u, const VectorR3& v )
{
	x += u.y*v.z - u.z*v.y;
	y += u.z*v.x - u.x*v.z;
	z += u.x*v.y - u.y*v.x;
	return *this;
}

inline VectorR3& VectorR3::ArrayProd (const VectorR3& v)		// Component-wise Product
{
	x *= v.x;
	y *= v.y;
	z *= v.z;
	return ( *this );
}

inline VectorR3& VectorR3::AddScaled( const VectorR3& u, double s ) 
{
	x += s*u.x;
	y += s*u.y;
	z += s*u.z;
	return(*this);
}

inline VectorR3& VectorR3::SubtractFrom( const VectorR3& u  ) 
{
	x = u.x - x;;
	y = u.y - y;;
	z = u.z - z;;
	return(*this);
}

inline VectorR3& VectorR3::ReNormalize()			// Convert near unit back to unit
{
	double nSq = NormSq();
	register double mFact = 1.0-0.5*(nSq-1.0);	// Multiplicative factor
	*this *= mFact;
	return *this;
}

inline double NormalizeError (const VectorR3& u)
{
	register double discrepancy;
	discrepancy = u.x*u.x + u.y*u.y + u.z*u.z - 1.0;
	if ( discrepancy < 0.0 ) {
		discrepancy = -discrepancy;
	}
	return discrepancy;
}

inline double VectorR3::Dist( const VectorR3& u ) const 	// Distance from u
{
	return sqrt( DistSq(u) );
}

inline double VectorR3::DistSq( const VectorR3& u ) const	// Distance from u
{
	return ( (x-u.x)*(x-u.x) + (y-u.y)*(y-u.y) + (z-u.z)*(z-u.z) );
}

//
// Interpolation routines (not just Spherical Interpolation)
//

// Interpolate(start,end,frac) - linear interpolation
//		- allows overshooting the end points
inline VectorR3 Interpolate( const VectorR3& start, const VectorR3& end, double a)
{
	VectorR3 ret;
	Lerp( start, end, a, ret );
	return ret;
}


// ******************************************************
// * Matrix3x3  class - inlined functions				*
// * * * * * * * * * * * * * * * * * * * * * * * * * * **

inline Matrix3x3::Matrix3x3() {}

inline Matrix3x3::Matrix3x3( const VectorR3& u, const VectorR3& v, 
							 const VectorR3& s )
{
	m11 = u.x;		// Column 1
	m21 = u.y;
	m31 = u.z;
	m12 = v.x;		// Column 2
	m22 = v.y;
	m32 = v.z;
	m13 = s.x;		// Column 3
	m23 = s.y;
	m33 = s.z;
}

inline Matrix3x3::Matrix3x3( double a11, double a21, double a31,
							 double a12, double a22, double a32,
							 double a13, double a23, double a33)
					// Values specified in column order!!!
{
	m11 = a11;		// Row 1
	m12 = a12;
	m13 = a13;
	m21 = a21;		// Row 2
	m22 = a22;
	m23 = a23;
	m31 = a31;		// Row 3
	m32 = a32;
	m33 = a33;
}
	
inline void Matrix3x3::SetIdentity ( )
{
	m11 = m22 = m33 = 1.0;
	m12 = m13 = m21 = m23 = m31 = m32 = 0.0;
}

inline void Matrix3x3::SetZero( ) 
{
	m11 = m12 = m13 = m21 = m22 = m23 = m31 = m32 = m33 = 0.0;
}

inline void Matrix3x3::Set ( const Matrix3x3& A )	// Set to the matrix.
{
	m11 = A.m11;
	m21 = A.m21;
	m31 = A.m31;
	m12 = A.m12;
	m22 = A.m22;
	m32 = A.m32;
	m13 = A.m13;
	m23 = A.m23;
	m33 = A.m33;
}

inline void Matrix3x3::Set( const VectorR3& u, const VectorR3& v, 
							 const VectorR3& w)
{
	m11 = u.x;		// Column 1
	m21 = u.y;
	m31 = u.z;
	m12 = v.x;		// Column 2
	m22 = v.y;
	m32 = v.z;
	m13 = w.x;		// Column 3
	m23 = w.y;
	m33 = w.z;
}

inline void Matrix3x3::Set( double a11, double a21, double a31, 
							 double a12, double a22, double a32,
							 double a13, double a23, double a33)
					// Values specified in column order!!!SetCrossProductMatrix
{
	m11 = a11;		// Row 1
	m12 = a12;
	m13 = a13;
	m21 = a21;		// Row 2
	m22 = a22;
	m23 = a23;
	m31 = a31;		// Row 3
	m32 = a32;
	m33 = a33;
}

inline void Matrix3x3::SetByRows( double a11, double a12, double a13, 
							 double a21, double a22, double a23,
							 double a31, double a32, double a33)
					// Values specified in row order!!!
{
	m11 = a11;		// Row 1
	m12 = a12;
	m13 = a13;
	m21 = a21;		// Row 2
	m22 = a22;
	m23 = a23;
	m31 = a31;		// Row 3
	m32 = a32;
	m33 = a33;
}

inline void Matrix3x3::LoadByRows( const double* a)
{
	// Values in ROW order
	m11 = *(a++);		// Row 1
	m12 = *(a++);
	m13 = *(a++);
	m21 = *(a++);		// Row 2
	m22 = *(a++);
	m23 = *(a++);
	m31 = *(a++);		// Row 3
	m32 = *(a++);
	m33 = *a;
}


inline void Matrix3x3::SetByRows( const VectorR3& u, const VectorR3& v, 
									const VectorR3& s )
{
	m11 = u.x;		// Row 1
	m12 = u.y;
	m13 = u.z;
	m21 = v.x;		// Row 2
	m22 = v.y;
	m23 = v.z;
	m31 = s.x;		// Row 3
	m32 = s.y;
	m33 = s.z;
}

inline void Matrix3x3::SetColumn1 ( double x, double y, double z)
{
	m11 = x; m21 = y; m31= z;
}

inline void Matrix3x3::SetColumn2 ( double x, double y, double z)
{
	m12 = x; m22 = y; m32= z;
}

inline void Matrix3x3::SetColumn3 ( double x, double y, double z)
{
	m13 = x; m23 = y; m33= z;
}

inline void Matrix3x3::SetColumn1 ( const VectorR3& u )
{
	m11 = u.x; m21 = u.y; m31 = u.z;
}

inline void Matrix3x3::SetColumn2 ( const VectorR3& u )
{
	m12 = u.x; m22 = u.y; m32 = u.z;
}

inline void Matrix3x3::SetColumn3 ( const VectorR3& u )
{
	m13 = u.x; m23 = u.y; m33 = u.z;
}

inline void Matrix3x3::SetRow1 ( double x, double y, double z )
{
	m11 = x;
	m12 = y;
	m13 = z;
}

inline void Matrix3x3::SetRow2 ( double x, double y, double z )
{
	m21 = x;
	m22 = y;
	m23 = z;
}

inline void Matrix3x3::SetRow3 ( double x, double y, double z )
{
	m31 = x;
	m32 = y;
	m33 = z;
}



inline VectorR3 Matrix3x3::Column1() const
{
	return ( VectorR3(m11, m21, m31) );
}

inline VectorR3 Matrix3x3::Column2() const
{
	return ( VectorR3(m12, m22, m32) );
}

inline VectorR3 Matrix3x3::Column3() const
{
	return ( VectorR3(m13, m23, m33) );
}

inline VectorR3 Matrix3x3::Row1() const
{
	return ( VectorR3(m11, m12, m13) );
}

inline VectorR3 Matrix3x3::Row2() const
{
	return ( VectorR3(m21, m22, m23) );
}

inline VectorR3 Matrix3x3::Row3() const
{
	return ( VectorR3(m31, m32, m33) );
}

inline void Matrix3x3::SetDiagonal( double x, double y, double z )
{
	m11 = x;
	m22 = y;
	m33 = z;
}

inline void Matrix3x3::SetDiagonal( const VectorR3& u )
{
	SetDiagonal ( u.x, u.y, u.z );
}

inline double Matrix3x3::Diagonal( int i ) const 
{
	switch (i) {
	case 0:
		return m11;
	case 1:
		return m22;
	case 2:
		return m33;
	default:
		assert(0);
		return 0.0;
	}
}

// Set this so that  (this)v  =  u*v   where * is vector cross product
inline void Matrix3x3::SetCrossProductMatrix( const VectorR3& u )
{
	m11 = m22 = m33 = 0.0;
	m21 = u.z;
	m12 = -u.z;
	m13 = u.y;
	m31 = -u.y;
	m32 = u.x;
	m23 = -u.x;
}


// Set this = u * v^T
inline void Matrix3x3::SetOuterProduct( const VectorR3& u, const VectorR3& v )
{
	m11 = u.x*v.x;
	m12 = u.x*v.y;
	m13 = u.x*v.z;
	m21 = u.y*v.x;
	m22 = u.y*v.y;
	m23 = u.y*v.z;
	m31 = u.z*v.x;
	m32 = u.z*v.y;
	m33 = u.z*v.z;
}

inline void Matrix3x3::MakeTranspose()	// Transposes it.
{
	register double temp;
	temp = m12;
	m12 = m21;
	m21=temp;
	temp = m13;
	m13 = m31;
	m31 = temp;
	temp = m23;
	m23 = m32;
	m32 = temp;
}

inline VectorR3 operator* ( const Matrix3x3& A, const VectorR3& u)
{
	return( VectorR3( A.m11*u.x + A.m12*u.y + A.m13*u.z,
					  A.m21*u.x + A.m22*u.y + A.m23*u.z,
					  A.m31*u.x + A.m32*u.y + A.m33*u.z ) ); 
}


// See LinearR4.h for the code for the VectorR4 versions of the next two functions.

inline void Matrix3x3::Transform( VectorR3* u ) const {
	double newX, newY;
	newX = m11*u->x + m12*u->y + m13*u->z;
	newY = m21*u->x + m22*u->y + m23*u->z;
	u->z = m31*u->x + m32*u->y + m33*u->z;
	u->x = newX;
	u->y = newY;
}

inline void Matrix3x3::Transform( const VectorR3& src, VectorR3* dest ) const {
	dest->x = m11*src.x + m12*src.y + m13*src.z;
	dest->y = m21*src.x + m22*src.y + m23*src.z;
	dest->z = m31*src.x + m32*src.y + m33*src.z;
}

inline void Matrix3x3::TransformTranspose( VectorR3* u ) const {
	double newX, newY;
	newX = m11*u->x + m21*u->y + m31*u->z;
	newY = m12*u->x + m22*u->y + m32*u->z;
	u->z = m13*u->x + m23*u->y + m33*u->z;
	u->x = newX;
	u->y = newY;
}

inline void Matrix3x3::TransformTranspose( const VectorR3& src, VectorR3* dest ) const {
	dest->x = m11*src.x + m21*src.y + m31*src.z;
	dest->y = m12*src.x + m22*src.y + m32*src.z;
	dest->z = m13*src.x + m23*src.y + m33*src.z;
}


// ******************************************************
// * LinearMapR3 class - inlined functions				*
// * * * * * * * * * * * * * * * * * * * * * * * * * * **

inline LinearMapR3::LinearMapR3()
{
	SetZero();
	return;
}	

inline LinearMapR3::LinearMapR3( const VectorR3& u, const VectorR3& v, 
							 const VectorR3& s )
:Matrix3x3 ( u, v, s )
{ }

inline LinearMapR3::LinearMapR3( 
							 double a11, double a21, double a31,
							 double a12, double a22, double a32,
							 double a13, double a23, double a33)
					// Values specified in column order!!!
:Matrix3x3 ( a11, a21, a31, a12, a22, a32, a13, a23, a33)
{ }

inline LinearMapR3::LinearMapR3 ( const Matrix3x3& A )
: Matrix3x3 (A) 
{}

inline void LinearMapR3::SetZero( ) 
{
	Matrix3x3::SetZero();
}

inline void LinearMapR3::Negate() 
{
	m11 = -m11;		// Row 1
	m12 = -m12;
	m13 = -m13;
	m21 = -m21;		// Row 2
	m22 = -m22;
	m23 = -m23;
	m31 = -m31;		// Row 3
	m32 = -m32;
	m33 = -m33;
}

	
inline LinearMapR3& LinearMapR3::operator+= (const Matrix3x3& B)
{
	m11 += B.m11;
	m12 += B.m12;
	m13 += B.m13;
	m21 += B.m21;
	m22 += B.m22;
	m23 += B.m23;
	m31 += B.m31;
	m32 += B.m32;
	m33 += B.m33;
	return ( *this );
}

inline LinearMapR3& LinearMapR3::operator-= (const Matrix3x3& B)
{
	m11 -= B.m11;
	m12 -= B.m12;
	m13 -= B.m13;
	m21 -= B.m21;
	m22 -= B.m22;
	m23 -= B.m23;
	m31 -= B.m31;
	m32 -= B.m32;
	m33 -= B.m33;
	return( *this );
}

inline LinearMapR3 operator+ (const LinearMapR3& A, const LinearMapR3& B)
{
	return (LinearMapR3( A.m11+B.m11, A.m21+B.m21, A.m31+B.m31,
						 A.m12+B.m12, A.m22+B.m22, A.m32+B.m32,
						 A.m13+B.m13, A.m23+B.m23, A.m33+B.m33 ) );
}

inline LinearMapR3 operator+ (const LinearMapR3& A, const Matrix3x3& B)
{
	return (LinearMapR3( A.m11+B.m11, A.m21+B.m21, A.m31+B.m31,
						 A.m12+B.m12, A.m22+B.m22, A.m32+B.m32,
						 A.m13+B.m13, A.m23+B.m23, A.m33+B.m33 ) );
}

inline LinearMapR3 operator+ (const Matrix3x3& A, const LinearMapR3& B)
{
	return (LinearMapR3( A.m11+B.m11, A.m21+B.m21, A.m31+B.m31,
						 A.m12+B.m12, A.m22+B.m22, A.m32+B.m32,
						 A.m13+B.m13, A.m23+B.m23, A.m33+B.m33 ) );
}

inline LinearMapR3 operator- (const LinearMapR3& A) 
{
	return( LinearMapR3( -A.m11, -A.m21, -A.m31,
						 -A.m12, -A.m22, -A.m32,
						 -A.m13, -A.m23, -A.m33 ) );
}

inline LinearMapR3 operator- (const LinearMapR3& A, const LinearMapR3& B)
{
	return( LinearMapR3( A.m11-B.m11, A.m21-B.m21, A.m31-B.m31,
						 A.m12-B.m12, A.m22-B.m22, A.m32-B.m32,
						 A.m13-B.m13, A.m23-B.m23, A.m33-B.m33 ) );
}

inline LinearMapR3 operator- (const Matrix3x3& A, const LinearMapR3& B)
{
	return( LinearMapR3( A.m11-B.m11, A.m21-B.m21, A.m31-B.m31,
						 A.m12-B.m12, A.m22-B.m22, A.m32-B.m32,
						 A.m13-B.m13, A.m23-B.m23, A.m33-B.m33 ) );
}

inline LinearMapR3 operator- (const LinearMapR3& A, const Matrix3x3& B)
{
	return( LinearMapR3( A.m11-B.m11, A.m21-B.m21, A.m31-B.m31,
						 A.m12-B.m12, A.m22-B.m22, A.m32-B.m32,
						 A.m13-B.m13, A.m23-B.m23, A.m33-B.m33 ) );
}

inline LinearMapR3& LinearMapR3::operator*= (register double b)
{
	m11 *= b;
	m12 *= b;
	m13 *= b;
	m21 *= b;
	m22 *= b;
	m23 *= b;
	m31 *= b;
	m32 *= b;
	m33 *= b;
	return ( *this);
}

inline LinearMapR3 operator* ( const LinearMapR3& A, register double b)
{
	return( LinearMapR3( A.m11*b, A.m21*b, A.m31*b,
						 A.m12*b, A.m22*b, A.m32*b,
						 A.m13*b, A.m23*b, A.m33*b ) );
}

inline LinearMapR3 operator* ( register double b, const LinearMapR3& A)
{
	return( LinearMapR3( A.m11*b, A.m21*b, A.m31*b,
						 A.m12*b, A.m22*b, A.m32*b,
						 A.m13*b, A.m23*b, A.m33*b ) );
}

inline LinearMapR3 operator/ ( const LinearMapR3& A, double b)
{
	register double bInv = 1.0/b;
	return( LinearMapR3( A.m11*bInv, A.m21*bInv, A.m31*bInv,
						 A.m12*bInv, A.m22*bInv, A.m32*bInv,
						 A.m13*bInv, A.m23*bInv, A.m33*bInv ) );
}

inline LinearMapR3& LinearMapR3::operator/= (register double b)
{
	register double bInv = 1.0/b;
	return ( *this *= bInv );
}

// Sets this = (m - this).
inline void LinearMapR3::SubtractFrom( const Matrix3x3& m)
{
	m11 = m.m11 - m11;
	m12 = m.m12 - m12;
	m13 = m.m13 - m13;
	m21 = m.m21 - m21;
	m22 = m.m22 - m22;
	m23 = m.m23 - m23;
	m31 = m.m31 - m31;
	m32 = m.m32 - m32;
	m33 = m.m33 - m33;
}

inline LinearMapR3& LinearMapR3::operator*= (const Matrix3x3& B)	// Matrix product
{
	OperatorTimesEquals( B );
	return( *this );
}

inline VectorR3 LinearMapR3::Solve(const VectorR3& u) const	// Returns solution
{
	return ( Matrix3x3::Solve( u ) );
}
	
inline LinearMapR3 LinearMapR3::Transpose() const	// Returns the transpose
{
	return ( LinearMapR3 ( m11, m12, m13, m21, m22, m23, m31, m32, m33) );
}

// ***************************************************************
// * 3-space vector and matrix utilities (inlined functions)	 *
// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *

// Returns the projection of u onto unit v
inline VectorR3 ProjectToUnit ( const VectorR3& u, const VectorR3& v)
{
	return (u^v)*v;
}

// Returns the projection of u onto the plane perpindicular to the unit vector v
inline VectorR3 ProjectPerpUnit ( const VectorR3& u, const VectorR3& v)
{
	return ( u - ((u^v)*v) );
}

// Returns the projection of u onto the plane perpindicular to the unit vector v
//    This one is more stable when u and v are nearly equal.
inline VectorR3 ProjectPerpUnitDiff ( const VectorR3& u, const VectorR3& v)
{
	VectorR3 ans = u;
	ans -= v;
	ans -= ((ans^v)*v);
	return ans;				// ans = (u-v) - ((u-v)^v)*v
}

// VectorProjectMap returns map projecting onto a given vector u.
//		u should be a unit vector (otherwise the returned map is
//		scaled according to the magnitude of u.
inline LinearMapR3 VectorProjectMap( const VectorR3& u )
{
	double a = u.x*u.y;
	double b = u.x*u.z;
	double c = u.y*u.z;
	return( LinearMapR3( u.x*u.x,     a,     b,
						    a,    u.y*u.y,   c,
							b,        c,   u.z*u.z ) );
}

// PlaneProjectMap returns map projecting onto a given plane.
//		The plane is the plane orthognal to w.
//		w must be a unit vector (otherwise the returned map is
//		garbage).
inline LinearMapR3 PlaneProjectMap ( const VectorR3& w )
{
	double a = -w.x*w.y;
	double b = -w.x*w.z;
	double c = -w.y*w.z;
	return( LinearMapR3( 1.0-w.x*w.x,     a,         b,
						    a,        1.0-w.y*w.y,   c,
							b,            c,       1.0-w.z*w.z ) );
}

// PlaneProjectMap returns map projecting onto a given plane.
//		The plane is the plane containing the two orthonormal vectors u,v.
//		If u, v are orthonormal, this is a projection with scaling.
//		If they are not orthonormal, the results are more difficult
//		to interpret.
inline LinearMapR3 PlaneProjectMap ( const VectorR3& u, const VectorR3 &v )
{
	double a = u.x*u.y + v.x*v.y;
	double b = u.x*u.z + v.x*v.z;
	double c = u.y*u.z + v.y*v.z;
	return( LinearMapR3( u.x*u.x+v.x*v.x,     a,            b,
						    a,           u.y*u.y+u.y*u.y,   c,
							b,                c,         u.z*u.z+v.z*v.z ) );
}  

// Returns the solid angle between unit vectors v and w.
inline double SolidAngle( const VectorR3& v, const VectorR3& w)
{
	return atan2 ( (v*w).Norm(), v^w );
}


#endif

// ******************* End of header material ********************
