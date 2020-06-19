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

#include "parameter_map.h"

namespace arg_parse_convert {

InvalidArguments ParameterMap::parse(int argc, const char** argv) {
  InvalidArguments invalid_arguments;
  std::map<int, size_t>::iterator current_positional_it;
  std::unordered_set<size_t>::iterator current_keyword_it;
  std::string argument, flag_name, parameter_name;
  size_t id;
  current_positional_it = this->positional_parameters_.begin();
  current_keyword_it = this->keyword_parameters_.end();
  std::stringstream error_message;

  this->arguments_.clear();
  this->arguments_.resize(this->parameter_configurations_.size());
  for (int i = 1; i < argc; ++i) {
    argument = argv[i];
    if (this->name_to_id_.count(argument)
        && this->keyword_parameters_.count(this->GetId(argument))) {
      // `argument` is a keyword of a keyword parameter
      id = this->GetId(argument);
      // Abandon old keyword parameter and begin argument list of new.
      current_keyword_it = this->keyword_parameters_.find(id);
      // If old positional parameter has already received arguments, abandon
      // if and begin argument list of next.
      if (current_positional_it != this->positional_parameters_.end()
          && this->arguments_.at(current_positional_it->second).size() > 0) {
        ++current_positional_it;
      }
    } else if (this->name_to_id_.count(argument)
               && this->flags_.count(this->GetId(argument))) {
      // Abandon old keyword parameter.
      current_keyword_it = this->keyword_parameters_.end();
      this->SetFlag(this->GetId(argument));
    } else if (argument[0] == '-') {
      // Abandon old keyword parameter.
      current_keyword_it = this->keyword_parameters_.end();
      for (size_t j = 1; j < argument.length(); ++j) {
        flag_name = "-";
        flag_name.push_back(argument.at(j));
        if (this->name_to_id_.count(flag_name)) {
          this->SetFlag(GetId(flag_name));
        } else {
          error_message << "Unkown flag: '" << id << "', or keyword: '"
                        << argument << "'." << std::endl;
          throw exceptions::UnkownFlagOrKeyword(error_message.str());
        }
      }
    } else {
      // `argument` is the argument for a keyword or positional argument unless
      // no keyword or positional argument is still expecting more arguments.
      if (current_keyword_it != this->keyword_parameters_.end()) {
        id = *current_keyword_it;
        if (this->AddArgument(id, argument)) {
          current_keyword_it = this->keyword_parameters_.end();
        }
      } else if (current_positional_it != this->positional_parameters_.end()) {
        id = current_positional_it->second;
        if (this->AddArgument(id, argument)) {
          ++current_positional_it;
        }
      } else {
        invalid_arguments.additional_arguments.push_back(argument);
      }
    }
  }
  for (auto position_and_id : this->positional_parameters_) {
    id = position_and_id.second;
    this->SetDefaultArgumentsIfNeeded(id);
    if (this->IsUnfilled(id)) {
      parameter_name = this->parameter_configurations_.at(id).names()[0];
      invalid_arguments.unfilled_parameters.push_back(parameter_name);
    }
  }
  for (size_t i : this->keyword_parameters_) {
    this->SetDefaultArgumentsIfNeeded(i);
    if (this->IsUnfilled(i)) {
      parameter_name = this->parameter_configurations_.at(i).names()[0];
      invalid_arguments.unfilled_parameters.push_back(parameter_name);
    }
  }
  return invalid_arguments;
}

std::string ParameterMap::HelpString() {
  std::stringstream ss;
  if (this->help_string_format_.header().length() > 0) {
    ss << help_string_format_.header() << '\n';
  }
  if (this->positional_parameters_.size() > 0) {
    ss << "\nPositional Arguments:\n";
    for (auto position_and_id : positional_parameters_) {
      ss << this->parameter_configurations_
                .at(position_and_id.second)
                .HelpString(this->help_string_format_);
    }
  }
  if (this->keyword_parameters_.size() > 0) {
    ss << "\nKeyword Arguments:\n";
    for (size_t id : this->keyword_parameters_) {
      ss << this->parameter_configurations_
                .at(id)
                .HelpString(this->help_string_format_);
    }
  }
  if (this->flags_.size() > 0) {
    ss << "\nFlags:\n";
    for (size_t id : this->flags_) {
      ss << this->parameter_configurations_
                .at(id)
                .HelpString(this->help_string_format_);
    }
  }
  if (this->help_string_format_.footer().length() > 0) {
    ss << help_string_format_.footer() << '\n';
  }
  return ss.str();
}

} // namespace arg_parse_convert