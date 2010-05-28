/**
 * stCExcept - a C exception mechanism used by SonTrace.
 *
 * This provides a simple setjmp based exception mechanism for C.  All errors
 * in the Kobol C API are reported via exceptions.  An exception object,
 * containing both a symbolic and natural language error description is used
 * to represent errors.  Exception chaining is supported.
 *
 * The usage idiom for catching errors is:
 * \code
 *  stTry {
 *      // user code here
 *  } stCatch(except) {
 *      // user exception handling here
 *      stCExcept_free(except);
 *  } stTryEnd;
 * \endcode
 *
 * To return within the try block, \code stTryReturn(value) \endcode must
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
 *  stTry {
 *      cnt++;
 *      if (cnt > 10) {
 *         stTryReturn(cnt);
 *      }
 *  } stCatch(except) {
 *      stCExcept_free(except);
 *      return -cnt;
 *  } stTryEnd;
 *  return cnt;
 * \endcode
 * @defgroup CExceptions Exceptions for C
 */

#ifndef CExcept_h
#define CExcept_h
#include <assert.h>
#include <setjmp.h>
#include <stdarg.h>
//@{
/// @defgroup CExcept class CExcept
/// @ingroup CExceptions
//@{

/**
 * Exception object that is thrown.
 * @ingroup stCExcept
 */
struct stCExcept {
    const char *id;           /**< symbolic exception id */
    const char *msg;          /**< natural-language error message */
    struct stCExcept *cause;  /**< error stack, NULL if no causing errors */
};

/**
 * Construct a new stCExcept object.
 * 
 * @param id symbolic exception id.  This should be a constant, static string.
 * @param msg print style format used to generate errMsg
 * @param args arguments to format into errMsg
 * @ingroup stCExcept
 */
struct stCExcept *stCExcept_newv(const char *id, const char *msg, va_list args);

/**
 * Construct a new stCExcept object.
 * 
 * @param id symbolic exception id.  This should be a constant, static string.
 * @param msg print style format used to generate errMsg
 * @param ... arguments to format into errMsg
 * @ingroup  stCExcept
 */
struct stCExcept *stCExcept_new(const char *id, const char *msg, ...);

/**
 * Construct a new stCExcept object, setting cause.
 * 
 * @param cause causing error cause, ownership passed to the new object.
 * @param id symbolic exception id.  This should be a constant, static string.
 * @param msg print style format used to generate errMsg
 * @param args arguments to format into errMsg
 * @ingroup  stCExcept
 */
struct stCExcept *stCExcept_newCausev(struct stCExcept *cause, const char *id, const char *msg, va_list args);

/**
 * Construct a new stCExcept object, setting cause.
 * 
 * @param cause causing error cause, ownership passed to the new object.
 * @param id symbolic exception id.  This should be a constant, static string.
 * @param msg print style format used to generate errMsg
 * @param ... arguments to format into errMsg
 * @ingroup  stCExcept
 */
struct stCExcept *stCExcept_newCause(struct stCExcept *cause, const char *id, const char *msg, ...);

/**
 * Free an stCExcept object.  Frees assocated causes as well.
 * @ingroup  stCExcept
 */
void stCExcept_free(struct stCExcept *except);
//@}

/*
 * Context allocated on the stack for an exception.
 * (Internal structure, don't use directly)
 */
struct _stCExceptContext {
    jmp_buf env;                   // setjmp environment 
    struct _stCExceptContext *prev;  // previous context on the stack.
    struct stCExcept *except;        // exception thrown into this context
};

/* 
 * Exception content Top Of Stack.
 * (Internal structure, don't use directly)
 */
extern struct _stCExceptContext *_cexceptTOS;

/// @defgroup CMacros C try/catch macros
/// @ingroup stCExceptions
//@{

/**
 * Begin a try block.
 */
#define stTry {\
    struct _stCExceptContext _cexceptContext;\
    _cexceptContext.prev = _cexceptTOS;\
    _cexceptTOS = &_cexceptContext;\
    _cexceptContext.except = NULL;\
    if (setjmp(_cexceptContext.env) == 0)
    
/**
 * Catch following a try block.
 */
#define stCatch(exceptVar) \
    if (_cexceptTOS->except != NULL) {\
        struct stCExcept *exceptVar = _cexceptTOS->except;\
        _cexceptTOS = _cexceptTOS->prev;

/**
 * End of try/cache block
 */
#define stTryEnd \
    } else {\
        _cexceptTOS = _cexceptTOS->prev;\
    }}

/**
 * Return a value inside of a ceTry.
 */
#define stTryReturn(val) {\
    _cexceptTOS = _cexceptTOS->prev;\
    return val;\
    }

/**
 * Raise an exception.  This function is not inlined to allow setting
 * breakpoints.
 * @param except object describing the exception.  Ownership of object
 *  is passed to exception mechanism.
 */
void stThrow(struct stCExcept *except);

/**
 * Construct and raise an exception.
 * @param id symbolic exception id.  This should be a constant, static string.
 * @param msg print style format used to generate errMsg
 * @param ... arguments to format into errMsg
 */
static inline void stThrowNew(const char *id, const char *msg, ...) {
    va_list args;
    va_start(args, msg);
    struct stCExcept *except = stCExcept_newv(id, msg, args);
    va_end(args);
    stThrow(except);
}

/**
 * Construct and raise an exception, setting cause.
 * @param cause causing error cause, ownership passed to the new object.
 * @param id symbolic exception id.  This should be a constant, static string.
 * @param msg print style format used to generate errMsg
 * @param ... arguments to format into errMsg
 */
static inline void stThrowNewCause(struct stCExcept *cause, const char *id, const char *msg, ...) {
    va_list args;
    va_start(args, msg);
    struct stCExcept *except = stCExcept_newCausev(cause, id, msg, args);
    va_end(args);
    stThrow(except);
}
//@}
//@}
#endif
