// vim: set sts=8 ts=4 sw=4 tw=99 et:
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

#include <assert.h>
#include <stdio.h>

#include <algorithm>
#include <memory>
#include <string>

#include <amtl/am-cxx.h>
#include <amtl/am-function.h>
#include <amtl/am-maybe.h>
#include <amtl/am-string.h>
#include <amtl/am-vector.h>

namespace ke {
namespace args {

class IOption;
class StopOption;

class Parser
{
    template <typename T>
    friend class Option;

  public:
    inline Parser();
    explicit inline Parser(const char* help);

    inline void add(IOption* option);
    inline bool parse(int argc, char** argv);
    inline bool parse(const std::vector<const char*>& args);
    inline bool parse(const std::vector<std::string>& args);
    inline bool parsev(const char* arg1, ...);

    inline void usage(FILE* fp, int argc, char** argv);
    inline void usage_line(char** argv, FILE* out);

    inline void add_usage_line(const char* option, const char* help);

    inline void reset();

    // Enable the following patterns:
    //    -s:N
    //    -sN
    // This is for compatibility with spcomp. It only works when the short
    // form is one character.
    void enable_inline_values() {
        inline_values_ = true;
    }

    // Allow /Option instead of -Option.
    void allow_slashes() {
        allow_slashes_ = true;
    }

    // Collect values that are not options (i.e. do not start with a dash or
    // slash), and are not positionals.
    void collect_extra_args() {
        collect_extra_args_ = true;
    }

    const std::vector<std::string>& extra_args() const {
        return extra_args_;
    }

    void set_usage_line(const char* usage) {
        usage_line_ = usage;
    }

    static void add_static_option(IOption* option) {
        if (!sStaticOptions)
            sStaticOptions = new std::vector<IOption*>();
        sStaticOptions->emplace_back(option);
    }

  private:
    inline bool parse_impl(const std::vector<const char*>& args);

    inline IOption* find_short(const char* short_form, size_t len);
    inline IOption* find_long(const char* long_form, size_t len);
    inline IOption* find_inline_short(char c);

    inline bool unrecognized_option(const char* arg);
    inline bool unrecognized_extra();
    inline bool option_needs_value(IOption* option);
    inline bool option_invalid_value(IOption* option, const char* value);
    inline bool missing_positional(IOption* option);
    inline bool option_already_specified(IOption* option);

    static std::vector<IOption*>* sStaticOptions;

  private:
    std::vector<IOption*> options_;
    std::vector<IOption*> positionals_;
    std::string help_;
    std::string error_;
    std::string usage_line_;
    bool inline_values_ = false;
    bool allow_slashes_ = false;
    bool collect_extra_args_ = false;
    std::unique_ptr<StopOption> help_option_;
    std::vector<std::string> extra_args_;
    std::vector<std::unique_ptr<IOption>> extra_usage_;
};

class IOption
{
    friend class Parser;

    virtual bool mustOmitValue() const {
        return false;
    }
    virtual bool canOmitValue() const {
        return false;
    }
    virtual bool haltOnValue() const {
        return false;
    }
    virtual bool canPrefixWithNo() const {
        return false;
    }
    virtual bool consumeValue(const char* arg) = 0;
    virtual void reset() {}
    virtual bool willRepeat() const {
        return false;
    }

  public:
    virtual ~IOption() {
    }

  protected:
    IOption(const char* short_form, const char* long_form, const char* name, const char* help)
     : short_form_(short_form),
       long_form_(long_form),
       name_(name),
       help_(help)
    {
        // Cannot have both short/longform and name.
        assert(((short_form || long_form) && !name_) || (!short_form && !long_form && name_));

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

    const char* pretty_name() const {
        if (name_)
            return name_;
        if (long_form_)
            return long_form_;
        return short_form_;
    }

  protected:
    const char* short_form_;
    const char* long_form_;
    const char* name_;
    const char* help_;
};

class UsageOnlyOption final : public IOption
{
  public:
    UsageOnlyOption(const char* option, const char* help)
      : IOption(nullptr, nullptr, option, help)
    {}

    bool consumeValue(const char* arg) override {
        return false;
    }
};

template <typename T>
struct ValuePolicy;

template <typename ValueType>
class Option : public IOption
{
    typedef ValuePolicy<ValueType> T;

  public:
    Option(Parser& parser, const char* short_form, const char* long_form,
           const Maybe<ValueType>& default_value, const char* help)
     : IOption(short_form, long_form, nullptr, help),
       default_value_(default_value)
    {
        parser.add(this);
    }

    Option(Parser& parser, const char* name, const char* help)
     : IOption(nullptr, nullptr, name, help)
    {
        parser.add(this);
    }

    Option(const char* short_form, const char* long_form,
           const Maybe<ValueType>& default_value, const char* help)
     : IOption(short_form, long_form, nullptr, help),
       default_value_(default_value)
    {
        Parser::add_static_option(this);
    }

    Option(const char* name, const char* help)
     : IOption(nullptr, nullptr, name, help)
    {
        Parser::add_static_option(this);
    }

    ~Option()
    {}

    bool hasValue() const {
        return value_.isValid() || default_value_.isValid();
    }
    const ValueType& value() const {
        return value_.isValid() ? value_.get() : default_value_.get();
    }
    Maybe<ValueType> maybeValue() const {
        if (value_.isValid())
            return value_;
        return default_value_;
    }
    bool willRepeat() const override {
        return !value_.isValid();
    }

  protected:
    bool canOmitValue() const override {
        return !name_ && T::canOmitValue();
    }
    bool consumeValue(const char* arg) override {
        ValueType value;
        if (!T::consumeValue(arg, &value))
            return false;
        value_ = Some(value);
        return true;
    }
    void reset() override {
        value_ = {};
    }

  private:
    Maybe<ValueType> default_value_;
    Maybe<ValueType> value_;
};

// This is an option that returns true if specified. If prefixed with "--no-",
// it returns false.
class EnableOption : public IOption
{
  public:
    EnableOption(Parser& parser, const char* short_form, const char* long_form, bool default_value,
                 const char* help)
      : IOption(short_form, long_form, nullptr, help),
        default_value_(default_value)
    {
        parser.add(this);
    }

    EnableOption(const char* short_form, const char* long_form, bool default_value,
                 const char* help)
      : IOption(short_form, long_form, nullptr, help),
        default_value_(default_value)
    {
        Parser::add_static_option(this);
    }

    bool value() const {
        return value_.isValid() ? value_.get() : default_value_;
    }
    bool consumeValue(const char* arg) override {
        value_ = Some(strcmp(arg, "true") == 0);
        return true;
    }
    void reset() override {
        value_ = {};
    }
    bool willRepeat() const override {
        return !value_.isValid();
    }

  protected:
    bool mustOmitValue() const override {
        return true;
    }
    bool canPrefixWithNo() const override {
        return true;
    }

  private:
    bool default_value_;
    Maybe<bool> value_;
};

// This is an option that flips a boolean (the default value, if none is given
// in the constructor, will be false -> true).
class ToggleOption : public IOption
{
    friend class Parser;

  public:
    ToggleOption(Parser& parser, const char* short_form, const char* long_form,
                 const Maybe<bool>& default_value, const char* help)
      : IOption(short_form, long_form, nullptr, help)
    {
        if (default_value.isValid())
            default_value_ = default_value.get();
        parser.add(this);
    }

    ToggleOption(const char* short_form, const char* long_form,
                 const Maybe<bool>& default_value, const char* help)
      : IOption(short_form, long_form, nullptr, help)
    {
        if (default_value.isValid())
            default_value_ = default_value.get();
        Parser::add_static_option(this);
    }

    bool value() const {
        return value_.isValid() ? value_.get() : default_value_;
    }
    bool willRepeat() const override {
        return !value_.isValid();
    }

  protected:
    virtual bool canOmitValue() const override {
        return true;
    }
    bool mustOmitValue() const override {
        return true;
    }
    bool consumeValue(const char* arg) override {
        if (arg)
            return false;
        value_ = Some(!default_value_);
        return true;
    }
    void reset() override {
        value_ = {};
    }

  private:
    bool default_value_ = false;
    Maybe<bool> value_;
};

// This is a ToggleOption that will stop argument processing and force
// parse_args to return true. It is the responsibility of the implementor
// to check all StopOptions before any other options, as a true return
// from parse_args() may have only filled one of them.
class StopOption : public ToggleOption
{
  public:
    StopOption(Parser& parser, const char* short_form, const char* long_form,
               const Maybe<bool>& default_value, const char* help)
     : ToggleOption(parser, short_form, long_form, default_value, help)
    {}

    StopOption(const char* short_form, const char* long_form,
               const Maybe<bool>& default_value, const char* help)
     : ToggleOption(short_form, long_form, default_value, help)
    {}

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
    VectorOption(Parser& parser, const char* short_form, const char* long_form, const char* help)
     : IOption(short_form, long_form, nullptr, help)
    {
        parser.add(this);
    }

    VectorOption(const char* short_form, const char* long_form, const char* help)
     : IOption(short_form, long_form, nullptr, help)
    {
        Parser::add_static_option(this);
    }

    std::vector<ValueType>& values() {
        return values_;
    }
    const std::vector<ValueType>& values() const {
        return values_;
    }

  protected:
    bool consumeValue(const char* arg) override {
        ValueType value;
        if (!T::consumeValue(arg, &value))
            return false;
        values_.emplace_back(value);
        return true;
    }
    void reset() override {
        values_.clear();
    }
    bool willRepeat() const override {
        return true;
    }

  protected:
    std::vector<ValueType> values_;
};

// This is for an option can be repeated multiple times.
template <typename Type>
class RepeatOption : public VectorOption<Type>
{
  public:
    RepeatOption(Parser& parser, const char* short_form, const char* long_form, const char* help)
     : VectorOption<Type>(parser, short_form, long_form, help)
    {}

    RepeatOption(const char* short_form, const char* long_form, const char* help)
     : VectorOption<Type>(short_form, long_form, help)
    {}
};

// Value policies should inherit from this.
struct BaseValuePolicy {
    // Return true if an argument of this type can omit a value.
    static constexpr bool canOmitValue() {
        return false;
    }
};

template <>
struct ValuePolicy<bool> {
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
struct ValuePolicy<std::string> : public BaseValuePolicy {
    static bool consumeValue(const char* arg, std::string* out) {
        if (strlen(arg) == 0)
            return false;
        *out = arg;
        return true;
    }
};

template <>
struct ValuePolicy<int> : public BaseValuePolicy {
    static bool consumeValue(const char* arg, int* out) {
        char* endptr = nullptr;
        *out = strtol(arg, &endptr, 10);
        if (endptr == arg || *endptr != '\0')
            return false;
        return true;
    }
};

typedef Option<std::string> StringOption;
typedef Option<int> IntOption;

inline Parser::Parser()
{
    if (sStaticOptions) {
        for (const auto& option : *sStaticOptions)
            add(option);
    }
}

inline Parser::Parser(const char* help)
{
    if (help)
        help_ = help;

    help_option_ = std::make_unique<StopOption>(*this, "h", "help", Some(false), "Display this help menu.");

    if (sStaticOptions) {
        for (const auto& option : *sStaticOptions)
            add(option);
    }
}

inline void
Parser::add(IOption* option)
{
    if (option->short_form() || option->long_form())
        options_.emplace_back(option);
    else
        positionals_.emplace_back(option);
}

inline bool
Parser::parse(int argc, char** argv)
{
    std::vector<const char*> args;
    for (int i = 1; i < argc; i++)
        args.emplace_back(argv[i]);
    return parse_impl(args);
}

inline bool
Parser::parse(const std::vector<const char*>& args)
{
    return parse_impl(args);
}

inline bool
Parser::parse(const std::vector<std::string>& args)
{
    std::vector<const char*> ptr_args;
    for (size_t i = 0; i < args.size(); i++)
        ptr_args.emplace_back(args[i].c_str());
    return parse_impl(ptr_args);
}

inline bool
Parser::parsev(const char* arg1, ...)
{
    va_list ap;
    va_start(ap, arg1);

    std::vector<const char*> argv;
    const char* arg = arg1;
    while (arg) {
        argv.emplace_back(arg);
        arg = va_arg(ap, const char*);
    }

    return parse_impl(argv);
}

inline bool
Parser::parse_impl(const std::vector<const char*>& args)
{
    size_t positional = 0;
    bool halted_on_value = false;

    for (size_t i = 0; i < args.size(); i++) {
        const char* arg = args[i];
        IOption* option = nullptr;

        const char* value_ptr = nullptr;
        if (arg[0] == '-' || (allow_slashes_ && arg[0] == '/')) {
            // Arguments can look like:
            //   -key
            //   -key val
            //   -key=val
            //   --key
            //   --key val
            //   --key=val
            // Or with inline values enabled,
            //   -keyval
            //   -key:val
            size_t len = strlen(arg);
            const char* sep = strchr(arg, '=');
            if (!sep && inline_values_)
                sep = strchr(arg, ':');
            if (inline_values_ && sep && strlen(sep + 1) == 0)
                sep = nullptr;    // Don't allow -key= or -key: with no arg.
            if (sep)
                len = sep - arg;

            if (arg[0] == '-' && arg[1] == '-') {
                option = find_long(&arg[2], len - 2);
            } else {
                option = find_short(&arg[1], len - 1);
                if (!option && inline_values_) {
                    option = find_inline_short(arg[1]);
                    if (option && len > 2)
                        sep = arg + 1;
                }
            }

            if (!option && strncmp(arg, "--no-", 5) == 0) {
                option = find_long(&arg[5], len - 5);
                if (option->canPrefixWithNo())
                    value_ptr = "false";
                else
                    option = nullptr;
            } else if (option && option->canPrefixWithNo()) {
                value_ptr = "true";
            }

            if (!option)
                return unrecognized_option(arg);
            if (!option->willRepeat())
                return option_already_specified(option);

            if (sep) {
                if (option->mustOmitValue())
                    return option_invalid_value(option, sep + 1);
                value_ptr = sep + 1;
            }
        } else {
            // Positional arguments have an implicit key - the next argument in
            // position.
            if (positional >= positionals_.size()) {
                if (!collect_extra_args_)
                    return unrecognized_extra();

                extra_args_.emplace_back(arg);
                continue;
            }

            option = positionals_[positional++];
            value_ptr = arg;
        }

        bool took_next_arg = false;
        if (!value_ptr) {
            if (i == args.size() - 1) {
                if (!option->canOmitValue())
                    return option_needs_value(option);
            } else if (!option->mustOmitValue()) {
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

        if (option->haltOnValue()) {
            halted_on_value = true;
            break;
        }
    }

    if (!halted_on_value && positional < positionals_.size())
        return missing_positional(positionals_[positional]);

    // Force the app to display a help message.
    if (help_option_ && help_option_->value())
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
            strncmp(option->short_form(), short_form, len) == 0) {
            return option;
        }
    }
    return nullptr;
}

inline IOption*
Parser::find_inline_short(char c)
{
    for (IOption* option : options_) {
        if (!option->short_form())
            continue;
        if (strlen(option->short_form()) == 1 && option->short_form()[0] == c)
            return option;
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
            strncmp(option->long_form(), long_form, len) == 0) {
            return option;
        }
    }
    return nullptr;
}

inline bool
Parser::unrecognized_option(const char* arg)
{
    error_ = StringPrintf("Unrecognized option: %s", arg);
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
    error_ = StringPrintf("Option '%s' needs a value.", option->pretty_name());
    return false;
}

inline bool
Parser::option_invalid_value(IOption* option, const char* value)
{
    error_ = StringPrintf("Argument '%s' did not recognize value: %s", option->pretty_name(), value);
    return false;
}

inline bool
Parser::missing_positional(IOption* option)
{
    error_ = StringPrintf("Missing value for '%s'.", option->pretty_name());
    return false;
}

inline bool
Parser::option_already_specified(IOption* option)
{
    error_ = StringPrintf("Option '%s' was specified more than once.", option->pretty_name());
    return false;
}

inline void
Parser::usage(FILE* fp, int argc, char** argv)
{
    bool display_full_help = false;

    if (error_.size()) {
        // Scan to see if we got a help request.
        for (int i = 1; i < argc; i++) {
            if (strcmp(argv[i], "-h") == 0 || strcmp(argv[i], "--help") == 0) {
                display_full_help = true;
                break;
            }
        }

        if (!help_option_ || !display_full_help) {
            usage_line(argv, fp);
            fprintf(fp, "error: %s\n", error_.c_str());
            return;
        }

        // Otherwise, discard the error, and show the full help menu.
    }

    if (!help_.empty()) {
        fprintf(fp, "%s\n", help_.c_str());
        fprintf(fp, "\n");
    }

    usage_line(argv, fp);

    struct Entry {
        explicit Entry(IOption* option)
          : col_width(0),
            option(option)
        {}
        Entry(Entry&& other)
          : opt_lines(std::move(other.opt_lines)),
            help_lines(std::move(other.help_lines)),
            col_width(other.col_width),
            option(other.option)
        {}
        Entry& operator =(Entry&& other) {
            opt_lines = std::move(other.opt_lines);
            help_lines = std::move(other.help_lines);
            col_width = other.col_width;
            option = other.option;
            return *this;
        }
        std::vector<std::string> opt_lines;
        std::vector<std::string> help_lines;
        size_t col_width;
        IOption* option;
    };

    static const size_t kIndent = 2;
    std::unique_ptr<char[]> indent = std::make_unique<char[]>(kIndent + 1);
    for (size_t i = 0; i < kIndent; i++)
        indent[i] = ' ';
    indent[kIndent] = '\0';

    // Add positional arguments first.
    std::vector<Entry> entries;
    for (IOption* option : positionals_) {
        Entry entry(option);

        std::string name = StringPrintf("%s%s", indent.get(), option->name());
        entry.opt_lines.emplace_back(std::move(name));
        entries.emplace_back(std::move(entry));
    }

    static const size_t kMaxLineLength = 80;
    static const size_t kMaxLeftColLength = 28;
    static const size_t kMaxRightColLength = kMaxLineLength - kMaxLeftColLength;

    // Next add long and short form arguments.
    for (IOption* option : options_) {
        Entry entry(option);

        std::string value_suffix;
        if (!option->mustOmitValue() && !option->canOmitValue()) {
            const char* name = option->long_form() ? option->long_form() : option->short_form();
            std::string decorated = Uppercase(Join(Split(name, "-"), "_").c_str());
            value_suffix = StringPrintf("=%s", decorated.c_str());
        }

        if (option->short_form() && option->long_form()) {
            std::string joined = StringPrintf("%s-%s, --%s%s ", indent.get(), option->short_form(),
                                              option->long_form(), value_suffix.c_str());

            // Try to fit both options in one cell.
            if (joined.size() <= kMaxLeftColLength) {
                entry.opt_lines.emplace_back(std::move(joined));
                entries.emplace_back(std::move(entry));
                continue;
            }
        }

        // We need to put each form on a separate line.
        if (option->short_form()) {
            std::string short_form = StringPrintf("%s-%s ", indent.get(), option->short_form());
            entry.opt_lines.emplace_back(std::move(short_form));
        }
        if (option->long_form()) {
            std::string long_form = StringPrintf("%s--%s%s ", indent.get(), option->long_form(), value_suffix.c_str());
            entry.opt_lines.emplace_back(std::move(long_form));
        }
        entries.emplace_back(std::move(entry));
    }

    // Add extra usage lines.
    for (const auto& option : extra_usage_) {
        Entry entry(option.get());

        std::string name = StringPrintf("%s%s", indent.get(), option->name());
        entry.opt_lines.emplace_back(std::move(name));
        entries.emplace_back(std::move(entry));
    }

    // If we don't have any arguments to display, stop here.
    if (entries.empty())
        return;

    // Precompute some information about each option.
    for (Entry& entry : entries) {
        // Compute the maximum size of each left hand cell.
        for (const std::string& line : entry.opt_lines)
            entry.col_width = std::max(entry.col_width, line.size());

        // Break help text into words, and fit some words on each line. Each line
        // gets at least one word even if the word is too long.
        std::vector<std::string> words = Split(entry.option->help(), " ");

        std::vector<std::string> line;
        size_t line_size = 0;
        for (std::string& word : words) {
            if (line_size + word.size() + 1 >= kMaxRightColLength && !line.empty()) {
                entry.help_lines.emplace_back(Join(line, " "));
                line.clear();
                line_size = 0;
            }
            line_size += word.size() + 1;
            line.emplace_back(std::move(word));
        }
        if (!line.empty())
            entry.help_lines.emplace_back(Join(line, " "));
    }

    // Finally, print detailed usage. When possible we put both the option and
    // its help on one line, but if the left-hand cell is too big we use
    // separate lines.
    if (!positionals_.empty()) {
        fprintf(fp, "\n");
        fprintf(fp, "positional arguments:\n");
    }

    char spaces[29];
    memset(spaces, ' ', sizeof(spaces) - 1);
    spaces[sizeof(spaces) - 1] = '\0';

    bool started_optional = false;
    for (const Entry& entry : entries) {
        if (!entry.option->name() && !started_optional) {
            fprintf(fp, "\n");
            fprintf(fp, "optional arguments:\n");
            started_optional = true;
        }

        size_t help_line_cursor = 0;
        if (entry.col_width >= kMaxLeftColLength) {
            for (const std::string& line : entry.opt_lines)
                fprintf(fp, "%s\n", line.c_str());
        } else {
            for (const std::string& line : entry.opt_lines) {
                const char* right_col = (help_line_cursor < entry.help_lines.size())
                                            ? entry.help_lines[help_line_cursor++].c_str()
                                            : "";
                fprintf(fp, "%-28s%s\n", line.c_str(), right_col);
            }
        }

        for (size_t i = help_line_cursor; i < entry.help_lines.size(); i++)
            fprintf(fp, "%s%s\n", spaces, entry.help_lines[i].c_str());
    }
}

void
Parser::usage_line(char** argv, FILE* out)
{
    std::vector<std::string> words;

    words.emplace_back("Usage:");
    words.emplace_back(argv ? argv[0] : " ");
    if (usage_line_.empty()) {
        words.emplace_back("[-h]");
        for (IOption* option : positionals_)
            words.emplace_back(option->name());
    } else {
        words.emplace_back(usage_line_);
    }

    std::string text = Join(words, " ");
    fprintf(out, "%s\n", text.c_str());
}

void
Parser::add_usage_line(const char* option, const char* help)
{
    extra_usage_.emplace_back(std::make_unique<UsageOnlyOption>(option, help));
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
