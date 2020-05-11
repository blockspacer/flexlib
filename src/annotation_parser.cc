#include "flexlib/annotation_parser.hpp" // IWYU pragma: associated

#include "flexlib/parser_constants.hpp"

namespace flexlib {

AnnotationParser::AnnotationParser(
  AnnotationMethods* supportedAnnotationMethods)
  : annotationMethods(supportedAnnotationMethods)
{
  DETACH_FROM_SEQUENCE(sequence_checker_);

  DCHECK(annotationMethods);
}

bool AnnotationParser::tryRemovePrefix(
    const std::string& unprocessed
    , std::string& resultWithoutPrefix
    , const std::string& prefix)
{
  DCHECK_CALLED_ON_VALID_SEQUENCE(sequence_checker_);

  const bool startsWithPrefix
    = unprocessed.rfind(prefix, 0) == 0;
  if(!startsWithPrefix) {
    return false;
  }
  resultWithoutPrefix = unprocessed;
  resultWithoutPrefix.erase(0, prefix.size());
  return true;
}

AnnotationMethods::const_iterator AnnotationParser::parseToMethods(
  const std::string& unprocessedAnnotation
  , std::string& resultWithoutMethod)
{
  DCHECK_CALLED_ON_VALID_SEQUENCE(sequence_checker_);

  std::string resultWithoutPrefix;
  const bool startsWithGen
    = tryRemovePrefix(unprocessedAnnotation
                      , resultWithoutPrefix
                      , ::flexlib::kRequiredAnnotationPrefix);
  if(!startsWithGen) {
    return annotationMethods->end();
  }

  DCHECK(annotationMethods);
  for(auto callback_iter = annotationMethods->begin();
      callback_iter != annotationMethods->end();
      ++callback_iter)
  {
    const bool startsWithMethod
      = tryRemovePrefix(resultWithoutPrefix
                        , resultWithoutMethod
                        , callback_iter->first);
    if(!startsWithMethod) {
      continue;
    }
    return callback_iter;
  }

  return annotationMethods->end();
}

} // namespace flexlib
