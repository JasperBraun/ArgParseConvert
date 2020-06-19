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

#ifndef ARG_PARSE_CONVERT_EXCEPTIONS_H_
#define ARG_PARSE_CONVERT_EXCEPTIONS_H_

namespace arg_parse_convert {

namespace exceptions {

/// @brief Base class for exceptions thrown by entities in the `arg_parse_convert`
///  namespace.
///
/// @details Only exceptions of a final derived type are thrown. Some entities
///  in the `arg_parse_convert` namespace, however, use STL operations which may throw their
///  own exceptions. This is documented for each entity separately.
///
struct BaseException : public std::logic_error {
  using std::logic_error::logic_error;
};

/// @brief Thrown when attempting to register a parameter with a name under
///  which another parameter is already registered
///
struct DuplicateParameterName final : public BaseException {
  using BaseException::BaseException;
};

/// @brief Thrown when attempting to check if a flag is set using a function
///  other than `ParameterMap::IsSet`.
///
struct InvalidFlagConversion final : public BaseException {
  using BaseException::BaseException;
};

/// @brief Thrown when attempting to set negative help-string formatting
///  parameters, or indentations equal to or larger than total width of
///  help-string.
///
struct InvalidFormattingParameters final : public BaseException {
  using BaseException::BaseException;
};

/// @brief Thrown when attempting to access conversion function of a parameter
///  using the wrong type.
///
struct MismatchedParameterType final : public BaseException {
  using BaseException::BaseException;
};

/// @brief Thrown when attempting to check if a flag is set using the name of a
///  registered parameter that is not a flag.
///
struct NoFlagWithName final : public BaseException {
  using BaseException::BaseException;
};

/// @brief Thrown when attempting to convert arguments from empty argument list.
///
struct UnfilledParameter final : public BaseException {
  using BaseException::BaseException;
};

/// @brief Thrown when attempting to parse an argument starting with '-', that
///  is not the name of a keyword parameter's key, or the name of a flag, or
///  when the list of letters following the '-' character contains a letter for
///  which not flag is registered.
///
struct UnkownFlagOrKeyword final : public BaseException {
  using BaseException::BaseException;
};

/// @brief Thrown when attempting to access unregistered parameter name.
///
struct UnknownParameterName final : public BaseException {
  using BaseException::BaseException;
};

/*
/// @brief Thrown when attempting to access an argument in an `ArgumentMap`
///  whose name was not registered with the object.
/// @see `ArgumentMap`, `ArgumentMap::at`.
///
struct UnknownArgumentName final : public BaseException {
  using BaseException::BaseException;
};



/// @brief Thrown when attempting to access or register an argument in an
///  `ArgumentMap` object which does not recognize the argument's type.
/// @see `ArgumentMap`, `ArgumentMap::at`, `ArgumentMap::operator()`.
///
struct UnknownArgumentType final : public BaseException {
  using BaseException::BaseException;
};

/// @brief Thrown when attempting to register a positional argument with an
///  `ArgumentMap` object at a position that was already taken by another
///  registered positional argument.
/// @see `ArgumentMap`, `ArgumentMap::operator()`.
///
struct DuplicateArgumentPosition final : public BaseException {
  using BaseException::BaseException;
};

/// @brief Thrown when attempting to access conversion function pointer of
///  argument whose conversion function pointer was not assigned.
/// @see `Argument`, `Argument::converter`.
///
struct EmptyConversionFunction final : public BaseException {
  using BaseException::BaseException;
};

/// @brief Thrown when the name of a non-positional argument appears more than
///  once in the list of arguments parsed.
/// @see `ArgumentMap`, `ArgumentMap::parse_arguments`.
///
struct DuplicateArgumentValue final : public BaseException {
  using BaseException::BaseException;
};
*/

namespace internal {

/// @brief Exception marking an unexpected case that should never occur.
///
struct UnexpectedCase final : public BaseException {
  using BaseException::BaseException;
};

} // namespace internal

} // namespace exceptions

} // namespace arg_parse_convert

#endif // ARG_PARSE_CONVERT_EXCEPTIONS_H_