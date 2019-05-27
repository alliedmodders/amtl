The AMTL style is based on the Google style, with some differences:
 - Indents are 4 spaces instead of 2.
 - A brace is always placed onto a new line if the previous parenthetical
   was multi-line.
 - Functions in the first column always have their return type and name on
   separate lines.
 - A function can only be a one-liner if the entire function declaration fits
   on one line, and it is not in the first column.
 - Control structures cannot be one-liners, but braces are optional if the
   body fits on a single line.
 - A space is placed after the "operator" keyword.

clang-format does not support these style changes, so for restyling existing
code, a simple clang-based C++ restyler tool is provided.

References:
 - https://github.com/eliben/llvm-clang-samples
 - https://github.com/nasa/trick/tree/master/trick\_source/codegen/Interface\_Code\_Gen
