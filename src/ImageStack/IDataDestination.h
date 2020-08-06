#ifndef IDataDestination_H
#define IDataDestination_H

#include "MemoryManagement.h"

#include <string>

namespace itkjs
  {
  namespace ImageStack
    {
      
    ////////////////////////////////////////////////////////////////////////
    class IDataDestination
      {
      public:
        virtual ~IDataDestination() = default;
        
        virtual void OnSuccess(TUniqueMallocPtr&& /*ip_buffer*/, unsigned /*i_size_in_bytes*/) = 0;
        virtual void OnSuccess(TUniqueBufferPtr&& /*ip_buffer*/, unsigned /*i_size_in_bytes*/) = 0;
        virtual void OnFailure(const std::string& /*i_description*/) = 0;
      };
      
    ////////////////////////////////////////////////////////////////////////
      
    }  
  }

#endif