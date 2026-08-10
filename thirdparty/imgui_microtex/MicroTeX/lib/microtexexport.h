#ifndef MICROTEX_MICROTEXEXPORT_H
#define MICROTEX_MICROTEXEXPORT_H

#ifdef _MSC_VER
#include "vcruntime.h"
#endif

// msvc
#ifdef _MSC_VER
//[ADAPT_IMGUI_BUNDLE]
//  Add a third branch for static-library builds. Without this, MSVC
//  consumers of microtex headers see __declspec(dllimport) and emit
//  __imp_* references that cannot be resolved against the static
//  microtex.lib (which only exports plain symbols). Defining
//  MICROTEX_STATIC tells the headers to expand MICROTEX_EXPORT to
//  nothing, so consumers reference the plain symbols directly.
#   if defined(MICROTEX_STATIC)
#       define MICROTEX_EXPORT
#   elif defined(MICROTEX_LIBRARY)
//[/ADAPT_IMGUI_BUNDLE]
#       define MICROTEX_EXPORT __declspec(dllexport)
#   else
#       define MICROTEX_EXPORT __declspec(dllimport)
#   endif
// gnuc and clang
#elif defined(__GNUC__) || defined(__GNUG__) || defined(__clang__)
#   define MICROTEX_EXPORT __attribute((visibility("default")))
// otherwise...
#else
#   define MICROTEX_EXPORT
#endif

// emscripten
#if defined(__EMSCRIPTEN__)
#   include <emscripten.h>
#   define MICROTEX_CAPI EMSCRIPTEN_KEEPALIVE
#else
#   define MICROTEX_CAPI MICROTEX_EXPORT
#endif

#endif //MICROTEX_MICROTEXEXPORT_H
