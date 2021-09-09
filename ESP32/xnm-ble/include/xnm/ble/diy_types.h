

#pragma once

#include <stdint.h>
#include <host/ble_uuid.h>

#ifndef __XNM_BLE_EXCLUDE_DIY_TYPES

typedef uint16_t ble_gatt_chr_flags;
typedef void ble_gap_event;

struct ble_gatt_chr_def {
    /**
     * Pointer to characteristic UUID; use BLE_UUIDxx_DECLARE macros to declare
     * proper UUID; NULL if there are no more characteristics in the service.
     */
    const ble_uuid_t *uuid;

    /**
     * Callback that gets executed when this characteristic is read or
     * written.
     */
    void *access_cb;

    /** Optional argument for callback. */
    void *arg;

    /**
     * Array of this characteristic's descriptors.  NULL if no descriptors.
     * Do not include CCCD; it gets added automatically if this
     * characteristic's notify or indicate flag is set.
     */
    void *descriptors;

    /** Specifies the set of permitted operations for this characteristic. */
    ble_gatt_chr_flags flags;

    /** Specifies minimum required key size to access this characteristic. */
    uint8_t min_key_size;

    /**
     * At registration time, this is filled in with the characteristic's value
     * attribute handle.
     */
    uint16_t *val_handle;
};


struct ble_gatt_svc_def {
    /**
     * One of the following:
     *     o BLE_GATT_SVC_TYPE_PRIMARY - primary service
     *     o BLE_GATT_SVC_TYPE_SECONDARY - secondary service
     *     o 0 - No more services in this array.
     */
    uint8_t type;

    /**
     * Pointer to service UUID; use BLE_UUIDxx_DECLARE macros to declare
     * proper UUID; NULL if there are no more characteristics in the service.
     */
    const ble_uuid_t *uuid;

    /**
     * Array of pointers to other service definitions.  These services are
     * reported as "included services" during service discovery.  Terminate the
     * array with NULL.
     */
     void **includes;

    /**
     * Array of characteristic definitions corresponding to characteristics
     * belonging to this service.
     */
    void *characteristics;
};

/**
 * Context for an access to a GATT characteristic or descriptor.  When a client
 * reads or writes a locally registered characteristic or descriptor, an
 * instance of this struct gets passed to the application callback.
 */
struct ble_gatt_access_ctxt {
    /**
     * Indicates the gatt operation being performed.  This is equal to one of
     * the following values:
     *     o  BLE_GATT_ACCESS_OP_READ_CHR
     *     o  BLE_GATT_ACCESS_OP_WRITE_CHR
     *     o  BLE_GATT_ACCESS_OP_READ_DSC
     *     o  BLE_GATT_ACCESS_OP_WRITE_DSC
     */
    uint8_t op;

    /**
     * A container for the GATT access data.
     *     o For reads: The application populates this with the value of the
     *       characteristic or descriptor being read.
     *     o For writes: This is already populated with the value being written
     *       by the peer.  If the application wishes to retain this mbuf for
     *       later use, the access callback must set this pointer to NULL to
     *       prevent the stack from freeing it.
     */
	  void *om;

    /**
     * The GATT operation being performed dictates which field in this union is
     * valid.  If a characteristic is being accessed, the chr field is valid.
     * Otherwise a descriptor is being accessed, in which case the dsc field
     * is valid.
     */
    union {
        /**
         * The characteristic definition corresponding to the characteristic
         * being accessed.  This is what the app registered at startup.
         */
        void *chr;

        /**
         * The descriptor definition corresponding to the descriptor being
         * accessed.  This is what the app registered at startup.
         */
        void *dsc;
    };
};

#else

#include "host/ble_gatt.h"
#include "host/ble_gap.h"

#endif