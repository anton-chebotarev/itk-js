find_path(RapidJSON_INCLUDE_DIR NAMES rapidjson/document.h)
include_directories(${RapidJSON_INCLUDE_DIR})

set(ITKJS_SRCS
  Dummy.cxx
  )
itk_module_add_library(ITKJS ${ITKJS_SRCS})

file(GLOB SOURCES Bindings.cxx ./ImageStack/*.cxx ./libde265/*.o)

set(target ImageStack)
add_executable(${target} ${SOURCES})
itk_module_target_label(${target})
itk_module_target_export(${target})
itk_module_target_install(${target})
set_property(TARGET ${target} APPEND_STRING PROPERTY LINK_FLAGS " --bind")
#set_property(TARGET ${target} APPEND_STRING PROPERTY LINK_FLAGS " -s ALLOW_BLOCKING_ON_MAIN_THREAD=0 -s USE_PTHREADS=1 -s ALLOW_MEMORY_GROWTH=0 -s TOTAL_MEMORY=536870912")
set_property(TARGET ${target} APPEND_STRING PROPERTY LINK_FLAGS " -s ALLOW_MEMORY_GROWTH=1")
set_property(TARGET ${target} APPEND_STRING PROPERTY LINK_FLAGS " -s ASSERTIONS=1")
set_property(TARGET ${target} APPEND_STRING PROPERTY LINK_FLAGS " -s DISABLE_EXCEPTION_CATCHING=0")
set_property(TARGET ${target} APPEND_STRING PROPERTY LINK_FLAGS " -s EXPORT_NAME=ImageStackModule -s MODULARIZE=1 -s WASM=0 -s LEGACY_VM_SUPPORT=1 -s POLYFILL_OLD_MATH_FUNCTIONS=1 -s EXIT_RUNTIME=0 -s INVOKE_RUN=0")
target_link_libraries(${target} LINK_PUBLIC ${ITK_LIBRARIES})

set(wasm_target ImageStackWasm)
add_executable(${wasm_target} ${SOURCES})
itk_module_target_label(${wasm_target})
itk_module_target_export(${wasm_target})
itk_module_target_install(${wasm_target})
set_property(TARGET ${wasm_target} APPEND_STRING PROPERTY LINK_FLAGS " --bind")
set_property(TARGET ${target} APPEND_STRING PROPERTY LINK_FLAGS " -s ALLOW_MEMORY_GROWTH=1")
set_property(TARGET ${wasm_target} APPEND_STRING PROPERTY LINK_FLAGS " -s EXPORT_NAME=ImageStackModule -s MODULARIZE=1 -s WASM=1 -s EXIT_RUNTIME=0 -s INVOKE_RUN=0")
target_link_libraries(${wasm_target} LINK_PUBLIC ${ITK_LIBRARIES})

