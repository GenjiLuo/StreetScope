//==============================================================================
/// \file MemoryPoolF.h
// created on July 1 2012 by editing MemoryPool.h
//==============================================================================

#ifndef ESTLIB_MEMORY_POOL_F
#define ESTLIB_MEMORY_POOL_F

#include "BitField.h"


//==============================================================================
/// A memory pool that hands out fixed sized pieces.
//==============================================================================

/*
 * Compare to MemoryPool.
 *
 * ToDo: make donate actually check _minDonationSize
 */

class MemoryPoolF {
//------------------------------------------------------------------------------
// SubClasses
private:
   /// The fundamental unit of storage of MemoryPoolF.
   /**
    * A MemoryBlock is a bookkeeping object placed at the beginning of every
    * block of memory that MemoryPoolF allocates. It has a pointer to the
    * next MemoryBlock (a MemoryPoolF is functionally a linked list of
    * MemoryBlocks), and the size of the block of memory that it is
    * at the head of.
    */
   class MemoryBlockRecord {
   private:
      char* _start;
      char* _end;
      BitField _occupied;
      unsigned _freeItems;
      unsigned _firstFree;

   public:
      MemoryBlockRecord (): _start(nullptr), _end(nullptr), _occupied(), _freeItems(0) {}
      void attach (void* ptr, unsigned blockSize);
      // called exclusively by MemoryPoolF::setItemSize when the pool is empty
      unsigned partition (unsigned itemSize);
      void release () { std::free(_start); }
      ~MemoryBlockRecord () { release(); }

      /// caller (ie MemoryPoolF) must check if there is space (this avoids unnecessary stack frames)
      void* alloc (unsigned itemSize);
      void free (unsigned index);
      void clear ();

      void operator= (MemoryBlockRecord && mbr);

      char* start () const { return _start; }
      char* end   () const { return _end; }
      unsigned freeItems () const { return _freeItems; }
      // only call these methods after partitioning!
      unsigned capacityItems (unsigned itemSize) const { return _occupied.bits(); }
      unsigned capacityBytes () const { return _end - _start; }
      bool operator>  (MemoryBlockRecord const& mbr) { return _freeItems > mbr._freeItems; }
      bool contains (char* ptr) { return (_start <= ptr and ptr < _end); }
      unsigned index (char* ptr, unsigned itemSize) { return (ptr - _start) / itemSize; }
   };

//------------------------------------------------------------------------------
// Members
private:
   MemoryBlockRecord* _block; ///< array of MemoryBlockRecords
   unsigned _blocks;          ///< number of MemoryBlockRecords currently being used
   unsigned _maxBlocks;       ///< can fit _maxBlocks MemoryBlockRecords in _block
   unsigned _itemSize;        ///< size of chunks that MemoryPoolFF will hand out
   unsigned _minFree;         ///< when the most free block can't fit this many more, make a new one
   unsigned _nextBlockSize;   ///< the number of items we intend to fit in the next block we allocate
   /// if a donated block's capacity is less than _minDonationSize, it is tossed
   unsigned _minDonationSize;

   // These members are for profiling and debugging purposes only.
   unsigned _allocs;          ///< number of times MemoryPoolF::alloc has been called
   unsigned _frees;           ///< number of times free has been called (on memory that wasn't already free)
   unsigned _capacityItems;   ///< total number of items we can fit in our current memory blocks
   unsigned _capacityBytes;   ///< total size of all memory blocks in bytes

//------------------------------------------------------------------------------
// Methods
public:
   // Construction and Destruction
   MemoryPoolF ();
   unsigned setItemSize (unsigned itemSize, unsigned alignment = 1);
   void setMinFree (unsigned minFree) { _minFree = minFree; }
   void setNextBlockSize (unsigned nextBlockSize) { _nextBlockSize = nextBlockSize; }
   void setMinDonationSize (unsigned minDonationSize) { _minDonationSize = minDonationSize; }
   ~MemoryPoolF ();

   // Essential Functions
   void* alloc ();            ///< returns _itemSize bytes of memory
   /// Nothing happens if pointer does no point to memory handed out by MemoryPoolF.
   /// Pointer does not have to point to the start of a block.
   void  free  (void* item);
   unsigned donate (void* start, unsigned size); ///< Adds a memory block to the list of reserve blocks.
   unsigned allocBlock (unsigned blockSize = 0);
   void resizeBlockArray ();
   void shiftBlockArray ();

   // Mass Free Methods
   void clear ();             ///< Empties the pool, but does not return the memory to the operating system.
   void releaseAll ();        ///< Returns all memory to the operating system.

   
   // All the remaining methods are purely for profiling and/or debugging purposes.
   unsigned blocks          () const { return _blocks;                   }
   unsigned maxBlocks       () const { return _maxBlocks;                }
   unsigned itemSize        () const { return _itemSize;                 }
   unsigned minFree         () const { return _minFree;                  }
   unsigned nextBlockSize   () const { return _nextBlockSize;            }
   unsigned minDonationSize () const { return _minDonationSize;          }
   unsigned allocs          () const { return _allocs;                   }
   unsigned frees           () const { return _frees;                    }
   unsigned capacityItems   () const { return _capacityItems;            }
   unsigned capacityBytes   () const { return _capacityBytes;            }
   unsigned memoryBlockSize () const { return sizeof(MemoryBlockRecord); }
   unsigned freeItemsTotal  () const { return capacityItems() + frees() - allocs(); }
   unsigned freeItemsBlock  () const { if (_blocks) return _block[0].freeItems(); return 0; }

   void print () const;             ///< Prints everything.

private:
   void sortBlocks ();
};

#endif // ESTLIB_MEMORY_POOL_F
