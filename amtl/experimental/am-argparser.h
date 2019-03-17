// vim: set sts=8 ts=2 sw=2 tw=99 et:
//
// Copyright (C) 2018, David Anderson and AlliedModders LLC
// All rights reserved.
// 
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
// 
//  * Redistributions of source code must retain the above copyright notice, this
//    list of conditions and the following disclaimer.
//  * Redistributions in binary form must reproduce the above copyright notice,
//    this list of conditions and the following disclaimer in the documentation
//    and/or other materials provided with the distribution.
//  * Neither the name of AlliedModders LLC nor the names of its contributors
//    may be used to endorse or promote products derived from this software
//    without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
// AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
// ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
// LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
// CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
// SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
// INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
// CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
// ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
// POSSIBILITY OF SUCH DAMAGE.
#ifndef _include_amtl_am_argparser_h_
#define _include_amtl_am_argparser_h_

#include <amtl/am-cxx.h>
#include <amtl/am-string.h>
#include <amtl/am-vector.h>
#include <amtl/am-maybe.h>
#include <amtl/am-algorithm.h>
#include <amtl/am-function.h>
#include <assert.h>
#include <stdio.h>

namespace ke {
namespace args {

class IOption;

class Parser
{
  template <typename T>
  friend class Option;

 public:
  explicit inline Parser(const char* help);

  inline void add(IOption* option);
  inline bool parse(int argc, char** argv);
  inline bool parse(const Vector<const char*>& args);
  inline bool parse(const Vector<AString>& args);
  inline bool parsev(const char* arg1, ...);

  inline void usage(FILE* fp, int argc, char** argv);
  inline void usage_line(char** argv, FILE* out);

  inline void reset();

 private:
  inline bool parse_impl(const Vector<const char*>& args);

  inline IOption* find_short(const char* short_form, size_t len);
  inline IOption* find_long(const char* long_form, size_t len);

  inline bool unrecognized_option(const char* arg);
  inline bool unrecognized_extra();
  inline bool option_needs_value(IOption* option);
  inline bool option_invalid_value(IOption* option, const char* value);
  inline bool missing_positional(IOption* option);
  inline bool option_already_specified(IOption* option);

 private:
  Vector<IOption*> options_;
  Vector<IOption*> positionals_;
  const char* help_;
  ke::AString error_;
  UniquePtr<IOption> help_option_;
};

class IOption
{
  friend class Parser;

  virtual bool isToggle() const {
    return false;
  }
  virtual bool canOmitValue() const {
    return false;
  }
  virtual bool repeatable() const {
    return false;
  }
  virtual bool haltOnValue() const {
    return false;
  }
  virtual bool consumeValue(const char* arg) = 0;

 public:
  virtual ~IOption() {
  }

 protected:
  IOption(const char* short_form,
          const char* long_form,
          const char* name,
          const char* help)
   : short_form_(short_form),
     long_form_(long_form),
     name_(name),
     help_(help),
     has_value_(false)
  {
    // Cannot have both short/longform and name.
    assert(((short_form || long_form) && !name_) ||
           (!short_form && !long_form && name_));

    if (short_form_ && short_form_[0] == '-')
      short_form_++;
    if (long_form_ && long_form_[0] == '-' && long_form_[1] == '-')
      long_form_ += 2;

    if (short_form_)
      assert(short_form_[0] && short_form_[0] != '-');
    if (long_form_)
      assert(long_form_[0] && long_form_[0] != '-');
  }

  const char* short_form() const {
    return short_form_;
  }
  const char* long_form() const {
    return long_form_;
  }
  const char* name() const {
    return name_;
  }
  const char* help() const {
    return help_;
  }
  bool has_value() const {
    return has_value_;
  }

  const char* pretty_name() const {
    if (name_)
      return name_;
    if (long_form_)
      return long_form_;
    return short_form_;
  }

  virtual void reset() {
    has_value_ = false;
  }

 protected:
  const char* short_form_;
  const char* long_form_;
  const char* name_;
  const char* help_;
  bool has_value_;
};

template <typename T>
struct ValuePolicy;

template <typename ValueType>
class Option : public IOption
{
  typedef ValuePolicy<ValueType> T;

 public:
  Option(Parser& parser,
         const char* short_form, const char* long_form,
         const Maybe<ValueType>& default_value,
         const char* help)
   : IOption(short_form, long_form, nullptr, help),
     default_value_(default_value)
  {
    if (default_value_)
      value_ = *default_value_;
    parser.add(this);
  }

  Option(Parser& parser, const char* name, const char* help)
   : IOption(nullptr, nullptr, name, help)
  {
    parser.add(this);
  }

  bool hasValue() const {
    return has_value_ || default_value_;
  }
  bool hasUserValue() const {
    return has_value_;
  }
  const ValueType& value() const {
    assert(hasValue());
    return value_;
  }
  Maybe<ValueType> maybeValue() const {
    if (hasValue())
      return Some(value_);
    return Nothing();
  }

 protected:
  bool canOmitValue() const override {
    return !name_ && T::canOmitValue();
  }
  bool consumeValue(const char* arg) override {
    if (!T::consumeValue(arg, &value_))
      return false;
    has_value_ = true;
    return true;
  }
  void reset() override {
    IOption::reset();
    if (default_value_)
      value_ = *default_value_;
  }

 private:
  Maybe<ValueType> default_value_;
  ValueType value_;
};

// This is an option that flips a boolean (the default value, if none is given
// in the constructor, will be false -> true).
class ToggleOption : public IOption
{
 public:
  ToggleOption(Parser& parser,
               const char* short_form, const char* long_form,
               const Maybe<bool>& default_value,
               const char* help)
   : IOption(short_form, long_form, nullptr, help)
  {
    if (default_value)
      default_value_ = *default_value;
    else
      default_value_ = false;
    parser.add(this);
  }

  bool value() const {
    if (has_value_)
      return value_;
    return default_value_;
  }
  bool hasValue() const {
    return true;
  }
  bool hasUserValue() const {
    return has_value_;
  }

 protected:
  virtual bool canOmitValue() const override {
    return true;
  }
  bool isToggle() const override {
    return true;
  }
  bool consumeValue(const char* arg) override {
    if (arg)
      return false;
    value_ = !default_value_;
    has_value_ = true;
    return true;
  }

 private:
  bool default_value_;
  bool value_;
};

// This is a ToggleOption that will stop argument processing and force
// parse_args to return true. It is the responsibility of the implementor
// to check all StopOptions before any other options, as a true return
// from parse_args() may have only filled one of them.
class StopOption : public ToggleOption
{
 public:
  StopOption(Parser& parser,
               const char* short_form, const char* long_form,
               const Maybe<bool>& default_value,
               const char* help)
   : ToggleOption(parser, short_form, long_form, default_value, help)
  {
  }

 protected:
  bool haltOnValue() const override {
    return true;
  }
};

// This is the base class for an option that can be repeated multiple times,
// or can have its value repeated multiple times. (The latter is not yet
// implemented).
template <typename ValueType>
class VectorOption : public IOption
{
  typedef ValuePolicy<ValueType> T;

 public:
  VectorOption(Parser& parser,
               const char* short_form, const char* long_form,
               const char* help)
   : IOption(short_form, long_form, nullptr, help)
  {
    parser.add(this);
  }

  Vector<ValueType>& values() {
    return values_;
  }
  const Vector<ValueType>& values() const {
    return values_;
  }

 protected:
  bool repeatable() const override {
    return true;
  }
  bool consumeValue(const char* arg) override {
    ValueType value;
    if (!T::consumeValue(arg, &value))
      return false;
    has_value_ = true;
    values_.append(value);
    return true;
  }
  void reset() override {
    IOption::reset();
    values_.clear();
  }

 private:
  Vector<ValueType> values_;
};

// This is for an option can be repeated multiple times.
template <typename Type>
class RepeatOption : public VectorOption<Type>
{
 public:
  RepeatOption(Parser& parser,
               const char* short_form, const char* long_form,
               const char* help)
   : VectorOption<Type>(parser, short_form, long_form, help)
  {}
};

// Value policies should inherit from this.
struct BaseValuePolicy
{ 
  // Return true if an argument of this type can omit a value.
  static constexpr bool canOmitValue() {
    return false;
  }
};

template <>
struct ValuePolicy<bool>
{
  static constexpr bool canOmitValue() {
    return true;
  }

  static bool consumeValue(const char* arg, bool* out) {
    if (!arg) {
      *out = true;
      return true;
    }
    if (StrCaseCmp(arg, "true") == 0) {
      *out = true;
      return true;
    }
    if (StrCaseCmp(arg, "false") == 0) {
      *out = false;
      return true;
    }
    return false;
  }
};

template <>
struct ValuePolicy<AString> : public BaseValuePolicy
{
  static bool consumeValue(const char* arg, ke::AString* out) {
    *out = arg;
    return true;
  }
};

template <>
struct ValuePolicy<int> : public BaseValuePolicy
{
  static bool consumeValue(const char* arg, int* out) {
    char* endptr = nullptr;
    *out = strtol(arg, &endptr, 10);
    if (endptr == arg || *endptr != '\0')
      return false;
    return true;
  }
};

typedef Option<bool> BoolOption;
typedef Option<AString> StringOption;
typedef Option<int> IntOption;

inline
Parser::Parser(const char* help)
 : help_(help)
{
  Option<bool>* help_option = new Option<bool>(
    *this,
    "h", "help",
    Nothing(),
    "Display this help menu.");
  help_option_ = UniquePtr<IOption>(help_option);
}

inline void
Parser::add(IOption* option)
{
  if (option->short_form() || option->long_form())
    options_.append(option);
  else
    positionals_.append(option);
}

inline bool
Parser::parse(int argc, char** argv)
{
  Vector<const char*> args;
  for (int i = 1; i < argc; i++)
    args.append(argv[i]);
  return parse_impl(args);
}

inline bool
Parser::parse(const Vector<const char*>& args)
{
  return parse_impl(args);
}

inline bool
Parser::parse(const Vector<AString>& args)
{
  Vector<const char*> ptr_args;
  for (size_t i = 0; i < args.length(); i++)
    ptr_args.append(args[i].chars());
  return parse_impl(ptr_args);
}

inline bool
Parser::parsev(const char* arg1, ...)
{
  va_list ap;
  va_start(ap, arg1);

  Vector<const char*> argv;
  const char* arg = arg1;
  while (arg) {
    argv.append(arg);
    arg = va_arg(ap, const char*);
  }

  return parse_impl(argv);
}

inline bool
Parser::parse_impl(const Vector<const char*>& args)
{
  size_t positional = 0;

  for (size_t i = 0; i < args.length(); i++) {
    const char* arg = args[i];
    IOption* option = nullptr;

    const char* value_ptr = nullptr;
    if (arg[0] == '-') {
      // Short form arguments can look like:
      //   -key
      //   -key val
      //   -key=val
      //   --key
      //   --key val
      //   --key=val
      size_t len = strlen(arg);
      const char* equals = strchr(arg, '=');
      if (equals)
        len = equals - arg;

      if (arg[1] == '-')
        option = find_long(&arg[2], len - 2);
      else
        option = find_short(&arg[1], len - 1);

      if (!option)
        return unrecognized_option(arg);
      if (option->has_value() && !option->repeatable())
        return option_already_specified(option);

      if (equals) {
        if (option->isToggle())
          return option_invalid_value(option, equals + 1);
        value_ptr = equals + 1;
      }
    } else {
      // Positional arguments have an implicit key - the next argument in
      // position.
      if (positional >= positionals_.length())
        return unrecognized_extra();

      option = positionals_[positional++];
      value_ptr = arg;
    }

    bool took_next_arg = false;
    if (!value_ptr) {
      if (i == args.length() - 1) {
        if (!option->canOmitValue())
          return option_needs_value(option);
      } else if (!option->isToggle()) {
        value_ptr = args[++i];
        took_next_arg = true;
      }
    }

    if (!option->consumeValue(value_ptr)) {
      if (!option->canOmitValue())
        return option_invalid_value(option, value_ptr);
      if (took_next_arg)
        i--;
    }

    if (option->haltOnValue())
      return true;
  }

  if (positional < positionals_.length())
    return missing_positional(positionals_[positional]);

  // Force the app to display a help message.
  if (help_option_ && help_option_->has_value())
    return false;

  return true;
}

inline IOption*
Parser::find_short(const char* short_form, size_t len)
{
  for (IOption* option : options_) {
    if (!option->short_form())
      continue;
    if (strlen(option->short_form()) == len &&
        strncmp(option->short_form(), short_form, len) == 0)
    {
      return option;
    }
  }
  return nullptr;
}

inline IOption*
Parser::find_long(const char* long_form, size_t len)
{
  for (IOption* option : options_) {
    if (!option->long_form())
      continue;
    if (strlen(option->long_form()) == len &&
        strncmp(option->long_form(), long_form, len) == 0)
    {
      return option;
    }
  }
  return nullptr;
}

inline bool
Parser::unrecognized_option(const char* arg)
{
  error_.format("Unrecognized option: %s", arg);
  return false;
}

inline bool
Parser::unrecognized_extra()
{
  error_ = "Unrecognized extra arguments at end of command.";
  return false;
}

inline bool
Parser::option_needs_value(IOption* option)
{
  error_.format("Option '%s' needs a value.", option->pretty_name());
  return false;
}

inline bool
Parser::option_invalid_value(IOption* option, const char* value)
{
  error_.format(
    "Argument '%s' did not recognize value: %s",
    option->pretty_name(), value);
  return false;
}

inline bool
Parser::missing_positional(IOption* option)
{
  error_.format("Missing value for '%s'.",
    option->pretty_name());
  return false;
}

inline bool
Parser::option_already_specified(IOption* option)
{
  error_.format("Option '%s' was specified more than once.",
    option->pretty_name());
  return false;
}

inline void
Parser::usage(FILE* fp, int argc, char** argv)
{
  bool display_full_help = false;

  if (error_.length()) {
    // Scan to see if we got a help request.
    for (int i = 1; i < argc; i++) {
      if (strcmp(argv[i], "-h") == 0 ||
          strcmp(argv[i], "--help") == 0)
      {
        display_full_help = true;
        break;
      }
    }

    if (!help_option_ || !display_full_help) {
      usage_line(argv, fp);
      fprintf(fp, "error: %s\n", error_.chars());
      return;
    }

    // Otherwise, discard the error, and show the full help menu.
  }

  fprintf(fp, "%s\n", help_);
  fprintf(fp, "\n");

  usage_line(argv, fp);

  struct Entry {
    explicit Entry(IOption* option)
     : col_width(0),
       option(option)
    {}
    Entry(Entry&& other)
     : opt_lines(Move(other.opt_lines)),
       help_lines(Move(other.help_lines)),
       col_width(other.col_width),
       option(other.option)
    {}
    Vector<AString> opt_lines;
    Vector<AString> help_lines;
    size_t col_width;
    IOption* option;
  };

  static const size_t kIndent = 2;
  UniquePtr<char[]> indent = MakeUnique<char[]>(kIndent + 1);
  for (size_t i = 0; i < kIndent; i++)
    indent[i] = ' ';
  indent[kIndent] = '\0';

  // Add positional arguments first.
  Vector<Entry> entries;
  for (IOption* option : positionals_) {
    Entry entry(option);

    AString name;
    name.format("%s%s", indent.get(), option->name());
    entry.opt_lines.append(Move(name));
    entries.append(Move(entry));
  }

  static const size_t kMaxLineLength = 80;
  static const size_t kMaxLeftColLength = 28;
  static const size_t kMaxRightColLength = kMaxLineLength - kMaxLeftColLength;

  // Next add long and short form arguments.
  for (IOption* option : options_) {
    Entry entry(option);

    AString value_suffix;
    if (!option->isToggle() && !option->canOmitValue()) {
      const char* name = option->long_form()
                         ? option->long_form()
                         : option->short_form();
      AString decorated = Join(Split(name, "-"), "_").uppercase();
      value_suffix.format("=%s", decorated.chars());
    }

    if (option->short_form() && option->long_form()) {
      AString joined;
      joined.format("%s-%s, --%s%s ",
        indent.get(),
        option->short_form(),
        option->long_form(),
        value_suffix.chars());

      // Try to fit both options in one cell.
      if (joined.length() <= kMaxLeftColLength) {
        entry.opt_lines.append(Move(joined));
        entries.append(Move(entry));
        continue;
      }
    }

    // We need to put each form on a separate line.
    if (option->short_form()) {
      AString short_form;
      short_form.format("%s-%s ", indent.get(), option->short_form());
      entry.opt_lines.append(Move(short_form));
    }
    if (option->long_form()) {
      AString long_form;
      long_form.format("%s--%s%s ",
        indent.get(),
        option->long_form(),
        value_suffix.chars());
      entry.opt_lines.append(Move(long_form));
    }
    entries.append(Move(entry));
  }

  // If we don't have any arguments to display, stop here.
  if (entries.empty())
    return;

  // Precompute some information about each option.
  for (Entry& entry : entries) {
    // Compute the maximum size of each left hand cell.
    for (const AString& line : entry.opt_lines)
      entry.col_width = ke::Max(entry.col_width, line.length());

    // Break help text into words, and fit some words on each line. Each line
    // gets at least one word even if the word is too long.
    Vector<AString> words = Split(entry.option->help(), " ");

    Vector<AString> line;
    size_t line_length = 0;
    for (AString& word : words) {
      if (line_length + word.length() + 1 >= kMaxRightColLength &&
          !line.empty())
      {
        entry.help_lines.append(Join(line, " "));
        line.clear();
        line_length = 0;
      }
      line_length += word.length() + 1;
      line.append(Move(word));
    }
    if (!line.empty())
      entry.help_lines.append(Join(line, " "));
  }

  // Finally, print detailed usage. When possible we put both the option and
  // its help on one line, but if the left-hand cell is too big we use
  // separate lines.
  fprintf(fp, "\n");
  if (!positionals_.empty())
    fprintf(fp, "positional arguments:\n");

  bool started_optional = false;
  for (const Entry& entry : entries) {
    if (!entry.option->name() && !started_optional) {
      fprintf(fp, "\n");
      fprintf(fp, "optional arguments:\n");
      started_optional = true;
    }

    size_t help_line_cursor = 0;
    if (entry.col_width >= kMaxLeftColLength) {
      for (const AString& line : entry.opt_lines)
        fprintf(fp, "%s\n", line.chars());
    } else {
      for (const AString& line : entry.opt_lines) {
        const char* right_col = (help_line_cursor < entry.help_lines.length())
                                ? entry.help_lines[help_line_cursor++].chars()
                                : "";
        fprintf(fp, "%-28s%s\n", line.chars(), right_col);
      }
    }

    for (size_t i = help_line_cursor; i < entry.help_lines.length(); i++)
      fprintf(fp, "%28s\n", entry.help_lines[i].chars());
  }
}

void
Parser::usage_line(char** argv, FILE* out)
{
  Vector<AString> words;

  words.append("Usage:");
  words.append(argv ? argv[0] : " ");
  words.append("[-h]");
  for (IOption* option : positionals_)
    words.append(option->name());

  AString text = Join(words, " ");
  fprintf(out, "%s\n", text.chars());
}

inline void
Parser::reset()
{
  for (IOption* option : options_)
    option->reset();
  for (IOption* option : positionals_)
    option->reset();
  if (help_option_)
    help_option_->reset();
}

} // namespace args
} // namespace ke

#endif // _include_amtl_am_argparser_h_
