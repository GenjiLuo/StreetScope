//------------------------------------------------------------------------------
/// \file MemoryPool.cpp
// created on Wed July 21 2010
//------------------------------------------------------------------------------

#include "MemoryPool.h"
#include <cstdlib>
#include <iostream>

/** \class MemoryPool
 *
 * MemoryPool contains a linked list of large blocks of memory. Each block
 * has a MemoryBlock object at its beginning that contains the size of the block
 * and a pointer to the next block. Each time the user calls MemoryPool::alloc,
 * the MemoryPool returns a pointer to at least the requested number of bytes of
 * free memory. It gets these smaller chunks of memory from the first block
 * in its list of large blocks. When this block runs out of room, it allocates
 * a new large block and adds it to the top of the list. (All blocks but the
 * first are full.)
 *
 * MemoryPool also has a list of reserve blocks. These can be given to the
 * MemoryPool with the MemoryPool::donate method. New blocks will not be
 * allocated until all the reserve blocks are filled.
 *
 * To delete the items in the pool, one can use MemoryPool::clear, which
 * turns all active memory into reserve memory, or MemoryPool::releaseAll,
 * which returns all the active and reserve memory it has to the operating system.
 * (Items must be deleted all at once; there is no free list.)
 *
 * MemoryPool will also ensure that the objects it allocates adhere to a
 * specific alignment. It does this by placing appropriate padding around all
 * the items.
 *
 * Technical Note: MemoryPool's definition of being aligned is that there
 * is an integer multiple of alignment bytes between the pointer given to the
 * pool by malloc and every pointer given to the user by MemoryPool::alloc.
 * This means that the actual alignment of MemoryPool cannot be greater than
 * that of malloc. (That is, you can tell MemoryPool to align to 16 bytes,
 * and it will space your items accordingly, but if malloc hands it memory aligned
 * to 8 bytes then half the time your items will actually be aligned to odd
 * multiples of 8.) Malloc is required to hand back memory aligned to fit
 * any fundamental type, though, so only in extremely bizarre and specific
 * circumstances should this be an issue.
 *
 */

//------------------------------------------------------------------------------
// Constructor
/**
 * InitialSize is the minimum number of bytes that the MemoryPool will be able to store
 * in the first MemoryBlock it creates. The only case in which the first MemoryBlock
 * created will have a larger capacity that initialSize is if the first allocation
 * is for more than initialSize bytes of memory.
 *
 * ToDo: ReWrite!
 * The MemoryPool guarantees that MemoryPool::alloc will return memory
 * that is aligned to multiples of alignment. The alignment must be a power of
 * two (including 2^0). (If you give a number that is not a power of two,
 * it will be rounded up for you.) If you don't specify an alignment (or set
 * it to zero), it will be set to the smaller of itemSize and 8 (and then
 * rounded up to the nearest power of two). If you give a number larger than
 * 32, it will be set to 32 (even 32 is absurdly large).
 *
 * The minimumCapacity is used to determine which donated blocks are worth
 * keeping. If a donated block can not hold minimumCapacity items, it is thrown
 * away. MinimumCapacity must be at least 1.
 */
MemoryPool::MemoryPool (unsigned initialSize, unsigned maxAlignment, unsigned minimumDonationSize)
: _activeBlock(0), _reserveBlock(0), _activeMemory(0), _pos(0), _end(0),
_maxAlignment(maxAlignment), _newBlockSize(sizeof(MemoryBlock)+initialSize),
_minimumDonationSize(minimumDonationSize),
_requestedPieces(0), _requestedBytes(0), _activeSize(0), _activeBlocks(0),
_reserveSize(0), _reserveBlocks(0)
{
   // The first block we allocate must be at least 4 bytes, so that the
   // memory pool will be able to grow (see resizing conditions in alloc).
   if (_newBlockSize < 4) {
      _newBlockSize = 4;
   }
   
   // The minimum donation size must be at least one.
   if (!_minimumDonationSize)
      ++_minimumDonationSize;

   // Alignment must be a power of two (between 1 and 32).
   if (!_maxAlignment) {
      _maxAlignment = 1;
   } else {
      if (_maxAlignment > 32)
         _maxAlignment = 32;
   }
   // If _alignment is not a power of two, round it up to one.
   if (_maxAlignment & (_maxAlignment-1)) {
      _maxAlignment |= _maxAlignment >> 1;
      _maxAlignment |= _maxAlignment >> 2;
      _maxAlignment |= _maxAlignment >> 4;
      _maxAlignment |= _maxAlignment >> 8;
      ++_maxAlignment;
   }

   // Figure out _headerSize.
   unsigned alignmentMask = _maxAlignment - 1;
   _headerSize = (sizeof(MemoryBlock) + alignmentMask) & ~alignmentMask;
}

//------------------------------------------------------------------------------
// Returns a pointer to a piece of memory with the specified size and alignment.
/**
 * This method does not ensure alignment is a power of two. (The assumption
 * is that if the caller cares and knows enough to specify specific alignments
 * for specific memory chunks, he can be trusted to specify feasible alignments.)
 *
 * If it is out of room and malloc fails, it returns a null pointer.
 */
void* MemoryPool::alloc (unsigned size, unsigned alignment)
{
   // See if the object will fit in the active block.
   // Note that this test fails when _insertPoint == _endOfBlock == 0,
   // as is the case when there is no active block at all.
   unsigned mask = alignment - 1;
   _pos = (_pos + mask) & ~mask;
   if (_pos + size > _end) {
      // First we see if there is a large enough reserve block.
      MemoryBlock* newBlock;
      while (_reserveBlock and _headerSize+size > _reserveBlock->_size) {
         newBlock = _reserveBlock;
         _reserveBlock = _reserveBlock->_next;
         free(newBlock);
      }
      if (_reserveBlock) {              // Use the reserve block.
         newBlock = _reserveBlock;
         _reserveBlock = _reserveBlock->_next;
         _reserveSize -= newBlock->_size;
         --_reserveBlocks;
      } else {                          // Otherwise allocate a new block.
         // make sure we will have enough space in our new MemoryBlock
         if (_newBlockSize < size + sizeof(MemoryBlock))
            _newBlockSize = size + sizeof(MemoryBlock);
         unsigned newBlockSize = _newBlockSize;
         // calloc is not necessary, but it is nice to get zeroed memory
         newBlock = static_cast<MemoryBlock*>(calloc(1, newBlockSize));
         if (!newBlock)                 // If calloc fails...
            return 0;
         newBlock->_size = newBlockSize;

         // increase _blockCapacity by 1/4 (for next time we need to reallocate)
         _newBlockSize += (_newBlockSize >> 2);
      }

      // Hook up the new MemoryBlock.
      newBlock->_next = _activeBlock;
      _activeBlock = newBlock;
      ++_activeBlocks;
      _activeSize += _activeBlock->_size;
      _activeMemory = reinterpret_cast<char*> (_activeBlock) + _headerSize;
      _pos = 0;
      _end = _activeBlock->_size - _headerSize;
   }

   // Update members, return pointer.
   void* writeHere = static_cast<void*> (&_activeMemory[_pos]);
   _pos += size;
   ++_requestedPieces;
   _requestedBytes += size;
   return writeHere;
}

//------------------------------------------------------------------------------
// Returns a pointer to a piece of memory adjacent to the last, or a null pointer.
/**
 * Does not add any alignment padding, because that would defeat the purpose
 * of being contiguous.
 */
void* MemoryPool::allocContiguous (unsigned size)
{
   if (_pos + size > _end)
      return 0;
   void* writeHere = static_cast<void*> (&_activeMemory[_pos]);
   _pos += size;
   ++_requestedPieces;
   _requestedBytes += size;
   return writeHere;
}

//------------------------------------------------------------------------------
// Empties the pool, but does not return the memory to the operating system.
/**
 * The capacity of the MemoryPool thus stays the same. This method does
 * not zero the memory.
 */
void MemoryPool::clear () {
   MemoryBlock* nextBlock;
   while (_activeBlock) {
      nextBlock = _activeBlock->_next;
      donate(_activeBlock, _activeBlock->_size);
      _activeBlock = nextBlock;
   }

   _activeMemory = 0;
   _pos = 0;
   _end = 0;

   _requestedPieces = 0;
   _requestedBytes  = 0;
   _activeSize   = 0;
   _activeBlocks = 0;
}

//------------------------------------------------------------------------------
// Returns all memory to the operating system.
void MemoryPool::releaseAll () {
   releaseReserve();
   
   MemoryBlock* nextBlock;
   while (_activeBlock) {
      nextBlock = _activeBlock->_next;
      free(_activeBlock);
      _activeBlock = nextBlock;
   }

   _activeMemory = 0;
   _pos = 0;
   _end = 0;

   _requestedPieces = 0;
   _requestedBytes  = 0;
   _activeSize   = 0;
   _activeBlocks = 0;
}

//------------------------------------------------------------------------------
// Returns all reserve memory to the operating system.
void MemoryPool::releaseReserve () {
   MemoryBlock* nextBlock;
   while (_reserveBlock) {
      nextBlock = _reserveBlock->_next;
      free(_reserveBlock);
      _reserveBlock = nextBlock;
   }

   _reserveSize   = 0;
   _reserveBlocks = 0;
}

//------------------------------------------------------------------------------
// Adds a memory block to the list of reserve blocks.
/**
 * The block starts at start, and is size bytes long.
 */
void MemoryPool::donate (void* start, unsigned size) {
   // If the donated block is too small, it's not worth keeping it around.
   if (size < _minimumDonationSize) {
      free(start);
      return;
   }
   MemoryBlock* newBlock = static_cast<MemoryBlock*>(start);
   newBlock->_next = _reserveBlock;
   newBlock->_size = size;
   _reserveBlock = newBlock;

   ++_reserveBlocks;
   _reserveSize += size;
}


//------------------------------------------------------------------------------
// Prints data reflecting what the MemoryPool is set up to store.
void MemoryPool::printParameters () const
{
   std::cout << "== MemoryPool Parameters ==\n";
   std::cout << "MemoryBlock Size:    " << memoryBlockSize() << '\n';
   std::cout << "MemoryBlock Padding: " << memoryBlockPadding() << '\n';
   std::cout << "Header Size:         " << headerSize() << '\n';
   std::cout << "Maximum Alignment:   " << maxAlignment() << '\n';
   std::cout << '\n';
}

//------------------------------------------------------------------------------
// Prints data reflecting the current state of the MemoryPool.
void MemoryPool::printState () const
{
   std::cout << "=== MemoryPool State ===\n";
   std::cout << "Requested Pieces: " << requestedPieces() << '\n';
   std::cout << "Requested Bytes:  " << requestedBytes() << '\n';
   std::cout << "Active Bytes:     " << activeBytes() << '\n';
   std::cout << "Active Blocks:    " << activeBlocks() << '\n';
   std::cout << "Reserve Bytes:    " << reserveBytes() << '\n';
   std::cout << "Reserve Blocks:   " << reserveBlocks() << '\n';
   std::cout << "Remaining Bytes:  " << remainingBytesOfActiveBlock() << '\n';
   std::cout << "Next block size:  " << sizeOfNextAllocatedBlock() << '\n';
   std::cout << '\n';
}

//------------------------------------------------------------------------------
// Prints everything.
void MemoryPool::print () const
{
   printParameters();
   printState();
}

