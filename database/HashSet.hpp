//==============================================================================
/// \file HashSet.h
// created           July      21 2010
// templatized find  September  6 2010
// added Wrap<ITEM>  January   28 2012
//==============================================================================

#ifndef HASH_SET_HPP
#define HASH_SET_HPP

#include <cstdlib>
#include <cstring>
#include <iostream>
#include "Wrap.hpp"
#include "MemoryPool.h"


//==============================================================================
// Theory
//==============================================================================
/*
 * Requirements:
 * ITEM must have a method "unsigned hash() const" that returns its hash,
 * and "==" must be defined for two ITEMs. If you want to use the print method of
 * HashSet, ITEM must also have a method "void print() const".
 *
 * This HashSet can be used for object types or pointer types (ie you can
 * create a HashSet<ITEM> that stores items themselves, or a HashSet<ITEM*>
 * that only stores pointers to items). When you store pointers to items
 * it is smart enough to dereference before comparisons, not return item**
 * when using find, etc.
 *
 * The find methods are templated so that any KEY with KEY.hash() and ITEM == KEY
 * defined, or any KEY that is a pointer to such an object,
 * can be used to look up objects in the HashSet. 
 *
 * Implementation Details:
 * A HashSet has an array of hash bins.
 * A hash bin is a linked list of pointers to items. All of the items
 * in the same hash bin have the same hash. Hashes are unsigned integers.
 * The length of the array of hash bins is always a power of two.
 * Assuming we have 2^n hash bins, to find the bin that a given ITEM should be in,
 * we just need to look at the bottom n bits of its hash. (This is done by
 * taking the bitwise and of the hash with _mask, which is always the number
 * of bins minus 1). When our hash bins start becoming too full, we double
 * the number of hash bins, and start looking at the bottom n+1 bits of
 * each item's hash. (This occurs when the number of items in the HashSet
 * exceeds _trigger.)
 */


//==============================================================================
// Memory Pool Wrappers
//==============================================================================

/*
 * These enable the same HashSet Code to be used with multiple types of Memory Pool.
 */

//------------------------------------------------------------------------------
// Base Class
template<class POOL>
class MPW;

//------------------------------------------------------------------------------
// Wrapper for MemoryPoolF
template<>
struct MPW<MemoryPoolF> {
private:
   MemoryPoolF* _memPoolF;

public:
   MPW (): _memPoolF(nullptr) {}
   void construct (unsigned itemSize, unsigned alignSize, unsigned initialCapacity) {
      _memPoolF = new MemoryPoolF;
      _memPoolF->setItemSize(itemSize, alignSize);
      _memPoolF->setMinFree(1);
      _memPoolF->setNextBlockSize(initialCapacity);
      _memPoolF->setMinDonationSize(initialCapacity >> 1);
   }
   ~MPW () { delete _memPoolF; }
   void* alloc () { return _memPoolF->alloc(); }
   void donate (void* ptr, unsigned size) { _memPoolF->donate(ptr, size); }
   void free (void* ptr) { _memPoolF->free(ptr); }
   void clear () { _memPoolF->clear(); }
};

//------------------------------------------------------------------------------
// Wrapper for MemoryPool
template<>
struct MPW<MemoryPool> {
private:
   MemoryPool* _memPool;
   unsigned _size;

public:
   MPW (): _memPool(nullptr), _size(0) {}
   void construct (unsigned itemSize, unsigned alignSize, unsigned initialCapacity) {
      _memPool = new MemoryPool(itemSize * initialCapacity, alignSize);
   }
   ~MPW () { delete _memPool; }
   void* alloc () { return _memPool->alloc(_size); }
   void donate (void* ptr, unsigned size) { _memPool->donate(ptr, size); }
   void clear () { _memPool->clear(); }
};


//==============================================================================
// Class HashSet<ITEM, POOL>
//==============================================================================

template<class ITEM, class POOL>
class HashSet {
//------------------------------------------------------------------------------
// SubClasses
private:
   /// We don't want to have to write Wrap<ITEM> all the time, so we're renaming it.
   typedef Wrap<ITEM> W;

   /// An object that stores an ITEM, the corresponding hash, and a pointer to another HashNode.
   struct HashNode {
      HashNode* _next;  ///< must be first (see HashSet::resize)
      W _item;          ///< either an ITEM itself of a pointer to one
      unsigned _hash;
      HashNode (HashNode* nextNode, typename W::Ex item, unsigned const& hash)
         : _next(nextNode), _item(item), _hash(hash) {}
   };

//------------------------------------------------------------------------------
// Iterators
public:
   /// Iterates through elements in a HashSet.
   class ConstIterator {
   private:
      HashSet const* _hashSet;       ///< the HashSet that the Iterator is iterating through
      unsigned _currentBin;          ///< the number of the bin the Iterator is iterating through
   protected:
      HashNode const* _currentNode;  ///< the HashNode that the Iterator is currently at
   public:
      ConstIterator (HashSet const& hashSet); ///< Constructor.
      bool valid () const { return _currentNode; } ///< false once everything has been iterated over
      typename W::CRef cref () const { return _currentNode->_item.cref(); }
      typename W::CPtr cptr () const { return _currentNode->_item.cptr(); }
      ConstIterator& operator++ ();       ///< Makes the Iterator point to the next ITEM in the HashSet.
   private:
      void findNextUsedBin ();       ///< Sets _currentBin to the index of the next nonempty bin in _hashSet.
   };
   friend class ConstIterator;

   /// Iterates through elements in a HashSet without allowing changes.
   class Iterator : public ConstIterator {
   public:
      Iterator (HashSet& hashSet): ConstIterator(hashSet) {} ///< Constructor.
      typename W::Ref ref () const { return const_cast<HashNode*>(this->_currentNode)->_item.ref(); }
      typename W::Ptr ptr () const { return const_cast<HashNode*>(this->_currentNode)->_item.ptr(); }
      /// Makes the Iterator point to the next ITEM in the HashSet.
      Iterator& operator++ () { ConstIterator::operator++(); return *this; }
   };

//------------------------------------------------------------------------------
// Member Data
private:
   MPW<POOL> _pool;    ///< memory pool where HashNodes live
   HashNode** _bin;    ///< array of bins
   unsigned _bins;     ///< The length of the _bin array. Always a power of 2.
   unsigned _size;     ///< number of items in the HashSet
   unsigned _mask;     ///< _mask = _bins - 1. _mask & hash gives item's bin number.
   unsigned _trigger;  ///< hash map doubles in size when _size > _trigger
   unsigned _maxNodes; ///< largest number of HashNodes in one bin

//------------------------------------------------------------------------------
// Interface
public:
   HashSet (unsigned initialBins, unsigned initialTrigger = 0);
   ~HashSet ();
   HashSet& operator= (HashSet const& hashSet);

   /// Adds a new item. If the item is already in the HashSet, it returns a reference to the existing item.
   typename W::Ref add (typename W::Ex item);
   /// Returns a pointer to the corresponding item, or a null pointer if the item is not in the HashSet.
   template<class KEY> typename W::CPtr find (KEY const& key) const;
   /// Returns a pointer to the corresponding item, or a null pointer if the item is not in the HashSet.
   template<class KEY> typename W::Ptr  find (KEY const& key) {
      return const_cast<ITEM*>(const_cast<HashSet const*>(this)->find(key));
   }
   // Note: remove can only be called with MemoryPoolF (MemoryPool will not work)
   template<class KEY> bool remove (KEY const& key);
   
   void clear ();          ///< Clears all ITEMs from the HashSet, without changing the number of bins.

   unsigned size () const; ///< Returns the number of items in the HashSet.
   Iterator iterator ();      ///< Returns an Iterator that points to some ITEM in the HashSet.
   ConstIterator constIterator () const; ///< Returns a ConstIterator that points to some ITEM in the HashSet.

   unsigned bins () const { return _bins; }
   void print () const;    /// A printing function for debugging purposes.

// Private Methods
private:
   void resize ();         ///< Doubles the length of _bin (and thus the functional capacity of the HashSet).
};


//==============================================================================
// Public HashSet Methods
//==============================================================================

//------------------------------------------------------------------------------
// Constructor
/**
 * The number of initialBins you pass in is used for two purposes. First, it is
 * used as the initial capacity for the HashSet's MemoryPoolFixed object, and
 * second, it is used to set the initial size of the array of hash bins within
 * HashSet. The latter must be a power of two, while the former can be any
 * nonzero unsigned. So, if initialBins is not a power of two,
 * the initial capacity of the memory pool will be set to initialBins,
 * while the initial size of the has bin array will be set to the smallest
 * power of two larger than initialBins.
 *
 * The parameter initialBins can be thought of practically as the initial
 * capacity of the HashSet. (However, since you can actually squeeze as many
 * items into as few hash bins as you like, it is called by its more
 * technically correct name.)
 *
 * The parameter initialTrigger is the maximum number of items that will be
 * allowed in the HashSet before it resizes itself. If no initialTrigger is
 * supplied, the HashSet will set it equal to initialBins by default.
 */
template<class ITEM, class POOL>
HashSet<ITEM, POOL>::HashSet(unsigned initialBins, unsigned initialTrigger)
   : _size(0), _maxNodes(0)
{
   // _bins cannot be zero because then the first add with fail
   _bins = initialBins ? initialBins : 2;
   // The pointers are the largest members of HashNode, so we want to align to these.
   // This should ensure proper alignment on both 32 and 64 bit systems.
   _pool.construct(sizeof(HashNode), sizeof(HashNode*), _bins);
   //_pool = new MemoryPool(_bins * sizeof(HashNode), sizeof(HashNode*));

   // if _bins is not a nonzero power of 2, round it up to one
   if (_bins & (_bins-1)) {
      _bins |= _bins >> 1;
      _bins |= _bins >> 2;
      _bins |= _bins >> 4;
      _bins |= _bins >> 8;
      _bins |= _bins >> 16;
      ++_bins;
   }

   _mask = _bins - 1;
   _trigger = initialTrigger? initialTrigger : _bins;
   
   // calloc zeros memory
   _bin = (HashNode**) calloc(_bins, sizeof(HashNode*));
   if (!_bin) {
      throw("Could not allocate memory in Geneva::HashSet constructor.");
   }
}

//------------------------------------------------------------------------------
// destructor
template<class ITEM, class POOL>
inline HashSet<ITEM, POOL>::~HashSet ()
{
   free(_bin);
   // pool is implicitly deleted by deletion of MPW<POOL>
   //delete _pool;
}

//------------------------------------------------------------------------------
// Copies a HashSet.
/**
 * Implementation Detail: This method calls add for every ITEM in the copied HashSet.
 * This means that it unnecessarily checks if these ITEMs are already in
 * the set being copied to. A private addNew method would be faster.
 */
template<class ITEM, class POOL>
HashSet<ITEM, POOL>& HashSet<ITEM, POOL>::operator= (HashSet<ITEM, POOL> const& hashSet)
{
   if (_bins != hashSet._bins) {
      _pool.donate(_bin, _bins * sizeof(HashNode*));
      _bin = (HashNode**) calloc(hashSet._bins, sizeof(HashNode*));
      _bins = hashSet._bins;
      _mask = hashSet._mask;
   } else {
      std::memset(_bin, 0, _bins*sizeof(HashNode*));
   }
   _trigger = hashSet._trigger;
   _pool.clear();
   _size = 0;
   
   for (typename HashSet::Iterator itr(hashSet); itr; ++itr) {
      add(*itr);
   }
   
   return *this;
}

//------------------------------------------------------------------------------
// Adds a new item. If the item is already in the HashSet, it returns a reference to the existing item.
template<class ITEM, class POOL>
typename Wrap<ITEM>::Ref HashSet<ITEM, POOL>::add (typename W::Ex item)
{
   // figure out where it should go
   unsigned hash = cref(item).hash();
   unsigned binNumber = hash & _mask;
   HashNode* node = _bin[binNumber];

   // check that it's not already there
   unsigned nodes = 1;  // start at one because we're about to add one
   while (node) {
      if (node->_hash == hash and node->_item.cref() == cref(item))
         return node->_item.ref();
      node = node->_next;
      ++nodes;
   }

   // resize if necessary
   if (++_size > _trigger) {
      resize();
      binNumber = hash & _mask;
   }

   // see if this bin has the most nodes
   if (nodes > _maxNodes) {
      _maxNodes = nodes;
   }
   
   // add a new HashNode
   _bin[binNumber] = new(_pool.alloc()) HashNode(_bin[binNumber], item, hash);
   return _bin[binNumber]->_item.ref();
}

//------------------------------------------------------------------------------
// Returns a pointer to the corresponding item, or a null pointer if the item is not in the HashSet.
/**
 * This function is templatized so that any object with the following properties
 * can be used as a key:
 * 1) properly defines unsigned KEY::hash() const
 * 2) can be compared to an ITEM using ITEM == KEY
 */
template<class ITEM, class POOL>
template<class KEY>
typename Wrap<ITEM>::CPtr HashSet<ITEM, POOL>::find (KEY const& key) const
{
   unsigned hash = cref(key).hash();
   HashNode* node = _bin[hash & _mask];
   while (node) {
      if (node->_hash == hash and node->_item.cref() == cref(key)) {
         return node->_item.ptr();
      }
      node = node->_next;
   }
   return 0;
}

//------------------------------------------------------------------------------
template<class ITEM, class POOL>
template<class KEY>
bool HashSet<ITEM, POOL>::remove (KEY const& key) {
   unsigned hash = cref(key).hash();
   HashNode* node = _bin[hash & _mask];
   if (node->_hash == hash and node->_item.cref() == cref(key)) {
      _bin[hash & _mask] = node->_next;
      _pool.free(node);
      return true;
   }
   HashNode* nextnode = node->_next;
   while (nextnode) {
      if (nextnode->_hash == hash and nextnode->_item.cref() == cref(key)) {
         node->_next = nextnode->_next;
         _pool.free(nextnode);
         return true;
      }
      node = nextnode;
      nextnode = nextnode->_next;
   }
   return false;
}

//------------------------------------------------------------------------------
// Clears all ITEMs from the HashSet, without changing the number of bins.
template<class ITEM, class POOL>
void HashSet<ITEM, POOL>::clear ()
{
   std::memset(_bin, 0, _bins*sizeof(HashNode*));
   _pool.clear();
   _size = 0;
}

//------------------------------------------------------------------------------
// Returns the number of items in the HashSet.
template<class ITEM, class POOL>
inline unsigned HashSet<ITEM, POOL>::size () const
{
   return _size;
}

//------------------------------------------------------------------------------
// Returns an Iterator that points to the "first" ITEM in the HashSet.
/**
 * The iterator will not always point to the same ITEM initially.
 */
template<class ITEM, class POOL>
typename HashSet<ITEM, POOL>::Iterator HashSet<ITEM, POOL>::iterator ()
{
   return Iterator(*this);
}

//------------------------------------------------------------------------------
// Returns a ConstIterator that points to the "first" ITEM in the HashSet.
/**
 * The iterator will not always point to the same ITEM initially.
 */
template<class ITEM, class POOL>
typename HashSet<ITEM, POOL>::ConstIterator HashSet<ITEM, POOL>::constIterator () const
{
   return ConstIterator(*this);
}


//------------------------------------------------------------------------------
// printing function for debugging
template<class ITEM, class POOL>
void HashSet<ITEM, POOL>::print () const
{
   for (unsigned i=0; i<_bins; ++i) {
      HashNode* node = _bin[i];
      std::cout << "Bin " << i << " : ";
      while(node) {
         std::cout << '{' << node->_hash << ", ";
         node->_item.cref().print();
         std::cout << "} ";
         node = node->_next;
      }
      std::cout << '\n';
   }
   std::cout << '\n';
}


//==============================================================================
// Private HashSet Methods
//==============================================================================

//------------------------------------------------------------------------------
// Doubles the length of _bin (and thus the functional capacity of the HashSet).
template<class ITEM, class POOL>
void HashSet<ITEM, POOL>::resize()
{
   unsigned newbins = _bins << 1;
   HashNode** newbin = (HashNode**) malloc(newbins * sizeof(HashNode*));
   HashNode* node;
   HashNode* high;
   HashNode* low;
   for (unsigned i=0; i<_bins; ++i) {
      node = _bin[i];
      // Makes high and low point to the pointers to the first HashNodes in their bins.
      // This is why _next must be the first item in HashNode.
      // (This allows us to treat a HashNode* as a HashNode, if
      // we only want the HashNode's _next pointer. We could
      // get rid of this requirement if we add extra ifs or
      // use a HashNode**, but these take longer to write
      // and longer to run.)
      high = (HashNode*) &newbin[i+_bins];
      low  = (HashNode*) &newbin[i];
      while (node) {
         if (_bins & (node->_hash)) {
            high->_next = node;
            high = node;
         } else {
            low->_next = node;
            low = node;
         }
         node = node->_next;
      }
      high->_next = 0;
      low->_next  = 0;
   }

   _pool.donate(_bin, sizeof(HashNode*) * _bins);
   _bin = newbin;
   _bins = newbins;
   _mask = _bins-1;
   _trigger <<= 1;   
}


//==============================================================================
// Iterator Methods
//==============================================================================

/**
 * \class HashSet::Iterator
 *
 * Elements must not be added to the HashSet while iteration is taking place.
 * (Doing otherwise results in undefined behavior, as the iterator could
 * (and would likely) end up pointing to an invalid memory location.)
 *
 * Calling any methods besides "operator ITEM*" or "++" on
 * invalid iterators (i.e. those created on empty HashSets, or those that have
 * been incremented past the end of their HashSets) will result in a bad
 * memory access. This shouldn't be a problem, because Iterators can be
 * implicitly typecast to booleans (via "operator ITEM*"), such that
 * valid iterators return true and invalid iterators return false.
 *
 * To iterate through all ITEMS in a HashSet<ITEM>& hashSet,
 * use for (HashSet<ITEM>::Iterator itr(hashSet); itr; ++itr).
 */

//------------------------------------------------------------------------------
// Constructor
template<class ITEM, class POOL>
HashSet<ITEM, POOL>::ConstIterator::ConstIterator (HashSet const& hashSet)
: _hashSet(&hashSet), _currentBin(0), _currentNode(0)
{
   findNextUsedBin();
   if (_currentBin < _hashSet->_bins)
      _currentNode = _hashSet->_bin[_currentBin];
}

//------------------------------------------------------------------------------
// Makes the Iterator point to the next ITEM in the HashSet.
template<class ITEM, class POOL>
typename HashSet<ITEM, POOL>::ConstIterator& HashSet<ITEM, POOL>::ConstIterator::operator++ ()
{
   if (_currentNode->_next) {
      _currentNode = _currentNode->_next;
   } else {
      ++_currentBin;
      findNextUsedBin();
      if (_currentBin == _hashSet->_bins) {
         _currentNode = 0;
      } else {
         _currentNode = _hashSet->_bin[_currentBin];
      }
   }
   return *this;
}

//------------------------------------------------------------------------------
// Sets _currentBin to the index of the next nonempty bin in _hashSet.
/**
 *If there are no more nonempty bins, _currentBin will be equal to _hashSet->_bins.
 */
template<class ITEM, class POOL>
void HashSet<ITEM, POOL>::ConstIterator::findNextUsedBin ()
{
   while ( (_currentBin < _hashSet->_bins) and !(_hashSet->_bin[_currentBin]) )
      ++_currentBin;
}


#endif // HASH_SET_HPP
