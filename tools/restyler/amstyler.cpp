//------------------------------------------------------------------------------
// Clang rewriter sample. Demonstrates:
//
// * How to use RecursiveASTVisitor to find interesting AST nodes.
// * How to use the Rewriter API to rewrite the source code.
//
// Eli Bendersky (eliben@gmail.com)
// This code is in the public domain
//------------------------------------------------------------------------------
#include <fcntl.h>
#include <unistd.h>

#include <algorithm>
#include <cstdio>
#include <deque>
#include <fstream>
#include <iostream>
#include <memory>
#include <sstream>
#include <string>
#include <unordered_set>

#include "clang/AST/ASTConsumer.h"
#include "clang/AST/RecursiveASTVisitor.h"
#include "clang/Basic/Diagnostic.h"
#include "clang/Basic/FileManager.h"
#include "clang/Basic/SourceManager.h"
#include "clang/Basic/TargetInfo.h"
#include "clang/Basic/TargetOptions.h"
#include "clang/Frontend/CompilerInstance.h"
#include "clang/Lex/Preprocessor.h"
#include "clang/Parse/ParseAST.h"
#include "clang/Lex/PreprocessorOptions.h"
#include "clang/Rewrite/Core/Rewriter.h"
#include "clang/Rewrite/Frontend/Rewriters.h"
#include "llvm/Support/Host.h"
#include "llvm/Support/raw_ostream.h"

using namespace clang;

llvm::cl::list<std::string> include_dirs("I", llvm::cl::Prefix,
                                         llvm::cl::desc("Include directory"),
                                         llvm::cl::value_desc("directory")); 
llvm::cl::opt<bool> modify_in_place("p", llvm::cl::desc("Modify in-place"), llvm::cl::init(false));
llvm::cl::list<std::string> input_files(llvm::cl::Positional, llvm::cl::desc("<input files>"),
                                        llvm::cl::OneOrMore);

class TextRewriter final
{
public:
  TextRewriter(SourceManager& src_mgr)
   : src_mgr_(src_mgr)
  {
    auto file_id = src_mgr_.getMainFileID();
    auto buffer = src_mgr_.getBufferData(file_id);
    text_ = std::string(buffer.data(), buffer.size());
  }

  void replace(const SourceLocation& begin, const SourceLocation& end, const std::string& text) {
    size_t start_pos = offset(begin);
    size_t end_pos = offset(end);
    ops_.push_back(Operation{
      .kind = OpKind::Replace,
      .offset = start_pos,
      .length = end_pos - start_pos,
      .text = text,
    });
  }

  void remove(const SourceLocation& begin, const SourceLocation& end) {
    size_t start_pos = offset(begin);
    size_t end_pos = offset(end);
    ops_.push_back(Operation{
      .kind = OpKind::Remove,
      .offset = start_pos,
      .length = end_pos - start_pos,
    });
  }

  void insertAt(const SourceLocation& at, const std::string& text) {
    ops_.push_back(Operation{
      .kind = OpKind::InsertAt,
      .offset = offset(at),
      .length = 0,
      .text = text,
    });
  }

  void insertAfter(const SourceLocation& at, const std::string& text) {
    ops_.push_back(Operation{
      .kind = OpKind::InsertAfter,
      .offset = offset(at),
      .length = 0,
      .text = text,
    });
  }

  std::string emit() {
    std::stringstream output;

    std::sort(ops_.begin(), ops_.end(), [](const Operation& a, const Operation& b) -> bool {
      return a.offset < b.offset;
    });

    size_t ignore_until = std::string::npos;
    for (size_t i = 0; i < text_.size(); i++) {
      if (ignore_until != std::string::npos) {
        if (i < ignore_until)
          continue;
        ignore_until = std::string::npos;
      }

      if (!(ops_.empty() || ops_.front().offset >= i))
        std::cout << output.str() << std::endl;
      assert(ops_.empty() || ops_.front().offset >= i);
      if (ops_.empty() || ops_.front().offset != i) {
        output << text_[i];
        continue;
      }

      Operation op = std::move(ops_.front());
      ops_.pop_front();
      switch (op.kind) {
        case OpKind::Replace:
          output << op.text;
          ignore_until = i + op.length;
          break;
        case OpKind::Remove:
          ignore_until = i + op.length;
          break;
        case OpKind::InsertAt:
          output << op.text << text_[i];
          break;
        case OpKind::InsertAfter:
          output << text_[i] << op.text;
          break;
        default:
          assert(false);
          break;
      }
    }

    assert(ops_.empty());
    return output.str();
  }

  SourceManager& getSourceMgr() const {
    return src_mgr_;
  }

private:
  size_t offset(const SourceLocation& loc) {
    auto file_id = src_mgr_.getFileID(loc);
    StringRef buffer_start = src_mgr_.getBufferData(file_id);
    const char* pos = src_mgr_.getCharacterData(loc);
    return pos - buffer_start.data();
  }

private:
  enum class OpKind {
    Replace,
    Remove,
    InsertAt,
    InsertAfter
  };

  struct Operation {
    OpKind kind;
    size_t offset;
    size_t length;
    std::string text;
  };

private:
  SourceManager& src_mgr_;
  std::string text_;
  std::deque<Operation> ops_;
};

// By implementing RecursiveASTVisitor, we can specify which AST nodes
// we're interested in by overriding relevant methods.
class MyASTVisitor : public RecursiveASTVisitor<MyASTVisitor> {
public:
  MyASTVisitor(CompilerInstance& ci, TextRewriter& R)
    : rewriter_(R),
      src_mgr_(ci.getSourceManager()),
      context_(ci.getASTContext()),
      pp_(ci.getPreprocessor())
  {}

  // Anything followed by a parenthetical should split the brace onto a newline
  // if the parenthetical is multi-line.
  bool VisitIfStmt(IfStmt* s) {
    return HandleCondStmt(s, s->getThen());
  }
  bool VisitWhileStmt(WhileStmt* s) {
    return HandleCondStmt(s, s->getBody());
  }
  bool VisitSwitchStmt(SwitchStmt* s) {
    return HandleCondStmt(s, s->getBody());
  }
  bool VisitForStmt(ForStmt* s) {
    if (!isMainFile(s->getLocStart()))
      return true;
    if (!isa<CompoundStmt>(s->getBody()))
      return true;

    return maybeSplitBrace(s->getLocStart(), s->getRParenLoc(), s->getBody());
  }
  bool VisitCXXCatchStmt(CXXCatchStmt* s) {
    if (!isMainFile(s->getLocStart()))
      return true;
    if (!isa<CompoundStmt>(s->getHandlerBlock()))
      return true;

    SourceLocation rparenLoc = getRParenLoc(s->getExceptionDecl()->getLocEnd());
    assert(rparenLoc.isValid());
    return maybeSplitBrace(s->getLocStart(), rparenLoc, s->getHandlerBlock());
  }

  template <typename T>
  bool HandleCondStmt(T* s, Stmt* body) {
    if (!isMainFile(s->getLocStart()))
      return true;
    if (!isa<CompoundStmt>(body))
      return true;

    SourceLocation rparenLoc = getRParenLoc(s);
    if (rparenLoc.isInvalid())
      rparenLoc = getRParenLocBackwards(s->getLocStart(), body->getLocStart());
    if (rparenLoc.isInvalid())
      return true;
    return maybeSplitBrace(s->getLocStart(), rparenLoc, body);
  }

  bool VisitFunctionDecl(FunctionDecl *f) {
    if (!isMainFile(f->getLocStart()))
      return true;

    auto raw_loc = f->getLocStart().getRawEncoding();
    if (visited_.count(raw_loc))
      return true;

    if (f->isOverloadedOperator())
      maybeFixOperatorKeyword(f);

    // Everything hereafter needs a declaration, not definition.
    if (!f->hasBody())
      return true;

    if (auto ctor = dyn_cast<CXXConstructorDecl>(f))
      maybeFixCtorInitializers(ctor);

    auto typeSourceInfo = f->getTypeSourceInfo();
    auto typeLoc = typeSourceInfo->getTypeLoc().getAs<FunctionTypeLoc>();
    auto typeBeginLoc = typeLoc.getLocStart();
    auto typeEndLoc = typeLoc.getLocEnd();

    if (!typeBeginLoc.isValid() || !typeEndLoc.isValid()) {
      // Not clear why this happens, but it does.
      return true;
    }

    // Mark as visited now that things look to be valid. If we failed earlier,
    // we might still get another invocation.
    visited_.emplace(raw_loc);

    Stmt *body = f->getBody();
    auto bodyBegin = body->getLocStart();
    auto nameBegin = f->getNameInfo().getLoc();
    unsigned nameCol = getCol(nameBegin);
    unsigned typeCol = getCol(typeBeginLoc);
    bool is_first_col = (typeCol == 1 || nameCol == 1);

    if (auto ctor = dyn_cast<CXXConstructorDecl>(f)) {
      if (ctor->init_rend() != ctor->init_rbegin()) {
        // This transforms:
        //   A::A()
        //    : a_(...) {
        //
        // Into:
        //   A::A()
        //    : a_(...)
        //   {
        //
        // clang can insert implicit ctor initializers at an earlier line, so
        // find the last-appearing initializer.
        SourceLocation last_rparen = typeEndLoc;
        for (const auto& init : ctor->inits()) {
          auto rparen_loc = init->getRParenLoc();
          if (!rparen_loc.isValid())
            continue;
          if (getLine(last_rparen) < getLine(rparen_loc))
            last_rparen = rparen_loc;
        }

        maybeFixEmptyFunction(f, last_rparen, is_first_col);
        if (getLine(last_rparen) == getLine(bodyBegin) &&
            getLine(typeEndLoc) != getLine(bodyBegin))
        {
          splitBraceAfter(typeBeginLoc, last_rparen, body);
        }
      }
      return true;
    }

    bool cannot_split = maybeFixEmptyFunction(f, typeEndLoc, is_first_col);
    if (!cannot_split && (is_first_col || getLine(typeBeginLoc) != getLine(typeEndLoc))) {
      // This transforms:
      //   void a() {
      //
      // Into:
      //   void a()
      //   {
      //
      // If |a| is in the first column, or the prototype is multi-line.
      if (getLine(typeEndLoc) == getLine(bodyBegin)) {
        auto end_loc = getFirstWhitespaceBeforeLoc(typeEndLoc, body->getLocStart());
        splitBrace(typeBeginLoc, end_loc, body);
      }
    }

    return true;
  }

private:
  // Return true if the brace should not be split.
  bool maybeFixEmptyFunction(FunctionDecl* f, const SourceLocation& last_rparen, bool is_first_col) {
    // We never fix an empty function in the first column, since the brace
    // will be split anyway.
    if (is_first_col)
      return false;

    // Check if the body is empty.
    auto compound = dyn_cast<CompoundStmt>(f->getBody());
    if (!compound || !compound->body_empty())
      return false;

    // Don't accidentally erase comments in an empty block.
    auto lbrace_loc = compound->getLBracLoc();
    auto rbrace_loc = compound->getRBracLoc();
    if (!isEmptyBetween(lbrace_loc, rbrace_loc))
      return false;

    bool cannot_split = false;
    if (getLine(last_rparen) == getLine(lbrace_loc)) {
      // Transform this:
      //   void f() {
      //   }
      //
      // Into:
      //   void f() {}
      if (getLine(lbrace_loc) == getLine(rbrace_loc))
        return false;
      cannot_split = true;
    } else {
      // Transform this:
      //   void f()
      //   {
      //   }
      //
      // Into:
      //   void f()
      //   {}
    }
    rewriter_.remove(lbrace_loc.getLocWithOffset(1), rbrace_loc);
    return cannot_split;
  }

  void maybeFixCtorInitializers(CXXConstructorDecl* ctor) {
    if (ctor->getNumCtorInitializers() <= 1)
      return;

    // Transform this:
    //   A::A()
    //     : a_(...)
    //     , b_(...)
    //
    // Into this:
    //   A::A()
    //     : a_(...),
    //       b_(...)
    //
    // Unfortunately, clang's BCIS_BeforeColon option appears broken, and has no
    // effect on binpacked items. BeforeComma works, but this style is a bit
    // ridiculous. It's designed to make moving/adding lines easier, but almost
    // no other comma-delimited list uses a convention like this, so how it ever
    // became a thing a mystery. So, we use BeforeComma, and fix it up.
    for (const auto& init : ctor->inits()) {
      auto rparen_loc = init->getRParenLoc();
      if (!rparen_loc.isValid()) {
        llvm::errs() << "Invalid rparen loc in ctor.\n";
        return;
      }
      auto next_loc = getNextToken(rparen_loc.getLocWithOffset(1));
      if (getTokenKind(next_loc) != tok::comma)
        continue;
      if (getLine(rparen_loc) == getLine(next_loc))
        continue;
      rewriter_.insertAfter(init->getRParenLoc(), ",");
      rewriter_.replace(next_loc, next_loc.getLocWithOffset(1), " ");
    }
  }

  void maybeFixOperatorKeyword(FunctionDecl* f) {
    // Transform this:
    //   void operator=()
    //
    // Into:
    //   void operator =()
    auto name_info = f->getNameInfo();
    auto name_loc = name_info.getLoc();

    Token tok;
    if (Lexer::getRawToken(name_loc, tok, src_mgr_, context_.getLangOpts()))
      assert(false);
    if (tok.getKind() != tok::raw_identifier)
      return;
    auto info = pp_.LookUpIdentifierInfo(tok);
    if (info->getTokenID() != tok::kw_operator)
      return;

    auto end_loc = name_loc.getLocWithOffset(info->getLength());
    if (!isWhitespace(end_loc))
      rewriter_.insertAt(end_loc, " ");
  }

  bool splitBraceAfter(const SourceLocation& line, const SourceLocation& where, Stmt* body) {
    // This is the first space after the prototype or right-paren.
    auto space = where.getLocWithOffset(1);
    return splitBrace(line, space, body);
  }

  bool splitBrace(const SourceLocation& line, const SourceLocation& where, Stmt* body) {
    // This is the first space after the prototype or right-paren.
    std::string indent = "\n" + getIndentForLine(line);
    SourceRange toReplace(where, body->getLocStart());
    rewriter_.replace(where, body->getLocStart(), indent);
    return true;
  }

  bool maybeSplitBrace(const SourceLocation& line, const SourceLocation& rparenLoc, Stmt* body) {
    if (getLine(line) == getLine(rparenLoc))
      return true;

    auto lbraceLoc = body->getLocStart();
    if (getLine(rparenLoc) != getLine(lbraceLoc))
      return true;
    return splitBraceAfter(line, rparenLoc, body);
  }

  bool isEmptyBetween(const SourceLocation& start, const SourceLocation& end) {
    return skipWhitespace(skipToken(start)) == end;
  }

  template <typename CondStmt>
  SourceLocation getRParenLoc(CondStmt* stmt) {
    SourceLocation end_loc = stmt->getCond()->getLocEnd();
    if (auto cond_var = stmt->getConditionVariableDeclStmt())
      end_loc = cond_var->getLocEnd();
    if (end_loc.isInvalid())
      return end_loc;
    return getRParenLoc(end_loc);
  }

  SourceLocation getRParenLocBackwards(const SourceLocation& stop_at, const SourceLocation& brace_loc) {
    SourceLocation loc = brace_loc;
    while (loc != stop_at) {
      FullSourceLoc full_loc(loc, src_mgr_);
      char c = *full_loc.getCharacterData();
      if (c == ')')
        return loc;
      loc = loc.getLocWithOffset(-1);
    }
    return SourceLocation();
  }

  SourceLocation getRParenLoc(SourceLocation end_loc) {
    end_loc = skipToken(end_loc);
    end_loc = getNextToken(end_loc);

    if (getTokenKind(end_loc) == tok::hash) {
      // This is too complex to handle.
      return SourceLocation();
    }

    assert(getTokenKind(end_loc) == tok::r_paren);
    return end_loc;
  }

  SourceLocation getNextToken(SourceLocation loc) {
    for (;;) {
      loc = skipWhitespace(loc);
      if (getTokenKind(loc) != tok::comment)
        return loc;
      loc = skipToken(loc);
    }
  }

  // Given |start_loc| and |end_loc|, find the first location that starts a run
  // of whitespace tokens before |end_loc|. For example,
  //
  //    blah) noexcept {
  //        ^         ^^--end
  //   start          |
  //          return -+
  SourceLocation getFirstWhitespaceBeforeLoc(const SourceLocation& start_loc,
                                             const SourceLocation& end_loc) {
    assert(src_mgr_.getFileID(start_loc) == src_mgr_.getFileID(end_loc));

    SourceLocation candidate;
    SourceLocation loc = start_loc;
    unsigned end_offset = src_mgr_.getFileOffset(end_loc);
    for (;;) {
      if (isWhitespace(loc)) {
        candidate = loc;
        loc = skipWhitespace(loc);
        if (src_mgr_.getFileOffset(loc) == end_offset)
          return candidate;
        assert(src_mgr_.getFileOffset(loc) < end_offset);
      } else {
        loc = skipToken(loc);
        assert(src_mgr_.getFileOffset(loc) < end_offset);
      }
    }
  }

  SourceLocation skipToken(const SourceLocation& loc) {
    assert(!isWhitespace(loc));
    return Lexer::getLocForEndOfToken(loc, 0, src_mgr_, context_.getLangOpts());
  }

  tok::TokenKind getTokenKind(const SourceLocation& loc) {
    Token tok;
    if (Lexer::getRawToken(loc, tok, src_mgr_, context_.getLangOpts()))
      assert(false);
    return tok.getKind();
  }

  SourceLocation skipWhitespace(SourceLocation loc) {
    for (;;) {
      if (!isWhitespace(loc))
        return loc;
      loc = loc.getLocWithOffset(1);
    }
    return loc;
  }

  bool isWhitespace(const SourceLocation& loc) {
    FullSourceLoc full_loc(loc, src_mgr_);
    return clang::isWhitespace(*full_loc.getCharacterData());
  }

  unsigned getCol(const SourceLocation& loc) {
    bool invalid = false;
    unsigned col = src_mgr_.getSpellingColumnNumber(loc, &invalid);
    assert(!invalid);
    return col;
  }

  unsigned getLine(const SourceLocation& loc) {
    bool invalid = false;
    unsigned line = src_mgr_.getSpellingLineNumber(loc, &invalid);
    assert(!invalid);
    return line;
  }

  bool isMainFile(const SourceLocation& loc) {
    return src_mgr_.getMainFileID() == src_mgr_.getFileID(loc);
  }

  std::string getIndentForLine(const SourceLocation& loc) {
    auto file_id = src_mgr_.getFileID(loc);
    auto bufferStart = src_mgr_.getBufferData(file_id);
    const char* pos = src_mgr_.getCharacterData(loc);

    // Walk backwards to the newline.
    const char* newline_pos = pos;
    while (newline_pos >= bufferStart.data()) {
      if (*newline_pos == '\n')
        break;
      newline_pos--;
    }

    // Walk forwards to the first non-space token.
    const char* line_start = newline_pos + 1;
    const char* iter = line_start;
    while (isspace(*iter))
      iter++;
    return std::string(line_start, iter - line_start);
  }

private:
  TextRewriter& rewriter_;
  SourceManager& src_mgr_;
  ASTContext& context_;
  Preprocessor& pp_;
  std::unordered_set<unsigned> visited_;
};

// Implementation of the ASTConsumer interface for reading an AST produced
// by the Clang parser.
class MyASTConsumer : public ASTConsumer {
public:
  MyASTConsumer(CompilerInstance& ci, TextRewriter &R)
    : Visitor(ci, R)
  {}

  // Override the method that gets called for each parsed top-level
  // declaration.
  virtual bool HandleTopLevelDecl(DeclGroupRef DR) {
    for (DeclGroupRef::iterator b = DR.begin(), e = DR.end(); b != e; ++b) {
      // Traverse the declaration using our AST visitor.
      Visitor.TraverseDecl(*b);
    }
    return true;
  }

private:
  MyASTVisitor Visitor;
};

class ToolMain final
{
public:
  ToolMain();

  bool Run();

private:
  bool RunForPath(const std::string& path);
  std::vector<std::string> GetSearchPaths();

private:
  std::vector<std::string> search_paths_;
};

ToolMain::ToolMain()
{
  search_paths_ = GetSearchPaths();
  for (const auto& path : include_dirs)
    search_paths_.emplace_back(path);
}

bool
ToolMain::Run()
{
  bool ok = true;
  for (const auto& input_file : input_files)
    ok &= RunForPath(input_file);
  return ok;
}

bool
ToolMain::RunForPath(const std::string& input_file)
{
  CompilerInstance ci;
  ci.createDiagnostics();
  ci.createFileManager();
  ci.createSourceManager(ci.getFileManager());

  // Initialize target info with the default triple for our platform.
  auto target_options = std::make_shared<TargetOptions>();
  auto triple_string = llvm::sys::getDefaultTargetTriple();
  target_options->Triple = triple_string;
  TargetInfo *target_info = TargetInfo::CreateTargetInfo(ci.getDiagnostics(), target_options);
  ci.setTarget(target_info);

  auto& ppo = ci.getPreprocessorOpts();
  ppo.UsePredefines = true;

  auto& hso = ci.getHeaderSearchOpts();
  for (const auto& path : search_paths_)
    hso.AddPath(StringRef(path), frontend::IncludeDirGroup::Angled, false, false);

  // Set language defaults to C++17.
  auto& lo = ci.getLangOpts();
  InputKind ik(InputKind::CXX, InputKind::Source);
  llvm::Triple triple(triple_string);
  CompilerInvocation::setLangDefaults(lo, ik, triple, ppo, LangStandard::lang_cxx17);

  // AM doesn't use exceptions, but handle them anyway.
  lo.Exceptions = 1;
  lo.CXXExceptions = 1;
  lo.RTTI = 1;
  lo.DoubleSquareBracketAttributes = 1;

  // Note: this must occur after setting lang options and header search paths.
  ci.createPreprocessor(TU_Complete);

  auto& pp = ci.getPreprocessor();
  pp.getBuiltinInfo().initializeBuiltins(pp.getIdentifierTable(), pp.getLangOpts());

  ci.createASTContext();

  // Set the main file handled by the source manager to the input file.
  auto file_in = ci.getFileManager().getFile(input_file);
  if (!file_in) {
    access(input_file.c_str(), F_OK);
    std::cerr << input_file << ": " << strerror(errno) << std::endl;
    return false;
  }

  auto& src_mgr = ci.getSourceManager();
  src_mgr.setMainFileID(src_mgr.createFileID(file_in, SourceLocation(), SrcMgr::C_User));
  ci.getDiagnosticClient().BeginSourceFile(ci.getLangOpts(), &ci.getPreprocessor());

  // Create an AST consumer instance which is going to get called by ParseAST.
  TextRewriter tr(src_mgr);
  MyASTConsumer consumer(ci, tr);

  // Parse the file to AST, registering our consumer as the AST consumer.
  ParseAST(ci.getPreprocessor(), &consumer, ci.getASTContext());

  auto& de = ci.getDiagnostics();
  auto client = de.getClient();
  if (client->getNumErrors())
    return false;

  auto new_text = tr.emit();
  if (modify_in_place) {
    auto open_flags = std::ofstream::out | std::ofstream::trunc | std::ofstream::binary;
    std::ofstream output_file;
    output_file.open(input_file, open_flags);
    if (!output_file.is_open()) {
      access(input_file.c_str(), W_OK);
      std::cerr << "write " << input_file << ": " << strerror(errno) << std::endl;
      return false;
    }
    output_file << new_text;
  } else {
    llvm::outs() << new_text;
  }
  return true;
}

std::vector<std::string>
ToolMain::GetSearchPaths()
{
  std::string command = "clang++ -E -x c++ - -v < /dev/null 2>&1";
  std::unique_ptr<FILE, decltype(&pclose)> fp(popen(command.c_str(), "r"), pclose);

  std::string start_string = "#include <...> search starts here:";
  std::string end_string = "End of search list.";

  char buffer[1024];
  std::vector<std::string> paths;
  bool is_parsing_paths = false;
  while (fgets(buffer, sizeof(buffer), fp.get()) != nullptr) {
    std::string line = buffer;
    if (line.compare(0, start_string.size(), start_string) == 0) {
      is_parsing_paths = true;
      continue;
    }
    if (line.compare(0, end_string.size(), end_string) == 0) {
      if (is_parsing_paths)
        break;
    }
    if (!is_parsing_paths)
      continue;

    size_t cursor = 0;
    while (cursor < line.size() && isspace(line[cursor]))
      cursor++;
    line = line.substr(cursor);

    while (!line.empty() && isspace(line.back()))
      line.pop_back();
    if (line.empty())
      continue;
    paths.emplace_back(line);
  }
  return paths;
}

int main(int argc, char *argv[]) {
  llvm::cl::ParseCommandLineOptions(argc, argv);

  ToolMain tool;
  return tool.Run() ? 0 : 1;
}
