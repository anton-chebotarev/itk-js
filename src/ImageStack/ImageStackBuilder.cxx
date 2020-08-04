#include "ImageStackBuilder.h"

#include "WgetAsyncDataLoader.h"
#include "HEVCDataDecoder.h"

#include <vector>

namespace itkjs
  {
  namespace ImageStack
    {
      
    ////////////////////////////////////////////////////////////////////////
            
    ImageStackBuilder::ImageStackBuilder()
      : mp_data_decoder(new HEVCDataDecoder())
      {
      mp_data_loader.reset(new WgetAsyncDataLoader(*this));
      }
          
    void ImageStackBuilder::OnLoadingProgress(TOnLoadingProgressCallback i_callback)
      {
      class _LoadingProgressReporterProxy : public IAsyncDataLoader::ILoadingProgressReporter
        {          
        public:
          explicit _LoadingProgressReporterProxy(TOnLoadingProgressCallback i_callback)
            : m_callback(i_callback)
            {
            }
          virtual void OnLoadingProgress(unsigned i_bytes_loaded, unsigned i_bytes_total) const override
            {
            m_callback(i_bytes_loaded, i_bytes_total);
            }          
        private:
          TOnLoadingProgressCallback m_callback;
        };
      mp_data_loader->SetProgressReporter(std::unique_ptr<IAsyncDataLoader::ILoadingProgressReporter>(new _LoadingProgressReporterProxy(i_callback)));  
      }
    
    void ImageStackBuilder::OnDecodingProgress(TOnDecodingProgressCallback i_callback)
      {
      class _DecodingProgressReporterProxy : public IDataDecoder::IDecodingProgressReporter
        {          
        public:
          explicit _DecodingProgressReporterProxy(TOnLoadingProgressCallback i_callback)
            : m_callback(i_callback)
            {
            }
          virtual void OnDecodingProgress(unsigned i_bytes_loaded, unsigned i_bytes_total) const override
            {
            m_callback(i_bytes_loaded, i_bytes_total);
            }          
        private:
          TOnLoadingProgressCallback m_callback;
        };
      mp_data_decoder->SetProgressReporter(std::unique_ptr<IDataDecoder::IDecodingProgressReporter>(new _DecodingProgressReporterProxy(i_callback)));        
      }
        
    void ImageStackBuilder::LoadDataAsync(const std::string& i_url, TOnReadyCallback i_on_ready_callback, TOnFailedCallback i_on_failed_callback)
      {
      m_on_ready_callback = i_on_ready_callback;
      m_on_failed_callback = i_on_failed_callback;
      mp_data_loader->LoadDataAsync(i_url);
      }
  
    void ImageStackBuilder::ProcessLoadedData(void* ip_buffer, unsigned i_buffer_size)
      {
      const unsigned dimensions[] = {512, 512, 759};
      const unsigned component_size = 2;
      std::vector<uint8_t> frame_buffer(dimensions[0] * dimensions[1] * dimensions[2] * component_size);
      try
        {
        mp_data_decoder->DecodeData(
          reinterpret_cast<void*>(&frame_buffer[0]),
          dimensions[0],
          dimensions[1],
          dimensions[2],
          component_size,
          ip_buffer,
          i_buffer_size);
        m_on_ready_callback();
        }
      catch (const std::exception& i_e)
        {
        m_on_failed_callback(i_e.what());
        }
      catch (...)
        {
        m_on_failed_callback("Exception during data decoding.");
        }
      }
      
    void ImageStackBuilder::OnDataLoadingFailed(const char* ip_description)
      {
      m_on_failed_callback(ip_description);  
      }
      
    ////////////////////////////////////////////////////////////////////////      
      
    }  
  }