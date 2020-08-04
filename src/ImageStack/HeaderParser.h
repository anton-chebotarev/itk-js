#ifndef HeaderParser_H
#define HeaderParser_H

#include "IHeaderParser.h"

namespace itkjs
  {
  namespace ImageStack
    {
      
    ////////////////////////////////////////////////////////////////////////
          
    class HeaderParser : public IHeaderParser
      {
      public:        
        virtual std::unique_ptr<Header> ParseHeader(
          void* ip_data_buffer,
          unsigned i_data_buffer_size) const override;
      };
      
    ////////////////////////////////////////////////////////////////////////
      
    }  
  }

#endif