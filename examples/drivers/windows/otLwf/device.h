/*
 *  Copyright (c) 2016, Microsoft Corporation.
 *  All rights reserved.
 *
 *  Redistribution and use in source and binary forms, with or without
 *  modification, are permitted provided that the following conditions are met:
 *  1. Redistributions of source code must retain the above copyright
 *     notice, this list of conditions and the following disclaimer.
 *  2. Redistributions in binary form must reproduce the above copyright
 *     notice, this list of conditions and the following disclaimer in the
 *     documentation and/or other materials provided with the distribution.
 *  3. Neither the name of the copyright holder nor the
 *     names of its contributors may be used to endorse or promote products
 *     derived from this software without specific prior written permission.
 *
 *  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 *  AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 *  IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 *  ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 *  LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 *  CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 *  SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 *  INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 *  CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 *  ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 *  POSSIBILITY OF SUCH DAMAGE.
 */

/**
 * @file
 * @brief
 *  This file defines the functions for managing the device IOCTL interface.
 */

#ifndef _DEVICE_H
#define _DEVICE_H

//
// The filter needs to handle IOCTRLs
//
#define LINKNAME_STRING             L"\\DosDevices\\otLwf"
#define NTDEVICE_STRING             L"\\Device\\otLwf"

// Context for IO Device Control callbacks
typedef struct _OTLWF_DEVICE_EXTENSION
{
    ULONG           Signature;
    NDIS_HANDLE     Handle;

    NDIS_SPIN_LOCK  Lock;
    _Guarded_by_(Lock)
    BOOLEAN         HasClient;
    _Guarded_by_(Lock)
    LIST_ENTRY      PendingNotificationList;
    _Guarded_by_(Lock)
    PIRP            PendingNotificationIRP;

} OTLWF_DEVICE_EXTENSION, *POTLWF_DEVICE_EXTENSION;

// Global context for device control callbacks
extern POTLWF_DEVICE_EXTENSION FilterDeviceExtension;

//
// Function prototypes
//

// Registers for Io Control callbacks
_No_competing_thread_
INITCODE
NDIS_STATUS
otLwfRegisterDevice(
    VOID
    );

// Unregisters for Io Control Callbacks
_No_competing_thread_
_IRQL_requires_max_(PASSIVE_LEVEL)
VOID
otLwfDeregisterDevice(
    VOID
    );

// Callback for general control IRPs
DRIVER_DISPATCH otLwfDispatch;

// Callback for IOCTLs
DRIVER_DISPATCH otLwfDeviceIoControl;

// Attempts to find and add a reference to the Thread interface
_IRQL_requires_max_(PASSIVE_LEVEL)
PMS_FILTER
otLwfFindAndRefInterface(
    _In_ PGUID  InterfaceGuid
    );

// Releases a successfully referenced Thread interface from a previous 
// call to otLwfFindAndRefInterface.
_IRQL_requires_max_(PASSIVE_LEVEL)
VOID
otLwfReleaseInterface(
    _In_ PMS_FILTER pFilter
    );

//
// Notification Type and Functions
//

// Notification structure
typedef struct _FILTER_NOTIFICATION_ENTRY
{
    LIST_ENTRY          Link;
    OTLWF_NOTIFICATION  Notif;

} FILTER_NOTIFICATION_ENTRY, *PFILTER_NOTIFICATION_ENTRY;

// Tag for allocating notification structures 'TNtf
#define FILTER_NOTIF_ALLOC_TAG 'ftNT'

// Helper to allocate a new notification entry
#define FILTER_ALLOC_NOTIF(_pFilter) \
    (PFILTER_NOTIFICATION_ENTRY)NdisAllocateMemoryWithTagPriority(_pFilter->FilterHandle, sizeof(FILTER_NOTIFICATION_ENTRY), FILTER_NOTIF_ALLOC_TAG, NormalPoolPriority)

// Indicates a new notification
_IRQL_requires_max_(DISPATCH_LEVEL)
VOID
otLwfIndicateNotification(
    _In_ PFILTER_NOTIFICATION_ENTRY NotifEntry
    );

// Queries the next notification
_IRQL_requires_max_(DISPATCH_LEVEL)
NTSTATUS
otLwfQueryNextNotification(
    _In_ PIRP Irp
    );

#endif // _DEVICE_H
