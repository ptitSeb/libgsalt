#ifndef MXQMETRIC_INCLUDED // -*- C++ -*-
#define MXQMETRIC_INCLUDED
#if !defined(__GNUC__)
#  pragma once
#endif

/************************************************************************

  n-D Quadric Error Metric

  Copyright (C) 1998 Michael Garland.  See "COPYING.txt" for details.
  
  $Id: MxQMetric.h,v 1.5 1998/10/26 21:09:17 garland Exp $

 ************************************************************************/

#include "MxQMetric3.h"
#include "MxVector.h"
#include "MxMatrix.h"

class MxQuadric
{
private:
    MxMatrix A;
    MxVector b;
    real c;

    real r;

public:
    MxQuadric(unsigned int N) : A(N), b(N) { clear(); }
    MxQuadric(const MxVector& p1, const MxVector& p2, const MxVector& p3,
	      real area=1.0);
    MxQuadric(const MxQuadric3&, unsigned int N);
    MxQuadric(const MxQuadric& Q)
	: A(Q.A.dim()), b(Q.b.dim()) { *this = Q; }

    const MxMatrix& tensor() const { return A; }
    const MxVector& vector() const { return b; }
    real offset() const { return c; }
    real area() const { return r; }
    MxMatrix& homogeneous(MxMatrix& H) const;

    void clear(real val=0.0) { A=val; b=val; c=val; r=val; }
    MxQuadric& operator=(const MxQuadric& Q)
	{ A=Q.A; b=Q.b; c=Q.c; r=Q.r; return *this; }
    MxQuadric& operator+=(const MxQuadric& Q)
	{ A+=Q.A; b+=Q.b; c+=Q.c; r+=Q.r; return *this; }
    MxQuadric& operator-=(const MxQuadric& Q)
	{ A-=Q.A; b-=Q.b; c-=Q.c; r-=Q.r; return *this; }
    MxQuadric& operator*=(real s)
	{ A*=s; b*=s; c*=s; return *this; }

    real evaluate(const MxVector& v) const;
    real operator()(const MxVector& v) const { return evaluate(v); }

    bool optimize(MxVector& v) const;
};

// MXQMETRIC_INCLUDED
#endif
