#ifndef MemoryManagement_H
#define MemoryManagement_H

#include <memory>

namespace itkjs
  {
  namespace ImageStack
    {
      
    ////////////////////////////////////////////////////////////////////////
          
    typedef std::unique_ptr<void, void(*)(void*)> TUniqueMallocPtr;  
    typedef std::unique_ptr<uint8_t[]> TUniqueBufferPtr;
      
    ////////////////////////////////////////////////////////////////////////
      
    }  
  }

#endif