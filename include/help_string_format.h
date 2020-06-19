// Copyright (c) 2020 Jasper Braun
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
// THE SOFTWARE.

#ifndef ARG_PARSE_CONVERT_HELP_FORMAT_H_
#define ARG_PARSE_CONVERT_HELP_FORMAT_H_

#include <sstream>
#include <string>

#include "exceptions.h"

namespace arg_parse_convert {

/// @brief Contains parameters values that determine formatting of help strings
///  created by `ArgumentMap::HelpString` and `Argument::HelpString`.
///
class HelpStringFormat {
 public:
  /// @name Constructors:
  ///
  /// @{
  
  HelpStringFormat() = default;

  HelpStringFormat(int help_string_width, int parameter_indentation,
                   int description_indentation)
      : width_{help_string_width},
        parameter_indentation_{parameter_indentation},
        description_indentation_{description_indentation} {}
  
  HelpStringFormat(const HelpStringFormat& other) = default;
  HelpStringFormat(HelpStringFormat&& other) = default;
  /// @}
  
  /// @name Assignment:
  ///
  /// @{
  
  HelpStringFormat& operator=(const HelpStringFormat& other) = default;
  HelpStringFormat& operator=(HelpStringFormat&& other) = default;
  /// @}

  /// @name Accessors:
  ///
  /// @{
  
  /// @brief Returns the help string's width.
  ///
  inline int width() const {return width_;}

  /// @brief Returns the help string's parameter name indentation.
  ///
  inline int parameter_indentation() const {return parameter_indentation_;}

  /// @brief Returns the help string's parameter description indentation.
  ///
  inline int description_indentation() const {return description_indentation_;}

  /// @brief Returns the help string's header.
  ///
  inline const std::string& header() const {return header_;}

  /// @brief Returns the help string's footer.
  ///
  inline const std::string& footer() const {return footer_;}
  /// @}

  /// @name Mutators:
  ///
  /// @{

  /// @brief Sets the help string's width.
  ///
  inline void width(int value) {
    TestParameters(value, parameter_indentation_, description_indentation_);
    width_ = value;
  }

  /// @brief Sets the help string's parameter indentation.
  ///
  inline void parameter_indentation(int value) {
    TestParameters(width_, value, description_indentation_);
    parameter_indentation_ = value;
  }

  /// @brief Sets the help string's description indentation.
  ///
  inline void description_indentation(int value) {
    TestParameters(width_, parameter_indentation_, value);
    description_indentation_ = value;
  }

  /// @brief Sets the help string's header.
  ///
  inline void header(const std::string& value) {header_ = value;}
  
  /// @brief Sets the help string's header.
  ///
  inline void header(std::string&& value) {header_ = value;}

  /// @brief Sets the help string's footer.
  ///
  inline void footer(const std::string& value) {footer_ = value;}
  
  /// @brief Sets the help string's footer.
  ///
  inline void footer(std::string&& value) {footer_ = value;}

  /// @brief Sets the formatting parameters.
  ///
  /// @details `w` is the total width (in characters) of the help message,
  ///  `name_indent` is the indentation length for the name of each of the
  ///  parameters, and `descr_indent` is the indentation length for each of the
  ///  parameters' description. Throws exception of type
  ///  `InvalidFormattingParameters` if one of the parameters is negative, or if
  ///  one of `name_indent`, or `descr_indent` is equal to or larger than `w`.
  ///
  inline void Set(int w = 40, int name_indent = 4, int descr_indent = 8) {
    TestParameters(w, name_indent, descr_indent);
    width_ = w;
    parameter_indentation_ = name_indent;
    description_indentation_ = descr_indent;
  }
  /// @}
 private:
  inline void TestParameters(int w, int name_indent, int descr_indent) {
    std::stringstream error_message;
    if (w < 0 || name_indent < 0 || descr_indent < 0) {
      error_message << "Formatting parameters must not be negative: "
                    << "(width = " << w << ", parameter_indentation = "
                    << name_indent << ", description_indentation = "
                    << descr_indent << ")." << std::endl;
      throw exceptions::InvalidFormattingParameters(error_message.str());
    } else if (name_indent + 1 >= w || descr_indent + 1 >= w) {
      error_message << "Indentation lengths must allow at least 2 characters of"
                    << " space for the rest of the line: (width = " << w
                    << ", parameter_indentation = " << name_indent
                    << ", description_indentation = " << descr_indent << ")."
                    << std::endl;
      throw exceptions::InvalidFormattingParameters(error_message.str());
    }
  }
  int width_;
  int parameter_indentation_;
  int description_indentation_;
  std::string header_;
  std::string footer_;
};

} // namespace arg_parse_convert

#endif // ARG_PARSE_CONVERT_HELP_FORMAT_H_