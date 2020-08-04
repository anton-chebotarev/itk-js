#ifndef IHeaderParser_H
#define IHeaderParser_H

#include <memory>

namespace itkjs
  {
  namespace ImageStack
    {
      
    ////////////////////////////////////////////////////////////////////////
    
    struct Header;
      
    ////////////////////////////////////////////////////////////////////////
          
    class IHeaderParser
      {
      public:
        
      public:
        virtual ~IHeaderParser() = default;
        
        virtual std::unique_ptr<Header> ParseHeader(
          void* ip_data_buffer,
          unsigned i_data_buffer_size) const = 0; 
      };
      
    ////////////////////////////////////////////////////////////////////////
      
    }  
  }

#endif