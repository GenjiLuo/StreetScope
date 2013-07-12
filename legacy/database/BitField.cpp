//==============================================================================
// BitField.cpp
// Created May 17 2012
//==============================================================================

#include "BitField.h"
#include <cstring>
#include <iostream>

using namespace std;


//==============================================================================
// Set Constants
//==============================================================================

//------------------------------------------------------------------------------
// We assume 32 bit integers (2^5 == 32).
// If this is not true then BitField may break
// unless these are changed accordingly.
const unsigned BitField::_shift = 5;
const unsigned BitField::_mask = 31;


//==============================================================================
// Public Method Definitions
//==============================================================================

//------------------------------------------------------------------------------
unsigned BitField::accomodate (unsigned bits) {
   unsigned words = wordsForBits(bits);
   if (words > _words) {
      resize(bits, words);
   }
   return words;
}

//------------------------------------------------------------------------------
void BitField::zero () {
   memset(_data, 0, _words * sizeof(unsigned));
}

//------------------------------------------------------------------------------
unsigned BitField::shrink () {
   unsigned words = usedWords();
   if (words < _words) {
      _data = static_cast<unsigned*> ( realloc(_data, words * sizeof(unsigned)) );
      _words = words;
   }
   return words;
}

//------------------------------------------------------------------------------
BitField& BitField::operator= (BitField const& ex) {
   unsigned words = accomodate(ex.bits());
   memcpy(_data, ex._data, words * sizeof(unsigned));
   return *this;
}

//------------------------------------------------------------------------------
BitField& BitField::operator= (BitField && ex) {
   delete _data;
   _bits = ex._bits;
   _words = ex._words;
   _data = ex._data;
   ex._data = nullptr;
   return *this;
}

//------------------------------------------------------------------------------
void BitField::swap (unsigned i, unsigned j) {
   unsigned temp = get(i);
   set(i, get(j));
   set(j, temp);
}

//------------------------------------------------------------------------------
void BitField::save (ofstream& file) const {
   file << _bits << '\n';
   for (unsigned i=0; i<_bits; ++i)
      file << (get(i) ? '1' : '0');
}

//------------------------------------------------------------------------------
void BitField::read (ifstream& file) {
   unsigned bits;
   file >> bits;
   accomodate(bits);
   zero();
   file.ignore(1);   // ignore newline
   char temp;
   for (unsigned i=0; i<bits; ++i) {
      file >> temp;
      if (temp == '1')
         set(i);
   }
}

//------------------------------------------------------------------------------
void BitField::print () const {
   unsigned setbits = 0;
   for (unsigned i=0; i<_bits; ++i) {
      cout << i << ": " << get(i) << '\n';
      if (get(i)) ++setbits;
   }
   cout << "Total set bits:   " << setbits << '\n';
   cout << "Total unset bits: " << _bits - setbits << '\n';
   cout << "Total bits: " << _bits << '\n';
}

//------------------------------------------------------------------------------
BitField& BitField::operator&= (BitField const& bf) {
   for (unsigned i=0; i<_words; ++i) {
      _data[i] &= bf._data[i];
   }
   return *this;
}

//------------------------------------------------------------------------------
BitField& BitField::operator|= (BitField const& bf) {
   for (unsigned i=0; i<_words; ++i) {
      _data[i] |= bf._data[i];
   }
   return *this;
}

//------------------------------------------------------------------------------
bool BitField::operator== (BitField const& bf) {
   // Sizes must agree.
   if (bits() != bf.bits())
      return false;

   // All bits in used words must agree.
   unsigned words = usedWords();
   for (unsigned i=0; i<words; ++i)
      if (_data[i] != bf._data[i])
         return false;

   // If we've made it this far they're the same.
   return true;
}

//==============================================================================
// Private Method Definitions
//==============================================================================

//------------------------------------------------------------------------------
void BitField::resize (unsigned bits, unsigned words) {
   // resize data
   _words = words;
   if (_data)
      delete[] _data;
   _data = new unsigned[_words];
   _bits = bits;

   // zero unused bits
   words = usedWords();
   unsigned unusedBits = (words << _shift) - _bits;
   unsigned mask = ~((1 << unusedBits) - 1);
   _data[words-1] &= mask;
}

//------------------------------------------------------------------------------
unsigned BitField::indexOfFirstSet () const {
   // find the first word with set bits
   unsigned i = 0;
   unsigned words = usedWords();
   while (i < words) {
      if (_data[i] != 0)
         break;
      ++i;
   }

   // if there is no word with set bits, return _bits
   if (i == words)
      return _bits;

   // index of first bit in the word we just found
   i <<= _shift;
   while (i<_bits and !get(i))
      ++i;
   return i;
}

//------------------------------------------------------------------------------
unsigned BitField::indexOfLastSet () const {
   // find the last word with set bits
   unsigned i = usedWords() - 1;
   while (i > 0) {
      if (_data[i] != 0)
         break;
      --i;
   }

   // if there is no word with set bits, return _bits
   if (i == 0 and _data[0] == 0)
      return _bits;

   // index of last bit in the word we just found
   i <<= _shift;
   i += _mask;
   while (i>0 and !get(i))
      --i;
   return i;
}


//==============================================================================
// Global Method Definitions
//==============================================================================

//------------------------------------------------------------------------------
BitField operator& (BitField const& bf1, BitField const& bf2) {
   BitField temp(bf1);
   return temp &= bf2;
}

//------------------------------------------------------------------------------
BitField operator| (BitField const& bf1, BitField const& bf2) {
   BitField temp(bf1);
   return temp |= bf2;
}

