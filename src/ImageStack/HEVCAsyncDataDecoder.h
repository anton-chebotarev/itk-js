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
        virtual void DecodeData(const Header& i_header, TUniqueMallocPtr&& ip_data_buffer, unsigned i_data_buffer_size) const override;
        
      private:
        struct _IContext
          {
          virtual ~_IContext() = default;
          };
      
      private:
        friend struct _IContext;
        IDataDestination& mr_data_destination;
        std::unique_ptr<IProgressReporter> mp_progress_reporter;
        mutable std::unique_ptr<_IContext> mp_context;
      };
      
    ////////////////////////////////////////////////////////////////////////
      
    }  
  }

#endif