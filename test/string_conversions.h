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

#ifndef ARG_PARSE_CONVERT_TEST_STRING_CONVERSIONS_H_
#define ARG_PARSE_CONVERT_TEST_STRING_CONVERSIONS_H_

#include "catch.h"
#include "arg_parse_convert.h"

namespace arg_parse_convert {

namespace test {

// Maps integers to string representation of `ParameterConfiguration`
// enumerators
//
const char* kParameterCategory[]{
  "kPositionalParameter",
  "kKeywordParameter",
  "kFlag"
};

} // namespace test

} // namespace arg_parse_convert

namespace Catch {

template<>
struct StringMaker<arg_parse_convert::ParameterCategory> {
  static std::string convert(
      const arg_parse_convert::ParameterCategory& category) {
    return arg_parse_convert::test::kParameterCategory[static_cast<int>(
        category)];
  }
};

template<>
struct StringMaker<arg_parse_convert::ParameterConfiguration> {
  static std::string convert(
      const arg_parse_convert::ParameterConfiguration& configuration) {
    std::stringstream ss;
    ss << "{names: [";
    if (!configuration.names().empty()) {
      ss << configuration.names().at(0);
      for (const std::string& name : configuration.names()) {
        ss << ", " << name;
      }
    }
    ss << "], category: "
       << StringMaker<arg_parse_convert::ParameterCategory>::convert(
           configuration.category())
       << ", default arguments: [";
    if (!configuration.default_arguments().empty()) {
      ss << configuration.default_arguments().at(0);
      for (const std::string& argument : configuration.default_arguments()) {
        ss << ", " << argument;
      }
    }
    ss << "], position: " << configuration.position()
       << ", min number of arguments: " << configuration.min_num_arguments()
       << ", max number of arguments: " << configuration.max_num_arguments()
       << ", description: " << configuration.description()
       << ", argument placeholder: " << configuration.placeholder()
       << "}.";
    return ss.str();
  }
};

template<class ParameterType>
struct StringMaker<arg_parse_convert::Parameter<ParameterType>> {
  static std::string convert(
      const arg_parse_convert::Parameter<ParameterType>& parameter) {
    std::stringstream ss;
    ss << "{configuration: "
       << StringMaker<arg_parse_convert::ParameterConfiguration>::convert(
           parameter.configuration())
       << ", has converter: " << std::boolalpha
       << static_cast<bool>(parameter.converter()) << "}.";
    return ss.str();
  }
};

template<>
struct StringMaker<arg_parse_convert::ParameterMap> {
  static std::string convert(
      const arg_parse_convert::ParameterMap& map) {
    std::stringstream ss;
    ss << "{size: " << map.size() << ", required: [";
    if (map.required_parameters().size() > 0) {
      for (int id : map.required_parameters()) {
        ss << '{';
        for (const std::string& name
             : map.GetConfiguration(map.GetPrimaryName(id)).names()) {
          ss << name  << ',';
        }
        ss << "}";
      }
    }
    ss << "], positional: [";
    if (map.positional_parameters().size() > 0) {
      for (auto pair : map.positional_parameters()) {
        ss << '(' << pair.first << ',';
        ss << '{';
        for (const std::string& name
             : map.GetConfiguration(map.GetPrimaryName(pair.second)).names()) {
          ss << name  << ',';
        }
        ss << "})";
      }
    }
    ss << "], keyword: [";
    if (map.keyword_parameters().size() > 0) {
      for (int id : map.keyword_parameters()) {
        ss << '{';
        for (const std::string& name
             : map.GetConfiguration(map.GetPrimaryName(id)).names()) {
          ss << name  << ',';
        }
        ss << "}";
      }
    }
    ss << "], flags: [";
    if (map.flags().size() > 0) {
      for (int id : map.flags()) {
        ss << '{';
        for (const std::string& name
             : map.GetConfiguration(map.GetPrimaryName(id)).names()) {
          ss << name  << ',';
        }
        ss << "}";
      }
    }
    ss << "]}";
    return ss.str();
  }
};

} // namespace Catch

#endif // ARG_PARSE_CONVERT_TEST_STRING_CONVERSIONS_H_