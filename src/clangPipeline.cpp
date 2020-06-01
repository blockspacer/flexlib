#include "flexlib/clangPipeline.hpp" // IWYU pragma: associated

#include <base/logging.h>

namespace clang_utils {

SourceTransformPipeline::SourceTransformPipeline()
{
  DETACH_FROM_SEQUENCE(sequence_checker_);
}

} // namespace clang_utils
