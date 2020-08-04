#include "AsyncDataLoader.h"

#include "IDataDestination.h"

#include <emscripten/emscripten.h>

#include <iostream>
#include <iomanip>
#include <math.h>

namespace itkjs
  {
  namespace ImageStack
    {
      
    ////////////////////////////////////////////////////////////////////////
            
    AsyncDataLoader::AsyncDataLoader(IDataDestination& ir_data_destination)
      : mr_data_destination(ir_data_destination)
      , mp_progress_reporter(std::unique_ptr<ILoadingProgressReporter>(new NullLoadingProgressReporter()))
      , mh_loader_handle(-1)
      {
      mp_callbacks_handler.reset(new CallbacksHandler(*this));
      }
      
    AsyncDataLoader::~AsyncDataLoader()
      {
      TerminateDataLoading();
      }
      
    void AsyncDataLoader::SetProgressReporter(std::unique_ptr<ILoadingProgressReporter>&& ip_reporter)
      {
      mp_progress_reporter = std::move(ip_reporter);
      }

    void AsyncDataLoader::LoadDataAsync(const std::string& i_data_url)
      {
      void* arg = reinterpret_cast<void*>(mp_callbacks_handler.get());
      mh_loader_handle = emscripten_async_wget2_data(
        i_data_url.c_str(), // url (const char*) – The URL of the file to load
        "GET",              // requesttype (const char*) – ‘GET’ or ‘POST’
        nullptr,            // param (const char*) – Request parameters for POST requests      
        arg,                // arg (void*) – User-defined data that is passed to the callbacks
        true,               // free (int) – Tells the runtime whether to free the returned buffer after onload is complete
        [](unsigned ih_request, void* ip_user_data, void* ip_received_buffer, unsigned i_size_in_bytes) -> void
          {                 // onload (em_async_wget2_data_onload_func) – Callback on successful load of the file.
          reinterpret_cast<AsyncDataLoader::CallbacksHandler*>(ip_user_data)->OnSuccess(ip_received_buffer, i_size_in_bytes);
          },
        [](unsigned ih_request, void* ip_user_data, int i_http_error_code, const char* i_status_description) -> void
          {                 // onerror (em_async_wget2_data_onerror_func) – Callback in the event of failure.
          reinterpret_cast<AsyncDataLoader::CallbacksHandler*>(ip_user_data)->OnFailure(i_http_error_code, i_status_description);
          },
        [](unsigned ih_request, void* ip_user_data, int i_bytes_loaded, int i_bytes_total) -> void
          {                 // onprogress (em_async_wget2_data_onprogress_func) – Callback called (regularly) during load of the file to update progress.
          reinterpret_cast<AsyncDataLoader::CallbacksHandler*>(ip_user_data)->OnProgress(i_bytes_loaded, i_bytes_total);
          }
        );
      }
      
    void AsyncDataLoader::TerminateDataLoading()
      {
      if (mh_loader_handle >= 0)
        emscripten_async_wget2_abort(mh_loader_handle);
      }
      
    ////////////////////////////////////////////////////////////////////////
    
    AsyncDataLoader::CallbacksHandler::CallbacksHandler(AsyncDataLoader& ir_data_loader)
      : mr_data_loader(ir_data_loader)
      {
      }

    void AsyncDataLoader::CallbacksHandler::OnSuccess(void* ip_received_buffer, unsigned i_size_in_bytes)
      {
      mr_data_loader.mh_loader_handle = -1;      
      mr_data_loader.mr_data_destination.ProcessLoadedData(ip_received_buffer, i_size_in_bytes);
      }
      
    void AsyncDataLoader::CallbacksHandler::OnFailure(int i_http_error_code, const char* i_status_description)
      {
      mr_data_loader.mh_loader_handle = -1;
      throw std::runtime_error(i_status_description);     
      }
      
    void AsyncDataLoader::CallbacksHandler::OnProgress(int i_bytes_loaded, int i_bytes_total)
      {
      mr_data_loader.mp_progress_reporter->OnProgress(i_bytes_loaded, i_bytes_total);              
      }
      
    ////////////////////////////////////////////////////////////////////////
      
    void NullLoadingProgressReporter::OnProgress(int /*i_bytes_loaded*/, int /*i_bytes_total*/) const
      {
      }
      
    ////////////////////////////////////////////////////////////////////////
      
    StdoutLoadingProgressReporter::StdoutLoadingProgressReporter(const std::string& i_stdout_prefix/* = "Data Loader: "*/)
      : m_stdout_prefix(i_stdout_prefix)
      {
      }
      
    void StdoutLoadingProgressReporter::OnProgress(int i_bytes_loaded, int i_bytes_total) const
      {
      std::cout << m_stdout_prefix << i_bytes_loaded;
      if (i_bytes_total > 0)
        std::cout << " of " << i_bytes_total << " bytes received (" << round(i_bytes_loaded / i_bytes_total * 100) << "%)";
      else
        std::cout << " of ? bytes received";
      std::cout << std::endl;  
      }
      
    ////////////////////////////////////////////////////////////////////////      
      
    }  
  }