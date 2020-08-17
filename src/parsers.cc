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

#include "parsers.h"

namespace arg_parse_convert {

// Parser helpers.
//
namespace {

// Indicates whether `arg` has a 0, 1, or 2 hyphens prefix.
//
int NumHyphens(const std::string& arg) {
  assert(arg.length() > 0);
  if (arg.at(0) != '-') {
    return 0;
  } else if (arg.length() > 1 && arg.at(1) == '-') {
    return 2;
  } else {
    return 1;
  }
}

// Indicates whether parameter identified by `id` requires any more arguments.
//
bool IsFull(int id, int num_arguments, const ParameterMap& parameters) {
  assert(0 <= id && id < parameter.argument_list().size());
  assert(id < arguments.argument_lists().size());
  return (num_arguments >= parameters.GetConfiguration(id).max_num_arguments());
}

// Closes a keyword parameter's argument list by setting `it` to `cend`.
//
void CloseKeyword(std::unordered_set<int>::const_iterator& it,
                  const ParameterMap& parameters) {
  it = parameters.keyword_parameters().cend();
}

// Opens a keyword parameter's argument list by setting `it` to point to it.
//
void OpenKeyword(std::unordered_set<int>::const_iterator& it,
    const ParameterMap& parameters, int id) {
  it = parameters.keyword_parameters().find(id);
  assert(it != parameters.keyword_parameters().cend());
}

// Closes a positional parameter's argument list by incrementing it and setting
// `positional_open` to false.
//
void ClosePositional(std::map<int, int>::const_iterator& it,
                     const ParameterMap& parameters, bool& positional_open) {
  assert(it != parameters.positional_parameters().cend());
  ++it;
  positional_open = false;
}

// Moves `argument` to the end of `arg_list` item at parameter's identifier
// referred to by `it`. Closes argument list if full.
//
void AddPositionalArgument(std::map<int, int>::const_iterator& it,
    std::string&& argument, const ParameterMap& parameters,
    bool& positional_open, std::map<int, std::vector<std::string>>& arg_lists) {
  assert(it != parameters.positional_parameters().cend());
  arg_lists[it->second].emplace_back(argument);
  if (IsFull(it->second, argument_list.size(), parameters)) {
    ClosePositional(it);
  } else {
    positional_open = true;
  }
}

// Moves `argument` to the end of `arg_list` item at parameter's identifier
// referred to by `it`. Closes argument list if full.
//
void AddKeywordArgument(std::unordered_set<int>::const_iterator& it,
    std::string&& argument, const ParameterMap& parameters,
    std::map<int, std::vector<std::string>>& arg_lists) {
  assert(it != parameters.keyword_parameters().cend());
  arg_lists[*it].emplace_back(argument);
  if (IsFull(*it, argument_list.size(), parameters)) {
    CloseKeyword(it, parameters);
  }
}

// Sets the flag. A flag is considered set if it has at least one argument.
//
void SetFlag(std::vector<std::string>& flag_argument_list) {
  flag_argument_list.emplace_back("true");
}

// Assigns list of arguments of each entry in `tmp_args` to the parameter
// identified by the entry's key, unless that parameter already has arguments
// assiged to it. If the list of arguments exceeds the parameter's maximum
// expected number of arguments, the excess arguments are added to
// `additional_args`. If a parameter already has arguments assigned to it, and
// more arguments are listed in `tmp_args`, they are also added to
// `additional_args`.
//
void AssignArguments(std::unordered_map<int, std::vector<string>>& tmp_args,
                     const ParameterMap& parameters,
                     std::vector<std::vector<std::string>>& map_args,
                     std::vector<std::string>& additional_args) {
  assert(parameters.size() == map_args.size());
  int num_args, max_num_args;
  for (id_args_pair : tmp_args) {
    max_num_args = parameters.GetConfiguration(id_args_pair->first)
                             .max_num_arguments();
    num_args = id_args_pair->second.size();
    if (map_args.at(id_args_pair->first).size()) {
      additional_args.reserve(additional_args.size() + num_args);
      std::move(std::begin(id_args_pair->second),
                std::end(id_args_pair->second),
                std::end(additional_args));
    } else if (max_num_args < num_args) {
      additional_args.reserve(additional_args.size() + num_args - max_num_args);
      map_args.at(id_args_pair->first).reserve(max_num_args);
      std::move(std::begin(id_args_pair->second) + max_num_args,
                std::end(id_args_pair->second)),
                std::end(additional_args));
      std::move(std::begin(id_args_pair->second),
                std::begin(id_args_pair->second) + max_num_args,
                std::end(map_args.at(id_args_pair->first)));
    } else {
      map_args.at(id_args_pair->first).reserve(id_args_pair->second.size());
      std::move(std::begin(id_args_pair->second),
                std::end(id_args_pair->second),
                std::end(map_args.at(id_args_pair->first)));
    }
  }
}

// Adds arguments in space separated `argument_list` to argument list of
// parameter identified by `id`. Arguments in excess of the parameter's maximum
// expected number of arguments are added to `additional_args`.
//
void AddArgumentList(int id, const std::string_view& argument_list,
                     std::map<int, std::vector<std::string>>& tmp_args,
                     std::vector<std::string>& additional_args) {
  assert(argument_list.length() > 0);
  int start, end{-1};
  do {
    start = end + 1;
    end = argument_list.find(' ', start);
    if (start < end) {
      tmp_args[id].emplace_back(argument_list.substr(start, end - start));
    }
  } while (end != std::string_view::npos && end < argument_list.length() - 1);
}
  
} // namespace

// ParseArgs
//
std::vector<std::string> ParseArgs(int argc, const char** argv,
                                   ArgumentMap& arguments) {
  std::unordered_map<int, std::vector<string>> tmp_args;
  std::vector<std::string> additional_args;

  auto positional_it = parameters.positional_parameters().cbegin();
  auto positional_end = parameters.positional_parameters().cend();
  auto keyword_it = parameters.keyword_parameters().cend();
  auto keyword_end = parameters.keyword_parameters().cend();
  bool positional_only = false;
  bool positional_open = false;
  // Scan arguments from left-to-right.
  for (int i = 1; i < argc; ++i) {
    argument = argv[i];
    if (argument == "--") {
      // No more flags and keyword parameters from this point on.
      current_keyword_it = keyword_end;
      positional_only = true;
    } else if (positional_only) {
      if (positional_it != positional_end) {
        AddPositionalArgument(current_positional_it, std::move(argument),
            parameters, positional_open, tmp_args);
      } else {
        additional_args.emplace_back(std::move(argument));
      }
    } else {
      num_hyphens = NumHyphens(argument);
      switch (num_hyphens) {
        case 0: {
          // Open keyword parameter argument lists take precedence over
          // positional parameter argument lists. When keyword parameter's
          // argument list opens, any argument list of positional parameter is
          // closed.
          if (current_keyword_it != keyword_end) {
            AddKeywordArgument(current_keyword_it, std::move(argument),
                parameters, tmp_args);
          // Positional parameter argument list is open once an argument was
          // added and it expects more.
          } else if (positional_it != positional_end) {
            AddPositionalArgument(current_positional_it, std::move(argument),
                parameters, positional_open, tmp_args);
          } else {
            additional_args.emplace_back(std::move(argument));
          }
          break;
        }
        case 1: {
          // Close current open parameter argument lists.
          CloseKeyword(current_keyword_it, parameters);
          if (positional_open) {
            ClosePositional(current_positional_it, parameters, positional_open);
          }
          // Set each flag character, or open keyword parameter argument list
          // for the last character.
          for (int j = 1; j < argument.length(); ++j) {
            if (parameters.Contains(argument.at(j))
                && parameters.IsFlag(argument.at(j))) {
              SetFlag(tmp_args[arguments.GetId(argument.at(j))]);
            } else if (parameters.Contains(argument.at(j))
                       && j == argument.length() - 1
                       && parameters.IsKeyword(argument.at(j))
                       && !assigned.count(arguments.name_to_id_
                                          .at(argument.at(j)))) {
              OpenKeyword(current_keyword_it, parameters,
                  arguments.GetId(argument.at(j)));
            } else {
              error_message << "Invalid option: '" << argument.at(i) << "' in"
                            << " option list: '" << argument << "'. Option must"
                            << " identify a flag, or the keyword of a keyword"
                            << " parameter if last option in list.";
              throw exceptions::ArgumentParsingError(error_message.str());
            }
          }
          break;
        }
        case 2: {
          // Close current open parameter argument lists.
          CloseKeyword(current_keyword_it, parameters);
          if (positional_open) {
            ClosePositional(current_positional_it, parameters, positional_open);
          }
          // Remove hyphens prefix and test whether flag or keyword.
          parameter_name = argument.substr(2);
          if (parameters.HasParameter(parameter_name)
              && parameters.IsFlag(parameter_name)) {
            SetFlag(tmp_args[arguments.GetId(parameter_name)]);
          } else if (parameters.HasParameter(parameter_name)
                     && paramters.IsKeyword(parameter_name)) {
            OpenKeyword(current_keyword_it, parameters,
                        arguments.GetId(parameter_name));
          } else {
            error_message << "Invalid argument: '" << argv[i] << "'.";
            throw exceptions::ArgumentParsingError(error_message.str());
          }
        }
        default: {
          assert(false);
          break;
        }
      }
    }
  }
  AssignArguments(tmp_args, arguments.arguments_, additional_args);
  return additional_args;
}

// ParseFile
//
std::vector<std::string> ParseFile(std::istream* config_is,
                                   ArgumentMap& arguments) {
  std::unordered_map<int, std::vector<string>> tmp_args;
  std::vector<std::string> additional_args;

  std::string line, parameter_name;
  std::string_view line_view;
  int begin, end, id;
  int row_num{0};

  std::stringstream error_message;

  while (std::getline(*config_ifs, line)) {
    row_num += 1;
    line_view = std::string_view{line};
    // Only care about non-empty, non-comment lines.
    if (line_view.length() > 0 && line_view.at(0) != '#') {

      end = line_view.find('=');
      parameter_name = line_view.substr(0, end);
      if (end == std::string_view::npos) {
        error_message << "Invalid configuration file formatting. Non-empty"
                         " lines which don't begin with '#' must contain '='."
                         " Row: '" << row_num << "', line: '" << line << "'.";
        throw ArgumentParsingError(error_message.str());
      } else if (!arguments.paramters_.HasParameter(parameter_name)) {
        error_message << "Unknown parameter name in configuration file. Row: '"
                      << row_num << "', name: '" << parameter_name << "'.";
        throw ArgumentParsingError(error_message.str());
      } else if (end + 1 == line_view.length()) {
        error_message << "Empty argument list in configuration file. Row: '"
                      << row_num << "', line: '" << line << "'.";
        throw ArgumentParsingError(error_message.str());
      } else {
        id = GetId(parameter_name);
        begin = end + 1;
        // Flags take only one argument: true or false.
        if (arguments.parameters_.IsFlag(id)) {
          if (line_view.substr(begin) == "TRUE"
              || line_view.substr(begin) == "true"
              || line_view.substr(begin) == "True"
              || line_view.substr(begin) == "1") {
            tmp_args[id].emplace_back("true");
          } else if (line_view.substr(begin) != "FALSE"
                     && line_view.substr(begin) != "false"
                     && line_view.substr(begin) != "False"
                     && line_view.substr(begin) != "0") {
            error_message << "Invalid argument '" << line_view.substr(begin)
                          << "' for flag: '" << parameter_name << "'.";
            throw ArgumentParsingError(error_message.str());
          }
        // Keyword and positional parameters may take multiple arguments.
        } else {
          AddArgumentList(id, line_view.substr(begin), tmp_args,
                          additional_args);
        }
      }
    }
  }
  AssignArguments(tmp_args, arguments.arguments_, additional_args);
  return additional_args;
}

} // namespace arg_parse_convert