#ifndef GFXMAT4_INCLUDED // -*- C++ -*-
#define GFXMAT4_INCLUDED
#if !defined(__GNUC__)
#  pragma once
#endif

/************************************************************************

  4x4 Matrix class

  $Id: mat4.h,v 1.10 2001/11/19 16:07:42 garland Exp $

 ************************************************************************/

#include "vec4.h"
#include <math.h>

class Mat4
{
private:
    Vec4 row[4];

public:
    // Standard constructors
    //
    Mat4() { *this = 0.0; }
    Mat4(const Vec4& r0,const Vec4& r1,const Vec4& r2,const Vec4& r3)
    	{ row[0]=r0; row[1]=r1; row[2]=r2; row[3]=r3; }
    Mat4(const Mat4& m) { *this = m; }

    // Descriptive interface
    //
    typedef real value_type;
    typedef Vec4 vector_type;
    typedef Mat4 inverse_type;
    static int dim() { return 4; }

    // Access methods
    //
    real& operator()(int i, int j)       { return row[i][j]; }
    real  operator()(int i, int j) const { return row[i][j]; }
    Vec4&       operator[](int i)       { return row[i]; }
    const Vec4& operator[](int i) const { return row[i]; }
    inline Vec4 col(int i) const
        { return Vec4(row[0][i],row[1][i],row[2][i],row[3][i]); }

    operator       real*()       { return row[0]; }
    operator const real*()       { return row[0]; }
    operator const real*() const { return row[0]; }

    // Assignment methods
    //
    inline Mat4& operator=(const Mat4& m);
    inline Mat4& operator=(real s);

    inline Mat4& operator+=(const Mat4& m);
    inline Mat4& operator-=(const Mat4& m);
    inline Mat4& operator*=(real s);
    inline Mat4& operator/=(real s);

    static Mat4 I();
};

////////////////////////////////////////////////////////////////////////
//
// Method definitions
//

inline Mat4& Mat4::operator=(const Mat4& m)
{
    row[0] = m[0]; row[1] = m[1]; row[2] = m[2]; row[3] = m[3];
    return *this;
}

inline Mat4& Mat4::operator=(real s)
{
    row[0]=s;  row[1]=s;  row[2]=s;  row[3]=s;
    return *this;
}

inline Mat4& Mat4::operator+=(const Mat4& m)
{
    row[0] += m[0]; row[1] += m[1]; row[2] += m[2]; row[3] += m[3];
    return *this;
}

inline Mat4& Mat4::operator-=(const Mat4& m)
{
    row[0] -= m[0]; row[1] -= m[1]; row[2] -= m[2]; row[3] -= m[3];
    return *this;
}

inline Mat4& Mat4::operator*=(real s)
{
    row[0] *= s; row[1] *= s; row[2] *= s; row[3] *= s;
    return *this;
}

inline Mat4& Mat4::operator/=(real s)
{
    row[0] /= s; row[1] /= s; row[2] /= s; row[3] /= s;
    return *this;
}

////////////////////////////////////////////////////////////////////////
//
// Operator definitions
//

inline Mat4 operator+(const Mat4& n, const Mat4& m)
	{ return Mat4(n[0]+m[0], n[1]+m[1], n[2]+m[2], n[3]+m[3]); }

inline Mat4 operator-(const Mat4& n, const Mat4& m)
	{ return Mat4(n[0]-m[0], n[1]-m[1], n[2]-m[2], n[3]-m[3]); }

inline Mat4 operator-(const Mat4& n)
	{ return Mat4(-n[0], -n[1], -n[2], -n[3]); }

inline Mat4 operator*(real s, const Mat4& m)
	{ return Mat4(m[0]*s, m[1]*s, m[2]*s, m[3]*s); }
inline Mat4 operator*(const Mat4& m, real s)
	{ return s*m; }

inline Mat4 operator/(const Mat4& m, real s)
	{ return Mat4(m[0]/s, m[1]/s, m[2]/s, m[3]/s); }

inline Vec4 operator*(const Mat4& m, const Vec4& v)
	{ return Vec4(m[0]*v, m[1]*v, m[2]*v, m[3]*v); }

extern Mat4 operator*(const Mat4& n, const Mat4& m);

//
// Transform a homogeneous 3-vector and reproject into normal 3-space
//
inline Vec3 operator*(const Mat4& m, const Vec3& v)
{
    Vec4 u=Vec4(v,1);
    real w=m[3]*u;

    if(w==0.0)  return Vec3(m[0]*u, m[1]*u, m[2]*u);
    else        return Vec3(m[0]*u/w, m[1]*u/w, m[2]*u/w);
}
/*
inline std::ostream &operator<<(std::ostream &out, const Mat4& M)
       { return out<<M[0]<<std::endl<<M[1]<<std::endl<<M[2]<<std::endl<<M[3]; }

inline std::istream &operator>>(std::istream &in, Mat4& M)
       { return in >> M[0] >> M[1] >> M[2] >> M[3]; }
*/
////////////////////////////////////////////////////////////////////////
//
// Transformations
//

extern Mat4 translation_matrix(const Vec3& delta);

extern Mat4 scaling_matrix(const Vec3& scale);

extern Mat4 rotation_matrix_rad(real theta, const Vec3& axis);

inline Mat4 rotation_matrix_deg(real theta, const Vec3& axis)
	{ return rotation_matrix_rad(theta*M_PI/180.0, axis); }

extern Mat4 perspective_matrix(real fovy, real aspect,
			       real zmin=0.0, real zmax=0.0);

extern Mat4 lookat_matrix(const Vec3& from, const Vec3& at, const Vec3& up);

extern Mat4 viewport_matrix(real w, real h);

////////////////////////////////////////////////////////////////////////
//
// Misc. function definitions
//

inline real det(const Mat4& m) { return m[0] * cross(m[1], m[2], m[3]); }

inline real trace(const Mat4& m) { return m(0,0)+m(1,1)+m(2,2)+m(3,3); }

inline Mat4 transpose(const Mat4& m)
	{ return Mat4(m.col(0), m.col(1), m.col(2), m.col(3)); }

extern Mat4 adjoint(const Mat4& m);
extern real invert(Mat4& m_inv, const Mat4& m);
extern real invert_cramer(Mat4& m_inv, const Mat4& m);

extern bool eigen(const Mat4& m, Vec4& eig_vals, Vec4 eig_vecs[4]);

// GFXMAT4_INCLUDED
#endif
