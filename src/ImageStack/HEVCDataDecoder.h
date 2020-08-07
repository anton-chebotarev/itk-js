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
            
    class IDataDestination;
      
    ////////////////////////////////////////////////////////////////////////
          
    class HEVCDataDecoder : public IDataDecoder
      {        
      public:
        explicit HEVCDataDecoder(IDataDestination& ir_data_destination);
        
        // IDataDecoder
        virtual void SetProgressReporter(std::unique_ptr<IProgressReporter>&& ip_reporter) override;
        virtual void DecodeData(const Header& i_header, TUniqueMallocPtr&& ip_data_buffer, unsigned i_data_buffer_size) const override;
      
      private:
        IDataDestination& mr_data_destination;
        std::unique_ptr<IProgressReporter> mp_progress_reporter;
      };
      
    ////////////////////////////////////////////////////////////////////////
      
    }  
  }

#endif