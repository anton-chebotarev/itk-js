find_path(RapidJSON_INCLUDE_DIR NAMES rapidjson/document.h)
include_directories(${RapidJSON_INCLUDE_DIR})

set(ITKJS_SRCS
  Dummy.cxx
  )
itk_module_add_library(ITKJS ${ITKJS_SRCS})

file(GLOB SOURCES Test.cxx ./ImageStack/*.cxx ./libde265/*.o)

set(target ImageStack)
add_executable(${target} ${SOURCES})
itk_module_target_label(${target})
itk_module_target_export(${target})
itk_module_target_install(${target})
set_property(TARGET ${target} APPEND_STRING PROPERTY LINK_FLAGS " --bind -s ASSERTIONS=1 -s TOTAL_MEMORY=536870912 -s ALLOW_MEMORY_GROWTH=1")
set_property(TARGET ${target} APPEND_STRING PROPERTY LINK_FLAGS " -s WASM=0 -lnodefs.js -s EXIT_RUNTIME=0 -s INVOKE_RUN=0")
target_link_libraries(${target} LINK_PUBLIC ${ITK_LIBRARIES})

#set(wasm_target ImageStackWasm)
#add_executable(${wasm_target} ${SOURCES})
#itk_module_target_label(${wasm_target})
#itk_module_target_export(${wasm_target})
#itk_module_target_install(${wasm_target})
#set_property(TARGET ${wasm_target} APPEND_STRING PROPERTY LINK_FLAGS " --bind -s ASSERTIONS=1 -s TOTAL_MEMORY=536870912 -s ALLOW_MEMORY_GROWTH=1")
#set_property(TARGET ${wasm_target} APPEND_STRING PROPERTY LINK_FLAGS " -s EXPORT_NAME=Version -s WASM_ASYNC_COMPILATION=0 -s MODULARIZE=1 -s WASM=1 -lworkerfs.js -s EXIT_RUNTIME=0 -s INVOKE_RUN=1")
#target_link_libraries(${wasm_target} LINK_PUBLIC ${ITK_LIBRARIES})

