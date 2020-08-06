#ifndef HEVCAsyncDataDecoder_H
#define HEVCAsyncDataDecoder_H

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
          
    class HEVCAsyncDataDecoder : public IDataDecoder
      {        
      public:
        explicit HEVCAsyncDataDecoder(IDataDestination& ir_data_destination);
        
        // IDataDecoder
        virtual void SetProgressReporter(std::unique_ptr<IProgressReporter>&& ip_reporter) override;
        virtual void DecodeData(const Header& i_header, void* ip_data_buffer, unsigned i_data_buffer_size) const override;
        
      private:
        void _DecodeData(const Header& i_header, void* ip_data_buffer, unsigned i_data_buffer_size) const;
      
      private:
        IDataDestination& mr_data_destination;
        std::unique_ptr<IProgressReporter> mp_progress_reporter;
      };
      
    ////////////////////////////////////////////////////////////////////////
      
    }  
  }

#endif