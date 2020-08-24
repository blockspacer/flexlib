include_guard( DIRECTORY )

list(APPEND flexlib_SOURCES
  ${flexlib_src_DIR}/per_plugin_settings.cpp
  ${flexlib_include_DIR}/per_plugin_settings.hpp
  #
  ${flexlib_src_DIR}/ToolPlugin.cc
  ${flexlib_include_DIR}/ToolPlugin.hpp
  #
  ${flexlib_src_DIR}/boost_command_line.cpp
  ${flexlib_include_DIR}/boost_command_line.hpp
  #
  ${flexlib_src_DIR}/CXXCTP_STD/CXXCTP_STD.cpp
  ${flexlib_include_DIR}/CXXCTP_STD/CXXCTP_STD.hpp
  #
  ${flexlib_src_DIR}/options/ctp/options.cpp
  ${flexlib_include_DIR}/options/ctp/options.hpp
  ${flexlib_include_DIR}/clangUtils.hpp
  ${flexlib_src_DIR}/clangUtils.cpp
  ${flexlib_include_DIR}/funcParser.hpp
  ${flexlib_src_DIR}/funcParser.cpp
  #${flexlib_src_DIR}/DispatchQueue.cpp
  ${flexlib_include_DIR}/DispatchQueue.hpp
  ${flexlib_include_DIR}/matchers/annotation_matcher.hpp
  ${flexlib_src_DIR}/matchers/annotation_matcher.cc
  ${flexlib_include_DIR}/annotation_match_handler.hpp
  ${flexlib_src_DIR}/annotation_match_handler.cc
  ${flexlib_include_DIR}/annotation_parser.hpp
  ${flexlib_src_DIR}/annotation_parser.cc
  ${flexlib_include_DIR}/parser_constants.hpp
  ${flexlib_src_DIR}/parser_constants.cc
  #
  ${flexlib_include_DIR}/clangPipeline.hpp
  #
  ${flexlib_src_DIR}/clangPipeline.cpp
  #
  ${flexlib_src_DIR}/ClingInterpreterModule.cpp
  ${flexlib_include_DIR}/ClingInterpreterModule.hpp
  ${flexlib_src_DIR}/utils.cpp
  ${flexlib_include_DIR}/utils.hpp
  ${flexlib_src_DIR}/inputThread.cpp
  ${flexlib_include_DIR}/inputThread.hpp
  ${flexlib_src_DIR}/reflect/ReflectionCache.cpp
  ${flexlib_include_DIR}/reflect/ReflectionCache.hpp
  ${flexlib_src_DIR}/reflect/ReflTypes.cpp
  ${flexlib_include_DIR}/reflect/ReflTypes.hpp
  ${flexlib_src_DIR}/reflect/TypeInfo.cpp
  ${flexlib_include_DIR}/reflect/TypeInfo.hpp
  ${flexlib_src_DIR}/reflect/ReflectAST.cpp
  ${flexlib_include_DIR}/reflect/ReflectAST.hpp
  ${flexlib_include_DIR}/reflect/ast_utils.hpp
  ${flexlib_include_DIR}/template_engine/CXTPL_AnyDict.hpp
  ${flexlib_src_DIR}/template_engine/CXTPL_AnyDict.cpp
  ${flexlib_include_DIR}/template_engine/I_Dict.hpp
  ${flexlib_src_DIR}/template_engine/I_Dict.cpp
  ${flexlib_include_DIR}/integrations/outcome/error_utils.hpp
  ${flexlib_include_DIR}/integrations/outcome/error_macros.hpp
  ${flexlib_src_DIR}/core/errors/errors_CodegenError.cpp
  ${flexlib_include_DIR}/core/errors/errors.hpp
  ${flexlib_src_DIR}/ctp_registry.cpp
  ${flexlib_include_DIR}/ctp_registry.hpp
)

# without rtti you will get error:
# ERROR: boost::bad_any_cast: failed conversion using boost::any_cast
set_source_files_properties(
  ${flexlib_src_DIR}/boost_command_line.cpp
  ${flexlib_include_DIR}/boost_command_line.hpp
  PROPERTIES
    COMPILE_FLAGS
      -frtti)

# LLVM and Clang are compiled with -fno-rtti by default.  You have three options:
# - You can re-enable RTTI when you build LLVM and Clang.
# - If you don't use RTTI in your project, you can just compile with -fno-rtti.
# - You can compile every file that emits the vtable for this class with -fno-rtti.  The easier way to do this is to anchor the vtable to a particular file and compile that with -fno-rtti.
set_source_files_properties(
  ${flexlib_src_DIR}/matchers/annotation_matcher.cc
  PROPERTIES
  COMPILE_FLAGS
  -fno-rtti)
#
set_source_files_properties(
  ${flexlib_src_DIR}/annotation_match_handler.cc
  PROPERTIES
  COMPILE_FLAGS
  -fno-rtti)
#
set_source_files_properties(
  ${flexlib_src_DIR}/annotation_parser.cc
  PROPERTIES
  COMPILE_FLAGS
  -fno-rtti)
