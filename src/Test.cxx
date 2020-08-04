#include "./ImageStack/IDataDestination.h"
#include "./ImageStack/AsyncDataLoader.h"
#include "./ImageStack/HEVCDataDecoder.h"

#include <emscripten/bind.h>

#include <iostream>
#include <iomanip>
#include <memory>
#include <string>

#include <itkVersion.h>

#include <libde265/de265.h>

using namespace itkjs::ImageStack;

namespace
  {
  void _Checked(de265_error i_err)
    {
    if (i_err != DE265_OK)
      throw std::runtime_error(std::string("Decoder error: ") + de265_get_error_text(i_err));
    }
    
  class _ImageStackBuilder : public IDataDestination
    {
    public:
      _ImageStackBuilder()
        : mp_data_decoder(new HEVCDataDecoder())
        , m_dimensions{512, 512, 759}
        , m_component_size(2)
        {
        mp_data_loader.reset(new AsyncDataLoader(*this));
        mp_data_loader->SetProgressReporter(std::unique_ptr<AsyncDataLoader::ILoadingProgressReporter>(new StdoutLoadingProgressReporter()));        
        mp_data_decoder->SetProgressReporter(std::unique_ptr<HEVCDataDecoder::IDecodingProgressReporter>(new StdoutDecodingProgressReporter()));
        }
        
      void LoadData(const std::string& i_url)
        {
        mp_data_loader->LoadDataAsync(i_url);
        }
    
      // IDataDestination
      virtual void ProcessLoadedData(void* ip_buffer, unsigned i_buffer_size) override
        {
        m_frame_buffer.resize(m_dimensions[0] * m_dimensions[1] * m_dimensions[2] * m_component_size);
        mp_data_decoder->DecodeData(
          reinterpret_cast<void*>(&m_frame_buffer[0]),
          m_dimensions[0],
          m_dimensions[1],
          m_dimensions[2],
          m_component_size,
          ip_buffer,
          i_buffer_size);
        }
        
    private:
      std::vector<uint8_t> m_frame_buffer;
      unsigned m_dimensions[3];
      unsigned m_component_size;
      std::unique_ptr<AsyncDataLoader> mp_data_loader;
      std::unique_ptr<HEVCDataDecoder> mp_data_decoder;
    };
          
  void _Info()
    {
    std::cout << "ImageStack Module: " << std::endl;
    std::cout << "  ITK Version - " << itk::Version::GetITKVersion() << std::endl;
    std::cout << "  libde265 Version - " << de265_get_version() << std::endl;
    }
  }

using namespace emscripten;
EMSCRIPTEN_BINDINGS(Test)
  {
  function("Info", &_Info);

  class_<_ImageStackBuilder>("ImageStackBuilder")
    .constructor<>()
    .function("LoadData", &_ImageStackBuilder::LoadData);
  }
