#include "./ImageStack/ImageStackBuilder.h"
#include "./ImageStack/IAsyncDataLoader.h"
#include "./ImageStack/IDataDecoder.h"
#include "./ImageStack/IHeaderParser.h"
#include "./ImageStack/Header.h"
#include "./ImageStack/ImageStack.h"

#include <emscripten/bind.h>

#include <iostream>
#include <iomanip>
#include <memory>
#include <string>

#include <itkVersion.h>

#include <libde265/de265.h>

namespace
  {
  class _ImageStack
    {
    public:
      explicit _ImageStack(std::unique_ptr<itkjs::ImageStack::ImageStack>&& ip_image_stack)
        : mp_image_stack(std::move(ip_image_stack))
        {
        }
        
      unsigned GetDimensions(unsigned i_index) const
        {
        return mp_image_stack->GetDimensions(i_index);
        }
      
      unsigned GetComponentSize() const
        {
        return mp_image_stack->GetComponentSize();
        }
      
    private:
      std::unique_ptr<itkjs::ImageStack::ImageStack> mp_image_stack;
    };
    
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
          [i_on_ready_callback](std::unique_ptr<itkjs::ImageStack::ImageStack>&& ip_image_stack) -> void
            {
            i_on_ready_callback(_ImageStack(std::move(ip_image_stack)));
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

  class_<_ImageStack>("ImageStack")
    .function("getDimensions", &_ImageStack::GetDimensions)
    .function("getComponentSize", &_ImageStack::GetComponentSize);

  class_<_ImageStackBuilder>("ImageStackBuilder")
    .constructor<>()
    .function("onLoadingProgress", &_ImageStackBuilder::OnLoadingProgress)
    .function("onDecodingProgress", &_ImageStackBuilder::OnDecodingProgress)
    .function("loadDataAsync", &_ImageStackBuilder::LoadDataAsync);
  }
