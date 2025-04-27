/*******************************************************************************
@File
@Title          Server bridge for cmm
@Copyright      Copyright (c) Imagination Technologies Ltd. All Rights Reserved
@Description    Implements the server side of the bridge for cmm
@License        Dual MIT/GPLv2

The contents of this file are subject to the MIT license as set out below.

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

Alternatively, the contents of this file may be used under the terms of
the GNU General Public License Version 2 ("GPL") in which case the provisions
of GPL are applicable instead of those above.

If you wish to allow use of your version of this file only under the terms of
GPL, and not to allow others to use your version of this file under the terms
of the MIT license, indicate your decision by deleting the provisions above
and replace them with the notice and other provisions required by GPL as set
out in the file called "GPL-COPYING" included in this distribution. If you do
not delete the provisions above, a recipient may use your version of this file
under the terms of either the MIT license or GPL.

This License is also included in this distribution in the file called
"MIT-COPYING".

EXCEPT AS OTHERWISE STATED IN A NEGOTIATED AGREEMENT: (A) THE SOFTWARE IS
PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING
BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR
PURPOSE AND NONINFRINGEMENT; AND (B) IN NO EVENT SHALL THE AUTHORS OR
COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*******************************************************************************/
#include "devicemem_server.h"

// clang-format off

struct TRACKED _DEVMEMINT_CTX_
{
  ATOMIC_T hRefCount;

  /* This handle is for devices that require notification when a new
     memory context is created and they need to store private data that
     is associated with the context. */
  IMG_HANDLE hPrivData;
};

struct _DEVMEMINT_CTX_EXPORT_
{
	DLLIST_NODE sNode;
  DEVMEMINT_CTX *psDevmemCtx;
  PMR *psPMR;
  ATOMIC_T hRefCount;
};

/*************************************************************************/ /*!
@Function       DevmemIntCtxAcquire
@Description    Acquire a reference to the provided device memory context.
@Return         None
*/ /**************************************************************************/
void DevmemIntCtxAcquire(RETAINED DEVMEMINT_CTX *psDevmemCtx)
{
  OSAtomicIncrement(&psDevmemCtx->hRefCount);
}

void DevmemIntCtxRelease(CONSUMED DEVMEMINT_CTX *psDevmemCtx)
{
  OSAtomicDecrement(&psDevmemCtx->hRefCount);
}

PVRSRV_ERROR
DevmemIntCtxDestroy(DEVMEMINT_CTX *psDevmemCtx)
{
  return PVRSRV_OK;
}

static POSWR_LOCK g_hExportCtxListLock;
static DLLIST_NODE g_sExportCtxList;

PVRSRV_ERROR
DevmemIntAcquireRemoteCtx(PMR *psPMR,
                          DEVMEMINT_CTX **ppsContext,
                          IMG_HANDLE *phPrivData)
{
  PDLLIST_NODE psListNode, psListNodeNext;
  DEVMEMINT_CTX_EXPORT *psCtxExport;

  OSWRLockAcquireRead(g_hExportCtxListLock);        // <----------------- lock the list lock
  /* Find context from list using PMR as key */
  dllist_foreach_node(&g_sExportCtxList, psListNode, psListNodeNext)
  {
    psCtxExport = IMG_CONTAINER_OF(psListNode, DEVMEMINT_CTX_EXPORT, sNode);
    if (psCtxExport->psPMR == psPMR)           // <---------------- if PMR object is equal
    {
      OSWRLockReleaseRead(g_hExportCtxListLock);      // <------------ [1] unlock the list lock
      DevmemIntCtxAcquire(psCtxExport->psDevmemCtx);  // <--------- [2] increase refcount
      *ppsContext = psCtxExport->psDevmemCtx;         // <-------------- [3]get the object
      *phPrivData = psCtxExport->psDevmemCtx->hPrivData;  // <---------- [4]get the object

      /* PMR should have been already exported to import it
       * If a PMR is exported, its immutable and the same is
       * checked here */
      PVR_ASSERT(IMG_TRUE == PMR_IsMemLayoutFixed(psPMR));

      return PVRSRV_OK;
    }
  }
  OSWRLockReleaseRead(g_hExportCtxListLock);

  /* Unable to find exported context, return error */
  PVR_DPF((PVR_DBG_ERROR,
      "%s: Failed to acquire remote context. Could not retrieve context with given PMR",
      __func__));
  return PVRSRV_ERROR_INVALID_PARAMS;
}

static PVRSRV_ERROR _DevmemIntAcquireRemoteCtxpsContextIntRelease(void *pvData)
{
	return PVRSRV_OK;
}

THREAD_ENTRY static IMG_INT
PVRSRVBridgeDevmemIntAcquireRemoteCtx(IMG_UINT32 ui32DispatchTableEntry,
				      IMG_UINT8 * psDevmemIntAcquireRemoteCtxIN_UI8,
				      IMG_UINT8 * psDevmemIntAcquireRemoteCtxOUT_UI8,
				      CONNECTION_DATA * psConnection)
{
	PVRSRV_BRIDGE_IN_DEVMEMINTACQUIREREMOTECTX *psDevmemIntAcquireRemoteCtxIN =
	    (PVRSRV_BRIDGE_IN_DEVMEMINTACQUIREREMOTECTX *)
	    IMG_OFFSET_ADDR(psDevmemIntAcquireRemoteCtxIN_UI8, 0);
	PVRSRV_BRIDGE_OUT_DEVMEMINTACQUIREREMOTECTX *psDevmemIntAcquireRemoteCtxOUT =
	    (PVRSRV_BRIDGE_OUT_DEVMEMINTACQUIREREMOTECTX *)
	    IMG_OFFSET_ADDR(psDevmemIntAcquireRemoteCtxOUT_UI8, 0);

	IMG_HANDLE hPMR = psDevmemIntAcquireRemoteCtxIN->hPMR;
	PMR *psPMRInt = NULL;
	DEVMEMINT_CTX *psContextInt = NULL;
	IMG_HANDLE hPrivDataInt = NULL;

	psDevmemIntAcquireRemoteCtxOUT->hContext = NULL;

	/* Lock over handle lookup. */
	LockHandle(psConnection->psHandleBase);

	/* Look up the address from the handle */
	psDevmemIntAcquireRemoteCtxOUT->eError =
	    PVRSRVLookupHandleUnlocked(psConnection->psHandleBase,
				       (void **)&psPMRInt,
				       hPMR, PVRSRV_HANDLE_TYPE_PHYSMEM_PMR, IMG_TRUE);
	if (unlikely(psDevmemIntAcquireRemoteCtxOUT->eError != PVRSRV_OK))
	{
		UnlockHandle(psConnection->psHandleBase);
		goto DevmemIntAcquireRemoteCtx_exit;
	}
	/* Release now we have looked up handles. */
	UnlockHandle(psConnection->psHandleBase);

	psDevmemIntAcquireRemoteCtxOUT->eError =
	    DevmemIntAcquireRemoteCtx(psPMRInt, &psContextInt, &hPrivDataInt);
	/* Exit early if bridged call fails */
	if (unlikely(psDevmemIntAcquireRemoteCtxOUT->eError != PVRSRV_OK))
	{
		goto DevmemIntAcquireRemoteCtx_exit;
	}

	/* Lock over handle creation. */
	LockHandle(psConnection->psHandleBase);

	psDevmemIntAcquireRemoteCtxOUT->eError =
	    PVRSRVAllocHandleUnlocked(psConnection->psHandleBase,
				      &psDevmemIntAcquireRemoteCtxOUT->hContext,
				      (void *)psContextInt, PVRSRV_HANDLE_TYPE_DEVMEMINT_CTX,
				      PVRSRV_HANDLE_ALLOC_FLAG_NONE,
				      (PFN_HANDLE_RELEASE) &
				      _DevmemIntAcquireRemoteCtxpsContextIntRelease);
	if (unlikely(psDevmemIntAcquireRemoteCtxOUT->eError != PVRSRV_OK))
	{
		UnlockHandle(psConnection->psHandleBase);
		goto DevmemIntAcquireRemoteCtx_exit;
	}

	psDevmemIntAcquireRemoteCtxOUT->eError =
	    PVRSRVAllocSubHandleUnlocked(psConnection->psHandleBase,
					 &psDevmemIntAcquireRemoteCtxOUT->hPrivData,
					 (void *)hPrivDataInt, PVRSRV_HANDLE_TYPE_DEV_PRIV_DATA,
					 PVRSRV_HANDLE_ALLOC_FLAG_NONE,
					 psDevmemIntAcquireRemoteCtxOUT->hContext);
	if (unlikely(psDevmemIntAcquireRemoteCtxOUT->eError != PVRSRV_OK))
	{
		UnlockHandle(psConnection->psHandleBase);
		goto DevmemIntAcquireRemoteCtx_exit;
	}

	/* Release now we have created handles. */
	UnlockHandle(psConnection->psHandleBase);

DevmemIntAcquireRemoteCtx_exit:

	/* Lock over handle lookup cleanup. */
	LockHandle(psConnection->psHandleBase);

	/* Unreference the previously looked up handle */
	if (psPMRInt)
	{
		PVRSRVReleaseHandleUnlocked(psConnection->psHandleBase,
					    hPMR, PVRSRV_HANDLE_TYPE_PHYSMEM_PMR);
	}
	/* Release now we have cleaned up look up handles. */
	UnlockHandle(psConnection->psHandleBase);

	if (psDevmemIntAcquireRemoteCtxOUT->eError != PVRSRV_OK)
	{
		if (psDevmemIntAcquireRemoteCtxOUT->hContext)
		{
			PVRSRV_ERROR eError;

			/* Lock over handle creation cleanup. */
			LockHandle(psConnection->psHandleBase);

			eError = PVRSRVDestroyHandleUnlocked(psConnection->psHandleBase,
							     (IMG_HANDLE)
							     psDevmemIntAcquireRemoteCtxOUT->
							     hContext,
							     PVRSRV_HANDLE_TYPE_DEVMEMINT_CTX);
			if (unlikely((eError != PVRSRV_OK) && (eError != PVRSRV_ERROR_RETRY)))
			{
				PVR_DPF((PVR_DBG_ERROR,
					 "%s: %s", __func__, PVRSRVGetErrorString(eError)));
			}
			/* Releasing the handle should free/destroy/release the resource.
			 * This should never fail... */
			PVR_ASSERT((eError == PVRSRV_OK) || (eError == PVRSRV_ERROR_RETRY));

			/* Release now we have cleaned up creation handles. */
			UnlockHandle(psConnection->psHandleBase);

		}

		else if (psContextInt)
		{
			DevmemIntCtxDestroy(psContextInt);
		}

	}

	return 0;
}

PVRSRV_ERROR
DevmemIntUnexportCtx(DEVMEMINT_CTX_EXPORT *psContextExport)
{
	PMRUnrefPMR(psContextExport->psPMR);
	DevmemIntCtxRelease(psContextExport->psDevmemCtx);
	OSWRLockAcquireWrite(g_hExportCtxListLock);
	dllist_remove_node(&psContextExport->sNode);
	OSWRLockReleaseWrite(g_hExportCtxListLock);
	free((void*)psContextExport);
	/* Unable to find exported context, return error */
	return PVRSRV_OK;
}


static PVRSRV_ERROR _DevmemIntExportCtxpsContextExportIntRelease(void *pvData)
{
	PVRSRV_ERROR eError;
	eError = DevmemIntUnexportCtx((DEVMEMINT_CTX_EXPORT *) pvData);
	return eError;
}
static IMG_INT
PVRSRVBridgeDevmemIntExportCtx(IMG_UINT32 ui32DispatchTableEntry,
			       IMG_UINT8 * psDevmemIntExportCtxIN_UI8,
			       IMG_UINT8 * psDevmemIntExportCtxOUT_UI8,
			       CONNECTION_DATA * psConnection)
{
	PVRSRV_BRIDGE_IN_DEVMEMINTEXPORTCTX *psDevmemIntExportCtxIN =
	    (PVRSRV_BRIDGE_IN_DEVMEMINTEXPORTCTX *) IMG_OFFSET_ADDR(psDevmemIntExportCtxIN_UI8, 0);
	PVRSRV_BRIDGE_OUT_DEVMEMINTEXPORTCTX *psDevmemIntExportCtxOUT =
	    (PVRSRV_BRIDGE_OUT_DEVMEMINTEXPORTCTX *) IMG_OFFSET_ADDR(psDevmemIntExportCtxOUT_UI8,
								     0);
	IMG_HANDLE hContext = psDevmemIntExportCtxIN->hContext;
	DEVMEMINT_CTX *psContextInt = NULL;
	IMG_HANDLE hPMR = psDevmemIntExportCtxIN->hPMR;
	PMR *psPMRInt = NULL;
	DEVMEMINT_CTX_EXPORT *psContextExportInt = NULL;
	/* Lock over handle lookup. */
	LockHandle(psConnection->psHandleBase);
	/* Look up the address from the handle */
	psDevmemIntExportCtxOUT->eError =
	    PVRSRVLookupHandleUnlocked(psConnection->psHandleBase,
				       (void **)&psContextInt,
				       hContext, PVRSRV_HANDLE_TYPE_DEVMEMINT_CTX, IMG_TRUE);
	if (unlikely(psDevmemIntExportCtxOUT->eError != PVRSRV_OK))
	{
		UnlockHandle(psConnection->psHandleBase);
		goto DevmemIntExportCtx_exit;
	}
	/* Look up the address from the handle */
	psDevmemIntExportCtxOUT->eError =
	    PVRSRVLookupHandleUnlocked(psConnection->psHandleBase,
				       (void **)&psPMRInt,
				       hPMR, PVRSRV_HANDLE_TYPE_PHYSMEM_PMR, IMG_TRUE);
	if (unlikely(psDevmemIntExportCtxOUT->eError != PVRSRV_OK))
	{
		UnlockHandle(psConnection->psHandleBase);
		goto DevmemIntExportCtx_exit;
	}
	/* Release now we have looked up handles. */
	UnlockHandle(psConnection->psHandleBase);
	psDevmemIntExportCtxOUT->eError =
	    DevmemIntExportCtx(psContextInt, psPMRInt, &psContextExportInt);
	/* Exit early if bridged call fails */
	if (unlikely(psDevmemIntExportCtxOUT->eError != PVRSRV_OK))
	{
		goto DevmemIntExportCtx_exit;
	}
	/* Lock over handle creation. */
	LockHandle(psConnection->psHandleBase);
	psDevmemIntExportCtxOUT->eError = PVRSRVAllocHandleUnlocked(psConnection->psHandleBase,
								    &psDevmemIntExportCtxOUT->
								    hContextExport,
								    (void *)psContextExportInt,
								    PVRSRV_HANDLE_TYPE_DEVMEMINT_CTX_EXPORT,
								    PVRSRV_HANDLE_ALLOC_FLAG_NONE,
								    (PFN_HANDLE_RELEASE) &
								    _DevmemIntExportCtxpsContextExportIntRelease);
	if (unlikely(psDevmemIntExportCtxOUT->eError != PVRSRV_OK))
	{
		UnlockHandle(psConnection->psHandleBase);
		goto DevmemIntExportCtx_exit;
	}
	/* Release now we have created handles. */
	UnlockHandle(psConnection->psHandleBase);
DevmemIntExportCtx_exit:
	/* Lock over handle lookup cleanup. */
	LockHandle(psConnection->psHandleBase);
	/* Unreference the previously looked up handle */
	if (psContextInt)
	{
		PVRSRVReleaseHandleUnlocked(psConnection->psHandleBase,
					    hContext, PVRSRV_HANDLE_TYPE_DEVMEMINT_CTX);
	}
	/* Unreference the previously looked up handle */
	if (psPMRInt)
	{
		PVRSRVReleaseHandleUnlocked(psConnection->psHandleBase,
					    hPMR, PVRSRV_HANDLE_TYPE_PHYSMEM_PMR);
	}
	/* Release now we have cleaned up look up handles. */
	UnlockHandle(psConnection->psHandleBase);
	if (psDevmemIntExportCtxOUT->eError != PVRSRV_OK)
	{
		if (psContextExportInt)
		{
			LockHandle(KERNEL_HANDLE_BASE);
			DevmemIntUnexportCtx(psContextExportInt);
			UnlockHandle(KERNEL_HANDLE_BASE);
		}
	}
	return 0;
}

CONNECTION_DATA* g_conn = NULL;

void* thread1(void* unused) {
	IMG_UINT8 in[0x100];
	IMG_UINT8 out[0x100];
	PVRSRVBridgeDevmemIntAcquireRemoteCtx(0, in, out, g_conn);
	return NULL;
}

#include <stdlib.h>

void* thread2(void* unused) {
	IMG_UINT8 in[0x100];
	IMG_UINT8 out[0x100];
	PVRSRVBridgeDevmemIntExportCtx(0, in, out, g_conn);
	return NULL;
}

int main() {
	g_conn = malloc(sizeof(CONNECTION_DATA));

	pthread_t t1, t2;
	pthread_create(&t1, NULL, thread1, NULL);
	pthread_create(&t2, NULL, thread2, NULL);
	pthread_join(t1, NULL);
	pthread_join(t2, NULL);
	return 0;
}
