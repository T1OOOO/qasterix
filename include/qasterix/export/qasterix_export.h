
#ifndef QASTERIX_EXPORT_H
#define QASTERIX_EXPORT_H

#ifdef QASTERIX_STATIC_DEFINE
#  define QASTERIX_EXPORT
#  define QASTERIX_NO_EXPORT
#else
#  ifndef QASTERIX_EXPORT
#    ifdef qasterix_EXPORTS
        /* We are building this library */
#      define QASTERIX_EXPORT 
#    else
        /* We are using this library */
#      define QASTERIX_EXPORT 
#    endif
#  endif

#  ifndef QASTERIX_NO_EXPORT
#    define QASTERIX_NO_EXPORT 
#  endif
#endif

#ifndef QASTERIX_DEPRECATED
#  define QASTERIX_DEPRECATED __attribute__ ((__deprecated__))
#endif

#ifndef QASTERIX_DEPRECATED_EXPORT
#  define QASTERIX_DEPRECATED_EXPORT QASTERIX_EXPORT QASTERIX_DEPRECATED
#endif

#ifndef QASTERIX_DEPRECATED_NO_EXPORT
#  define QASTERIX_DEPRECATED_NO_EXPORT QASTERIX_NO_EXPORT QASTERIX_DEPRECATED
#endif

#if 0 /* DEFINE_NO_DEPRECATED */
#  ifndef QASTERIX_NO_DEPRECATED
#    define QASTERIX_NO_DEPRECATED
#  endif
#endif

#endif /* QASTERIX_EXPORT_H */
