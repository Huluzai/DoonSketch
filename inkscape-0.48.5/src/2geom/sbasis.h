/**
 * \file
 * \brief Defines S-power basis function class
 *
 *  Authors:
 *   Nathan Hurst <njh@mail.csse.monash.edu.au>
 *   Michael Sloan <mgsloan@gmail.com>
 *
 * Copyright (C) 2006-2007 authors
 *
 * This library is free software; you can redistribute it and/or
 * modify it either under the terms of the GNU Lesser General Public
 * License version 2.1 as published by the Free Software Foundation
 * (the "LGPL") or, at your option, under the terms of the Mozilla
 * Public License Version 1.1 (the "MPL"). If you do not alter this
 * notice, a recipient may use your version of this file under either
 * the MPL or the LGPL.
 *
 * You should have received a copy of the LGPL along with this library
 * in the file COPYING-LGPL-2.1; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 * You should have received a copy of the MPL along with this library
 * in the file COPYING-MPL-1.1
 *
 * The contents of this file are subject to the Mozilla Public License
 * Version 1.1 (the "License"); you may not use this file except in
 * compliance with the License. You may obtain a copy of the License at
 * http://www.mozilla.org/MPL/
 *
 * This software is distributed on an "AS IS" basis, WITHOUT WARRANTY
 * OF ANY KIND, either express or implied. See the LGPL or the MPL for
 * the specific language governing rights and limitations.
 */

#ifndef SEEN_SBASIS_H
#define SEEN_SBASIS_H
#include <vector>
#include <cassert>
#include <iostream>

#include <2geom/linear.h>
#include <2geom/interval.h>
#include <2geom/utils.h>
#include <2geom/exception.h>

//#define USE_SBASISN 1


#if defined(USE_SBASIS_OF)

#include "sbasis-of.h"

#elif defined(USE_SBASISN)

#include "sbasisN.h"
namespace Geom{

/*** An empty SBasis is identically 0. */
class SBasis : public SBasisN<1>;

};
#else

namespace Geom{

/*** An empty SBasis is identically 0. */
class SBasis{
    std::vector<Linear> d;
    void push_back(Linear const&l) { d.push_back(l); }

public:
    // As part of our migration away from SBasis isa vector we provide this minimal set of vector interface methods.
    size_t size() const {return d.size();}
    Linear operator[](unsigned i) const {
        return d[i];
    }
    Linear& operator[](unsigned i) { return d.at(i); }
	
    //Linear const* begin() const { return (Linear const*)&*d.begin();}
    //Linear const* end() const { return (Linear const*)&*d.end();}
    //Linear* begin() { return (Linear*)&*d.begin();}
    //Linear* end() { return (Linear*)&*d.end();}
	
	std::vector<Linear>::const_iterator begin() const { return d.begin(); }
	std::vector<Linear>::const_iterator end() const { return d.end(); }
	
	std::vector<Linear>::iterator begin() { return d.begin(); }
	std::vector<Linear>::iterator end() { return d.end(); }
	
    bool empty() const {return d.empty();}
    Linear &back() {return d.back();}
    Linear const &back() const {return d.back();}
    void pop_back() { d.pop_back();}
    void resize(unsigned n) { d.resize(n);}
    void resize(unsigned n, Linear const& l) { d.resize(n, l);}
    void reserve(unsigned n) { d.reserve(n);}
    void clear() {d.clear();}
	
    void insert(std::vector<Linear>::iterator before, std::vector<Linear>::const_iterator src_begin, std::vector<Linear>::const_iterator src_end) {
		d.insert(before, src_begin, src_end);
	}
	
    //void insert(Linear* aa, Linear* bb, Linear* cc} { d.insert(aa, bb, cc);}
    Linear& at(unsigned i) { return d.at(i);}
    //void insert(Linear* before, int& n, Linear const &l) { d.insert(std::vector<Linear>::iterator(before), n, l);}
    bool operator==(SBasis const&B) const { return d == B.d;}
    bool operator!=(SBasis const&B) const { return d != B.d;}
    operator std::vector<Linear>() { return d;}

    
    SBasis() {}
    explicit SBasis(double a) {
        push_back(Linear(a,a));
    }
    explicit SBasis(double a, double b) {
        push_back(Linear(a,b));
    }
    SBasis(SBasis const & a) :
        d(a.d)
    {}
    SBasis(Linear const & bo) {
        push_back(bo);
    }
    SBasis(Linear* bo) {
        push_back(*bo);
    }
    explicit SBasis(size_t n, Linear const&l) : d(n, l) {}

    //IMPL: FragmentConcept
    typedef double output_type;
    inline bool isZero() const {
        if(empty()) return true;
        for(unsigned i = 0; i < size(); i++) {
            if(!(*this)[i].isZero()) return false;
        }
        return true;
    }
    inline bool isConstant() const {
        if (empty()) return true;
        for (unsigned i = 0; i < size(); i++) {
            if(!(*this)[i].isConstant()) return false;
        }
        return true;
    }

    bool isFinite() const;
    inline double at0() const { 
        if(empty()) return 0; else return (*this)[0][0];
    }
    inline double at1() const{
        if(empty()) return 0; else return (*this)[0][1];
    }
    
    int degreesOfFreedom() const { return size()*2;}

    double valueAt(double t) const {
        double s = t*(1-t);
        double p0 = 0, p1 = 0;
        for(unsigned k = size(); k > 0; k--) {
            const Linear &lin = (*this)[k-1];
            p0 = p0*s + lin[0];
            p1 = p1*s + lin[1];
        }
        return (1-t)*p0 + t*p1;
    }
    //double valueAndDerivative(double t, double &der) const {
    //}
    double operator()(double t) const {
        return valueAt(t);
    }

    std::vector<double> valueAndDerivatives(double t, unsigned n) const;

    SBasis toSBasis() const { return SBasis(*this); }

    double tailError(unsigned tail) const;

// compute f(g)
    SBasis operator()(SBasis const & g) const;

//MUTATOR PRISON
    //remove extra zeros
    void normalize() {
        while(!empty() && 0 == back()[0] && 0 == back()[1])
            pop_back();
    }

    void truncate(unsigned k) { if(k < size()) resize(k); }
private:
    void derive(); // in place version
};

//TODO: figure out how to stick this in linear, while not adding an sbasis dep
inline SBasis Linear::toSBasis() const { return SBasis(*this); }

//implemented in sbasis-roots.cpp
OptInterval bounds_exact(SBasis const &a);
OptInterval bounds_fast(SBasis const &a, int order = 0);
OptInterval bounds_local(SBasis const &a, const OptInterval &t, int order = 0);

/** Returns a function which reverses the domain of a.
 \param a sbasis function

useful for reversing a parameteric curve.
*/
inline SBasis reverse(SBasis const &a) {
    SBasis result(a.size(), Linear());
    
    for(unsigned k = 0; k < a.size(); k++)
        result[k] = reverse(a[k]);
    return result;
}

//IMPL: ScalableConcept
inline SBasis operator-(const SBasis& p) {
    if(p.isZero()) return SBasis();
    SBasis result(p.size(), Linear());
        
    for(unsigned i = 0; i < p.size(); i++) {
        result[i] = -p[i];
    }
    return result;
}
SBasis operator*(SBasis const &a, double k);
inline SBasis operator*(double k, SBasis const &a) { return a*k; }
inline SBasis operator/(SBasis const &a, double k) { return a*(1./k); }
SBasis& operator*=(SBasis& a, double b);
inline SBasis& operator/=(SBasis& a, double b) { return (a*=(1./b)); }

//IMPL: AddableConcept
SBasis operator+(const SBasis& a, const SBasis& b);
SBasis operator-(const SBasis& a, const SBasis& b);
SBasis& operator+=(SBasis& a, const SBasis& b);
SBasis& operator-=(SBasis& a, const SBasis& b);

//TODO: remove?
/*inline SBasis operator+(const SBasis & a, Linear const & b) {
    if(b.isZero()) return a;
    if(a.isZero()) return b;
    SBasis result(a);
    result[0] += b;
    return result;
}
inline SBasis operator-(const SBasis & a, Linear const & b) {
    if(b.isZero()) return a;
    SBasis result(a);
    result[0] -= b;
    return result;
}
inline SBasis& operator+=(SBasis& a, const Linear& b) {
    if(a.isZero())
        a.push_back(b);
    else
        a[0] += b;
    return a;
}
inline SBasis& operator-=(SBasis& a, const Linear& b) {
    if(a.isZero())
        a.push_back(-b);
    else
        a[0] -= b;
    return a;
    }*/

//IMPL: OffsetableConcept
inline SBasis operator+(const SBasis & a, double b) {
    if(a.isZero()) return Linear(b, b);
    SBasis result(a);
    result[0] += b;
    return result;
}
inline SBasis operator-(const SBasis & a, double b) {
    if(a.isZero()) return Linear(-b, -b);
    SBasis result(a);
    result[0] -= b;
    return result;
}
inline SBasis& operator+=(SBasis& a, double b) {
    if(a.isZero())
        a = SBasis(Linear(b,b));
    else
        a[0] += b;
    return a;
}
inline SBasis& operator-=(SBasis& a, double b) {
    if(a.isZero())
        a = SBasis(Linear(-b,-b));
    else
        a[0] -= b;
    return a;
}

SBasis shift(SBasis const &a, int sh);
SBasis shift(Linear const &a, int sh);

inline SBasis truncate(SBasis const &a, unsigned terms) {
    SBasis c;
	
	std::vector<Linear>::const_iterator e = a.begin();
	std::advance(e, std::min(terms, (unsigned)a.size()));
	
    //c.insert(c.begin(), a.begin(), a.begin() + std::min(terms, (unsigned)a.size()));
	c.insert(c.begin(), a.begin(), e);
	
    return c;
}

SBasis multiply(SBasis const &a, SBasis const &b);
// This performs a multiply and accumulate operation in about the same time as multiply.  return a*b + c
SBasis multiply_add(SBasis const &a, SBasis const &b, SBasis c);

SBasis integral(SBasis const &c);
SBasis derivative(SBasis const &a);

SBasis sqrt(SBasis const &a, int k);

// return a kth order approx to 1/a)
SBasis reciprocal(Linear const &a, int k);
SBasis divide(SBasis const &a, SBasis const &b, int k);

inline SBasis operator*(SBasis const & a, SBasis const & b) {
    return multiply(a, b);
}

inline SBasis& operator*=(SBasis& a, SBasis const & b) {
    a = multiply(a, b);
    return a;
}

/** Returns the degree of the first non zero coefficient.
 \param a sbasis function
 \param tol largest abs val considered 0
 \returns first non zero coefficient
*/
inline unsigned 
valuation(SBasis const &a, double tol=0){
    unsigned val=0;
    while( val<a.size() &&
           fabs(a[val][0])<tol &&
           fabs(a[val][1])<tol ) 
        val++;
    return val;
}

// a(b(t))
SBasis compose(SBasis const &a, SBasis const &b);
SBasis compose(SBasis const &a, SBasis const &b, unsigned k);
SBasis inverse(SBasis a, int k);
//compose_inverse(f,g)=compose(f,inverse(g)), but is numerically more stable in some good cases...
//TODO: requires g(0)=0 & g(1)=1 atm. generalization should be obvious.
SBasis compose_inverse(SBasis const &f, SBasis const &g, unsigned order=2, double tol=1e-3);

/** Returns the sbasis on domain [0,1] that was t on [from, to]
 \param a sbasis function
 \param from,to interval
 \returns sbasis

*/
inline SBasis portion(const SBasis &t, double from, double to) { return compose(t, Linear(from, to)); }
inline SBasis portion(const SBasis &t, Interval ivl) { return compose(t, Linear(ivl[0], ivl[1])); }

// compute f(g)
inline SBasis
SBasis::operator()(SBasis const & g) const {
    return compose(*this, g);
}
 
inline std::ostream &operator<< (std::ostream &out_file, const Linear &bo) {
    out_file << "{" << bo[0] << ", " << bo[1] << "}";
    return out_file;
}

inline std::ostream &operator<< (std::ostream &out_file, const SBasis & p) {
    for(unsigned i = 0; i < p.size(); i++) {
        out_file << p[i] << "s^" << i << " + ";
    }
    return out_file;
}

// These are deprecated, use sbasis-math.h versions if possible
SBasis sin(Linear bo, int k);
SBasis cos(Linear bo, int k);

std::vector<double> roots(SBasis const & s);
std::vector<std::vector<double> > multi_roots(SBasis const &f,
                                 std::vector<double> const &levels,
                                 double htol=1e-7,
                                 double vtol=1e-7,
                                 double a=0,
                                 double b=1);
    
}
#endif

/*
  Local Variables:
  mode:c++
  c-file-style:"stroustrup"
  c-file-offsets:((innamespace . 0)(inline-open . 0)(case-label . +))
  indent-tabs-mode:nil
  fill-column:99
  End:
*/
// vim: filetype=cpp:expandtab:shiftwidth=4:tabstop=8:softtabstop=4:encoding=utf-8:textwidth=99 :
#endif
