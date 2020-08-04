#ifndef WgetAsyncDataLoader_H
#define WgetAsyncDataLoader_H

#include "IAsyncDataLoader.h"

#include <memory>
#include <string>

namespace itkjs
  {
  namespace ImageStack
    {
      
    ////////////////////////////////////////////////////////////////////////
            
    class IDataDestination;
      
    ////////////////////////////////////////////////////////////////////////
          
    class WgetAsyncDataLoader : public IAsyncDataLoader
      {
      public:
        class CallbacksHandler;
              
      public:
        explicit WgetAsyncDataLoader(IDataDestination& ir_data_destination);
        virtual ~WgetAsyncDataLoader();
        
        // IWgetAsyncDataLoader
        virtual void SetProgressReporter(std::unique_ptr<ILoadingProgressReporter>&& ip_reporter) override;
        virtual void LoadDataAsync(const std::string& i_data_url) override;        
        virtual void TerminateDataLoading() override;
        
      private:
        friend class CallbacksHandler;
        IDataDestination& mr_data_destination;
        std::unique_ptr<CallbacksHandler> mp_callbacks_handler;
        std::unique_ptr<ILoadingProgressReporter> mp_progress_reporter;
        int mh_loader_handle;
      };
      
    ////////////////////////////////////////////////////////////////////////
    
    class WgetAsyncDataLoader::CallbacksHandler
      {
      public:
        explicit CallbacksHandler(WgetAsyncDataLoader& ir_data_loader);
      
        void OnSuccess(void* ip_received_buffer, unsigned i_size_in_bytes);
        void OnFailure(int i_http_error_code, const char* i_status_description);
        void OnProgress(unsigned i_bytes_loaded, unsigned i_bytes_total);
        
      private:
        WgetAsyncDataLoader& mr_data_loader;
      };
      
    ////////////////////////////////////////////////////////////////////////
      
    }  
  }

#endif