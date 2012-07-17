//==============================================================================
// Random.h
// Created 4/26/12.
//==============================================================================

#ifndef ESTDLIB_RANDOM
#define ESTDLIB_RANDOM


//------------------------------------------------------------------------------
// Fixed length types
// these should be fixed to work for both i386 and x86_64!
typedef long long unsigned u_64;
typedef unsigned u_32;
typedef double f_64;
void checkSizes ();


//==============================================================================
// Uniform Random Number Generators
//==============================================================================

//------------------------------------------------------------------------------
class XorShift32 {
private:
   u_32 _x, _y;

public:
   XorShift32 (u_64 seed) { setState(seed); }
   XorShift32 (u_32 lowSeed, u_32 highSeed) { setState(lowSeed, highSeed); }
   XorShift32 (): XorShift32(0, 0) {}
   void setState (u_64 seed);
   void setState (u_32 lowSeed, u_32 highSeed);
   void state (unsigned& x, unsigned& y) const { x = _x; y = _y; }
   // these functions automatically increment state
   u_32 u32 () { next(); return _y; }
   f_64 f64 (); //return uniform random double in [0.0, 1.0)

   // moves to next state
   XorShift32& next ();
   // this does not change the state - it returns the same number until next is called
   u_32 const_u32 () { return _y; }
};

//------------------------------------------------------------------------------
class CombinedGen1 {
private:
   u_64 _u, _v, _w;
   
public:
   inline CombinedGen1 (u_64 seed);
   u_64 u64 ();
   f_64 f64 () { return 5.42101086242752217E-20 * u64(); } 
   u_32 u32 () { return u_32(u64()); }
};

//------------------------------------------------------------------------------
class XorShift64 {
private:
   u_64 _x;
   static const unsigned a1 = 21;
   static const unsigned a2 = 35;
   static const unsigned a3 = 4;   
   
public:
   XorShift64 (u_64 seed): _x(seed) {}
   inline XorShift64& next ();
   inline XorShift64& next2 ();
   inline u_64 u64 () const { return _x; }
   inline unsigned low32  () const { return static_cast<unsigned> (_x); }
   inline unsigned high32 () const { return static_cast<unsigned> (_x >> 32); }
};

//------------------------------------------------------------------------------
// Multiplicative Linear Congruential Generator
class MLCG {
private:
   u_64 _x;
   static const u_64 a = 2685821657736338717LL;
   
public:
   MLCG (u_64 seed): _x(seed) {}
   inline MLCG& next () { _x = a * _x; return *this; }
   inline unsigned high32 () const { return static_cast<unsigned> (_x >> 32); }
};


//==============================================================================
// Non-Uniform Random Number Generators
//==============================================================================

class NormalGen {
private:
   XorShift32* _unif;
   double _mean, _var;

public:
   NormalGen(XorShift32* uniformGenerator, double mean=0.0, double variance=1.0)
      :_unif(uniformGenerator), _mean(mean), _var(variance) {}
   double next ();
};


//==============================================================================
// Inline Method Definitions
//==============================================================================

//------------------------------------------------------------------------------
XorShift64& XorShift64::next () {
   _x = _x ^ (_x >> a1);
   _x = _x ^ (_x << a2);
   _x = _x ^ (_x >> a3);
   return *this;
}

//------------------------------------------------------------------------------
XorShift64& XorShift64::next2 () {
   _x = _x ^ (_x << a1);
   _x = _x ^ (_x >> a2);
   _x = _x ^ (_x << a3);
   return *this;
}

#endif // ESTDLIB_RANDOM
