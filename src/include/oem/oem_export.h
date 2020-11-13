
#ifndef OEM_EXPORT_H
#define OEM_EXPORT_H

#ifdef OEM_STATIC_DEFINE
#  define OEM_EXPORT
#  define OEM_NO_EXPORT
#else
#  ifndef OEM_EXPORT
#    ifdef oem_EXPORTS
        /* We are building this library */
#      define OEM_EXPORT __attribute__((visibility("default")))
#    else
        /* We are using this library */
#      define OEM_EXPORT __attribute__((visibility("default")))
#    endif
#  endif

#  ifndef OEM_NO_EXPORT
#    define OEM_NO_EXPORT __attribute__((visibility("hidden")))
#  endif
#endif

#ifndef OEM_DEPRECATED
#  define OEM_DEPRECATED 
#endif

#ifndef OEM_DEPRECATED_EXPORT
#  define OEM_DEPRECATED_EXPORT OEM_EXPORT OEM_DEPRECATED
#endif

#ifndef OEM_DEPRECATED_NO_EXPORT
#  define OEM_DEPRECATED_NO_EXPORT OEM_NO_EXPORT OEM_DEPRECATED
#endif

#if 0 /* DEFINE_NO_DEPRECATED */
#  ifndef OEM_NO_DEPRECATED
#    define OEM_NO_DEPRECATED
#  endif
#endif

#endif /* OEM_EXPORT_H */
