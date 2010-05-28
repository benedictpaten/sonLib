#include "kobol/src/c/CExcept.h"
#include <stdlib.h>
#include "kobol/src/c/safec.h"

/* 
 * Exception content Top Of Stack.
 * FIXME: should be in thread-local storage.
 */
struct _CExceptContext *_cexceptTOS = NULL;

/* Construct a new CExcept object. */
struct CExcept *CExcept_newv(const char *id, const char *msg, va_list args) {
    struct CExcept *except = safeCalloc(sizeof(struct CExcept));
    except->id = id;
    except->msg = safeDynFmtv(msg, args);
    return except;
}

/* Construct a new CExcept object. */
struct CExcept *CExcept_new(const char *id, const char *msg, ...) {
    va_list args;
    va_start(args, msg);
    struct CExcept *except = CExcept_newv(id, msg, args);
    va_end(args);
    return except;
}

/* Construct a new CExcept object, setting cause. */
struct CExcept *CExcept_newCausev(struct CExcept *cause, const char *id, const char *msg, va_list args) {
    struct CExcept *except = CExcept_newv(id, msg, args);
    except->cause = cause;
    return except;
}

/* Construct a new CExcept object, setting cause. */
struct CExcept *CExcept_newCause(struct CExcept *cause, const char *id, const char *msg, ...) {
    va_list args;
    va_start(args, msg);
    struct CExcept *except = CExcept_newCausev(cause, id, msg, args);
    va_end(args);
    return except;
}

/* Free an CExcept object. */
void CExcept_free(struct CExcept *except) {
    if (except != NULL) {
        if (except->cause != NULL) {
            CExcept_free(except->cause);
        }
        safeFree((char*)except->msg);
        safeFree(except);
    }
}

/* Raise an exception. */
void ceThrow(struct CExcept *except) {
    assert(except != NULL);
    if (_cexceptTOS == NULL) {
        safeErr("Exception: %s: %s", except->id, except->msg);
    }
    _cexceptTOS->except = except;
    longjmp(_cexceptTOS->env, 1);
}

