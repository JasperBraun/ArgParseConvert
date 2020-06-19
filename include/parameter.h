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

#ifndef ARG_PARSE_CONVERT_PARAMETER_H_
#define ARG_PARSE_CONVERT_PARAMETER_H_

#include <string>
#include <vector>

#include "conversion_functions.h"
#include "exceptions.h"
#include "help_string_format.h"

namespace arg_parse_convert {

enum class ParameterCategory {
  kPositionalParameter,
  kKeywordParameter,
  kFlag
};

class ParameterConfiguration {
 public:
  /// @name Constructors:
  ///
  /// @{
  
  ParameterConfiguration(const ParameterConfiguration& other) = default;
  ParameterConfiguration(ParameterConfiguration&& other) = default;
  /// @}
  
  /// @name Assignment:
  ///
  /// @{
  
  ParameterConfiguration& operator=(const ParameterConfiguration& other) = default;
  ParameterConfiguration& operator=(ParameterConfiguration&& other) = default;
  /// @}

  /// @name Accessors:
  ///
  /// @{
  
  /// @brief Returns the parameter's names.
  ///
  inline const std::vector<std::string>& names() const {return names_;}

  /// @brief Returns the parameter's category.
  ///
  inline ParameterCategory category() const {return category_;}

  /// @brief Returns the parameter's default argument list.
  ///
  inline const std::vector<std::string>& default_arguments() const {return default_arguments_;}

  /// @brief Returns the parameter's position.
  ///
  inline int position() const {return position_;}

  /// @brief Returns the parameter's minimum number of arguments.
  ///
  inline int min_num_arguments() const {return min_num_arguments_;}

  /// @brief Returns the parameter's maximum number of arguments.
  ///
  inline int max_num_arguments() const {return max_num_arguments_;}
  /// @}

  /// @name Other:
  ///
  /// @{
  
  /// @brief Returns a formatted help string of the parameter.
  ///
  /// @details The string consists of the parameter names indented by
  ///  `format.parameter_indentation` followed by its default values followed by
  ///  the parameter's description indented by `format.description_indentation`
  ///  on the next line. Only lines in the parameter's description are subject
  ///  to the width `format.width`.
  ///
  std::string HelpString(const HelpStringFormat& format);
  /// @}
 protected:
  // ParameterConfiguration objects are initialized through Parameter factories.
  ParameterConfiguration() = default;
  template <class ParameterType>
  friend class Parameter;
  static ParameterConfiguration Create(const std::vector<std::string>& names,
      ParameterCategory category);
  static ParameterConfiguration Create(
      const std::vector<std::string>& names, ParameterCategory category,
      int position);
 private:
  std::vector<std::string> names_;
  ParameterCategory category_;
  std::vector<std::string> default_arguments_;
  int position_;
  int min_num_arguments_{0};
  int max_num_arguments_{1};
  std::string description_;
  std::string argument_placeholder_{"<ARG>"};
};

template <class ParameterType>
class Parameter {
 public:
  /// @name Factories:
  ///
  /// @{

  /// @brief Creates a flag identified by `names`.
  ///
  static Parameter<bool> Flag(const std::vector<std::string>& names);

  /// @brief Creates a flag identified by `-c`.
  ///
  static Parameter<bool> Flag(char c);

  /// @brief Creates a positional parameter identified by `names`, with provided
  ///  relative position and conversion function.
  ///
  static Parameter<ParameterType> Positional(
      const std::vector<std::string>& names,
      int position,
      std::function<ParameterType(const std::string&)> converter);

  /// @brief Creates a positional parameter identified by `names`, with provided
  ///  relative position using `ParameterType's` static `from_string` function
  ///  member as conversion function.
  ///
  static Parameter<ParameterType> Positional(
      const std::vector<std::string>& names, int position);

  /// @brief Creates a keyword parameter identified by `names`, with provided
  ///  conversion function.
  ///
  static Parameter<ParameterType> Keyword(
      const std::vector<std::string>& names,
      std::function<ParameterType(const std::string&)> converter);

  /// @brief Creates a keyword parameter identified by `names`, using
  ///  `ParameterType's` static `from_string` function member as conversion
  ///  function.
  ///
  static Parameter<ParameterType> Keyword(
      const std::vector<std::string>& names);
  /// @}

  /// @name Constructors:
  ///
  /// @{

  Parameter(const Parameter& other) = default;
  Parameter(Parameter&& other) = default;
  /// @}
  
  /// @name Assignment:
  ///
  /// @{
  
  Parameter& operator=(const Parameter& other) = default;
  Parameter& operator=(Parameter&& other) = default;
  /// @}

  /// @name Accessors:
  ///
  /// @{
  
  /// @brief Returns the parameter's configuration.
  ///
  inline ParameterConfiguration configuration() const {return configuration_;}

  /// @brief Returns the parameter's converter.
  ///
  inline std::function<ParameterType(const std::string&)> converter() const {
    return converter_;
  }
  /// @}

  /// @name Mutators:
  ///
  /// @{

  /// @brief Appends a single default argument to the parameter's list of
  ///  default arguments.
  ///
  inline Parameter<ParameterType>& AddDefault(std::string argument) {
    configuration_.default_arguments_.push_back(argument);
    return *this;
  }
  
  /// @brief Sets the parameter's list of default arguments.
  ///
  inline Parameter<ParameterType>& SetDefault(
      std::vector<std::string> default_arguments) {
    configuration_.default_arguments_ = default_arguments;
    return *this;
  }

  /// @brief Sets the minimum number of arguments.
  ///
  inline Parameter<ParameterType>& MinArgs(int min) {
    configuration_.min_num_arguments_ = min;
    if (configuration_.max_num_arguments_ > 0
        && configuration_.max_num_arguments_ < min) {
      configuration_.max_num_arguments_ = min;
    }
    return *this;
  }

  /// @brief Sets the maximum number of arguments.
  ///
  inline Parameter<ParameterType>& MaxArgs(int max) {
    configuration_.max_num_arguments_ = max;
    return *this;
  }

  /// @brief Sets the parameter's description.
  ///
  inline Parameter<ParameterType>& Description(std::string description) {
    configuration_.description_ = description;
    return *this;
  }

  /// @brief Sets the argument's placeholder in the parameter's help string.
  ///
  inline Parameter<ParameterType>& ArgPlaceholder(std::string placeholder) {
    configuration_.argument_placeholder_ = placeholder;
    return *this;
  }
  /// @}
 protected:
  // Used only in factories to ensure proper initialization.
  Parameter() = default;
  static Parameter<ParameterType> Create(
      ParameterConfiguration&& configuration,
      std::function<ParameterType(const std::string&)> converter);
 private:
  ParameterConfiguration configuration_;
  std::function<ParameterType(const std::string&)> converter_;
};

template <class ParameterType>
Parameter<ParameterType> Parameter<ParameterType>::Create(
    ParameterConfiguration&& configuration,
    std::function<ParameterType(const std::string&)> converter) {
  Parameter<ParameterType> parameter;
  parameter.configuration_ = configuration;
  parameter.converter_ = converter;
  return parameter;
}

template <class ParameterType>
Parameter<ParameterType> Parameter<ParameterType>::Positional(
    const std::vector<std::string>& names,
    int position,
    std::function<ParameterType(const std::string&)> converter) {
  ParameterConfiguration configuration = ParameterConfiguration::Create(
      names, ParameterCategory::kPositionalParameter, position);
  return Parameter<ParameterType>::Create(std::move(configuration), converter);
}

template <class ParameterType>
Parameter<ParameterType> Parameter<ParameterType>::Positional(
    const std::vector<std::string>& names, int position) {
  std::function<ParameterType(const std::string&)> converter
      = ParameterType::from_string;
  return Parameter<ParameterType>::Positional(names, position, converter);
}

template <class ParameterType>
Parameter<ParameterType> Parameter<ParameterType>::Keyword(
    const std::vector<std::string>& names,
    std::function<ParameterType(const std::string&)> converter) {
  ParameterConfiguration configuration = ParameterConfiguration::Create(
      names, ParameterCategory::kKeywordParameter);
  return Parameter<ParameterType>::Create(std::move(configuration), converter);
}

template <class ParameterType>
Parameter<ParameterType> Parameter<ParameterType>::Keyword(
    const std::vector<std::string>& names) {
  std::function<ParameterType(const std::string&)> converter
      = ParameterType::from_string;
  return Parameter<ParameterType>::Keyword(names, converter);
}

template <class ParameterType>
Parameter<bool> Parameter<ParameterType>::Flag(
    const std::vector<std::string>& names) {
  ParameterConfiguration configuration = ParameterConfiguration::Create(
      names, ParameterCategory::kFlag);
  return Parameter<bool>::Create(std::move(configuration), FlagConverter);
}

template <class ParameterType>
Parameter<bool> Parameter<ParameterType>::Flag(char c) {
  std::string name;
  std::vector<std::string> names;
  name.push_back('-');
  name.push_back(c);
  names.push_back(name);
  return Parameter<ParameterType>::Flag(names);
}

} // namespace arg_parse_convert

#endif // ARG_PARSE_CONVERT_PARAMETER_H_