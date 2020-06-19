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

#include "parameter.h"

namespace arg_parse_convert {

ParameterConfiguration ParameterConfiguration::Create(
    const std::vector<std::string>& names, ParameterCategory category) {
  ParameterConfiguration configuration{};
  configuration.names_ = names;
  configuration.category_ = category;
  return configuration;
}

ParameterConfiguration ParameterConfiguration::Create(
    const std::vector<std::string>& names, ParameterCategory category,
    int position) {
  ParameterConfiguration configuration
      = ParameterConfiguration::Create(names, category);
  configuration.position_ = position;
  return configuration;
}

std::string ParameterConfiguration::HelpString(const HelpStringFormat& format) {
  std::stringstream ss;
  int description_width = format.width() - format.description_indentation();
  std::string parameter_spacer(
      static_cast<size_t>(format.parameter_indentation()), ' ');
  std::string description_spacer(
      static_cast<size_t>(format.description_indentation()), ' ');
  ss << parameter_spacer << this->names_.at(0);
  for (size_t i = 1; i < this->names_.size(); ++i) {
    ss << " | " << this->names_.at(i);
  }
  if (this->category_ != ParameterCategory::kFlag) {
    ss << ' ' << this->argument_placeholder_;
    if (this->default_arguments_.size() > 0) {
      ss << " ( =";
      for (const std::string& argument : this->default_arguments_) {
        ss << ' ' << argument;
      }
      ss << ')';
    }
  }
  ss << '\n';
  for (size_t pos = 0;
       pos < this->description_.length();
       pos += description_width) {
    ss << description_spacer
       << this->description_.substr(pos, description_width) << '\n';
  }
  return ss.str();
}

} // namespace arg_parse_convert