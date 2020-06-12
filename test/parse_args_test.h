// MIT License
// 
// Copyright (c) 2020 Jasper Braun
// 
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
// 
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
// 
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.

#include "lcl-parser.h"

#define CATCH_CONFIG_MAIN
#include "catch.h"

SCENARIO("Arguments can be stored with correct type.") {
  lclp::parser parser{};

  // assert parser's default construction
  //REQUIRE(parser.member() == member_default_value);

  GIVEN("Arguments are specified as boolean flags.") {
    int argc = 5;
    char* argv[5] = {"command", "-abc", "-d", "-ef", "-g"};
    bool a, b{true}, c{false}, d, e, f, g, h, i{true}, j{false};

    WHEN("Flags are anticipated.") {
      parser.add_arguments()
          (lclp::AddArg::flag('a', &a)->description("Description of flag a."))
          (lclp::AddArg::flag('b', &b)->description("Description of flag b."))
          (lclp::AddArg::flag('c', &c)->description("Description of flag c."))
          (lclp::AddArg::flag('d', &d)->description("Description of flag d."))
          (lclp::AddArg::flag('e', &e)->description("Description of flag e."))
          (lclp::AddArg::flag('f', &f)->description("Description of flag f."))
          (lclp::AddArg::flag('g', &g)->description("Description of flag g."))
          (lclp::AddArg::flag('h', &g)->description("Description of flag h."))
          (lclp::AddArg::flag('i', &g)->description("Description of flag i."))
          (lclp::AddArg::flag('j', &g)->description("Description of flag j."));

      THEN("The correct flags are set to true and all others to false.") {
        parser.parse_args(argc, argv);
        CHECK(a);
        CHECK(b);
        CHECK(c);
        CHECK(d);
        CHECK(e);
        CHECK(f);
        CHECK(g);
        CHECK_FALSE(h);
        CHECK_FALSE(i);
        CHECK_FALSE(j);
      }
    }
    WHEN("Some flags are not recognized.") {
      parser.add_arguments()
          (lclp::AddArg::flag('a', &a)->description("Description of flag a."))
          (lclp::AddArg::flag('b', &b)->description("Description of flag b."))
          (lclp::AddArg::flag('c', &c)->description("Description of flag c."))
          (lclp::AddArg::flag('d', &d)->description("Description of flag d."))
          // 'e' is missing
          (lclp::AddArg::flag('f', &f)->description("Description of flag f."))
          (lclp::AddArg::flag('g', &g)->description("Description of flag g."))
          (lclp::AddArg::flag('h', &g)->description("Description of flag h."))
          (lclp::AddArg::flag('i', &g)->description("Description of flag i."))
          (lclp::AddArg::flag('j', &g)->description("Description of flag j."));

      THEN("A fatal exception is thrown.") {
        REQUIRE_THROWS_MATCHES(parser.parse_args(argc, argv),
                               lclp::UnknownFlag,
                               Catch::StartsWith("Unknown flag: '-")
                               && Catch::EndsWith(
                                  "Specify '-h' for usage information."));
      }
    }
  }
  GIVEN("Arguments are specified as valid keyword-value pair-list.") {
    int argc = 5;
    char* argv[5] = {"command", "--foo", "foo_val", "--bar", "bar_val"};

    WHEN("Arguments are anticipated.")
      // define individual parsers
      struct Bar {
        static Bar from_str(const char*& arg);
        std::string data;
      };
      Bar Bar::from_str() {
        Bar result;
        result.data = arg;
        result.data.append("_BAR");
        return result;
      }
      std::string append_BAZ(const char*& arg) {
        std::string result{arg};
        result.append("_BAZ");
        return result;
      }

      std::string foo, baz, bif;
      Bar bar;

      parser.add_arguments()
          (lclp::AddArg::kwarg<std::string>(
              "foo", &foo,
              [](const char*& arg){return std::string{arg}.append("_FOO");})
              ->description("Description of foo."))
          (lclp::AddArg::kwarg<Bar>("bar", &bar)
              ->description("Description of bar."))
          (lclp::AddArg::kwarg<std::string>("baz", &baz, append_BAZ)
              ->description("Description of baz."))
          (lclp::AddArg::kwarg<std::string>("bif", &bif, append_BAZ)
              ->description("Description of bif.")
              ->default("bif_val_BIF"));

      THEN("Their values are converted according to specified converter.") {
        parser.parse_args(argc, argv);
        CHECK(foo == "foo_val_FOO");
        CHECK(bar == "bar_val_BAR");
        CHECK(baz == "");
        CHECK(bif == "bif_val_BIF");
        CHECK_FALSE(foo.default());
        CHECK_FALSE(bar.default());
        CHECK_FALSE(baz.default());
        CHECK_TRUE(bif.default());
      }

      THEN("Missing required arguments cause exception to be thrown.") {
        parser.args<std::string>->at("bif").required();
        REQUIRE_THROWS_MATCHES(parser.parse_args(argc, argv),
                               lclp::MissingKeywordArgument,
                               Catch::StartsWith(
                                  "Missing keyword argument: '--")
                               && Catch::EndsWith(
                                  "Specify '-h' for usage information."));
      }

    WHEN("Arguments are not anticipated.") {

      std::string append_BAZ(const char*& arg) {
        std::string result{arg};
        result.append("_BAZ");
        return result;
      }

      std::string foo, baz;

      parser.add_arguments()
          (lclp::AddArg::kwarg<std::string>(
              "foo", &foo,
              [](const char*& arg){return std::string{arg}.append("_FOO");})
              ->description("Description of foo."))
          // --bar bar_val is not anticipated
          (lclp::AddArg::kwarg<std::string>("baz", &baz, append_BAZ)
              ->description("Description of baz."));

      THEN("Unrecognized arguments cause exception to be thrown.") {
        REQUIRE_THROWS_MATCHES(parser.parse_args(argc, argv),
                               lclp::UnknownKeywordArgument,
                               Catch::StartsWith(
                                  "Unknown keyword argument: '--")
                               && Catch::EndsWith(
                                  "Specify '-h' for usage information."));
      }

    }

  }
  GIVEN("Arguments are specified as positional arguments.") {
    int argc = 4;
    char* argv[4] = {"command", "foo_val", "bar_val"};
    std::string foo, bar, baz, bif;

    WHEN("Arguments are anticipated.")
      // define individual parsers
      struct Bar {
        static Bar from_str(const char*& arg);
        std::string data;
      };
      Bar Bar::from_str() {
        Bar result;
        result.data = arg;
        result.data.append("_BAR");
        return result;
      }
      std::string append_BAZ(const char*& arg) {
        std::string result{arg};
        result.append("_BAZ");
        return result;
      }

      std::string foo, baz, bif;
      Bar bar;

      parser.add_arguments()
          (lclp::AddArg::posarg<std::string>(
              "foo", &foo, -1
              [](const char*& arg){return std::string{arg}.append("_FOO");})
              ->description("Description of foo."))
          (lclp::AddArg::posarg<Bar>("bar", &bar, 2)
              ->description("Description of bar."))
          (lclp::AddArg::posarg<std::string>("baz", &baz, 5, append_BAZ)
              ->description("Description of baz."))
          (lclp::AddArg::posarg<std::string>("bif", &bif, 9, append_BAZ)
              ->description("Description of bif.")
              ->default("bif_val_BIF"));

      THEN("Their values are converted according to specified converter.") {
        parser.parse_args(argc, argv);
        CHECK(foo == "foo_val_FOO");
        CHECK(bar == "bar_val_BAR");
        CHECK(baz == "");
        CHECK(bif == "bif_val_BIF");
        CHECK_FALSE(foo.default());
        CHECK_FALSE(bar.default());
        CHECK_FALSE(baz.default());
        CHECK_TRUE(bif.default());
      }

      THEN("Missing required arguments cause exception to be thrown.") {
        parser.args<std::string>->at("baz").required();
        REQUIRE_THROWS_MATCHES(parser.parse_args(argc, argv),
                               lclp::MissingPositionalArgument,
                               Catch::StartsWith(
                                  "Missing positional argument: '")
                               && Catch::EndsWith(
                                  "Specify '-h' for usage information."));
      }

    WHEN("Arguments are not anticipated.") {

      std::string append_BAZ(const char*& arg) {
        std::string result{arg};
        result.append("_BAZ");
        return result;
      }

      std::string foo, baz;

      parser.add_arguments()
          (lclp::AddArg::posarg<std::string>(
              "foo", &foo, 1,
              [](const char*& arg){return std::string{arg}.append("_FOO");})
              ->description("Description of foo."))
          // --bar bar_val is not anticipated
          (lclp::AddArg::posarg<std::string>("baz", &baz, 2, append_BAZ)
              ->description("Description of baz."));

      THEN("Unrecognized arguments cause exception to be thrown.") {
        REQUIRE_THROWS_MATCHES(parser.parse_args(argc, argv),
                               lclp::UnknownPositionalArgument,
                               Catch::StartsWith(
                                  "Unknown positional argument: '")
                               && Catch::EndsWith(
                                  "Specify '-h' for usage information."));
      }

    }
  }
  GIVEN("Arguments are specified as a valid mixture.") {
    int argc = 5;
    char* argv[5] = {"command", "-ac", "foo", "--bar", "bar_val", "-d", "baz"};
    std::string foo, bar, baz, bif, bof;
    bool a, b, c, d, e, f;

    WHEN("Arguments are anticipated.")
      // define individual parsers
      std::string append_x(const char*& arg) {
        std::string result{arg};
        result.append("_x");
        return result;
      }

      parser.add_arguments()
          (lclp::AddArg::posarg<std::string>("foo", &foo, 1, append_x)
              ->description("Description of foo."))
          (lclp::AddArg::posarg<std::string>("baz", &baz, 2, append_x)
              ->description("Description of baz."))
          (lclp::AddArg::posarg<std::string>("bif", &bif, 3, append_x)
              ->description("Description of bif.")
              ->default("bif_val_BIF"))
          (lclp::AddArg::kwarg<std::string>("bar", &bar, append_x)
              ->description("Description of bar."))
          (lclp::AddArg::kwarg<std::string>("bof", &bof, append_x)
              ->default("bof_val_BOF"))
          (lclp::AddArg::flag('a', &a)->description("Description of flag a."))
          (lclp::AddArg::flag('b', &b)->description("Description of flag b."))
          (lclp::AddArg::flag('c', &c)->description("Description of flag c."))
          (lclp::AddArg::flag('d', &d)->description("Description of flag d."))
          (lclp::AddArg::flag('e', &e)->description("Description of flag e."))
          (lclp::AddArg::flag('f', &f)->description("Description of flag f."));

      THEN("Their values are converted according to specified converter.") {
        parser.parse_args(argc, argv);
        CHECK(foo == "foo_x");
        CHECK(bar == "baz_x");
        CHECK(bif == "bif_val_BIF");
        check(bar == "bar_val_x");
        CHECK(bof == "bof_val_BOF");
        CHECK_TRUE(a);
        CHECK_TRUE(c);
        CHECK_TRUE(d);
        CHECK_FALSE(b);
        CHECK_FALSE(e);
        CHECK_FALSE(f);
        CHECK_FALSE(foo.default());
        CHECK_FALSE(baz.default());
        CHECK_TRUE(bif.default());
        CHECK_FALSE(bar.default());
        CHECK_TRUE(bof.default());
      }

      THEN("Missing required arguments cause exception to be thrown.") {
        parser.args<std::string>->at("bif").required();
        REQUIRE_THROWS_MATCHES(parser.parse_args(argc, argv),
                               lclp::MissingPositionalArgument,
                               Catch::StartsWith(
                                  "Missing positional argument: '")
                               && Catch::EndsWith(
                                  "Specify '-h' for usage information."));
      }

      THEN("Missing required arguments cause exception to be thrown.") {
        parser.args<std::string>->at("bof").required();
        REQUIRE_THROWS_MATCHES(parser.parse_args(argc, argv),
                               lclp::MissingPositionalArgument,
                               Catch::StartsWith(
                                  "Missing keyword argument: '")
                               && Catch::EndsWith(
                                  "Specify '-h' for usage information."));
      }

    WHEN("Arguments are not anticipated.") {

      // define individual parsers
      std::string append_x(const char*& arg) {
        std::string result{arg};
        result.append("_x");
        return result;
      }

      parser.add_arguments()
          (lclp::AddArg::posarg<std::string>("foo", &foo, 1, append_x)
              ->description("Description of foo."))
          (lclp::AddArg::kwarg<std::string>("bof", &bof, append_x)
              ->default("bof_val_BOF"))
          (lclp::AddArg::flag('b', &b)->description("Description of flag b."))
          (lclp::AddArg::flag('c', &c)->description("Description of flag c."))
          (lclp::AddArg::flag('d', &d)->description("Description of flag d."))
          (lclp::AddArg::flag('e', &e)->description("Description of flag e."))
          (lclp::AddArg::flag('f', &f)->description("Description of flag f."));

      THEN("Unrecognized positional arguments cause exception to be thrown.") {
        parser.add_arguments()
            (lclp::AddArg::kwarg<std::string>("bar", &bar, append_x)
                ->description("Description of bar."))
            (lclp::AddArg::flag('a', &a)
                ->description("Description of flag a."));
        REQUIRE_THROWS_MATCHES(parser.parse_args(argc, argv),
                               lclp::UnknownPositionalArgument,
                               Catch::StartsWith(
                                  "Unknown positional argument: '")
                               && Catch::EndsWith(
                                  "Specify '-h' for usage information."));
      }

      THEN("Unrecognized keyword arguments cause exception to be thrown.") {
        parser.add_arguments()
            (lclp::AddArg::posarg<std::string>("baz", &baz, 2, append_x)
                ->description("Description of baz."))
            (lclp::AddArg::posarg<std::string>("bif", &bif, 3, append_x)
                ->description("Description of bif.")
                ->default("bif_val_BIF"))
            (lclp::AddArg::flag('a', &a)
                ->description("Description of flag a."));
        REQUIRE_THROWS_MATCHES(parser.parse_args(argc, argv),
                               lclp::UnknownKeywordArgument,
                               Catch::StartsWith(
                                  "Unknown keyword argument: '")
                               && Catch::EndsWith(
                                  "Specify '-h' for usage information."));
      }

      THEN("Unrecognized flags cause exception to be thrown.") {
        parser.add_arguments()
            (lclp::AddArg::posarg<std::string>("baz", &baz, 2, append_x)
                ->description("Description of baz."))
            (lclp::AddArg::posarg<std::string>("bif", &bif, 3, append_x)
                ->description("Description of bif.")
                ->default("bif_val_BIF"))
            (lclp::AddArg::kwarg<std::string>("bar", &bar, append_x)
                ->description("Description of bar."));
        REQUIRE_THROWS_MATCHES(parser.parse_args(argc, argv),
                               lclp::UnknownFlag,
                               Catch::StartsWith(
                                  "Unknown flag: '")
                               && Catch::EndsWith(
                                  "Specify '-h' for usage information."));
      }

    }
  }
}