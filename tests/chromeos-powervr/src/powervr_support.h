//
// Created by Tommy Muir on 27/01/2025.
//

#ifndef POWERVR_SUPPORT_H
#define POWERVR_SUPPORT_H

#include <stdint.h>
#include <stdbool.h>
#include <pthread.h>
#include <assert.h>
#include <stddef.h>

#define INLINE inline
#define __packed __attribute__((__packed__))
#define RETAINED __attribute__((annotate("rc_ownership_retained")))
#define RETURNS_RETAINED __attribute__((annotate("rc_ownership_returns_retained")))

#include "dllist.h"

/* Get a structure's address from the address of a member */
#define IMG_CONTAINER_OF(ptr, type, member) \
(type *) (ptr); _Static_assert(offsetof(type, member) == 0)

/* Get a new pointer with an offset (in bytes) from a base address, useful
 * when traversing byte buffers and accessing data in buffers through struct
 * pointers.
 * Note, this macro is not equivalent to or replacing offsetof() */
#define IMG_OFFSET_ADDR(addr, offset_in_bytes) \
(void*)&(((IMG_UINT8*)(void*)(addr))[offset_in_bytes])

#define unlikely(x) (x)
#define likely(x) (x)

#define PVR_DPF(...)
#define PVR_ASSERT assert

typedef unsigned int IMG_UINT;
typedef int IMG_INT;

typedef uint8_t IMG_UINT8, *IMG_PUINT8;
typedef uint8_t IMG_BYTE, *IMG_PBYTE;
typedef int8_t IMG_INT8;
typedef char IMG_CHAR, *IMG_PCHAR;

typedef uint16_t IMG_UINT16, *IMG_PUINT16;
typedef int16_t IMG_INT16;
typedef uint32_t IMG_UINT32, *IMG_PUINT32;
typedef int32_t IMG_INT32, *IMG_PINT32;

typedef enum tag_img_bool {
  IMG_FALSE = 0,
  IMG_TRUE = 1,
  IMG_FORCE_ALIGN = 0x7FFFFFFF
} IMG_BOOL, *IMG_PBOOL;

/*!
 *****************************************************************************
 * Error values
 *****************************************************************************/
typedef enum PVRSRV_ERROR_TAG {
  PVRSRV_OK,
#define PVRE(x) x,
#include "pvrsrv_errors.h"
#undef PVRE
  PVRSRV_ERROR_FORCE_I32 = 0x7fffffff
} PVRSRV_ERROR;

typedef enum {
#define HANDLETYPE(x) PVRSRV_HANDLE_TYPE_##x,
#include "handle_types.h"
#undef HANDLETYPE
} PVRSRV_HANDLE_TYPE;

/*! Physical Memory Resource type.
 */
struct _PMR_ {
};

typedef struct _PMR_ PMR;

typedef void *IMG_HANDLE;

typedef enum {
  /* No flags */
  PVRSRV_HANDLE_ALLOC_FLAG_NONE = 0,
  /* Multiple handles can point at the given data pointer */
  PVRSRV_HANDLE_ALLOC_FLAG_MULTI = 0x01,
  /* Subhandles are allocated in a private handle space */
  PVRSRV_HANDLE_ALLOC_FLAG_PRIVATE = 0x02
} PVRSRV_HANDLE_ALLOC_FLAG;

struct _HANDLE_BASE_ {
};

typedef struct _HANDLE_BASE_ PVRSRV_HANDLE_BASE;

void LockHandle(PVRSRV_HANDLE_BASE *psBase) {
}

void UnlockHandle(PVRSRV_HANDLE_BASE *psBase) {
}

typedef struct _CONNECTION_DATA_ {
  PVRSRV_HANDLE_BASE *psHandleBase;
} CONNECTION_DATA;

typedef PVRSRV_ERROR (*PFN_HANDLE_RELEASE)(void *pvData);

PVRSRV_ERROR PVRSRVLookupHandleUnlocked(PVRSRV_HANDLE_BASE *psBase,
                                        void **ppvData,
                                        IMG_HANDLE hHandle,
                                        PVRSRV_HANDLE_TYPE eType,
                                        IMG_BOOL bRef) {
  return PVRSRV_OK;
}

PVRSRV_ERROR PVRSRVAllocHandleUnlocked(PVRSRV_HANDLE_BASE *psBase,
                                       IMG_HANDLE *phHandle,
                                       void *pvData,
                                       PVRSRV_HANDLE_TYPE eType,
                                       PVRSRV_HANDLE_ALLOC_FLAG eFlag,
                                       PFN_HANDLE_RELEASE pfnReleaseData) {
  return PVRSRV_OK;
}

PVRSRV_ERROR PVRSRVAllocSubHandleUnlocked(PVRSRV_HANDLE_BASE *psBase,
                                          IMG_HANDLE *phHandle,
                                          void *pvData,
                                          PVRSRV_HANDLE_TYPE eType,
                                          PVRSRV_HANDLE_ALLOC_FLAG eFlag,
                                          IMG_HANDLE hParent) {
  return PVRSRV_OK;
}

void PVRSRVReleaseHandleUnlocked(PVRSRV_HANDLE_BASE *psBase,
                                 IMG_HANDLE hHandle,
                                 PVRSRV_HANDLE_TYPE eType) {
}

PVRSRV_ERROR PVRSRVDestroyHandleUnlocked(PVRSRV_HANDLE_BASE *psBase,
                                         IMG_HANDLE hHandle,
                                         PVRSRV_HANDLE_TYPE eType) {
  return PVRSRV_OK;
}

IMG_BOOL PMR_IsMemLayoutFixed(PMR *psPMR) {
  return IMG_TRUE;
}

/*******************************************
            DevmemIntAcquireRemoteCtx
 *******************************************/

/* Bridge in structure for DevmemIntAcquireRemoteCtx */
typedef struct PVRSRV_BRIDGE_IN_DEVMEMINTACQUIREREMOTECTX_TAG {
  IMG_HANDLE hPMR;
}
    __packed PVRSRV_BRIDGE_IN_DEVMEMINTACQUIREREMOTECTX;

/* Bridge out structure for DevmemIntAcquireRemoteCtx */
typedef struct PVRSRV_BRIDGE_OUT_DEVMEMINTACQUIREREMOTECTX_TAG {
  IMG_HANDLE hContext;
  IMG_HANDLE hPrivData;
  PVRSRV_ERROR eError;
}
    __packed PVRSRV_BRIDGE_OUT_DEVMEMINTACQUIREREMOTECTX;

// Dummy implementations:

#define ATOMIC_T uint32_t

void OSAtomicIncrement(ATOMIC_T *i) {
}

#define POSWR_LOCK pthread_mutex_t
#define OSWRLockAcquireRead(lock) pthread_mutex_lock(&lock)
#define OSWRLockReleaseRead(lock) pthread_mutex_unlock(&lock)

#endif //POWERVR_SUPPORT_H
