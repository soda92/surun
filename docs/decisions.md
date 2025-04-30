# Decisions

- always use debug builds for simplified debugging,
and, no create so much intermidiate directories that's hard to clean.

- always build a python package around C++ files.
that makes you see the structure, and simplify it;
that means, you will create directories for scripts, docs, and tests.

- Do not use cmake.

- Visual Studio for code editing & language server, VSCode for data analysis, Github Desktop for code commit.

- Don't try to fix any of the project build warnings, except for some very basic ones.

- only do absolute-necessary fix/improvements:
    - update icon
    - fix desktop background

- things don't consider:
    - font size? -hard to test, no fix!
    - icon size in GUI? -no fix!
    - these problems can be fixed once I learned WinUI
