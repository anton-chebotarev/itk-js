#ifndef HEVCDataDecoder_H
#define HEVCDataDecoder_H

#include "IDataDecoder.h"

#include <memory>
#include <string>

namespace itkjs
  {
  namespace ImageStack
    {
      
    ////////////////////////////////////////////////////////////////////////
          
    class HEVCDataDecoder : public IDataDecoder
      {        
      public:
        HEVCDataDecoder();
        
        // IDataDecoder
        virtual void SetProgressReporter(std::unique_ptr<IDecodingProgressReporter>&& ip_reporter) override;        
        virtual void DecodeData(
          void* op_image_buffer,
          unsigned i_width,
          unsigned i_height,
          unsigned i_count,
          unsigned i_component_size,
          void* ip_data_buffer,
          unsigned i_data_buffer_size) const override;
        
      private:
        std::unique_ptr<IDecodingProgressReporter> mp_progress_reporter;
      };
      
    ////////////////////////////////////////////////////////////////////////
      
    }  
  }

#endif