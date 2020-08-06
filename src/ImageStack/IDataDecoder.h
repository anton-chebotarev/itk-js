#ifndef IDataDecoder_H
#define IDataDecoder_H

#include <memory>

namespace itkjs
  {
  namespace ImageStack
    {
      
    ////////////////////////////////////////////////////////////////////////
    
    class IProgressReporter;
    
    struct Header;
      
    ////////////////////////////////////////////////////////////////////////
          
    class IDataDecoder
      {        
      public:
        virtual ~IDataDecoder() = default;      
        virtual void SetProgressReporter(std::unique_ptr<IProgressReporter>&& /*ip_reporter*/) = 0;        
        virtual void DecodeData(const Header& /*i_header*/, void* /*ip_data_buffer*/, unsigned /*i_data_buffer_size*/) const = 0;
      };
      
    ////////////////////////////////////////////////////////////////////////
      
    }  
  }

#endif