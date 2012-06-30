//==============================================================================
// Hypergrid.hPP
// Created 1/29/12.
//==============================================================================

#ifndef HYPERGRID_HPP
#define HYPERGRID_HPP

#include <iostream>

/*
 * If N = 1 conversion between HyperIndex and HyperCoords will fail
 * (by shifting off the edge of the integer).
 * N = 1 is silly though because then you just have a single number.
 */


//==============================================================================
// Class Definitions
//==============================================================================

typedef long unsigned GridInt;
const GridInt One = 1ul;

//------------------------------------------------------------------------------
// A hyper index in N dimensions.
template<unsigned N>
struct HyperIndex {
   // The number of Cuts we can store in N dimensions in one GridInt.
   static const unsigned Cuts = 8*sizeof(GridInt) / N;
   
   GridInt _index;
   
   HyperIndex (GridInt i = 0): _index(i) {}
   
   // The index is in this half of the ith cut of dimension d.
   inline unsigned getCut (unsigned i, unsigned d);
   
   // Gives info about the ith cut for all dimensions.
   inline unsigned getSubIndex (unsigned i);
   
   // Moves the index to the specified half of the ith cut of dimension d.
   inline void setCut (unsigned i, unsigned d, unsigned half);
   
   // Sets the subIndex of the ith cut for all dimensions
   inline void setSubIndex (unsigned i, unsigned subIndex);
   
   // Automatic typecast to GridInt
   operator GridInt () { return _index; }
};

//------------------------------------------------------------------------------
// A set of hyper coordinates in N dimensions.
/*
 * Coordinates are still stored in one GridInt.
 */
template<unsigned N>
struct HyperCoords {
   GridInt _coords;
   
   // Default Constructor
   HyperCoords () {}

   // Sets each coordinate to u
   HyperCoords (unsigned u);
   
   inline unsigned getCoord (unsigned d) const;
   inline void setCoord (unsigned d, unsigned u);
   inline void zero ();
   inline HyperIndex<N> index () const;
};

//------------------------------------------------------------------------------
// A box in N-dimensional hyper coordinate space.
template<unsigned N>
struct HyperBox {
   HyperCoords<N> _min;    // minimums of box (inclusive)
   HyperCoords<N> _max;    // maximums of box (inclusive)
   
   inline HyperBox (HyperCoords<N> min, HyperCoords<N> max);
   inline HyperBox (HyperIndex<N> min, HyperIndex<N> max);
   
   bool intersects (HyperBox<N> const& box) const;
   bool contains (HyperCoords<N> const& coords) const;
};


//==============================================================================
// Global Method Declarations
//==============================================================================

//------------------------------------------------------------------------------
// Returns the hyperindex of a set of hyper coordinates.
template<unsigned N>
HyperIndex<N> getIndex (HyperCoords<N> coords);

//------------------------------------------------------------------------------
// Returns the hyper coordinates of a hyper index.
template<unsigned N>
HyperCoords<N> getCoords (HyperIndex<N> index);

//------------------------------------------------------------------------------
// Prints a HyperIndex.
template<unsigned N>
std::ostream& operator<< (std::ostream& os, HyperIndex<N> index);

//------------------------------------------------------------------------------
// Prints a HyperCoord like this: (x_1, x_2, ..., x_N)
template<unsigned N>
std::ostream& operator<< (std::ostream& os, HyperCoords<N> coords);


//==============================================================================
// Class Method Definitions
//==============================================================================

//------------------------------------------------------------------------------
// The index is in this half of the ith cut of dimension d.
template<unsigned N>
unsigned HyperIndex<N>::getCut (unsigned i, unsigned d) {
   GridInt  mask  = One;
   unsigned shift = N*(Cuts-1u) - N*i + d;
   return (_index & (mask << shift)) >> shift;
}

// Gives info about the ith cut for all dimensions.
template<unsigned N>
unsigned HyperIndex<N>::getSubIndex (unsigned i) {
   GridInt  mask  = (One << N) - One;
   unsigned shift = N*(Cuts-1u) - N*i;
   return (_index & (mask << shift)) >> shift;
}

// Moves the index to the specified half of the ith cut of dimension d.
template<unsigned N>
void HyperIndex<N>::setCut(unsigned i, unsigned d, unsigned half) {
   GridInt  mask  = One;
   unsigned shift = N*(Cuts-1u) - N*i + d;
   _index &= ~(mask << shift);
   _index |= static_cast<GridInt>(half) << shift;
}

// Sets the subIndex of the ith cut for all dimensions
template<unsigned N>
void HyperIndex<N>::setSubIndex (unsigned i, unsigned subIndex) {
   GridInt  mask  = (One << N) - One;
   unsigned shift = N*(Cuts-1u) - N*i;
   _index &= ~(mask << shift);
   _index |= static_cast<GridInt>(subIndex) << shift;
}


//------------------------------------------------------------------------------
template<unsigned N>
HyperCoords<N>::HyperCoords (unsigned u) {
   for (unsigned d=0; d<N; ++d) {
      setCoord(d, u);
   }
}

template<unsigned N>
unsigned HyperCoords<N>::getCoord (unsigned d) const {
   GridInt  mask  = (One << HyperIndex<N>::Cuts) - One;
   unsigned shift = d * HyperIndex<N>::Cuts;
   return ( _coords & (mask << shift) ) >> shift;
}

template<unsigned N>
void HyperCoords<N>::setCoord (unsigned d, unsigned u) {
   GridInt  mask  = (One << HyperIndex<N>::Cuts) - One;
   unsigned shift = d*HyperIndex<N>::Cuts;
   _coords &= ~ (mask << shift);
   _coords |= static_cast<GridInt>(u) << shift;
}

template<unsigned N>
void HyperCoords<N>::zero () {
   _coords = 0;
}

template<unsigned N>
HyperIndex<N> HyperCoords<N>::index () const {
   return getIndex(*this);
}

//------------------------------------------------------------------------------
template<unsigned N>
HyperBox<N>::HyperBox (HyperCoords<N> min, HyperCoords<N> max)
: _min(min), _max(max)
{}

template<unsigned N>
HyperBox<N>::HyperBox (HyperIndex<N> min, HyperIndex<N> max)
: _min(getCoords(min)), _max(getCoords(max))
{}

template<unsigned N>
bool HyperBox<N>::intersects (HyperBox<N> const& box) const {
   for (unsigned d=0; d<N; ++d) {
      if (_max.getCoord(d) < box._min.getCoord(d) or _min.getCoord(d) > box._max.getCoord(d))
         return false;
   }
   return true;
}

template<unsigned N>
bool HyperBox<N>::contains (HyperCoords<N> const& coords) const {
   for (unsigned d=0; d<N; ++d) {
      if (coords.getCoord(d) < _min.getCoord(d) or _max.getCoord(d) < coords.getCoord(d))
         return false;
   }
   return true;
}


//==============================================================================
// Global Method Definitions
//==============================================================================

//------------------------------------------------------------------------------
// Returns the hyper index of a set of hyper coordinates.
template<unsigned N>
HyperIndex<N> getIndex (HyperCoords<N> coords) {
   // Recursively looks at quadrants.
   HyperIndex<N> index;
   unsigned shift;
   for (unsigned i=0; i<HyperIndex<N>::Cuts; ++i) {
      shift = HyperIndex<N>::Cuts - 1u - i;
      for (unsigned d=0; d<N; ++d) {
         index.setCut(i, d, ( coords.getCoord(d) & (1u << shift) ) >> shift);
      }
   }
   return index;
}

//------------------------------------------------------------------------------
// Returns the hyper coordinates of a hyper index.
template<unsigned N>
HyperCoords<N> getCoords (HyperIndex<N> index) {
   HyperCoords<N> coords;
   coords.zero();
   unsigned offset;
   unsigned x;
   for (unsigned d=0; d<N; ++d) {
      x = 0;
      for (unsigned i=0; i<HyperIndex<N>::Cuts; ++i) {
         offset = HyperIndex<N>::Cuts - 1u - i;
         x |= index.getCut(i, d) << offset;
      }
      coords.setCoord(d, x);
   }
   return coords;
}

//------------------------------------------------------------------------------
template<unsigned N>
std::ostream& operator<< (std::ostream& os, HyperIndex<N> index) {
   os << index._index;
   return os;
}

//------------------------------------------------------------------------------
template<unsigned N>
std::ostream& operator<< (std::ostream& os, HyperCoords<N> coords) {
   os << '(';
   for (unsigned i=0; i<N-1u; ++i) {
      os << coords.getCoord(i) << ", ";
   }
   os << coords.getCoord(N-1u);
   os << ')';
   return os;
}


#endif // HYPERGRID_HPP
