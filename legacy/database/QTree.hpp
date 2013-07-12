//==============================================================================
// QTree.hpp
// Created 1/29/12.
//==============================================================================

#ifndef Q_TREE
#define Q_TREE

#include "Wrap.hpp"
#include "MemoryPool.h"
#include "LinkedList.hpp"
#include "Hypergrid.hpp"

#include <iostream>  //db
using namespace std; //db


//==============================================================================
// Class QTree<ITEM>
//==============================================================================

// stores ITEMs, each level divides into 2^N pieces
template<typename ITEM, unsigned N>
class QTree {
// Subclasses
private:
   // A non-leaf Node of the tree
   struct Node {
      bool _isLeaf;     // Always false. Must be first!
      Node* _child[1u<<N];
      Node (): _isLeaf(false) {
         memset(_child, 0, (1u<<N)*sizeof(Node*));
      }
   };

   // Keeps track of the (hyper) area enclosed by a Node
   // Only used by find.
   struct NodeRange {
      Node const* _node;
      HyperIndex<N> _low;     // all points within this box have indeces between
      HyperIndex<N> _high;    // low and high (inclusive)
      
      NodeRange (Node const* node, HyperIndex<N> const& low, HyperIndex<N> const& high)
      :_node(node), _low(low), _high(high) {}
   };
   
   // The leaves contain linked lists of ITEMs.
   struct Link {
      Link* _next;
      Wrap<ITEM> _item;
      
      Link (Link* next, typename Wrap<ITEM>::Ex item): _next(next), _item(item) {}
   };

   // A leaf node in the tree
   struct Leaf {
   public:
      bool _isLeaf;     // Always true. Must be first!
      Link* _first;     // first Link in LinkedList of ITEMs
      unsigned _items;  // number of items in this Leaf
      
      Leaf (): _isLeaf(true), _first(0), _items(0) {}
   };

public:
   // Iterator
   class ConstIterator {
   private:
      QTree const& _tree;     // tree being iterated over
      Link const* _link;      // current link
      unsigned _depth;        // depth of current leaf
      HyperIndex<N> _index;   // index of current leaf
   public:
      ConstIterator (QTree const& tree);
      bool valid () const { return _link; }
      ConstIterator& operator++ ();
//      typename Wrap<ITEM>::Ref  ref  () const { return _link->_item.ref(); }
      typename Wrap<ITEM>::CRef cref () const { return _link->_item.cref(); }
//      typename Wrap<ITEM>::Ptr  ptr  () const { return _link->_item.ptr(); }
      typename Wrap<ITEM>::CPtr cptr () const { return _link->_item.cptr(); }
   };
   friend class ConstIterator;

// Members
private:
   MemoryPool* _pool;
   Node _root;
   unsigned _items;     // total number if items in the QTRee
   unsigned _trigger;   // each leaf subdivides when it has >= _trigger items

// Interface
public:
   QTree (unsigned c, unsigned t = 16);
   ~QTree () { delete _pool; }
   typename Wrap<ITEM>::Ref add (typename Wrap<ITEM>::Ex item);
   // Finds all items with coordinates x such that
   // | target[d] - x[d] | <= tolerance[d] for all d in 0, ..., N-1.
   LinkedList<typename Wrap<ITEM>::T>* find (HyperBox<N> const& range) const;
   unsigned size () const { return _items; }
   ConstIterator constIterator () const { return ConstIterator(*this); }

private:
   // disallow copies
   QTree (QTree const& tree) {}
   // Splits node->_child[child] into 2^N pieces.
   void split (Node* node, unsigned child, unsigned i);
};


//==============================================================================
// Public Member Function Definitions
//==============================================================================

//------------------------------------------------------------------------------
template<typename ITEM, unsigned N>
QTree<ITEM, N>::QTree (unsigned c, unsigned t)
: _root(), _items(0), _trigger(t)
{   
   // Instantiate our MemoryPool
   unsigned poolSize = c*sizeof(Link);       // need a link for every item
   poolSize += c*sizeof(Leaf)/_trigger;      // need a leaf for # items / trigger
   poolSize += c*sizeof(Node)/(((1u<<N)-1u)*_trigger); // need 1/(2^N-1) as many nodes as leaves
   _pool = new MemoryPool(poolSize, sizeof(Node*));

   // Create the first four leaves
   for (unsigned i=0; i<(1u<<N); ++i) {
      _root._child[i] = reinterpret_cast<Node*> (new(_pool->alloc(sizeof(Leaf))) Leaf());
   }
}

//------------------------------------------------------------------------------
template<typename ITEM, unsigned N>
typename Wrap<ITEM>::Ref QTree<ITEM, N>::add (typename Wrap<ITEM>::Ex item) {
   // get the index
   HyperIndex<N> index = cref(item).index();

   // find the right leaf
   unsigned i=0;
   Node* node;
   Node* nextNode = &_root;
   unsigned child;
   while (i<HyperIndex<N>::Cuts) {
      if (nextNode->_isLeaf) {
         break;
      }
      node = nextNode;
      child = index.getSubIndex(i);
      nextNode = nextNode->_child[child];
      ++i;
   }

   // add a new Link
   Leaf* leaf = reinterpret_cast<Leaf*> (nextNode);
   Link* newlink = new(_pool->alloc(sizeof(Link))) Link(leaf->_first, item);
   leaf->_first = newlink;
   ++leaf->_items;
   ++_items;

   // if we have too many items and we have room to split, split
   if (leaf->_items > _trigger and i < HyperIndex<N>::Cuts) {
      split(node, child, i);
   }
   
   // return the added link's item
   return newlink->_item.ref();
}

//------------------------------------------------------------------------------
template<typename ITEM, unsigned N>
LinkedList<typename Wrap<ITEM>::T>* QTree<ITEM, N>::find (HyperBox<N> const& range) const {
   // Nodes we still have to search
   LinkedList<NodeRange> queue;

   // all the items we've found
   LinkedList<typename Wrap<ITEM>::T>* results = new LinkedList<typename Wrap<ITEM>::T>();

   // Define the root NodeRange
   HyperIndex<N> rootmin(0);
   HyperIndex<N> rootmax(~ 0);
   queue.add( NodeRange(&_root, rootmin, rootmax) );

   // While we still have nodes to search...
   while (queue.items()) {
      // pop off the first from the list
      NodeRange node = queue.removeFirst();

      // can't just write (high - low + 1) >> N since high - low + 1 will overflow on root
      GridInt newsize = (node._high >> N) + One - (node._low >> N);
      // low and high are the indeces of the lowest and highest points in each child
      HyperIndex<N> low = node._low;
      HyperIndex<N> high = low + newsize - 1u;

      // for each child of the node...
      for (unsigned i=0; i<(1u<<N); ++i) {
         Node* child = node._node->_child[i];
         // see if the child intersects our search range
//         cout << "low:  " << getCoords(low) << '\n';
//         cout << "high: " << getCoords(high) << '\n';
         if (range.intersects(HyperBox<N>(low, high))) {
            if (child->_isLeaf) {
               // add all the items in this Leaf that are within range
               Link* link = reinterpret_cast<Leaf*>(child)->_first;
               while (link) {
//                  cout << link->_item.cref().id() << "   ";
                  if ( range.contains( getCoords(link->_item.cref().index()) ) ) {
//                     cout << "y\n";
                     results->add(link->_item.t());
                  }
                  link = link->_next;
               }
            } else {
               queue.add(NodeRange(node._node->_child[i], low, high));
            }
         }
         // update low and high
         low = (GridInt)high + One;
         high = (GridInt)high + newsize;
      }
   }
   return results;
}


//==============================================================================
// Private Member Function Definitions
//==============================================================================

//------------------------------------------------------------------------------
template<typename ITEM, unsigned N>
void QTree<ITEM, N>::split (Node* node, unsigned child, unsigned i) {
   // Make the new Node
   Leaf* oldLeaf = reinterpret_cast<Leaf*> (node->_child[child]);
   Node* newNode = new(_pool->alloc(sizeof(Node))) Node();
   node->_child[child] = newNode;

   // Make new leaves for the new Node
   // oldLeaf will be become the last child of newNode
   oldLeaf->_items = 0;
   Leaf* newLeaves[(1u<<N)-1u];
   for (unsigned j=0; j<(1u<<N)-1u; ++j) {
      newLeaves[j] = new(_pool->alloc(sizeof(Leaf))) Leaf();
      newNode->_child[j] = reinterpret_cast<Node*> (newLeaves[j]);
   }
   newNode->_child[(1u<<N)-1u] = reinterpret_cast<Node*> (oldLeaf);

   // Redistribute the Links of the old Leaf
   Link** link1 = &oldLeaf->_first;
   Link*  link2 = *link1;;   // used for swaps
   unsigned c;    // which child link gets added to
   while (link2) {
      c = link2->_item.cref().index().getSubIndex(i);
      if (c != (1u<<N)-1u) {
         ++(newLeaves[c]->_items);
         link2                = link2->_next;
         (**link1)._next      = newLeaves[c]->_first;
         newLeaves[c]->_first = *link1;
         *link1               = link2;
      } else {
         ++(oldLeaf->_items);
         link1 = &link2->_next;
         link2 =  link2->_next;
      }
   }
}


//==============================================================================
// Iterator Member Function Definitions
//==============================================================================

//------------------------------------------------------------------------------
template<typename ITEM, unsigned N>
QTree<ITEM, N>::ConstIterator::ConstIterator (QTree const& tree): _tree(tree), _depth(0), _index(0) {
   Node* node = tree._root._child[0];
   while (!node->_isLeaf) {
      node = node->_child[0];
      ++_depth;
   }
   _link = reinterpret_cast<Leaf const*>(node)->_first;
   if (!_link) {
      ++*this;
   }
}

//------------------------------------------------------------------------------
template<typename ITEM, unsigned N>
typename QTree<ITEM, N>::ConstIterator& QTree<ITEM, N>::ConstIterator::operator++ () { 
   if (_link and _link->_next) {
      _link = _link->_next;
   } else {
      // find the next nonempty leaf (and update index accordingly)
      while (true) {
         // find the next cut that we can increment
         while (_index.getSubIndex(_depth) == (1u<<N)-1u) {
            if (_depth > 0) {
               --_depth;
            } else {
               // we've iterated past the last item
               _link = 0;
               return *this;
            }
         }
         _index.setSubIndex(_depth, _index.getSubIndex(_depth) + 1u);

         // crawl down the tree til we reach said cut
         Node const* node = &(_tree._root);
         for (unsigned i=0; i<=_depth; ++i) {
            node = node->_child[_index.getSubIndex(i)];
         }

         // keep moving down the tree til we hit a leaf
         while (!node->_isLeaf) {
            ++_depth;
            _index.setSubIndex(_depth, 0);
            node = node->_child[0];
         }
         
         // if the leaf is empty we have to keep searching,
         // otherwise we're done
         _link = reinterpret_cast<Leaf const*>(node)->_first;
         if (_link) {
            break;
         }
      }
   }
   return *this;
}



#endif // Q_TREE
