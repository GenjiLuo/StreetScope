//==============================================================================
// Wrap.hpp
// Created 2012.1.29
//==============================================================================

#ifndef WRAP_HPP
#define WRAP_HPP


//==============================================================================
// Class Wrap<ITEM>
//==============================================================================
/*
 * Wraps the objects you store in estlib template container classes,
 * so that object and pointer to object versions don't need to be specialized.
 *
 * ITEM can be an object or a pointer to one. Either way,
 * Wrap<ITEM>::T    specifies what the type of ITEM really is,
 * Wrap<ITEM>::Ex   specifies an example of an object (ie const& or *),
 * Wrap<ITEM>::Ref  specifies a reference to an object,
 * Wrap<ITEM>::CRef specifies a reference to a const object,
 * Wrap<ITEM>::Ptr  specifies a pointer to an object, and
 * Wrap<ITEM>::CPtr specifies a pointer to a const object.
 *
 * ToDo: Make sure Wrapping doesn't slow things down with unneccesary copies.
 */

//------------------------------------------------------------------------------
template<class ITEM>
struct Wrap {
   ITEM _item;

   typedef ITEM         T;
   typedef ITEM  const& Ex;
   typedef ITEM&        Ref;
   typedef ITEM  const& CRef;
   typedef ITEM*        Ptr;
   typedef ITEM  const* CPtr;

   Wrap (): _item() {}
   Wrap (Ex item): _item(item) {}

   T    t    () const { return  _item; }
   Ex   ex   () const { return  _item; }
   Ref  ref  ()       { return  _item; }
   CRef cref () const { return  _item; }
   Ptr  ptr  ()       { return &_item; }
   CPtr cptr () const { return &_item; }
};

//------------------------------------------------------------------------------
template<class ITEM>
struct Wrap<ITEM const&> {
   ITEM const& _item;
   
   typedef ITEM  const& T;
   typedef ITEM  const& Ex;
   typedef ITEM&        Ref;
   typedef ITEM  const& CRef;
   typedef ITEM*        Ptr;
   typedef ITEM  const* CPtr;
   
   Wrap (Ex item): _item(item) {}
   
   T    t    () const { return  _item; }
   Ex   ex   () const { return  _item; }
   CRef cref () const { return  _item; }
   CPtr cptr () const { return &_item; }
};

//------------------------------------------------------------------------------
// Note: The Ex type for pointers should arguably be ITEM* const,
// however when passing by value this is superfluous.
// Hence we define Ex to be the same as T.
template<class ITEM>
struct Wrap<ITEM*> {
   ITEM* _item;

   typedef ITEM*        T;
   typedef ITEM*        Ex;
   typedef ITEM&        Ref;
   typedef ITEM  const& CRef;
   typedef ITEM*        Ptr;
   typedef ITEM  const* CPtr;

   Wrap (): _item(0) {}
   Wrap (Ex item): _item(item) {}

   T    t    () const { return  _item; }
   Ex   ex   () const { return  _item; }
   Ref  ref  () const { return *_item; }
   CRef cref () const { return *_item; }
   Ptr  ptr  () const { return  _item; }
   CPtr cptr () const { return  _item; }
};


//==============================================================================
// Dereferencing Functions
//==============================================================================
/*
 * The overloaded template function cref dereferences an object if necessary.
 */

//------------------------------------------------------------------------------
template<class ITEM>
ITEM const& cref (ITEM const& ex) { return ex; }

//------------------------------------------------------------------------------
template<class ITEM>
ITEM const& cref (ITEM* const ex) { return *ex; }


#endif // WRAP_HPP
