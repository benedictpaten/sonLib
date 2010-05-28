#include "stCExcept.h"
#include <stdlib.h>
#include "stSafeC.h"

/*
 * Exception content Top Of Stack.
 * FIXME: should be in thread-local storage.
 */
struct _stCExceptContext *_cexceptTOS = NULL;

/* Construct a new stCExcept object. */
struct stCExcept *stCExcept_newv(const char *id, const char *msg, va_list args) {
    struct stCExcept *except = stSafeCCalloc(sizeof(struct stCExcept));
    except->id = id;
    except->msg = stSafeCDynFmtv(msg, args);
    return except;
}

/* Construct a new stCExcept object. */
struct stCExcept *stCExcept_new(const char *id, const char *msg, ...) {
    va_list args;
    va_start(args, msg);
    struct stCExcept *except = stCExcept_newv(id, msg, args);
    va_end(args);
    return except;
}

/* Construct a new stCExcept object, setting cause. */
struct stCExcept *stCExcept_newCausev(struct stCExcept *cause, const char *id, const char *msg, va_list args) {
    struct stCExcept *except = stCExcept_newv(id, msg, args);
    except->cause = cause;
    return except;
}

/* Construct a new stCExcept object, setting cause. */
struct stCExcept *stCExcept_newCause(struct stCExcept *cause, const char *id, const char *msg, ...) {
    va_list args;
    va_start(args, msg);
    struct stCExcept *except = stCExcept_newCausev(cause, id, msg, args);
    va_end(args);
    return except;
}

/* Free an stCExcept object. */
void stCExcept_free(struct stCExcept *except) {
    if (except != NULL) {
        if (except->cause != NULL) {
            stCExcept_free(except->cause);
        }
        stSafeCFree((char*)except->msg);
        stSafeCFree(except);
    }
}

/* Raise an exception. */
void ceThrow(struct stCExcept *except) {
    assert(except != NULL);
    if (_cexceptTOS == NULL) {
        stSafeCErr("Exception: %s: %s", except->id, except->msg);
    }
    _cexceptTOS->except = except;
    longjmp(_cexceptTOS->env, 1);
}

