#include "./ImageStack/IAsyncDataLoader.h"
#include "./ImageStack/IDataDecoder.h"
#include "./ImageStack/IHeaderParser.h"
#include "./ImageStack/Header.h"
#include "./ImageStack/ContrastFunction.h"
#include "./ImageStack/ImageStackBuilder.h"
#include "./ImageStack/ImageStack.h"
#include "./ImageStack/ImageSlice.h"
#include "./ImageStack/ImageView.h"
#include "./ImageStack/Vector.h"

#include <emscripten/bind.h>

#include <iostream>
#include <iomanip>
#include <memory>
#include <string>

#include <itkVersion.h>

#include <libde265/de265.h>

namespace
  {

  void _Info()
    {
    std::cout << "ImageStack Module: " << std::endl;
    std::cout << "  ITK Version - " << itk::Version::GetITKVersion() << std::endl;
    std::cout << "  libde265 Version - " << de265_get_version() << std::endl;
    }
    
  template<class TImage>
  class _Image
    {
    public:
      typedef std::unique_ptr<TImage> TImagePtr;
      explicit _Image(std::unique_ptr<TImage>&& ip_image)
        : mp_image(std::move(ip_image))
        {
        }   
    protected:
      const TImage& _GetImage() const
        {
        return *mp_image;
        }
      std::unique_ptr<TImage>& _AccessImagePtr()
        {
        return mp_image;
        }
    private:
      std::unique_ptr<TImage> mp_image;
    };

  class _ImageView : public _Image<itkjs::ImageStack::ImageView>
    {
    public:
      explicit _ImageView(TImagePtr&& ip_image)
        : _Image(std::move(ip_image))
        {
        }
      void Dispose()
        {
        _AccessImagePtr().reset();
        }        
      unsigned GetDimensions(unsigned i_index) const
        {
        return _GetImage().GetDimensions(i_index);
        }      
      unsigned GetComponentSize() const
        {
        return _GetImage().GetComponentSize();
        }
      void FillRGBAPixelBuffer(std::uintptr_t i_output_buf_ptr, unsigned i_output_buf_size) const
        {
        auto* p_output_buf = reinterpret_cast<unsigned char*>(i_output_buf_ptr);
        _GetImage().FillRGBAPixelBuffer(p_output_buf, i_output_buf_size);
        }
    };

  class _ImageSlice : public _Image<itkjs::ImageStack::ImageSlice>
    {
    public:
      explicit _ImageSlice(TImagePtr&& ip_image)
        : _Image(std::move(ip_image))
        {
        }
      void Dispose()
        {
        _AccessImagePtr().reset();
        }
      unsigned GetDimensions(unsigned i_index) const
        {
        return _GetImage().GetDimensions(i_index);
        }      
      unsigned GetComponentSize() const
        {
        return _GetImage().GetComponentSize();
        }
      _ImageView CalculateView(unsigned short i_contrast_from_min, unsigned short i_contrast_from_max, unsigned char i_contrast_to_min, unsigned char i_contrast_to_max) const
        {
        itkjs::ImageStack::ContrastFunction contrast_function;
        contrast_function.from[0] = i_contrast_from_min;
        contrast_function.from[1] = i_contrast_from_max;
        contrast_function.to[0] = i_contrast_to_min;
        contrast_function.to[1] = i_contrast_to_max;
        return _ImageView(_GetImage().CalculateView(contrast_function));
        }
    };
    
  class _ImageStack : public _Image<itkjs::ImageStack::ImageStack>
    {
    public:
      explicit _ImageStack(TImagePtr&& ip_image)
        : _Image(std::move(ip_image))
        {
        }
      void Dispose()
        {
        _AccessImagePtr().reset();
        }
      unsigned GetDimensions(unsigned i_index) const
        {
        return _GetImage().GetDimensions(i_index);
        }
      unsigned GetComponentSize() const
        {
        return _GetImage().GetComponentSize();
        }
      double GetSpacings(unsigned i_index) const
        {
        return _GetImage().GetSpacings()[i_index];
        }
      double GetOrigin(unsigned i_index) const
        {
        return _GetImage().GetOrigin()[i_index];
        }
      double GetVecX(unsigned i_index) const
        {
        return _GetImage().GetVecX()[i_index];
        }
      double GetVecY(unsigned i_index) const
        {
        return _GetImage().GetVecY()[i_index];
        }
      double GetVecZ(unsigned i_index) const
        {
        return _GetImage().GetVecZ()[i_index];
        }
      _ImageSlice GetSlice(unsigned i_index) const
        {
        return _ImageSlice(_GetImage().GetSlice(i_index));
        }
      _ImageSlice GetArbitrarySlice(double i_ox, double i_oy, double i_oz, double i_xx, double i_xy, double i_xz, double i_yx, double i_yy, double i_yz) const
        {
        return _ImageSlice(_GetImage().GetArbitrarySlice({i_ox, i_oy, i_oz}, {i_xx, i_xy, i_xz}, {i_yx, i_yy, i_yz}));
        }
    };
    
  class _ImageStackBuilder
    {
    public:
      _ImageStackBuilder()
        : mp_impl(new itkjs::ImageStack::ImageStackBuilder())
        {
        }
        
      void Dispose()
        {
        mp_impl.reset();
        }
        
      void OnHeaderLoadingProgress(emscripten::val i_callback)
        {
        mp_impl->OnHeaderLoadingProgress(
          [i_callback](unsigned i_processed_cnt, unsigned i_total_cnt) -> void
            {
            i_callback(i_processed_cnt, i_total_cnt);
            } );
        }
        
      void OnDataLoadingProgress(emscripten::val i_callback)
        {
        mp_impl->OnDataLoadingProgress(
          [i_callback](unsigned i_processed_cnt, unsigned i_total_cnt) -> void
            {
            i_callback(i_processed_cnt, i_total_cnt);
            } );
        }
        
      void OnDataDecodingProgress(emscripten::val i_callback)
        {
        mp_impl->OnDataDecodingProgress(
          [i_callback](unsigned i_processed_cnt, unsigned i_total_cnt) -> void
            {
            i_callback(i_processed_cnt, i_total_cnt);
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
          [i_on_failed_callback](const std::string& i_description) -> void
            {
            i_on_failed_callback(i_description);
            },
          [i_status_callback](const std::string& i_description) -> void
            {
            i_status_callback(i_description);
            } );
        }
        
    private:
      std::unique_ptr<itkjs::ImageStack::ImageStackBuilder> mp_impl;
    };
  }

using namespace emscripten;
EMSCRIPTEN_BINDINGS(Test)
  {
  function("Info", &_Info);

  class_<_ImageView>("ImageView")
    .function("dispose", &_ImageView::Dispose)
    .function("getDimensions", &_ImageView::GetDimensions)
    .function("getComponentSize", &_ImageView::GetComponentSize)
    .function("fillRGBAPixelBuffer", &_ImageView::FillRGBAPixelBuffer);

  class_<_ImageSlice>("ImageSlice")
    .function("dispose", &_ImageSlice::Dispose)
    .function("getDimensions", &_ImageSlice::GetDimensions)
    .function("getComponentSize", &_ImageSlice::GetComponentSize)
    .function("calculateView", &_ImageSlice::CalculateView);

  class_<_ImageStack>("ImageStack")
    .function("dispose", &_ImageStack::Dispose)
    .function("getDimensions", &_ImageStack::GetDimensions)
    .function("getComponentSize", &_ImageStack::GetComponentSize)
    .function("getSpacings", &_ImageStack::GetSpacings)
    .function("getOrigin", &_ImageStack::GetOrigin)
    .function("getVecX", &_ImageStack::GetVecX)
    .function("getVecY", &_ImageStack::GetVecY)
    .function("getVecZ", &_ImageStack::GetVecZ)
    .function("getSlice", &_ImageStack::GetSlice)
    .function("getArbitrarySlice", &_ImageStack::GetArbitrarySlice);

  class_<_ImageStackBuilder>("ImageStackBuilder")
    .constructor<>()
    .function("dispose", &_ImageStackBuilder::Dispose)
    .function("onHeaderLoadingProgress", &_ImageStackBuilder::OnHeaderLoadingProgress)
    .function("onDataLoadingProgress", &_ImageStackBuilder::OnDataLoadingProgress)
    .function("onDataDecodingProgress", &_ImageStackBuilder::OnDataDecodingProgress)
    .function("loadDataAsync", &_ImageStackBuilder::LoadDataAsync);
  }
