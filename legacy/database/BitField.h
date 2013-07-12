//==============================================================================
// BitField.h
// Created May 17 2012
//==============================================================================

#ifndef ESTDLIB_BITFIELD
#define ESTDLIB_BITFIELD

#include <fstream>
#include <iostream>


//==============================================================================
// Class BitField
//==============================================================================

//------------------------------------------------------------------------------
/*
 * A BitField is a vector of unsigneds, each of whose individual bits
 * may be accessed as though they were separate boolean variables. 
 * Bits are numbered such that the first bit of each word is the least significant bit.
 *
 * Bitwise operations are defined. In all cases the second BitField must be at
 * least as long as the third, or you will access invalid memory. (The second
 * being the rhs in &= etc, and the second argument for & etc.)
 */

//------------------------------------------------------------------------------
class BitField {

//------------------------------------------------------------------------------
// Members
private:
   unsigned _bits;      // number of bits stored in the BitField
   unsigned _words;     // lenth of _data
   unsigned* _data;     // note _data may be longer than necessary
   static const unsigned _shift;
   static const unsigned _mask;

//------------------------------------------------------------------------------
// Iterator Classes
public:
   // to iterate over all bits in the BitField, use for (BitField::Itr itr(bitfield); itr.valid(); ++itr)
   class CItr {
   protected:
      BitField const& _bitField; // the BitField being iterated over
      unsigned _i;               // current index
   public:
      // creates a CItr initialized to the specified bit
      CItr (BitField const& bitField, unsigned i = 0): _bitField(bitField), _i(i) {}
      CItr& operator++ () { ++_i; return *this; }
      CItr& nextSet () { do { ++(*this); } while (valid() and !get()); return *this; }
      CItr& nextUnset () { do { ++(*this); } while (valid() and get()); return *this; }
      CItr& firstSet ();
      CItr& lastSet ();
      bool valid () const { return _i < _bitField.bits(); }
      unsigned get () const { return _bitField.get(_i); }
      unsigned i () const { return _i; }
   };

   class Itr : public CItr {
   public:
      Itr (BitField& bitField): CItr(bitField) {}
      Itr (BitField& bitField, unsigned i): CItr(bitField, i) {}
      Itr& operator++ () { CItr::operator++(); return *this; }
      Itr& nextSet () { CItr::nextSet(); return *this; }
      // read and write are not legal if iterator is past end - caller must check this
      void unset () { const_cast<BitField&>(_bitField).unset(_i); }
      void set   () { const_cast<BitField&>(_bitField).set(_i); }
      void set (unsigned value) { const_cast<BitField&>(_bitField).set(_i, value); }
   };

//------------------------------------------------------------------------------
// Public Methods
public:
   //---------------------------------------------------------------------------
   // Ctors and Dtors
   BitField (): _bits(0), _words(0), _data(nullptr) {}
   BitField (unsigned bits): _words(0), _data(nullptr) { resize(bits); }
   BitField (BitField const& bf): BitField() { *this = bf; }
   BitField (BitField && bf): BitField() { *this = bf; }
   ~BitField () { delete[] _data; }

   //---------------------------------------------------------------------------
   // Memory Management
   // Does not test anything or zero - simply resizes.
   // Would be more efficient to have a resizeAndZero method when this is desired
   void resize (unsigned bits) { resize(bits, wordsForBits(bits)); }
   // Ensures the BitField can hold at least bits bits. Does not copy data, does not zero.
   unsigned accomodate (unsigned bits);
   void zero ();
   // Frees unused memory, if any exists.
   unsigned shrink ();

   //---------------------------------------------------------------------------
   // Assignment
   BitField& operator= (BitField const& ex);
   BitField& operator= (BitField && ex);

   //---------------------------------------------------------------------------
   // Basic Interaction
   unsigned get (unsigned i) const { return (_data[i >> _shift] >> (i & _mask)) & 0x1; }
   void unset (unsigned i) { _data[i >> _shift] &= ~(0x1 << (i & _mask)); }
   void set   (unsigned i) { _data[i >> _shift] |=  (0x1 << (i & _mask)); }
   inline void set (unsigned i, unsigned value);
   void swap (unsigned i, unsigned j);

   unsigned bits () const { return _bits; }
   unsigned words () const { return _words; }
   unsigned usedWords () const { return wordsForBits(_bits); }
   
   void save (std::ofstream& file) const;
   void read (std::ifstream& file);
   void print () const;

   //---------------------------------------------------------------------------
   // Iterators
   Itr  itr  ()       { return  Itr(*this); }
   CItr citr () const { return CItr(*this); }
   Itr  firstSet  ()       { return  Itr(*this, indexOfFirstSet()); }
   Itr  lastSet   ()       { return  Itr(*this, indexOfLastSet());  }
   CItr cFirstSet () const { return CItr(*this, indexOfFirstSet()); }
   CItr cLastSet  () const { return CItr(*this, indexOfLastSet());  }
         
   //---------------------------------------------------------------------------
   // Bitwise Operations
   BitField& operator&= (BitField const& bf);
   BitField& operator|= (BitField const& bf);

   //---------------------------------------------------------------------------
   // Logical Operations
   bool operator== (BitField const& bf);
   bool operator!= (BitField const& bf) { return !(*this == bf); }

   //---------------------------------------------------------------------------
   // Static Methods
   static unsigned charsForBits (unsigned bits) { return (bits + 7) >> 3; }
   static unsigned wordsForBits (unsigned bits) { return (bits + _mask) >> _shift; }

//------------------------------------------------------------------------------
// Private Methods
private:
   // words must be large enough to fit specified number of bits (can be larger)
   void resize (unsigned bits, unsigned words);

   // if there are no set bits these methods return _bits
   unsigned indexOfFirstSet () const;
   unsigned indexOfLastSet () const;
};


//==============================================================================
// Global Method Declarations
//==============================================================================

BitField operator& (BitField const& bf1, BitField const& bf2);
BitField operator| (BitField const& bf1, BitField const& bf2);


//==============================================================================
// Inline Method Definitions
//==============================================================================

//------------------------------------------------------------------------------
void BitField::set (unsigned i, unsigned value) {
   unsigned word = i >> 5;
   unsigned bit  = i & 31;
   _data[word] &= ~(0x1 << bit);
   _data[word] |= (value & 0x1) << bit;
}


#endif // ESTDLIB_BITFIELD
