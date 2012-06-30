/*
 *  SimpleCharPool.cpp
 *  Created 7/18/11.
 */

#include "SimpleCharPool.h"
#include <cstring>
#include <fstream>

using namespace std;


//==============================================================================
// Member Function Definitions
//==============================================================================

//------------------------------------------------------------------------------
SimpleCharPool::SimpleCharPool (unsigned initialChars) {
	_beginning = static_cast<char*> (malloc(initialChars));
	_beginning[0] = 0;	// not really sure why I do this, but it seems nice
	_write = _beginning;
	_end = _beginning + initialChars;
}

//------------------------------------------------------------------------------
CharPoolIndex SimpleCharPool::addString (char const* s) { 
   unsigned length = strlen(s)+1;   // we have to count the null terminator
   CharPoolIndex index = alloc(length);
	memcpy(&_beginning[index], s, length);
	return index;
}

//------------------------------------------------------------------------------
/*
 * Doubles the capacity of SimpleCharPool, or adds minChange additional characters
 * (whichever makes it larger).
 */
void SimpleCharPool::resize (unsigned minChange) {
	unsigned capacity = _end - _beginning;
	unsigned filled = _write - _beginning;
	unsigned newcapacity = capacity > minChange ? (capacity<<1) : filled + minChange;
   
	_beginning = static_cast<char*> (realloc(_beginning, newcapacity));
	_write = _beginning + filled;
	_end = _beginning + newcapacity;
}

//------------------------------------------------------------------------------
bool SimpleCharPool::save (char const* filename) const {
   ofstream file(filename, ios_base::binary | ios_base::out | ios_base::trunc);
   if (!file.good())
      return false;

   unsigned s = size();
//   os << s << '\n';
   file.write(reinterpret_cast<char*> (&s), sizeof(unsigned));
   file.write(firstChar(), s);
   return true;
}

//------------------------------------------------------------------------------
bool SimpleCharPool::load (char const* filename) {
   // Open file
   ifstream file(filename, ios_base::binary | ios_base::in);
   if (!file.good())
      return false;

   _write = _beginning;
   unsigned s;
   file.read(reinterpret_cast<char*> (&s), sizeof(unsigned));
   cout << "Size: " << s << " (from Simple Char Pool)" << '\n';
   if (_write + s > _end)
      resize(s);
   file.read(_beginning, s);
   _write = _beginning + s;
   return true;
}
