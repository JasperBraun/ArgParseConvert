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

#ifndef ARG_PARSE_CONVERT_PARAMETER_MAP_H_
#define ARG_PARSE_CONVERT_PARAMETER_MAP_H_

#include <any>
#include <map>
#include <sstream>
#include <string>
#include <typeinfo>
#include <unordered_map>
#include <unordered_set>

#include "parameter.h"

namespace arg_parse_convert {

struct InvalidArguments {
  std::vector<std::string> additional_arguments;
  std::vector<std::string> unfilled_parameters;

  inline bool empty() {
    return (additional_arguments.empty() && unfilled_parameters.empty());
  }
};

class ParameterMap {
 public:
  /// @name Constructors:
  ///
  /// @{
  
  ParameterMap() = default;

  ParameterMap(HelpStringFormat format) : help_string_format_{format} {}

  ParameterMap(int help_string_width, int parameter_indentation,
               int description_indentation)
      : help_string_format_{HelpStringFormat{help_string_width,
                                             parameter_indentation,
                                             description_indentation}} {}

  ParameterMap(const ParameterMap& other) = default;
  ParameterMap(ParameterMap&& other) = default;
  /// @}
  
  /// @name Assignment:
  ///
  /// @{
  
  ParameterMap& operator=(const ParameterMap& other) = default;
  ParameterMap& operator=(ParameterMap&& other) = default;
  /// @}

  /// @name Accessors:
  ///
  /// @{

  /// @brief Indicates if at least one argument was assigned to parameter with
  ///  name `name`.
  ///
  /// @details Throws exception of type `exceptions::UnknownParameterName` if
  ///  no parameter with name `name` was registered with the object.
  ///
  inline bool HasArgument(const std::string& name) {
    if (GetId(name) < arguments_.size() && arguments_.at(GetId(name)).size() > 0) {
      return true;
    } else {
      return false;
    }
  }

  /// @brief Returns value of converter associated to registered parameter with
  ///  name `name` evaluated at its first parsed argument.
  ///
  /// @details `ParameterType` must be the correct type associated with the
  ///  parameter. Throws exception of type `exceptions::UnknownParameterName` if
  ///  no parameter with name `name` was registered with the object. Throws
  ///  exception of type `exceptions::MismatchedParameterType` if `name` was
  ///  registered with the object, but as parameter of type `ParameterType`.
  ///  Throws exception of type `exceptions::UnfilledParameter` if no argument
  ///  was parsed for the parameter unless the parameter is a flag. Throws
  ///  exception of type `exceptions::InvalidFlagConversion` if `name` is the
  ///  name of a flag.
  ///
  template <class ParameterType>
  ParameterType convert(const std::string& name) const;

  /// @brief Returns the list of values of converter associated to registered
  ///  parameter with name `name` evaluated at all of its parsed arguments.
  ///
  /// @details `ParameterType` must be the correct type associated with the
  ///  parameter. Throws exception of type `exceptions::UnknownParameterName` if
  ///  no parameter with name `name` was registered with the object. Throws
  ///  exception of type `exceptions::MismatchedParameterType` if `name` was
  ///  registered with the object, but as parameter of type `ParameterType`.
  ///  Throws exception of type `exceptions::InvalidFlagConversion` if `name` is
  ///  the name of a flag. Returns empty list if no argument was parsed for the
  ///  parameter.
  ///
  template <class ParameterType>
  std::vector<ParameterType> convert_all(const std::string& name) const;

  /// @brief Returns whether or not the flag named `name` is set.
  ///
  /// @details Throws exception of type `exceptions::UnknownParameterName` if
  ///  no parameter with name `name` was registered with the object. Throws
  ///  exception of type `exceptions::NoFlagWithName` if the name does not
  ///  correspond to a flag with name `name` registered with the object.
  ///
  inline bool IsSet(const std::string& name) const {
    size_t id{GetId(name)};
    std::stringstream error_message;
    if (parameter_configurations_.at(id).category()
        != ParameterCategory::kFlag) {
      error_message << "Parameter with name: '" << name << "' is not a flag."
                    << " Call `ParameterMap::IsSet` only to check if a flag is"
                    << " set." << std::endl;
      throw exceptions::NoFlagWithName(error_message.str());
    }
    if (arguments_.at(id).size() > 0) {
      return true;
    } else {
      return false;
    }
  }

  /// @brief Returns whether or not the flag named `-c` is set.
  ///
  /// @details Throws exception of type `exceptions::UnknownParameterName` if
  ///  no parameter with name `name` was registered with the object. Throws
  ///  exception of type `exceptions::NoFlagWithName` if the name does not
  ///  correspond to a flag with name `name` registered with the object.
  ///
  inline bool IsSet(char c) const {
    std::string name;
    name.push_back('-');
    name.push_back(c);
    size_t id{GetId(name)};
    std::stringstream error_message;
    if (parameter_configurations_.at(id).category()
        != ParameterCategory::kFlag) {
      error_message << "Parameter with name: '" << name << "' is not a flag."
                    << " Call `ParameterMap::IsSet` only to check if a flag is"
                    << " set." << std::endl;
      throw exceptions::NoFlagWithName(error_message.str());
    }
    if (arguments_.at(id).size() > 0) {
      return true;
    } else {
      return false;
    }
  }
  /// @}

  /// @name Mutators:
  ///
  /// @{
  
  /// @brief Registers `parameter` with the object.
  ///
  /// @details Throws exception of type `exceptions::DuplicateParameterName` if
  ///  a parameter of the same name was already registered.
  ///
  template<class ParameterType>
  ParameterMap& operator()(Parameter<ParameterType> parameter);

  /// @brief Assigns members of `argv` to the parameters registered with the
  ///  object.
  ///
  /// @details From left-to-right, a keyword parameter is assigned all arguments
  ///  immediately following its keyword until either its maximum argument
  ///  number is reached, or the name of a flag or keyword of a keyword
  ///  parameter is encountered. Any flags set the value of the flag to true,
  ///  regardless of how often the flag appears. Function attempts to assign all
  ///  remaining arguments to positional parameters from left to right. Each
  ///  positional parameter's argument list ends if a flag, a keywords of a
  ///  keyword parameter, or the maximum number of arguments of the positional
  ///  parameter is reached. The return object lists all remaining arguments in
  ///  its `additional_arguments` data member. All parameters for which fewer
  ///  arguments than their minimum argument number were parsed, are listed in
  ///  the return object's `unfilled_parameters` data member. Throws exception
  ///  of type `exceptions::UnkownFlagOrKeyword` if an argument appears that
  ///  begins with the character '-' and is neither the keyword of a keyword
  ///  parameter nor a list of valid flags.
  ///
  InvalidArguments parse(int argc, const char** argv);

  /// @brief Sets the help string's header.
  ///
  /// @see HelpString().
  ///
  inline void SetHelpStringHeader(const std::string& header) {
    help_string_format_.header(header);
  }

  /// @brief Sets the help string's header.
  ///
  /// @see HelpString().
  ///
  inline void SetHelpStringHeader(std::string&& header) {
    help_string_format_.header(header);
  }

  /// @brief Sets the help string's footer.
  ///
  /// @see HelpString().
  ///
  inline void SetHelpStringFooter(const std::string& footer) {
    help_string_format_.header(footer);
  }

  /// @brief Sets the help string's footer.
  ///
  /// @see HelpString().
  ///
  inline void SetHelpStringFooter(std::string&& footer) {
    help_string_format_.header(footer);
  }

  /// @brief Sets the help string's width.
  ///
  /// @see HelpString().
  ///
  inline void SetHelpStringWidth(int width) {
    help_string_format_.width(width);
  }

  /// @brief Sets the help string's parameter indentation.
  ///
  /// @see HelpString().
  ///
  inline void SetHelpStringParameterIndentation(int width) {
    help_string_format_.parameter_indentation(width);
  }

  /// @brief Sets the help string's description indentation.
  ///
  /// @see HelpString().
  ///
  inline void SetHelpStringDescriptionIndentation(int width) {
    help_string_format_.description_indentation(width);
  }

  /// @brief Sets the help string's formatting.
  ///
  inline void SetHelpStringFormat(HelpStringFormat value) {
    help_string_format_ = value;
  }
  /// @}

  /// @name Other:
  ///
  /// @{
  
  /// @brief Returns a formatted help string derived from the registered
  ///  parameters and the set header, footer, with and indentations.
  ///
  /// @details The help string consists of the set footer followed by
  ///  help strings of all parameters registered with the object and finally the
  ///  set header. The parameters are printed in categories in the order
  ///  positional parameters, keyword parameters, and flags.
  /// @see `Parameter::HelpString`, `SetHelpStringHeader`,
  ///  `SetHelpStringFooter`, `SetHelpStringWidth`,
  ///  `SetHelpStringParamterIndentation`, and
  ///  `SetHelpStringDescriptionIndentation`.
  ///
  std::string HelpString();
  /// @}

 private:
  inline size_t GetId(const std::string& name) const {
    std::stringstream error_message;
    if (name_to_id_.count(name)) {
      return name_to_id_.at(name);
    } else {
      error_message << "Parameter with name: '" << name << "' was never"
                    << " registered. Cannot access unregistered parameters."
                    << std::endl;
      throw exceptions::UnknownParameterName(error_message.str());
    }
  }
  // Returns true if parameter reached its maximum number of arguments.
  inline bool AddArgument(size_t id, const std::string& argument) {
    arguments_.at(id).push_back(argument);
    return (parameter_configurations_.at(id).max_num_arguments() > 0
            && arguments_.at(id).size()
                == parameter_configurations_.at(id).max_num_arguments());
  }
  // Assumes that `id` is id of a flag.
  inline void SetFlag(size_t id) {
    // Flag conversion function returns true if flag has at least one argument.
    arguments_.at(id).push_back("true");
  }
  // Sets default arguments if they were provided and no arguments are assigned.
  inline void SetDefaultArgumentsIfNeeded(size_t id) {
    if (arguments_.at(id).size() == 0
        && parameter_configurations_.at(id).default_arguments().size() > 0) {
      arguments_.at(id) = parameter_configurations_.at(id).default_arguments();
    }
  }
  inline bool IsUnfilled(size_t id) {
    return (arguments_.at(id).size()
            < parameter_configurations_.at(id).min_num_arguments());
  }
  inline void AddParameterToCategory(size_t id, ParameterCategory category,
                                     int position) {
    std::stringstream error_message;
    switch (category) {
      case ParameterCategory::kPositionalParameter: {
        positional_parameters_.insert({position, id});
        break;
      }
      case ParameterCategory::kKeywordParameter: {
        keyword_parameters_.insert(id);
        break;
      }
      case ParameterCategory::kFlag: {
        flags_.insert(id);
        break;
      }
      default: {
        error_message << "Unable to determine parameter's category: '"
                      << static_cast<int>(category) << "' in"
                      << "`ParameterMap::AddParameterInCategory`; cases should"
                      << " be exhaustive." << std::endl;
        throw exceptions::internal::UnexpectedCase(error_message.str());
      }
    }
  }
  std::unordered_map<std::string, size_t> name_to_id_;
  std::vector<ParameterConfiguration> parameter_configurations_;
  std::vector<std::any> converters_;

  std::vector<std::vector<std::string>> arguments_;

  // Positional parameters are ordered by position.
  std::map<int, size_t> positional_parameters_;
  std::unordered_set<size_t> keyword_parameters_;
  std::unordered_set<size_t> flags_;

  HelpStringFormat help_string_format_;
};

template <class ParameterType>
ParameterType ParameterMap::convert(const std::string& name) const {
  size_t id{GetId(name)}; // may throw
  std::function<ParameterType(const std::string&)> converter;
  std::stringstream error_message;
  if (flags_.count(id)) {
    error_message << "Attempted to use `ParameterMap::convert` to check if flag"
                  << " named: '" << name << "' was set. Use"
                  << " `ParameterMap::IsSet` instead." << std::endl;
    throw exceptions::InvalidFlagConversion(error_message.str());
  }
  try {
    converter =
        std::any_cast<std::function<ParameterType(const std::string&)>>
            (converters_.at(id));
  } catch (const std::bad_any_cast& e) {
    error_message << "Attempted to access parameter with name: '" << name
                  << "' and type: '"
                  << converters_.at(id).type().name() << "' as type: '"
                  << typeid(ParameterType).name() << "'." << std::endl;
    throw exceptions::MismatchedParameterType(error_message.str());
  }
  if (arguments_.at(id).size() <= 0) {
    error_message << "Attempted to access argument of parameter with name: '"
                  << name << "', but none was parsed." << std::endl;
    throw exceptions::UnfilledParameter(error_message.str());
  }
  return converter(arguments_.at(id)[0]);
}

template <class ParameterType>
std::vector<ParameterType> ParameterMap::convert_all(const std::string& name) const {
  std::vector<ParameterType> result;
  size_t id{GetId(name)}; // may throw
  std::function<ParameterType(const std::string&)> converter;
  std::stringstream error_message;
  if (flags_.count(id)) {
    error_message << "Attempted to use `ParameterMap::convert_all` to check if"
                  << " flag named: '" << name << "' was set. Use"
                  << " `ParameterMap::IsSet` instead." << std::endl;
    throw exceptions::InvalidFlagConversion(error_message.str());
  } else {
    try {
      converter =
          std::any_cast<std::function<ParameterType(const std::string&)>>
              (converters_.at(id));
    } catch (const std::bad_any_cast& e) {
      error_message << "Attempted to access parameter with name: '" << name
                    << "' and type: '"
                    << converters_.at(id).type().name() << "' as type: '"
                    << typeid(ParameterType).name() << "'." << std::endl;
      throw exceptions::MismatchedParameterType(error_message.str());
    }
    for (const std::string& argument : arguments_.at(id)) {
      result.push_back(converter(argument));
    }
  }
  return result;
}

template<class ParameterType>
ParameterMap& ParameterMap::operator()(Parameter<ParameterType> parameter) {
  size_t id;
  std::string name;
  std::stringstream error_message;
  id = parameter_configurations_.size();
  for (const std::string& name : parameter.configuration().names()) {
    if (name_to_id_.count(name)) {
      error_message << "Parameter named: '" << name << "' already exists."
                    << " Choose unique parameter names." << std::endl;
      throw exceptions::DuplicateParameterName(error_message.str());
    }
  }
  for (size_t i = 0; i < parameter.configuration().names().size(); ++i) {
    name = parameter.configuration().names().at(i);
    name_to_id_.insert({name, id});
  }
  parameter_configurations_.push_back(parameter.configuration());
  converters_.push_back(parameter.converter());
  AddParameterToCategory(id, parameter.configuration().category(),
                         parameter.configuration().position());
  return *this;
}

} // namespace arg_parse_convert

#endif // ARG_PARSE_CONVERT_PARAMETER_MAP_H_