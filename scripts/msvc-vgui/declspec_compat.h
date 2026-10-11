/* MinGW headers test #ifdef __declspec. On the MSVC target it is a keyword,
   so the headers take the branch that emits __declspec(...) after a
   declarator, which clang rejects. Turn it into an attribute. */
#define __declspec(x) __attribute__((x))
