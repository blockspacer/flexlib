﻿#pragma once

#if 0
#include <functional>
#include <memory>
#include <string>
#include <sstream>

namespace cxtpl_util {

class I_Dict {
public:
    typedef std::function<void(std::unique_ptr<std::string> res)>
      interp_callback;

    virtual ~I_Dict();

    void createFromFile(const std::string& path);

    void createFromString(const std::string& code);

    void buildToFile(const std::string& path);

    void loadBuiltFromString(const std::string &code);

    void loadBuiltFromFile(const std::string &path);

    std::string buildToString();

    template <typename K>
    std::string argRefToCling(const std::string varType,
                              const std::string varName, const K& arg) {
        std::ostringstream sstr;
        sstr << " ; \n " << varType << " & " << varName << " = ";
        sstr << "*( " << varType << " * )("
             // Pass a pointer into cling as a string.
             << std::hex << std::showbase
             << reinterpret_cast<size_t>(&arg) << "); \n";
        return sstr.str();
    }

protected:
    void rebuild();

    // TODO: WASM & Node.js support
    // TODO: filtering
    // TODO: parse-time code exec
    // TODO: both runtime cling exec & native compile run support
    // TODO: error reporting with line numbers
    // TODO: unit tests
    // TODO: open/close tag customization
    // TODO: refactor
    // TODO: multithreaded transpiling of templates
    // TODO: docs, sanitize, iwyu, examples, CI/CD
    // TODO: support library, ranges-style
    // TODO: similar projects
    // + https://github.com/burner/sweet.hpp/tree/master/amber
    // TODO: example with includes workaround
    // TODO: example with external function call workaround
    std::string buildFromString(const std::string& input);

    // see
    // https://bits.theorem.co/how-to-write-a-template-library/
    // https://lambda.xyz/blog/maud-is-fast/
    // https://dzone.com/articles/modern-type-safe-template-engines
    // http://www.wilxoft.com/
    // https://github.com/djc/askama
    // https://www.reddit.com/r/rust/comments/b06z9m/cuach_a_compiletime_html_template_system/
    std::string prepareForCling(const std::string& input
      /*, const std::string& clinja_args*/);

    std::string code_before_build_;
    std::string code_after_build_;
    std::string code_for_cling_after_build_;
};

template<typename T>
class Dict {
};

} // namespace cxtpl_util
#endif
