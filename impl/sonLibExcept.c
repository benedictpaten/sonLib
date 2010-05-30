#include "sonLibExcept.h"
#include <stdlib.h>
#include "stSafeC.h"

/**
 * Exception object that is thrown.
 * @ingroup stExcept
 */
struct stExcept {
    const char *id;          /**< symbolic exception id */
    const char *msg;         /**< natural-language error message */
    struct stExcept *cause;  /**< error stack, NULL if no causing errors */
};

/*
 * Exception content Top Of Stack.
 * FIXME: should be in thread-local storage.
 */
struct _stExceptContext *_cexceptTOS = NULL;

/* Construct a new stExcept object. */
stExcept *stExcept_newv(const char *id, const char *msg, va_list args) {
    stExcept *except = stSafeCCalloc(sizeof(stExcept));
    except->id = id;
    except->msg = stSafeCDynFmtv(msg, args);
    return except;
}

/* Construct a new stExcept object. */
stExcept *stExcept_new(const char *id, const char *msg, ...) {
    va_list args;
    va_start(args, msg);
    stExcept *except = stExcept_newv(id, msg, args);
    va_end(args);
    return except;
}

/* Construct a new stExcept object, setting cause. */
stExcept *stExcept_newCausev(stExcept *cause, const char *id, const char *msg, va_list args) {
    stExcept *except = stExcept_newv(id, msg, args);
    except->cause = cause;
    return except;
}

/* Construct a new stExcept object, setting cause. */
stExcept *stExcept_newCause(stExcept *cause, const char *id, const char *msg, ...) {
    va_list args;
    va_start(args, msg);
    stExcept *except = stExcept_newCausev(cause, id, msg, args);
    va_end(args);
    return except;
}

/* Free an stExcept object. */
void stExcept_free(stExcept *except) {
    if (except != NULL) {
        if (except->cause != NULL) {
            stExcept_free(except->cause);
        }
        stSafeCFree((char*)except->msg);
        stSafeCFree(except);
    }
}

/** Get the id for a stExcept. */
const char* stExcept_getId(const stExcept *except) {
    return except->id;
}

/** Get the message for a stExcept. */
const char* stExcept_getMsg(const stExcept *except) {
    return except->msg;
}

/** Get the cause for a stExcept. */
stExcept *stExcept_getCause(const stExcept *except) {
    return except->cause;
}

/* Raise an exception. */
void stThrow(stExcept *except) {
    assert(except != NULL);
    if (_cexceptTOS == NULL) {
        stSafeCErr("Exception: %s: %s", except->id, except->msg);
    }
    _cexceptTOS->except = except;
    longjmp(_cexceptTOS->env, 1);
}

/** Construct and raise an exception. */
void stThrowNew(const char *id, const char *msg, ...) {
    va_list args;
    va_start(args, msg);
    stExcept *except = stExcept_newv(id, msg, args);
    va_end(args);
    stThrow(except);
}

/** Construct and raise an exception, setting cause. */
void stThrowNewCause(stExcept *cause, const char *id, const char *msg, ...) {
    va_list args;
    va_start(args, msg);
    stExcept *except = stExcept_newCausev(cause, id, msg, args);
    va_end(args);
    stThrow(except);
}

