#include "./ImageStack/ImageStackBuilder.h"
#include "./ImageStack/IAsyncDataLoader.h"
#include "./ImageStack/IDataDecoder.h"

#include <emscripten/bind.h>

#include <iostream>
#include <iomanip>
#include <memory>
#include <string>

#include <itkVersion.h>

#include <libde265/de265.h>

namespace
  {
  void _Checked(de265_error i_err)
    {
    if (i_err != DE265_OK)
      throw std::runtime_error(std::string("Decoder error: ") + de265_get_error_text(i_err));
    }
    
  class _ImageStackBuilder
    {
    public:
      _ImageStackBuilder()
        : mp_impl(new itkjs::ImageStack::ImageStackBuilder())
        {
        }
        
      void OnLoadingProgress(emscripten::val i_callback)
        {
        mp_impl->OnLoadingProgress(
          [i_callback](int i_id, unsigned i_bytes_loaded, unsigned i_bytes_total) -> void
            {
            i_callback(i_id, i_bytes_loaded, i_bytes_total);
            } );
        }
      
      void OnDecodingProgress(emscripten::val i_callback)
        {
        mp_impl->OnDecodingProgress(
          [i_callback](unsigned i_frame_number, unsigned i_frames_total) -> void
            {
            i_callback(i_frame_number, i_frames_total);
            } );
        }
        
      void LoadDataAsync(std::string i_header_url, std::string i_data_url, emscripten::val i_on_ready_callback, emscripten::val i_on_failed_callback, emscripten::val i_status_callback)
        {
        mp_impl->LoadDataAsync(
          i_header_url,
          i_data_url,
          [i_on_ready_callback](void) -> void
            {
            i_on_ready_callback();
            },
          [i_on_failed_callback](const char* ip_description) -> void
            {
            i_on_failed_callback(std::string(ip_description));
            },
          [i_status_callback](const char* ip_description) -> void
            {
            i_status_callback(std::string(ip_description));
            } );
        }
        
    private:
      std::unique_ptr<itkjs::ImageStack::ImageStackBuilder> mp_impl;
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
    .function("OnLoadingProgress", &_ImageStackBuilder::OnLoadingProgress)
    .function("OnDecodingProgress", &_ImageStackBuilder::OnDecodingProgress)
    .function("LoadDataAsync", &_ImageStackBuilder::LoadDataAsync);
  }
