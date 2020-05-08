#include "flexlib/parser_constants.hpp" // IWYU pragma: associated

namespace cxxctp {

// USAGE:
// auto hasAnnotate = clang::ast_matchers::hasAttr( clang::attr::Annotate );
// auto finderMatcher = clang::ast_matchers::decl( hasAnnotate ).bind( kAnnotateAttrName );
// Finder_.addMatcher(finderMatcher, &Checker_);
const char kAnnotateAttrName[] = "bind_gen";

// USAGE:
// // will be replaced with 1234
// __attribute__((annotate("{gen};{executeCodeAndReplace};\
// new llvm::Optional<std::string>{\"1234\"};")))
// int SOME_UNIQUE_NAME2
// ;
const char kRequiredAnnotationPrefix[] = "{gen};";

} // namespace cxxctp
