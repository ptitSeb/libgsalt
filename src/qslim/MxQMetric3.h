#ifndef MXQMETRIC3_INCLUDED // -*- C++ -*-
#define MXQMETRIC3_INCLUDED
#if !defined(__GNUC__)
#  pragma once
#endif

/************************************************************************

  3D Quadric Error Metric

  Copyright (C) 1998 Michael Garland.  See "COPYING.txt" for details.
  
  $Id: MxQMetric3.h,v 1.14 1999/12/15 18:07:45 garland Exp $

 ************************************************************************/

#include "MxMath.h"
#include "MxMat3.h"
#include "MxMat4.h"

const real FEQ_EPS = 1e-6;
inline bool  FEQ(real a, real b, real e=FEQ_EPS)  {return fabs(a-b)<e;}

class MxQuadric3
{
private:
    real a2, ab, ac, ad;
    real     b2, bc, bd;
    real         c2, cd;
    real             d2;

    real r;

    void init(real a, real b, real c, real d, real area);
    void init(const Mat4& Q, real area);

public:
    MxQuadric3() { clear(); }
    MxQuadric3(real a, real b, real c, real d, real area=1.0)
	{ init(a, b, c, d, area); }
#ifndef USE_FLOAT
    MxQuadric3(const float *n, real d, real area=1.0)
	{ init(n[X], n[Y], n[Z], d, area); }
#endif
    MxQuadric3(const real *n, real d, real area=1.0)
	{ init(n[X], n[Y], n[Z], d, area); }
    MxQuadric3(const MxQuadric3& Q) { *this = Q; }

    Mat3 tensor() const;
    Vec3 vector() const { return Vec3(ad, bd, cd); }
    real offset() const { return d2; }
    real area() const { return r; }
    Mat4 homogeneous() const;

    void set_coefficients(const real *);
    void set_area(real a) { r=a; }
    void point_constraint(const float *);

    void clear(real val=0.0) { a2=ab=ac=ad=b2=bc=bd=c2=cd=d2=r=val; }
    MxQuadric3& operator=(const MxQuadric3& Q);
    MxQuadric3& operator+=(const MxQuadric3& Q);
    MxQuadric3& operator-=(const MxQuadric3& Q);
    MxQuadric3& operator*=(real s);
    MxQuadric3& transform(const Mat4& P);

    real evaluate(real x, real y, real z) const;
    real evaluate(const real *v) const {return evaluate(v[X], v[Y], v[Z]);}
#ifndef USE_FLOAT
    real evaluate(const float *v) const  {return evaluate(v[X], v[Y], v[Z]);}
#endif

    real operator()(real x, real y, real z) const
	{ return evaluate(x, y, z); }
    real operator()(const real *v) const {return evaluate(v[X],v[Y],v[Z]);}
#ifndef USE_FLOAT
    real operator()(const float *v) const  {return evaluate(v[X],v[Y],v[Z]);}
#endif

    bool optimize(Vec3& v) const;
    bool optimize(float *x, float *y, float *z) const;

    bool optimize(Vec3& v, const Vec3& v1, const Vec3& v2) const;
    bool optimize(Vec3& v,const Vec3& v1,const Vec3& v2,const Vec3& v3) const;

/*
    ostream& write(ostream& out)
	{
	    return out << a2 << " " << ab << " " << ac << " " << ad << " "
		       << b2 << " " << bc << " " << bd << " " << c2 << " "
		       << cd << " " << d2 << " " << r;
	}

    ostream& write_full(ostream& out)
	{
	    return out << a2 << " " << ab << " " << ac << " " << ad << " "
		       << ab << " " << b2 << " " << bc << " " << bd << " "
		       << ac << " " << bc << " " << c2 << " " << cd << " "
		       << ad << " " << bd << " " << cd << " " << d2;
	}


    istream& read(istream& in)
	{
	    return in >> a2 >> ab >> ac >> ad >> b2
		      >> bc >> bd >> c2 >> cd >> d2 >> r;
	}
    

    istream& read_full(istream& in)
	{
	    return in >> a2 >> ab >> ac >> ad
		      >> ab >> b2 >> bc >> bd
		      >> ac >> bc >> c2 >> cd
		      >> ad >> bd >> cd >> d2;
	}
*/
};

/*
inline ostream& operator<<(ostream& out, MxQuadric3& Q) {return Q.write(out);}
inline istream& operator>>(istream& in, MxQuadric3& Q) { return Q.read(in); }
*/

////////////////////////////////////////////////////////////////////////
//
// Quadric visualization routines
//
/*
#define MX_RED_ELLIPSOIDS 0x1
#define MX_GREEN_ELLIPSOIDS 0x2
#define MX_CHARCOAL_ELLIPSOIDS 0x3

extern void mx_quadric_shading(int c=MX_GREEN_ELLIPSOIDS, bool twosided=true);
extern void mx_draw_quadric(const MxQuadric3& Q, real r, const float*v=NULL);
extern void mx_draw_osculant(float k1, float k2, float extent=1.0);
*/
// MXQMETRIC3_INCLUDED
#endif
