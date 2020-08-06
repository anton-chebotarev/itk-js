#include "HEVCAsyncDataDecoder.h"

#include "IDataDestination.h"
#include "NullProgressReporter.h"
#include "Header.h"
#include "MemoryManagement.h"

#include <libde265/de265.h>

#include <emscripten/html5.h>

namespace
  {
  void _Checked(de265_error i_err)
    {
    if (i_err != DE265_OK)
      throw std::runtime_error(std::string("Decoder error: ") + de265_get_error_text(i_err));
    }
  }

namespace itkjs
  {
  namespace ImageStack
    {
      
    ////////////////////////////////////////////////////////////////////////
            
    HEVCAsyncDataDecoder::HEVCAsyncDataDecoder(IDataDestination& ir_data_destination)
      : mr_data_destination(ir_data_destination)
      , mp_progress_reporter(std::unique_ptr<IProgressReporter>(new NullProgressReporter()))
      {
      }
      
    void HEVCAsyncDataDecoder::SetProgressReporter(std::unique_ptr<IProgressReporter>&& ip_reporter)
      {
      mp_progress_reporter = std::move(ip_reporter);
      }
      
    void HEVCAsyncDataDecoder::DecodeData(const Header& i_header, TUniqueMallocPtr&& ip_data_buffer, unsigned i_data_buffer_size) const
      {
        struct _UserData : public _IContext
          {
          const HEVCAsyncDataDecoder& parent;
          Header header;
          TUniqueMallocPtr p_data_buffer;
          const unsigned image_buffer_size;
          TUniqueBufferPtr p_image_buffer;
          uint8_t* p_buffer;
          uint16_t* p_buffer16;
          std::unique_ptr<de265_decoder_context, decltype(&de265_free_decoder)> p_ctx;
          uint8_t* p_input_buffer;
          unsigned remaining;
          unsigned count;
          unsigned num_frames;
          bool stop;
          
          _UserData(const HEVCAsyncDataDecoder& i_parent, const Header& i_header, TUniqueMallocPtr&& ip_data_buffer, unsigned i_data_buffer_size)
            : parent(i_parent)
            , header(i_header)
            , p_data_buffer(std::move(ip_data_buffer))
            , image_buffer_size(i_header.dimensions[0] * i_header.dimensions[1] * i_header.dimensions[2] * i_header.component_size)
            , p_ctx(de265_new_decoder(), de265_free_decoder)
            , remaining(i_data_buffer_size)
            , count(4096)
            , num_frames(0)
            , stop(0)
            {
            p_input_buffer = reinterpret_cast<uint8_t*>(p_data_buffer.get());
            p_image_buffer.reset(new uint8_t[image_buffer_size]);
            p_buffer = reinterpret_cast<uint8_t*>(p_image_buffer.get());
            p_buffer16 = reinterpret_cast<uint16_t*>(p_image_buffer.get());
            }
          };
          
        std::unique_ptr<_UserData> p_user_data(new _UserData(*this, i_header, std::move(ip_data_buffer), i_data_buffer_size));
        
        emscripten_request_animation_frame_loop([](double i_browser_time, void* ip_user_data) -> int
          {            
          _UserData& data = *reinterpret_cast<_UserData*>(ip_user_data);
          try
            {            
            // read a chunk of input data
            if (data.remaining <= data.count)
              {
              data.count = data.remaining;
              data.stop = true;
              }
            uint8_t* p_chunk = data.p_input_buffer;
            data.p_input_buffer += data.count;
            data.remaining -= data.count;

            // push input data chunk to decoder
            _Checked(de265_push_data(data.p_ctx.get(), p_chunk, data.count, 0, 0));              
            if (data.stop)
              _Checked(de265_flush_data(data.p_ctx.get()));

            int more = 1;
            while (more != 0)
              {
              // decode some more
              de265_error err = de265_decode(data.p_ctx.get(), &more);
              if (err == DE265_ERROR_WAITING_FOR_INPUT_DATA)
                break; // another chunk is required, breaking to outer loop to get it
              else
                _Checked(err);

              // get image
              const de265_image* p_img = de265_get_next_picture(data.p_ctx.get());
              if (!p_img)
                continue;

              int width = de265_get_image_width(p_img, 0);
              int height = de265_get_image_height(p_img, 0);
              if ((width != data.header.dimensions[0]) || (height != data.header.dimensions[1]))
                throw std::runtime_error("Invalid input data");

              de265_chroma chroma = de265_get_chroma_format(p_img);
              if (chroma != de265_chroma_mono)
                throw std::runtime_error("Invalid input data");

              int stride_y;
              const uint8_t* p_y = de265_get_image_plane(p_img, 0, &stride_y);
              if (!p_y)
                throw std::runtime_error("Invalid input data");

              int bpp_y = de265_get_bits_per_pixel(p_img, 0);
              if (bpp_y > 8)
                {
                if ((bpp_y != 12) || data.header.component_size != 2)
                  throw std::runtime_error("Invalid input data");
                int bpp_y_diff = 16 - bpp_y;
                const uint16_t* p_y16 = reinterpret_cast<const uint16_t*>(p_y);
                for (unsigned i = 0, sz = width * height; i < sz; ++i)
                  {
                  *data.p_buffer16 = *p_y16 << bpp_y_diff;
                  ++data.p_buffer16;
                  ++p_y16;
                  }
                }
              else
                {
                if ((bpp_y != 8) || data.header.component_size != 1)
                  throw std::runtime_error("Invalid input data");
                for (unsigned i = 0, sz = width * height; i < sz; ++i)
                  {
                  *data.p_buffer = *p_y;
                  ++data.p_buffer;
                  ++p_y;
                  }
                }

              ++data.num_frames;
              data.parent.mp_progress_reporter->OnProgress(data.num_frames, data.header.dimensions[2]);
              }
            
            if (data.stop)
              {
              if (data.num_frames != data.header.dimensions[2])
                throw std::runtime_error("Invalid input data");
              data.p_data_buffer.reset();
              data.p_ctx.reset();
              data.parent.mr_data_destination.OnSuccess(std::move(data.p_image_buffer), data.image_buffer_size);
              data.parent.mp_context.reset();
              return false; // break [emscripten_request_animation_frame_loop]
              }
              
            return true; // continue [emscripten_request_animation_frame_loop]
            }
          catch (const std::exception& i_e)
            {
            data.parent.mp_context.reset();
            data.parent.mr_data_destination.OnFailure(i_e.what());
            return false; // break [emscripten_request_animation_frame_loop]
            }
          catch (...)
            {
            data.parent.mp_context.reset();
            data.parent.mr_data_destination.OnFailure("Exception during HEVC decoding");
            return false; // break [emscripten_request_animation_frame_loop]
            }
          }, reinterpret_cast<void*>(p_user_data.get()));
          
        mp_context.reset(p_user_data.release());
      }
      
    ////////////////////////////////////////////////////////////////////////      
      
    }  
  }