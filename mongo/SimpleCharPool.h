//==============================================================================
// SimpleCharPool.h
// Created 7/18/11.
//==============================================================================

#ifndef ESSTDLIB_SIMPLE_CHAR_POOL
#define ESSTDLIB_SIMPLE_CHAR_POOL

#include <cstdlib>
#include <iostream>


//==============================================================================
// SimpleCharPool
//==============================================================================

/*
 * Stores many C strings contiguously. (Each is null terminated.)
 * Hands out unsigned indeces into itself, that the user must keep track of.
 * Strings cannot be modified once they are stored (if you changed the length
 * you could write over the next string).
 */

typedef unsigned CharPoolIndex;

class SimpleCharPool {
private:
	char* _beginning;	 // first character in the SimpleCharPool
	char* _write;		 // where next string will start
	char* _end;        // one past the last character
   
public:
	SimpleCharPool (unsigned initialChars);
	inline ~SimpleCharPool ();
   
   char const* firstChar () const { return _beginning; }
   unsigned size () const { return _write - _beginning; }

	CharPoolIndex addString (char const* s);
	char const* getString (CharPoolIndex index) const { return _beginning + index; }
   // be very careful you don't overwrite things!
   char* editString (CharPoolIndex index) { return _beginning + index; }

   void clear () { _write = _beginning; }
   
   bool save (char const* filename) const;
   bool load (char const* filename);

private:
   // size must include the space needed for the null terminator
   inline CharPoolIndex alloc (unsigned size);
	void resize (unsigned minChange);   
};


//==============================================================================
// Inline Member Function Definitions
//==============================================================================

//------------------------------------------------------------------------------
SimpleCharPool::~SimpleCharPool () {
   free(_beginning);
}

//------------------------------------------------------------------------------
CharPoolIndex SimpleCharPool::alloc (unsigned size) {
	if (_write + size > _end)
      resize(size);
	CharPoolIndex index = _write - _beginning;
	_write += size;
	return index;
}



#endif // ESSTDLIB_SIMPLE_CHAR_POOL
