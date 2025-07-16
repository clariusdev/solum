
#ifndef SOLUM_EXPORT_H
#define SOLUM_EXPORT_H

#ifdef SOLUM_STATIC_DEFINE
#  define SOLUM_EXPORT
#  define SOLUM_NO_EXPORT
#else
#  ifndef SOLUM_EXPORT
#    ifdef solum_EXPORTS
        /* We are building this library */
#      define SOLUM_EXPORT __attribute__((visibility("default")))
#    else
        /* We are using this library */
#      define SOLUM_EXPORT __attribute__((visibility("default")))
#    endif
#  endif

#  ifndef SOLUM_NO_EXPORT
#    define SOLUM_NO_EXPORT __attribute__((visibility("hidden")))
#  endif
#endif

#ifndef SOLUM_DEPRECATED
#  define SOLUM_DEPRECATED __attribute__ ((__deprecated__))
#endif

#ifndef SOLUM_DEPRECATED_EXPORT
#  define SOLUM_DEPRECATED_EXPORT SOLUM_EXPORT SOLUM_DEPRECATED
#endif

#ifndef SOLUM_DEPRECATED_NO_EXPORT
#  define SOLUM_DEPRECATED_NO_EXPORT SOLUM_NO_EXPORT SOLUM_DEPRECATED
#endif

/* NOLINTNEXTLINE(readability-avoid-unconditional-preprocessor-if) */
#if 0 /* DEFINE_NO_DEPRECATED */
#  ifndef SOLUM_NO_DEPRECATED
#    define SOLUM_NO_DEPRECATED
#  endif
#endif

#endif /* SOLUM_EXPORT_H */
