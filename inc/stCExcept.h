/**
 * CExcept - a C exception mechanism used by Kobol.
 *
 * This provides a simple setjmp based exception mechanism for C.  All errors
 * in the Kobol C API are reported via exceptions.  An exception object,
 * containing both a symbolic and natural language error description is used
 * to represent errors.  Exception chaining is supported.
 *
 * The usage idiom for catching errors is:
 * \code
 *  ceTry {
 *      // user code here
 *  } ceCatch(except) {
 *      // user exception handling here
 *      CExcept_free(except);
 *  } ceEnd;
 * \endcode
 *
 * To return within the try block, \code ceTryReturn(value) \endcode must
 * be used.  There is no macro to return within the try block without
 * returning a value, as this is considered tacky. A standard
 * \code return \endcode can be used in the catch block.
 *
 * Any variable who's scope is outside of the try block and is modified
 * within the try block must be declared \code volatile \endcode.  For
 * example:
 * \code
 *  volatile int cnt = 0;
 *  ...
 *  ceTry {
 *      cnt++;
 *      if (cnt > 10) {
 *         ceTryReturn(cnt);
 *      }
 *  } ceCatch(except) {
 *      CExcept_free(except);
 *      return -cnt;
 *  } ceEnd;
 *  return cnt;
 * \endcode
 * @defgroup CExceptions Exceptions for C
 */

#ifndef CExcept_h
#define CExcept_h
#include "kobol/src/c/reconStd.h"
#include <assert.h>
#include <setjmp.h>
#include <stdarg.h>
inclBegin;
//@{

/// @defgroup CExcept class CExcept
/// @ingroup CExceptions
//@{

/**
 * Exception object that is thrown.
 * @ingroup  CExcept
 */
struct CExcept {
    const char *id;         /**< symbolic exception id */
    const char *msg;        /**< natural-language error message */
    struct CExcept *cause;  /**< error stack, NULL if no causing errors */
};

/**
 * Construct a new CExcept object.
 * 
 * @param id symbolic exception id.  This should be a constant, static string.
 * @param msg print style format used to generate errMsg
 * @param args arguments to format into errMsg
 * @ingroup  CExcept
 */
struct CExcept *CExcept_newv(const char *id, const char *msg, va_list args);

/**
 * Construct a new CExcept object.
 * 
 * @param id symbolic exception id.  This should be a constant, static string.
 * @param msg print style format used to generate errMsg
 * @param ... arguments to format into errMsg
 * @ingroup  CExcept
 */
struct CExcept *CExcept_new(const char *id, const char *msg, ...);

/**
 * Construct a new CExcept object, setting cause.
 * 
 * @param cause causing error cause, ownership passed to the new object.
 * @param id symbolic exception id.  This should be a constant, static string.
 * @param msg print style format used to generate errMsg
 * @param args arguments to format into errMsg
 * @ingroup  CExcept
 */
struct CExcept *CExcept_newCausev(struct CExcept *cause, const char *id, const char *msg, va_list args);

/**
 * Construct a new CExcept object, setting cause.
 * 
 * @param cause causing error cause, ownership passed to the new object.
 * @param id symbolic exception id.  This should be a constant, static string.
 * @param msg print style format used to generate errMsg
 * @param ... arguments to format into errMsg
 * @ingroup  CExcept
 */
struct CExcept *CExcept_newCause(struct CExcept *cause, const char *id, const char *msg, ...);

/**
 * Free an CExcept object.  Frees assocated causes as well.
 * @ingroup  CExcept
 */
void CExcept_free(struct CExcept *except);
//@}

/*
 * Context allocated on the stack for an exception.
 * (Internal structure, don't use directly)
 */
struct _CExceptContext {
    jmp_buf env;                   // setjmp environment 
    struct _CExceptContext *prev;  // previous context on the stack.
    struct CExcept *except;        // exception thrown into this context
};

/* 
 * Exception content Top Of Stack.
 * (Internal structure, don't use directly)
 */
extern struct _CExceptContext *_cexceptTOS;

/// @defgroup CMacros C try/catch macros
/// @ingroup CExceptions
//@{

/**
 * Begin a try block.
 */
#define ceTry {\
    struct _CExceptContext _cexceptContext;\
    _cexceptContext.prev = _cexceptTOS;\
    _cexceptTOS = &_cexceptContext;\
    _cexceptContext.except = NULL;\
    if (setjmp(_cexceptContext.env) == 0)
    
/**
 * Catch following a try block.
 */
#define ceCatch(exceptVar) \
    if (_cexceptTOS->except != NULL) {\
        struct CExcept *exceptVar = _cexceptTOS->except;\
        _cexceptTOS = _cexceptTOS->prev;

/**
 * End of try/cache block
 */
#define ceEnd \
    } else {\
        _cexceptTOS = _cexceptTOS->prev;\
    }}

/**
 * Return a value inside of a ceTry.
 */
#define ceTryReturn(val) {\
    _cexceptTOS = _cexceptTOS->prev;\
    return val;\
    }

/**
 * Raise an exception.  This function is not inlined to allow setting
 * breakpoints.
 * @param except object describing the exception.  Ownership of object
 *  is passed to exception mechanism.
 */
void ceThrow(struct CExcept *except);

/**
 * Construct and raise an exception.
 * @param id symbolic exception id.  This should be a constant, static string.
 * @param msg print style format used to generate errMsg
 * @param ... arguments to format into errMsg
 */
static inline void ceThrowNew(const char *id, const char *msg, ...) {
    va_list args;
    va_start(args, msg);
    struct CExcept *except = CExcept_newv(id, msg, args);
    va_end(args);
    ceThrow(except);
}

/**
 * Construct and raise an exception, setting cause.
 * @param cause causing error cause, ownership passed to the new object.
 * @param id symbolic exception id.  This should be a constant, static string.
 * @param msg print style format used to generate errMsg
 * @param ... arguments to format into errMsg
 */
static inline void ceThrowNewCause(struct CExcept *cause, const char *id, const char *msg, ...) {
    va_list args;
    va_start(args, msg);
    struct CExcept *except = CExcept_newCausev(cause, id, msg, args);
    va_end(args);
    ceThrow(except);
}
//@}
//@}
inclEnd;
#endif
