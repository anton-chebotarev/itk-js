#ifndef ImageStackBuilder_H
#define ImageStackBuilder_H

#include "IDataDestination.h"

#include <memory>
#include <string>

namespace itkjs
  {
  namespace ImageStack
    {
      
    ////////////////////////////////////////////////////////////////////////
    
    class IAsyncDataLoader;
    class IDataDecoder;
      
    ////////////////////////////////////////////////////////////////////////
          
    class ImageStackBuilder : public IDataDestination
      {
      public:
        typedef std::function<void(void)> TOnReadyCallback;
        typedef std::function<void(const char*)> TOnFailedCallback;
        typedef std::function<void(int, int)> TOnLoadingProgressCallback;
        typedef std::function<void(unsigned, unsigned)> TOnDecodingProgressCallback;
        
      public:
        ImageStackBuilder();
        
        void OnLoadingProgress(TOnLoadingProgressCallback i_callback);
        void OnDecodingProgress(TOnDecodingProgressCallback i_callback);
        
        void LoadDataAsync(const std::string& i_url, TOnReadyCallback i_on_ready_callback, TOnFailedCallback i_on_failed_callback);
        
        // IDataDestination
        virtual void ProcessLoadedData(void* ip_buffer, unsigned i_buffer_size) override;
        virtual void OnDataLoadingFailed(const char* ip_description) override;
        
      private:
        TOnReadyCallback m_on_ready_callback;
        TOnFailedCallback m_on_failed_callback;
        std::unique_ptr<IAsyncDataLoader> mp_data_loader;
        std::unique_ptr<IDataDecoder> mp_data_decoder;
      };
      
    ////////////////////////////////////////////////////////////////////////
      
    }  
  }

#endif