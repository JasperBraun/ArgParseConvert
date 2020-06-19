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

#include "arg_parse_convert.h"

#define CATCH_CONFIG_MAIN
#define CATCH_CONFIG_COLOUR_NONE
#include "catch.h"

#include <unordered_set>

#include "conversion_functions.h"

namespace arg_parse_convert {

namespace test {

struct WithoutConversion {
  int data;
};
bool operator==(const WithoutConversion& lhs, const WithoutConversion& rhs) {
  return lhs.data == rhs.data;
}
WithoutConversion external_converter(const std::string& arg) {
  WithoutConversion result{};
  result.data = std::stoi(arg);
  return result;
}
struct WithConversion {
  int data;
  static WithConversion from_string(const std::string& arg) {
    WithConversion result{};
    result.data = std::stoi(arg);
    return result;
  }
};
bool operator==(const WithConversion& lhs, const WithConversion& rhs) {
  return lhs.data == rhs.data;
}

// [positional]
SCENARIO("Positional arguments need to be parsed.",
         "[positional]") {

  GIVEN("Arguments take single value and are not required.") {
    ParameterMap parameter_map{};
    parameter_map(Parameter<WithConversion>::Positional("FOO", 1))
           (Parameter<WithConversion>::Positional("BAR", 0))
           (Parameter<WithoutConversion>::Positional("BAZ", 2, external_converter));

    WHEN("Correct number of values are provided.") {
      InvalidArguments invalid_args;
      int argc = 4;
      const char* argv[4] = {"command", "3", "7", "11"};
      invalid_args = parameter_map.parse(argc, argv);

      THEN("Argument values are assigned in correct order.") {
        CHECK(parameter_map.convert<WithConversion>("BAR")
              == WithConversion::from_string("3"));
        CHECK(parameter_map.convert<WithConversion>("FOO")
              == WithConversion::from_string("7"));
        CHECK(parameter_map.convert<WithoutConversion>("BAZ")
              == external_converter("11"));
        CHECK(invalid_args.empty());
      }
    }

    WHEN("Too few values are provided.") {
      InvalidArguments invalid_args;
      int argc = 2;
      const char* argv[2] = {"command", "7"};
      invalid_args = parameter_map.parse(argc, argv);

      THEN("Only as many values as possible are assigned in correct order.") {
        CHECK(parameter_map.convert<WithConversion>("BAR")
              == WithConversion::from_string("7"));
        CHECK_THROWS_AS(parameter_map.convert<WithConversion>("FOO"),
                        exceptions::UnfilledParameter);
        CHECK_THROWS_AS(parameter_map.convert<WithoutConversion>("BAZ"),
                        exceptions::UnfilledParameter);
        CHECK(invalid_args.empty());
      }
    }

    WHEN("Too many values are provided.") {
      InvalidArguments invalid_args;
      int argc = 6;
      const char* argv[6] = {"command", "3", "7", "11", "zig", "zag"};
      invalid_args = parameter_map.parse(argc, argv);

      THEN("Additional values are listed in return object.") {
        CHECK(parameter_map.convert<WithConversion>("BAR")
              == WithConversion::from_string("3"));
        CHECK(parameter_map.convert<WithConversion>("FOO")
              == WithConversion::from_string("7"));
        CHECK(parameter_map.convert<WithoutConversion>("BAZ")
              == external_converter("11"));
        CHECK_FALSE(invalid_args.empty());
        CHECK(invalid_args.unfilled_parameters.empty());
        CHECK(invalid_args.additional_arguments
              == std::vector<std::string>{"zig", "zag"});
      }
    }
  }
  GIVEN("Arguments take single value and some are required.") {
    ParameterMap parameter_map{};
    parameter_map(Parameter<WithConversion>::Positional("FOO", 1))
           (Parameter<WithConversion>::Positional("BAR", 0)
                .MinArgs(1))
           (Parameter<WithoutConversion>::Positional("BAZ", 2, external_converter)
                .MinArgs(1));

    WHEN("Correct number of values are provided.") {
      InvalidArguments invalid_args;
      int argc = 4;
      const char* argv[4] = {"command", "3", "7", "11"};
      invalid_args = parameter_map.parse(argc, argv);

      THEN("Argument values are assigned in correct order.") {
        CHECK(parameter_map.convert<WithConversion>("BAR")
              == WithConversion::from_string("3"));
        CHECK(parameter_map.convert<WithConversion>("FOO")
              == WithConversion::from_string("7"));
        CHECK(parameter_map.convert<WithoutConversion>("BAZ")
              == external_converter("11"));
        CHECK(invalid_args.empty());
      }
    }

    WHEN("Too few values are provided.") {
      InvalidArguments invalid_args;
      int argc = 2;
      const char* argv[2] = {"command", "7"};
      invalid_args = parameter_map.parse(argc, argv);

      THEN("Only as many values as possible are assigned in correct order.") {
        CHECK(parameter_map.convert<WithConversion>("BAR")
              == WithConversion::from_string("7"));
        CHECK_THROWS_AS(parameter_map.convert<WithConversion>("FOO"),
                        exceptions::UnfilledParameter);
        CHECK_THROWS_AS(parameter_map.convert<WithoutConversion>("BAZ"),
                        exceptions::UnfilledParameter);
        CHECK_FALSE(invalid_args.empty());
        CHECK(invalid_args.unfilled_parameters
              == std::vector<std::string>{"BAZ"});
        CHECK(invalid_args.additional_arguments.empty());
      }
    }

    WHEN("Too many values are provided.") {
      InvalidArguments invalid_args;
      int argc = 6;
      const char* argv[6] = {"command", "3", "7", "11", "zig", "zag"};
      invalid_args = parameter_map.parse(argc, argv);

      THEN("Additional values are listed in return object.") {
        CHECK(parameter_map.convert<WithConversion>("BAR")
              == WithConversion::from_string("3"));
        CHECK(parameter_map.convert<WithConversion>("FOO")
              == WithConversion::from_string("7"));
        CHECK(parameter_map.convert<WithoutConversion>("BAZ")
              == external_converter("11"));
        CHECK_FALSE(invalid_args.empty());
        CHECK(invalid_args.unfilled_parameters.empty());
        CHECK(invalid_args.additional_arguments
              == std::vector<std::string>{"zig", "zag"});
      }
    }
  }
  GIVEN("Arguments take multiple values and are not required.") {
    ParameterMap parameter_map{};
    parameter_map(Parameter<WithConversion>::Positional("FOO", 1)
                .MinArgs(2)
                .MaxArgs(2))
           (Parameter<WithConversion>::Positional("BAR", 0)
                .MinArgs(2)
                .MaxArgs(2))
           (Parameter<WithoutConversion>::Positional("BAZ", 2, external_converter)
                .MaxArgs(4));

    WHEN("Correct number of values are provided.") {
      InvalidArguments invalid_args;
      int argc = 8;
      const char* argv[8] = {"command", "3", "7", "11", "13", "17", "19", "23"};
      invalid_args = parameter_map.parse(argc, argv);

      THEN("Argument values are assigned in correct order.") {
        CHECK(parameter_map.convert_all<WithConversion>("BAR")
              == std::vector<WithConversion>{
                    WithConversion::from_string("3"),
                    WithConversion::from_string("7")});
        CHECK(parameter_map.convert_all<WithConversion>("FOO")
              == std::vector<WithConversion>{
                    WithConversion::from_string("11"),
                    WithConversion::from_string("13")});
        CHECK(parameter_map.convert_all<WithoutConversion>("BAZ")
              == std::vector<WithoutConversion>{
                    external_converter("17"),
                    external_converter("19"),
                    external_converter("23")});
        CHECK(invalid_args.empty());
      }
    }

    WHEN("Too few values are provided.") {
      InvalidArguments invalid_args;
      int argc = 4;
      const char* argv[4] = {"command", "3", "7", "11"};
      invalid_args = parameter_map.parse(argc, argv);

      THEN("Violations are listed as appropriate.") {
        CHECK(parameter_map.convert_all<WithConversion>("BAR")
              == std::vector<WithConversion>{
                    WithConversion::from_string("3"),
                    WithConversion::from_string("7")});
        CHECK(parameter_map.convert_all<WithConversion>("FOO")
              == std::vector<WithConversion>{
                    WithConversion::from_string("11")});
        CHECK(parameter_map.convert_all<WithoutConversion>("BAZ")
              == std::vector<WithoutConversion>{});
        CHECK_FALSE(invalid_args.empty());
        CHECK(invalid_args.unfilled_parameters == std::vector<std::string>{"FOO"});
        CHECK(invalid_args.additional_arguments.empty());
      }
    }

    WHEN("Too many values are provided.") {
      InvalidArguments invalid_args;
      int argc = 10;
      const char* argv[10] = {"command", "3", "7", "11", "13", "17", "19", "23",
                              "29", "31"};
      invalid_args = parameter_map.parse(argc, argv);

      THEN("Additional values are listed in return object.") {
        CHECK(parameter_map.convert_all<WithConversion>("BAR")
              == std::vector<WithConversion>{
                    WithConversion::from_string("3"),
                    WithConversion::from_string("7")});
        CHECK(parameter_map.convert_all<WithConversion>("FOO")
              == std::vector<WithConversion>{
                    WithConversion::from_string("11"),
                    WithConversion::from_string("13")});
        CHECK(parameter_map.convert_all<WithoutConversion>("BAZ")
              == std::vector<WithoutConversion>{
                    external_converter("17"),
                    external_converter("19"),
                    external_converter("23"),
                    external_converter("29")});
        CHECK_FALSE(invalid_args.empty());
        CHECK(invalid_args.unfilled_parameters.empty());
        CHECK(invalid_args.additional_arguments
              == std::vector<std::string>{"31"});
      }
    }
  }
  GIVEN("Arguments take multiple values and some are required.") {
    ParameterMap parameter_map{};
    parameter_map(Parameter<WithConversion>::Positional("FOO", 1)
                .MinArgs(2)
                .MaxArgs(2))
           (Parameter<WithConversion>::Positional("BAR", 0)
                .MinArgs(2)
                .MaxArgs(2))
           (Parameter<WithoutConversion>::Positional("BAZ", 2, external_converter)
                .MinArgs(0)
                .MaxArgs(4));

    WHEN("Correct number of values are provided.") {
      InvalidArguments invalid_args;
      int argc = 8;
      const char* argv[8] = {"command", "3", "7", "11", "13", "17", "19", "23"};
      invalid_args = parameter_map.parse(argc, argv);

      THEN("Argument values are assigned in correct order.") {
        CHECK(parameter_map.convert_all<WithConversion>("BAR")
              == std::vector<WithConversion>{
                    WithConversion::from_string("3"),
                    WithConversion::from_string("7")});
        CHECK(parameter_map.convert_all<WithConversion>("FOO")
              == std::vector<WithConversion>{
                    WithConversion::from_string("11"),
                    WithConversion::from_string("13")});
        CHECK(parameter_map.convert_all<WithoutConversion>("BAZ")
              == std::vector<WithoutConversion>{
                    external_converter("17"),
                    external_converter("19"),
                    external_converter("23")});
        CHECK(invalid_args.empty());
      }
    }

    WHEN("Too few values are provided.") {
      InvalidArguments invalid_args;
      int argc = 4;
      const char* argv[4] = {"command", "3", "7", "11"};
      invalid_args = parameter_map.parse(argc, argv);

      THEN("Violations are listed as appropriate.") {
        CHECK(parameter_map.convert_all<WithConversion>("BAR")
              == std::vector<WithConversion>{
                    WithConversion::from_string("3"),
                    WithConversion::from_string("7")});
        CHECK(parameter_map.convert_all<WithConversion>("FOO")
              == std::vector<WithConversion>{
                    WithConversion::from_string("11")});
        CHECK(parameter_map.convert_all<WithoutConversion>("BAZ")
              == std::vector<WithoutConversion>{});
        CHECK_FALSE(invalid_args.empty());
        CHECK(invalid_args.unfilled_parameters
              == std::vector<std::string>{"FOO"});
        CHECK(invalid_args.additional_arguments.empty());
      }
    }

    WHEN("Too many values are provided.") {
      InvalidArguments invalid_args;
      int argc = 10;
      const char* argv[10] = {"command", "3", "7", "11", "13", "17", "19", "23",
                              "29", "31"};
      invalid_args = parameter_map.parse(argc, argv);

      THEN("Additional values are listed in return object.") {
        CHECK(parameter_map.convert_all<WithConversion>("BAR")
              == std::vector<WithConversion>{
                    WithConversion::from_string("3"),
                    WithConversion::from_string("7")});
        CHECK(parameter_map.convert_all<WithConversion>("FOO")
              == std::vector<WithConversion>{
                    WithConversion::from_string("11"),
                    WithConversion::from_string("13")});
        CHECK(parameter_map.convert_all<WithoutConversion>("BAZ")
              == std::vector<WithoutConversion>{
                    external_converter("17"),
                    external_converter("19"),
                    external_converter("23"),
                    external_converter("29")});
        CHECK_FALSE(invalid_args.empty());
        CHECK(invalid_args.unfilled_parameters.empty());
        CHECK(invalid_args.additional_arguments
              == std::vector<std::string>{"31"});
      }
    }
  }
}

// [keyword]
SCENARIO("Keyword arguments need to be parsed.",
         "[keyword]") {

  GIVEN("Arguments take single value and are not required.") {
    ParameterMap parameter_map{};
    parameter_map(Parameter<WithConversion>::Keyword("--foo"))
           (Parameter<WithConversion>::Keyword("--bar"))
           (Parameter<WithoutConversion>::Keyword("baz", external_converter));

    WHEN("Correct number of values are provided.") {
      InvalidArguments invalid_args;
      int argc = 7;
      const char* argv[7] = {"command", "--bar", "3", "--foo", "5", "baz",
                             "11"};
      invalid_args = parameter_map.parse(argc, argv);

      THEN("Argument values are assigned in correct order.") {
        CHECK(parameter_map.convert<WithConversion>("--bar")
              == WithConversion::from_string("3"));
        CHECK(parameter_map.convert<WithConversion>("--foo")
              == WithConversion::from_string("5"));
        CHECK(parameter_map.convert<WithoutConversion>("baz")
              == external_converter("11"));
        CHECK(invalid_args.empty());
      }
    }

    WHEN("Too few values are provided.") {
      InvalidArguments invalid_args;
      int argc = 4;
      const char* argv[4] = {"command", "--foo", "baz", "11"};
      invalid_args = parameter_map.parse(argc, argv);

      THEN("Only as many values as possible are assigned in correct order.") {
        CHECK_THROWS_AS(parameter_map.convert<WithConversion>("--bar"),
                        exceptions::UnfilledParameter);
        CHECK_THROWS_AS(parameter_map.convert<WithConversion>("--foo"),
                        exceptions::UnfilledParameter);
        CHECK(parameter_map.convert<WithoutConversion>("baz")
              == external_converter("11"));
        CHECK(invalid_args.empty());
      }
    }

    WHEN("Too many values are provided.") {
      InvalidArguments invalid_args;
      int argc = 10;
      const char* argv[10] = {"command", "--bar", "3", "--foo", "5", "7", "baz",
                              "11", "13", "17"};
      invalid_args = parameter_map.parse(argc, argv);

      THEN("Additional values are listed in return object.") {
        CHECK(parameter_map.convert<WithConversion>("--bar")
              == WithConversion::from_string("3"));
        CHECK(parameter_map.convert<WithConversion>("--foo")
              == WithConversion::from_string("5"));
        CHECK(parameter_map.convert<WithoutConversion>("baz")
              == external_converter("11"));
        CHECK_FALSE(invalid_args.empty());
        CHECK(invalid_args.unfilled_parameters.empty());
        CHECK(invalid_args.additional_arguments
              == std::vector<std::string>{"7", "13", "17"});
      }
    }

    WHEN("Argument order is scrambled.") {
      InvalidArguments invalid_args;
      int argc = 7;
      const char* argv[7] = {"command", "3", "--bar", "--foo", "5", "baz",
                             "11"};
      invalid_args = parameter_map.parse(argc, argv);

      THEN("Out-of-place arguments are listed as additional.") {
        CHECK_THROWS_AS(parameter_map.convert<WithConversion>("--bar"),
                        exceptions::UnfilledParameter);
        CHECK(parameter_map.convert<WithConversion>("--foo")
              == WithConversion::from_string("5"));
        CHECK(parameter_map.convert<WithoutConversion>("baz")
              == external_converter("11"));
        CHECK_FALSE(invalid_args.empty());
        CHECK(invalid_args.unfilled_parameters.empty());
        CHECK(invalid_args.additional_arguments == std::vector<std::string>{"3"});
      }
    }
  }
  GIVEN("Arguments take single value and some are required.") {
    ParameterMap parameter_map{};
    parameter_map(Parameter<WithConversion>::Keyword("--foo"))
           (Parameter<WithConversion>::Keyword("--bar")
                .MinArgs(1))
           (Parameter<WithoutConversion>::Keyword("baz", external_converter)
                .MinArgs(1));

    WHEN("Correct number of values are provided.") {
      InvalidArguments invalid_args;
      int argc = 7;
      const char* argv[7] = {"command", "--bar", "3", "--foo", "5", "baz",
                             "11"};
      invalid_args = parameter_map.parse(argc, argv);

      THEN("Argument values are assigned in correct order.") {
        CHECK(parameter_map.convert<WithConversion>("--bar")
              == WithConversion::from_string("3"));
        CHECK(parameter_map.convert<WithConversion>("--foo")
              == WithConversion::from_string("5"));
        CHECK(parameter_map.convert<WithoutConversion>("baz")
              == external_converter("11"));
        CHECK(invalid_args.empty());
      }
    }

    WHEN("Too few values are provided.") {
      InvalidArguments invalid_args;
      int argc = 4;
      const char* argv[4] = {"command", "--foo", "baz", "11"};
      invalid_args = parameter_map.parse(argc, argv);

      THEN("Only as many values as possible are assigned in correct order.") {
        CHECK_THROWS_AS(parameter_map.convert<WithConversion>("--bar"),
                        exceptions::UnfilledParameter);
        CHECK_THROWS_AS(parameter_map.convert<WithConversion>("--foo"),
                        exceptions::UnfilledParameter);
        CHECK(parameter_map.convert<WithoutConversion>("baz")
              == external_converter("11"));
        CHECK_FALSE(invalid_args.empty());
        CHECK(invalid_args.unfilled_parameters
              == std::vector<std::string>{"--bar"});
        CHECK(invalid_args.additional_arguments.empty());
      }
    }

    WHEN("Too many values are provided.") {
      InvalidArguments invalid_args;
      int argc = 10;
      const char* argv[10] = {"command", "--bar", "3", "--foo", "5", "7", "baz",
                              "11", "13", "17"};
      invalid_args = parameter_map.parse(argc, argv);

      THEN("Additional values are listed in return object.") {
        CHECK(parameter_map.convert<WithConversion>("--bar")
              == WithConversion::from_string("3"));
        CHECK(parameter_map.convert<WithConversion>("--foo")
              == WithConversion::from_string("5"));
        CHECK(parameter_map.convert<WithoutConversion>("baz")
              == external_converter("11"));
        CHECK_FALSE(invalid_args.empty());
        CHECK(invalid_args.unfilled_parameters.empty());
        CHECK(invalid_args.additional_arguments
              == std::vector<std::string>{"7", "13", "17"});
      }
    }

    WHEN("Argument order is scrambled.") {
      InvalidArguments invalid_args;
      int argc = 7;
      const char* argv[7] = {"command", "3", "--bar", "--foo", "5", "baz",
                             "11"};
      invalid_args = parameter_map.parse(argc, argv);

      THEN("Out-of-place arguments are listed as additional.") {
        CHECK_THROWS_AS(parameter_map.convert<WithConversion>("--bar"),
                        exceptions::UnfilledParameter);
        CHECK(parameter_map.convert<WithConversion>("--foo")
              == WithConversion::from_string("5"));
        CHECK(parameter_map.convert<WithoutConversion>("baz")
              == external_converter("11"));
        CHECK_FALSE(invalid_args.empty());
        CHECK(invalid_args.unfilled_parameters
              == std::vector<std::string>{"--bar"});
        CHECK(invalid_args.additional_arguments == std::vector<std::string>{"3"});
      }
    }
  }
  GIVEN("Arguments take multiple values and are not required.") {
    ParameterMap parameter_map{};
    parameter_map(Parameter<WithConversion>::Keyword("--foo")
                .MinArgs(2)
                .MaxArgs(2))
           (Parameter<WithConversion>::Keyword("--bar")
                .MinArgs(2)
                .MaxArgs(2))
           (Parameter<WithoutConversion>::Keyword("baz", external_converter)
                .MaxArgs(4));

    WHEN("Correct number of values are provided.") {
      InvalidArguments invalid_args;
      int argc = 11;
      const char* argv[11] = {"command", "--bar", "3", "7", "--foo", "11", "13",
                              "baz", "17", "19", "23"};
      invalid_args = parameter_map.parse(argc, argv);

      THEN("Argument values are assigned in correct order.") {
        CHECK(parameter_map.convert_all<WithConversion>("--bar")
              == std::vector<WithConversion>{
                    WithConversion::from_string("3"),
                    WithConversion::from_string("7")});
        CHECK(parameter_map.convert_all<WithConversion>("--foo")
              == std::vector<WithConversion>{
                    WithConversion::from_string("11"),
                    WithConversion::from_string("13")});
        CHECK(parameter_map.convert_all<WithoutConversion>("baz")
              == std::vector<WithoutConversion>{
                    external_converter("17"),
                    external_converter("19"),
                    external_converter("23")});
        CHECK(invalid_args.empty());
      }
    }

    WHEN("Too few values are provided.") {
      InvalidArguments invalid_args;
      int argc = 6;
      const char* argv[6] = {"command", "--bar", "3", "7", "--foo", "11"};
      invalid_args = parameter_map.parse(argc, argv);

      THEN("Violations are listed as appropriate.") {
        CHECK(parameter_map.convert_all<WithConversion>("--bar")
              == std::vector<WithConversion>{
                    WithConversion::from_string("3"),
                    WithConversion::from_string("7")});
        CHECK(parameter_map.convert_all<WithConversion>("--foo")
              == std::vector<WithConversion>{
                    WithConversion::from_string("11")});
        CHECK(parameter_map.convert_all<WithoutConversion>("baz")
              == std::vector<WithoutConversion>{});
        CHECK_FALSE(invalid_args.empty());
        CHECK(invalid_args.unfilled_parameters
              == std::vector<std::string>{"--foo"});
        CHECK(invalid_args.additional_arguments.empty());
      }
    }

    WHEN("Too many values are provided.") {
      InvalidArguments invalid_args;
      int argc = 14;
      const char* argv[14] = {"command", "--bar", "3", "5", "7", "--foo", "11",
                              "13", "baz", "17", "19", "23", "29", "31"};
      invalid_args = parameter_map.parse(argc, argv);

      THEN("Additional values are listed in return object.") {
        CHECK(parameter_map.convert_all<WithConversion>("--bar")
              == std::vector<WithConversion>{
                    WithConversion::from_string("3"),
                    WithConversion::from_string("5")});
        CHECK(parameter_map.convert_all<WithConversion>("--foo")
              == std::vector<WithConversion>{
                    WithConversion::from_string("11"),
                    WithConversion::from_string("13")});
        CHECK(parameter_map.convert_all<WithoutConversion>("baz")
              == std::vector<WithoutConversion>{
                    external_converter("17"),
                    external_converter("19"),
                    external_converter("23"),
                    external_converter("29")});
        CHECK_FALSE(invalid_args.empty());
        CHECK(invalid_args.unfilled_parameters.empty());
        CHECK(invalid_args.additional_arguments
              == std::vector<std::string>{"7", "31"});
      }
    }

    WHEN("Argument order is scrambled.") {
      InvalidArguments invalid_args;
      int argc = 11;
      const char* argv[11] = {"command", "3", "--bar", "7", "--foo", "11", "13",
                        "17", "19", "baz", "23"};
      invalid_args = parameter_map.parse(argc, argv);

      THEN("Out-of-place arguments are listed as additional.") {
        CHECK(parameter_map.convert_all<WithConversion>("--bar")
              == std::vector<WithConversion>{WithConversion::from_string("7")});
        CHECK(parameter_map.convert_all<WithConversion>("--foo")
              == std::vector<WithConversion>{
                    WithConversion::from_string("11"),
                    WithConversion::from_string("13")});
        CHECK(parameter_map.convert_all<WithoutConversion>("baz")
              == std::vector<WithoutConversion>{external_converter("23")});
        CHECK_FALSE(invalid_args.empty());
        CHECK(invalid_args.unfilled_parameters
              == std::vector<std::string>{"--bar"});
        CHECK(invalid_args.additional_arguments
              == std::vector<std::string>{"3", "17", "19"});
      }
    }
  }
  GIVEN("Arguments take multiple values and some are required.") {
    ParameterMap parameter_map{};
    parameter_map(Parameter<WithConversion>::Keyword("--foo")
                .MinArgs(2)
                .MaxArgs(2))
           (Parameter<WithConversion>::Keyword("--bar")
                .MinArgs(2)
                .MaxArgs(2))
           (Parameter<WithoutConversion>::Keyword("baz", external_converter)
                .MaxArgs(4)
                .MinArgs(1));

    WHEN("Correct number of values are provided.") {
      InvalidArguments invalid_args;
      int argc = 11;
      const char* argv[11] = {"command", "--bar", "3", "7", "--foo", "11", "13",
                        "baz", "17", "19", "23"};
      invalid_args = parameter_map.parse(argc, argv);

      THEN("Argument values are assigned in correct order.") {
        CHECK(parameter_map.convert_all<WithConversion>("--bar")
              == std::vector<WithConversion>{
                    WithConversion::from_string("3"),
                    WithConversion::from_string("7")});
        CHECK(parameter_map.convert_all<WithConversion>("--foo")
              == std::vector<WithConversion>{
                    WithConversion::from_string("11"),
                    WithConversion::from_string("13")});
        CHECK(parameter_map.convert_all<WithoutConversion>("baz")
              == std::vector<WithoutConversion>{
                    external_converter("17"),
                    external_converter("19"),
                    external_converter("23")});
        CHECK(invalid_args.empty());
      }
    }

    WHEN("Too few values are provided.") {
      InvalidArguments invalid_args;
      int argc = 6;
      const char* argv[6] = {"command", "--bar", "3", "7", "--foo", "11"};
      invalid_args = parameter_map.parse(argc, argv);

      THEN("Violations are listed as appropriate.") {
        CHECK(parameter_map.convert_all<WithConversion>("--bar")
              == std::vector<WithConversion>{
                    WithConversion::from_string("3"),
                    WithConversion::from_string("7")});
        CHECK(parameter_map.convert_all<WithConversion>("--foo")
              == std::vector<WithConversion>{
                    WithConversion::from_string("11")});
        CHECK(parameter_map.convert_all<WithoutConversion>("baz")
              == std::vector<WithoutConversion>{});
        CHECK_FALSE(invalid_args.empty());
        CHECK(std::unordered_set<std::string>(
                  invalid_args.unfilled_parameters.begin(),
                  invalid_args.unfilled_parameters.end())
              == std::unordered_set<std::string>{"--foo", "baz"});
        CHECK(invalid_args.additional_arguments.empty());
      }
    }

    WHEN("Too many values are provided.") {
      InvalidArguments invalid_args;
      int argc = 14;
      const char* argv[14] = {"command", "--bar", "3", "5", "7", "--foo", "11",
                              "13", "baz", "17", "19", "23", "29", "31"};
      invalid_args = parameter_map.parse(argc, argv);

      THEN("Additional values are listed in return object.") {
        CHECK(parameter_map.convert_all<WithConversion>("--bar")
              == std::vector<WithConversion>{
                    WithConversion::from_string("3"),
                    WithConversion::from_string("5")});
        CHECK(parameter_map.convert_all<WithConversion>("--foo")
              == std::vector<WithConversion>{
                    WithConversion::from_string("11"),
                    WithConversion::from_string("13")});
        CHECK(parameter_map.convert_all<WithoutConversion>("baz")
              == std::vector<WithoutConversion>{
                    external_converter("17"),
                    external_converter("19"),
                    external_converter("23"),
                    external_converter("29")});
        CHECK_FALSE(invalid_args.empty());
        CHECK(invalid_args.unfilled_parameters.empty());
        CHECK(invalid_args.additional_arguments
              == std::vector<std::string>{"7", "31"});
      }
    }

    WHEN("Argument order is scrambled.") {
      InvalidArguments invalid_args;
      int argc = 11;
      const char* argv[11] = {"command", "3", "--bar", "7", "--foo", "11", "13",
                        "17", "19", "baz", "23"};
      invalid_args = parameter_map.parse(argc, argv);

      THEN("Out-of-place arguments are listed as additional.") {
        CHECK(parameter_map.convert_all<WithConversion>("--bar")
              == std::vector<WithConversion>{WithConversion::from_string("7")});
        CHECK(parameter_map.convert_all<WithConversion>("--foo")
              == std::vector<WithConversion>{
                    WithConversion::from_string("11"),
                    WithConversion::from_string("13")});
        CHECK(parameter_map.convert_all<WithoutConversion>("baz")
              == std::vector<WithoutConversion>{external_converter("23")});
        CHECK_FALSE(invalid_args.empty());
        CHECK(invalid_args.unfilled_parameters == std::vector<std::string>{"--bar"});
        CHECK(invalid_args.additional_arguments
              == std::vector<std::string>{"3", "17", "19"});
      }
    }
  }
}

// [flag]
SCENARIO("Flags need to be parsed.", "[flag]") {

  GIVEN("Arguments take single value and are not required.") {
    ParameterMap parameter_map{};
    parameter_map(Parameter<bool>::Flag('a'))
           (Parameter<bool>::Flag('b'))
           (Parameter<bool>::Flag('c'))
           (Parameter<bool>::Flag('d'))
           (Parameter<bool>::Flag('e'));

    WHEN("Flags are provided separately.") {
      InvalidArguments invalid_args;
      int argc = 4;
      const char* argv[4] = {"command", "-a", "-c", "-d"};
      invalid_args = parameter_map.parse(argc, argv);

      THEN("Correct flags are set.") {
        CHECK(parameter_map.IsSet("-a"));
        CHECK_FALSE(parameter_map.IsSet("-b"));
        CHECK(parameter_map.IsSet('c'));
        CHECK(parameter_map.IsSet("-d"));
        CHECK_FALSE(parameter_map.IsSet("-e"));
        CHECK(invalid_args.empty());
      }
    }

    WHEN("Flags are provided together.") {
      InvalidArguments invalid_args;
      int argc = 2;
      const char* argv[2] = {"command", "-acd"};
      invalid_args = parameter_map.parse(argc, argv);

      THEN("Correct flags are set.") {
        CHECK(parameter_map.IsSet("-a"));
        CHECK_FALSE(parameter_map.IsSet("-b"));
        CHECK(parameter_map.IsSet("-c"));
        CHECK(parameter_map.IsSet("-d"));
        CHECK_FALSE(parameter_map.IsSet("-e"));
        CHECK(invalid_args.empty());
      }
    }

    WHEN("Some flags are provided separately and some together.") {
      InvalidArguments invalid_args;
      int argc = 3;
      const char* argv[3] = {"command", "-ad", "-c"};
      invalid_args = parameter_map.parse(argc, argv);

      THEN("Correct flags are set.") {
        CHECK(parameter_map.IsSet("-a"));
        CHECK_FALSE(parameter_map.IsSet("-b"));
        CHECK(parameter_map.IsSet("-c"));
        CHECK(parameter_map.IsSet("-d"));
        CHECK_FALSE(parameter_map.IsSet("-e"));
        CHECK(invalid_args.empty());
      }
    }
  }
}

} // namespace test

} // namespace arg_parse_convert