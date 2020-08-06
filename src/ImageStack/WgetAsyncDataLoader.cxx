#include "WgetAsyncDataLoader.h"

#include "IDataDestination.h"

#include "NullProgressReporter.h"

#include <emscripten/emscripten.h>

#include <iostream>
#include <iomanip>
#include <sstream>
#include <math.h>

namespace itkjs
  {
  namespace ImageStack
    {
      
    ////////////////////////////////////////////////////////////////////////
            
    WgetAsyncDataLoader::WgetAsyncDataLoader(IDataDestination& ir_data_destination)
      : mr_data_destination(ir_data_destination)
      , mp_progress_reporter(std::unique_ptr<IProgressReporter>(new NullProgressReporter()))
      , mh_loader_handle(-1)
      {
      mp_callbacks_handler.reset(new CallbacksHandler(*this));
      }
      
    WgetAsyncDataLoader::~WgetAsyncDataLoader()
      {
      TerminateDataLoading();
      }
      
    void WgetAsyncDataLoader::SetProgressReporter(std::unique_ptr<IProgressReporter>&& ip_reporter)
      {
      mp_progress_reporter = std::move(ip_reporter);
      }

    void WgetAsyncDataLoader::LoadDataAsync(const std::string& i_data_url)
      {
      void* arg = reinterpret_cast<void*>(mp_callbacks_handler.get());
      mh_loader_handle = emscripten_async_wget2_data(
        i_data_url.c_str(), // url (const char*) – The URL of the file to load
        "GET",              // requesttype (const char*) – ‘GET’ or ‘POST’
        nullptr,            // param (const char*) – Request parameters for POST requests      
        arg,                // arg (void*) – User-defined data that is passed to the callbacks
        0,                  // free (int) – Tells the runtime whether to free the returned buffer after onload is complete
        [](unsigned /*ih_request*/, void* ip_user_data, void* ip_received_buffer, unsigned i_size_in_bytes) -> void
          {                 // onload (em_async_wget2_data_onload_func) – Callback on successful load of the file.
          reinterpret_cast<WgetAsyncDataLoader::CallbacksHandler*>(ip_user_data)->OnSuccess(TUniqueMallocPtr(ip_received_buffer, &free), i_size_in_bytes);
          },
        [](unsigned /*ih_request*/, void* ip_user_data, int i_http_error_code, const char* i_status_description) -> void
          {                 // onerror (em_async_wget2_data_onerror_func) – Callback in the event of failure.
          std::ostringstream ss;
          ss << "HTTP_" << i_http_error_code << ": " << i_status_description;
          reinterpret_cast<WgetAsyncDataLoader::CallbacksHandler*>(ip_user_data)->OnFailure(ss.str());
          },
        [](unsigned /*ih_request*/, void* ip_user_data, int i_bytes_loaded, int i_bytes_total) -> void
          {                 // onprogress (em_async_wget2_data_onprogress_func) – Callback called (regularly) during load of the file to update progress.
          reinterpret_cast<WgetAsyncDataLoader::CallbacksHandler*>(ip_user_data)->OnProgress(static_cast<unsigned>(i_bytes_loaded), static_cast<unsigned>(i_bytes_total));
          } );
      }
      
    void WgetAsyncDataLoader::TerminateDataLoading()
      {
      if (mh_loader_handle >= 0)
        emscripten_async_wget2_abort(mh_loader_handle);
      }
      
    ////////////////////////////////////////////////////////////////////////
    
    WgetAsyncDataLoader::CallbacksHandler::CallbacksHandler(WgetAsyncDataLoader& ir_data_loader)
      : mr_data_loader(ir_data_loader)
      {
      }

    void WgetAsyncDataLoader::CallbacksHandler::OnSuccess(TUniqueMallocPtr&& ip_buffer, unsigned i_size_in_bytes)
      {
      mr_data_loader.mh_loader_handle = -1;
      mr_data_loader.mr_data_destination.OnSuccess(std::move(ip_buffer), i_size_in_bytes);
      }
      
    void WgetAsyncDataLoader::CallbacksHandler::OnFailure(const std::string& i_description)
      {
      mr_data_loader.mh_loader_handle = -1;
      mr_data_loader.mr_data_destination.OnFailure(i_description); 
      }
      
    void WgetAsyncDataLoader::CallbacksHandler::OnProgress(unsigned i_bytes_loaded, unsigned i_bytes_total)
      {
      mr_data_loader.mp_progress_reporter->OnProgress(i_bytes_loaded, i_bytes_total);              
      }
      
    ////////////////////////////////////////////////////////////////////////      
      
    }  
  }