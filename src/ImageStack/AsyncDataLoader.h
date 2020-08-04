#ifndef AsyncDataLoader_H
#define AsyncDataLoader_H

#include <memory>
#include <string>

namespace itkjs
  {
  namespace ImageStack
    {
      
    ////////////////////////////////////////////////////////////////////////
            
    class IDataDestination;
      
    ////////////////////////////////////////////////////////////////////////
          
    class AsyncDataLoader
      {
      public:
        class CallbacksHandler;
        class ILoadingProgressReporter;
              
      public:
        explicit AsyncDataLoader(IDataDestination& ir_data_destination);
        ~AsyncDataLoader();
        
        void SetProgressReporter(std::unique_ptr<ILoadingProgressReporter>&& ip_reporter);
        void LoadDataAsync(const std::string& i_data_url);        
        void TerminateDataLoading();
        
      private:
        friend class CallbacksHandler;
        IDataDestination& mr_data_destination;
        std::unique_ptr<CallbacksHandler> mp_callbacks_handler;
        std::unique_ptr<ILoadingProgressReporter> mp_progress_reporter;
        int mh_loader_handle;
      };
      
    ////////////////////////////////////////////////////////////////////////
    
    class AsyncDataLoader::CallbacksHandler
      {
      public:
        explicit CallbacksHandler(AsyncDataLoader& ir_data_loader);
      
        void OnSuccess(void* ip_received_buffer, unsigned i_size_in_bytes);
        void OnFailure(int i_http_error_code, const char* i_status_description);
        void OnProgress(int i_bytes_loaded, int i_bytes_total);
        
      private:
        AsyncDataLoader& mr_data_loader;
      };
      
    ////////////////////////////////////////////////////////////////////////
      
    class AsyncDataLoader::ILoadingProgressReporter
      {
      public:
        virtual ~ILoadingProgressReporter() = default;
        
        virtual void OnProgress(int /*i_bytes_loaded*/, int /*i_bytes_total*/) const = 0;
      };
      
    ////////////////////////////////////////////////////////////////////////
      
    class NullLoadingProgressReporter : public AsyncDataLoader::ILoadingProgressReporter
      {
      public:
        virtual void OnProgress(int /*i_bytes_loaded*/, int /*i_bytes_total*/) const override;
      };
      
    ////////////////////////////////////////////////////////////////////////
      
    class StdoutLoadingProgressReporter : public AsyncDataLoader::ILoadingProgressReporter
      {
      public:
        explicit StdoutLoadingProgressReporter(const std::string& i_stdout_prefix = "Data Loader: ");
        
        virtual void OnProgress(int i_bytes_loaded, int i_bytes_total) const override;
        
      private:
        std::string m_stdout_prefix;
      };
      
    ////////////////////////////////////////////////////////////////////////
      
    }  
  }

#endif