//==============================================================================
// LinkedList.hpp
// Created 1/28/12.
//==============================================================================

#ifndef LINKED_LIST_HPP
#define LINKED_LIST_HPP

#include "Wrap.hpp"
#include "MemoryPool.h"

#include <iostream>

//==============================================================================
// Struct LinkedList<ITEM>
//==============================================================================

template<typename ITEM>
class LinkedList {
// SubClasses
public:
   struct Link {
      Link* _next;
      Wrap<ITEM> _item;
      Link () {}
      Link (Link* next, typename Wrap<ITEM>::Ex item): _next(next), _item(item) {}
   };
   
// Iterators
public:
   class Iterator {
   private:
      Link* _current;
      
   public:
      Iterator (): _current(0) {}
      Iterator (Link* start): _current(start) {}
      bool valid () const { return _current; }
      bool last  () const { return !(_current->_next); }
      Iterator& operator++ () { _current = _current->_next; return *this; }
      typename Wrap<ITEM>::Ref  ref  () const { return _current->_item.ref(); }
      typename Wrap<ITEM>::CRef cref () const { return _current->_item.cref(); }
      typename Wrap<ITEM>::Ptr  ptr  () const { return _current->_item.ptr(); }
      typename Wrap<ITEM>::CPtr cptr () const { return _current->_item.cptr(); }
      Link* current () const { return _current; }
   };
   friend class Iterator;

   class ConstIterator {
   private:
      Link* _current;
      
   public:
      ConstIterator (): _current(0) {}
      ConstIterator (Link* start): _current(start) {}
      bool valid () const { return _current; }
      bool last  () const { return !(_current->_next); }
      ConstIterator& operator++ () { _current = _current->_next; return *this; }
      typename Wrap<ITEM>::CRef cref () const { return _current->_item.cref(); }
      typename Wrap<ITEM>::CPtr cptr () const { return _current->_item.cptr(); }
      Link const* current () const { return _current; }
   };
   friend class ConstIterator;

// Members
private:
   Link* _first;     // Must be first, so it looks like a Link 
   unsigned _items;

// Interface
public:
   // Constructor
   LinkedList (): _first(0), _items(0) {}

   // Destructor
   ~LinkedList ();

   // Returns the number of items in the LinkedList.
   unsigned items () const { return _items; }
   // Returns the first item.
   typename Wrap<ITEM>::T first () { return _first->_item.t(); }

   // Iterators
   Iterator iterator () { return Iterator(_first); }
   ConstIterator constIterator () const { return ConstIterator(_first); }
   
   // Adds an ITEM
   inline void add (typename Wrap<ITEM>::Ex item);
   // Adds an ITEM, using the provided MemoryPool as an allocator.
   inline void add (typename Wrap<ITEM>::Ex item, MemoryPool& pool);
   // Adds a Link that you have constructed yourself.
   inline void addLink (Link* link);

   // Note: do not use any of these methods within a MemoryPool!
   // Removes the link following parent.
   inline typename Wrap<ITEM>::T removeNext (Link* parent);
   inline typename Wrap<ITEM>::T removeNext (Iterator const& itr) { return removeNext(itr.current()); }
   // Removes the first link.
   inline typename Wrap<ITEM>::T removeFirst ();
   // Removes all links.
   void removeAll ();
   
   // Leaks the link following parent.
   inline void leakNext (Link* parent);
   inline void leakNext (Iterator const& itr) { leakNext(itr.current()); }
   // Leaks the first link.
   inline void leakFirst ();
   // Leaks all the tags.
   inline void leakAll ();
};


//==============================================================================
// Inline Method Definitions
//==============================================================================

//------------------------------------------------------------------------------
// Destructor
template<typename ITEM>
LinkedList<ITEM>::~LinkedList () {
   Link* kill = _first;
   Link* temp;
   while (kill) {
      temp = kill->_next;
      delete kill;
      kill = temp;
   }
}

//------------------------------------------------------------------------------
// Adds an ITEM
template<typename ITEM>
void LinkedList<ITEM>::add (typename Wrap<ITEM>::Ex item) {
   _first = new Link(_first, item);
   ++_items;
}

//------------------------------------------------------------------------------
// Adds an ITEM, using the provided MemoryPool as an allocator.
template<typename ITEM>
void LinkedList<ITEM>::add (typename Wrap<ITEM>::Ex item, MemoryPool& pool) {
   _first = new(pool.alloc(sizeof(Link))) Link(_first, item);
   ++_items;
}

//------------------------------------------------------------------------------
// Adds a Link that you have constructed yourself.
template<typename ITEM>
void LinkedList<ITEM>::addLink (Link* link) {
   link->_next = _first;
   _first = link;
   ++_items;
}

//------------------------------------------------------------------------------
// Removes the link following parent
template<typename ITEM>
typename Wrap<ITEM>::T LinkedList<ITEM>::removeNext (Link* parent) {
   Link* temp = parent->_next;
   typename Wrap<ITEM>::T item = temp->_item.t();
   parent->_next = temp->_next;
   delete temp;
   --_items;
   return item;
}

//------------------------------------------------------------------------------
template<typename ITEM>
typename Wrap<ITEM>::T LinkedList<ITEM>::removeFirst () {
   return removeNext(reinterpret_cast<Link*> (this));
}

//------------------------------------------------------------------------------
template<typename ITEM>
void LinkedList<ITEM>::removeAll () {
   Link* link1 = _first;
   Link* link2;
   for (unsigned i=0; i<_items; ++i) {
      link2 = link1->_next;
      delete link1;
      link1 = link2;
   }
   _first = 0;
   _items = 0;
}

//------------------------------------------------------------------------------
// Leaks the link following parent, but doesn't delete it.
// This causes a memory leak, unless everything is in a MemoryPool.
template<typename ITEM>
void LinkedList<ITEM>::leakNext (Link* parent) {
   parent->_next = parent->_next->_next;
   --_items;
}

//------------------------------------------------------------------------------
template<typename ITEM>
void LinkedList<ITEM>::leakFirst () {
   leakNext(reinterpret_cast<Link*> (this));
}

//------------------------------------------------------------------------------
template<typename ITEM>
void LinkedList<ITEM>::leakAll () {
   _first = 0;
   _items = 0;
}


//==============================================================================
// Method Definitions
//==============================================================================


#endif // LINKED_LIST_HPP
