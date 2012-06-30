//------------------------------------------------------------------------------
/// \file MemoryPool.h
// created on Wed July 21 2010
//------------------------------------------------------------------------------

#ifndef ESTLIB_MEMORY_POOL
#define ESTLIB_MEMORY_POOL

//------------------------------------------------------------------------------
/// A simple memory pool that hands out variably sized pieces.
//------------------------------------------------------------------------------

class MemoryPool  {
private:
   /// The fundamental unit of storage of MemoryPool.
   /**
    * A MemoryBlock is a bookkeeping object placed at the beginning of every
    * block of memory that MemoryPool allocates. It has a pointer to the
    * next MemoryBlock (a MemoryPool is functionally a linked list of
    * MemoryBlocks), and the size of the block of memory that it is
    * at the head of.
    */
   struct MemoryBlock {
      MemoryBlock* _next;      ///< the next MemoryBlock
      unsigned _size;          ///< size in bytes of the MemoryBlock (including size of MemoryBlock itself)
      // (_size - sizeof(MemoryBlock)) bytes of memory go here
   };
   
   MemoryBlock* _activeBlock;  ///< current block, then chain of filled blocks
   MemoryBlock* _reserveBlock; ///< first block in chain of reserve (empty) blocks
   char* _activeMemory;        ///< points to the beginning of the writable portion of the active MemoryBlock
   unsigned _pos;              ///< _activeMemory[_pos] is the first free byte of mem
   unsigned _end;              ///< _activeMemory[_end] is one past the last byte of the memory block

   unsigned _maxAlignment;     ///< largest alignment the MemoryPool can accomodate (also default alignment)
   /// bytes reserved for the MemoryBlock (can be larger than sizeof(MemoryBlock) to preserve alignment)
   unsigned _headerSize;
   unsigned _newBlockSize;     ///< the planned size of the next block we allocate (including header)
   /// if a donated block's capacity is less than _minimumCapacity, it is tossed
   unsigned _minimumDonationSize;

   // These members are for profiling and debugging purposes only.
   unsigned _requestedPieces;  ///< number of times MemoryPool::alloc has been called
   unsigned _requestedBytes;   ///< total amount of memory requested (excludes alignment and bookkeeping overhead)
   unsigned _activeSize;       ///< total size of all active blocks (including size of MemoryBlocks themselves)
   unsigned _activeBlocks;     ///< number of blocks in active chain
   unsigned _reserveSize;      ///< total size of all free blocks (including size of MemoryBlocks themselves)
   unsigned _reserveBlocks;    ///< number of blocks in reserve chain

public:
   /// Constructor
   MemoryPool (unsigned initialSize,
               unsigned maxAlignment = 1,
               unsigned minimumDonationSize = 512);
   ~MemoryPool ();        ///< Destructor

   /// Returns a pointer to a piece of memory with the specified size, with the default (max) alignment.
   void* alloc (unsigned size);
   /// Returns a pointer to a piece of memory with the specified size and alignment.
   void* alloc (unsigned size, unsigned alignment);
   /// Returns a pointer to a piece of memory adjacent to the last (if possible).
   void* allocContiguous (unsigned size);

   void clear ();              ///< Empties the pool, but does not return the memory to the operating system.
   void releaseAll ();         ///< Returns all memory to the operating system.
   void releaseReserve ();     ///< Returns all reserve memory to the operating system.
   void donate (void* start, unsigned size); ///< Adds a memory block to the list of reserve blocks.

   
   // All the remaining methods are purely for profiling and/or debugging purposes.
   
   unsigned memoryBlockSize () const; ///< Returns the size of one MemoryBlock object.
   /// Returns the number of blank bytes that are added after each MemoryBlock object to preserve alignment.
   unsigned memoryBlockPadding () const;
   unsigned headerSize () const;    ///< Returns the size of the header at the beginning of each block of memory.
   unsigned maxAlignment () const;  ///< Returns the maximum allowable (and default) alignment of the MemoryPool.

   unsigned requestedPieces () const; ///< Returns the number of times MemoryPool::alloc has been called.
   unsigned requestedBytes () const;  ///< Returns the total amount of memory that has been requested.
   unsigned activeBytes() const;    ///< Returns the total number of bytes being used for storage.
   unsigned activeBlocks () const;  ///< Returns the number of active memory blocks.
   unsigned reserveBytes () const;  ///< Returns the total number of bytes being held in reserve.
   unsigned reserveBlocks () const; ///< Returns the number of reserve memory blocks.

   /// Returns the number of bytes of the active memory block that are currently free.
   unsigned remainingBytesOfActiveBlock () const;
   /// Returns the size that the MemoryPool plans to make the next block it allocates.
   unsigned sizeOfNextAllocatedBlock () const;
   /// Returns the minimum size a donated block can be before being thrown away.
   unsigned minimumDonationSize () const;
   
   void printParameters () const;   ///< Prints data reflecting what the MemoryPool is set up to store.
   void printState () const;        ///< Prints data reflecting the current state of the MemoryPool.
   void print () const;             ///< Prints everything.
};


//------------------------------------------------------------------------------
// Inline Declarations
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// Destructor
inline MemoryPool::~MemoryPool ()
{
   releaseAll();
}

//------------------------------------------------------------------------------
inline void* MemoryPool::alloc (unsigned size)
{
   return alloc(size, _maxAlignment);
}

//------------------------------------------------------------------------------
// Returns the size of one MemoryBlock object.
/**
 * Padding is potentially added between the MemoryBlock and the first item in
 * the block. For the total size of the header including padding, use headerSize.
 */
inline unsigned MemoryPool::memoryBlockSize () const
{
   return sizeof(MemoryBlock);
}

//------------------------------------------------------------------------------
// Returns the number of blank bytes that are added after each MemoryBlock object to preserve alignment.
inline unsigned MemoryPool::memoryBlockPadding () const
{
   return _headerSize - sizeof(MemoryBlock);
}

//------------------------------------------------------------------------------
// Returns the size of the header at the beginning of each block of memory.
/**
 * This is equal to memoryBlockSize + memoryBlockPadding.
 */
inline unsigned MemoryPool::headerSize () const
{
   return _headerSize;
}

//------------------------------------------------------------------------------
// Returns the alignment of the MemoryPool.
inline unsigned MemoryPool::maxAlignment () const
{
   return _maxAlignment;
}

//------------------------------------------------------------------------------
// Returns the number of times MemoryPool::alloc has been called.
inline unsigned MemoryPool::requestedPieces () const
{
   return _requestedPieces;
}

//------------------------------------------------------------------------------
// Returns the total amount of memory that has been requested.
/**
 * This does not include the alignment overhead or the space used to store the
 * MemoryBlocks themselves. It is simply the sum of the sizes of all the pieces
 * that have been requested with MemoryPool::alloc.
 */
inline unsigned MemoryPool::requestedBytes () const
{
   return _requestedBytes;
}

//------------------------------------------------------------------------------
// Returns the total number of bytes being held in reserve.
/**
 * This count includes the size of all padding and all active MemoryBlock
 * objects themselves, but not the size of the MemoryPool itself.
 */
inline unsigned MemoryPool::activeBytes () const
{
   return _activeSize;
}

//------------------------------------------------------------------------------
// Returns the number of active memory blocks.
inline unsigned MemoryPool::activeBlocks () const
{
   return _activeBlocks;
}

//------------------------------------------------------------------------------
// Returns the total number of bytes being held in reserve.
/**
 * Note that not all of this memory will be available to the user.
 * This is because each reserve block must store a MemoryBlock object itself
 * and all necessary padding in the header and between items, and there could
 * be space left at the end of the block even after it has been packed
 * with all the items that will fit.
 */
inline unsigned MemoryPool::reserveBytes () const
{
   return _reserveSize;
}

//------------------------------------------------------------------------------
// Returns the number of reserve memory blocks.
inline unsigned MemoryPool::reserveBlocks () const
{
   return _reserveBlocks;
}

//------------------------------------------------------------------------------
// Returns the number of bytes of the active memory block that are currently free.
inline unsigned MemoryPool::remainingBytesOfActiveBlock () const
{
   return _end - _pos;
}

//------------------------------------------------------------------------------
// Returns the size that the MemoryPool plans to make the next block it allocates.
inline unsigned MemoryPool::sizeOfNextAllocatedBlock () const
{
   return _newBlockSize;
}

//------------------------------------------------------------------------------
// Returns the minimum size that a donated block can be before being thrown away.
inline unsigned MemoryPool::minimumDonationSize () const
{
   return _minimumDonationSize;
}


#endif // ESTLIB_MEMORY_POOL
