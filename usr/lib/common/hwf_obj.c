/*
 * COPYRIGHT (c) International Business Machines Corp. 2001-2017
 *
 * This program is provided under the terms of the Common Public License,
 * version 1.0 (CPL-1.0). Any use, reproduction or distribution for this
 * software constitutes recipient's acceptance of CPL-1.0 terms which can be
 * found in the file LICENSE file or at
 * https://opensource.org/licenses/cpl1.0.php
 */

// File:  hwf_obj.c
//
// Hardware Feature Object functions

#include <pthread.h>
#include <stdlib.h>

#include <string.h>             // for memcmp() et al

#include "pkcs11types.h"
#include "defs.h"
#include "host_defs.h"
#include "h_extern.h"
#include "trace.h"

#include "tok_spec_struct.h"


// hwf_object_check_required_attributes()
//
// Check required common attributes for hardware feature objects
//
CK_RV hwf_object_check_required_attributes(TEMPLATE *tmpl, CK_ULONG mode)
{
    CK_ULONG val;
    CK_RV rc;

    rc = template_attribute_get_ulong(tmpl, CKA_HW_FEATURE_TYPE, &val);
    if (rc != CKR_OK) {
        if (mode == MODE_CREATE) {
            TRACE_ERROR("Could not find CKA_HW_FEATURE_TYPE\n");
            return rc;
        }
    }

    return template_check_required_base_attributes(tmpl, mode);
}

CK_RV clock_check_required_attributes(TEMPLATE *tmpl, CK_ULONG mode)
{
    CK_ATTRIBUTE *attr = NULL;
    CK_BBOOL found;

    if (mode == MODE_CREATE) {
        found = template_attribute_find(tmpl, CKA_VALUE, &attr);
        if (!found) {
            TRACE_ERROR("%s\n", ock_err(ERR_TEMPLATE_INCOMPLETE));
            return CKR_TEMPLATE_INCOMPLETE;
        }
    }

    return hwf_object_check_required_attributes(tmpl, mode);
}

CK_RV counter_check_required_attributes(TEMPLATE *tmpl, CK_ULONG mode)
{
    CK_ATTRIBUTE *attr = NULL;
    CK_BBOOL found, val;
    CK_RV rc;

    if (mode == MODE_CREATE) {
        found = template_attribute_find(tmpl, CKA_VALUE, &attr);
        if (!found) {
            TRACE_ERROR("%s\n", ock_err(ERR_TEMPLATE_INCOMPLETE));
            return CKR_TEMPLATE_INCOMPLETE;
        }

        rc = template_attribute_get_bool(tmpl, CKA_HAS_RESET, &val);
        if (rc != CKR_OK) {
            TRACE_ERROR("Could not find CKA_HAS_RESET\n");
            return rc;
        }

        rc = template_attribute_get_bool(tmpl, CKA_RESET_ON_INIT, &val);
        if (rc != CKR_OK) {
            TRACE_ERROR("Could not find CKA_RESET_ON_INIT\n");
            return rc;
        }
    }

    return hwf_object_check_required_attributes(tmpl, mode);
}


// hwf_object_set_default_attributes()
//
CK_RV hwf_object_set_default_attributes(TEMPLATE *tmpl, CK_ULONG mode)
{
#if 0
    CK_ATTRIBUTE *local_attr = NULL;
    CK_RV rc;

    local_attr =
        (CK_ATTRIBUTE *) malloc(sizeof(CK_ATTRIBUTE) + sizeof(CK_BBOOL));

    if (!local_attr) {
        TRACE_ERROR("%s\n", ock_err(ERR_HOST_MEMORY));
        return CKR_HOST_MEMORY;
    }

    local_attr->type = CKA_LOCAL;
    local_attr->ulValueLen = sizeof(CK_BBOOL);
    local_attr->pValue = (CK_BYTE *) local_attr + sizeof(CK_ATTRIBUTE);
    *(CK_BBOOL *) local_attr->pValue = FALSE;

    rc = template_update_attribute(tmpl, local_attr);
    if (rc != CKR_OK) {
        TRACE_DEVEL("template_update_attribute failed\n");
        free(local_attr);
        return rc;
    }

#endif
    UNUSED(tmpl);
    UNUSED(mode);

    return CKR_OK;
}


// hwf_object_validate_attribute()
//
CK_RV hwf_validate_attribute(TEMPLATE *tmpl, CK_ATTRIBUTE *attr,
                             CK_ULONG mode)
{
    switch (attr->type) {
    case CKA_HW_FEATURE_TYPE:
        if (attr->ulValueLen != sizeof(CK_HW_FEATURE_TYPE) ||
            attr->pValue == NULL) {
            TRACE_ERROR("%s\n", ock_err(ERR_ATTRIBUTE_VALUE_INVALID));
            return CKR_ATTRIBUTE_VALUE_INVALID;
        }
        if (mode == MODE_CREATE)
            return CKR_OK;

        TRACE_ERROR("%s\n", ock_err(ERR_ATTRIBUTE_READ_ONLY));
        return CKR_ATTRIBUTE_READ_ONLY;
    default:
        return template_validate_base_attribute(tmpl, attr, mode);
    }

    TRACE_ERROR("%s: %lx\n", ock_err(ERR_ATTRIBUTE_TYPE_INVALID), attr->type);
    return CKR_ATTRIBUTE_TYPE_INVALID;
}

//
//
CK_RV clock_validate_attribute(TEMPLATE *tmpl, CK_ATTRIBUTE *attr,
                               CK_ULONG mode)
{
    UNUSED(mode);

    switch (attr->type) {
    case CKA_VALUE:
        return CKR_OK;
    default:
        return hwf_validate_attribute(tmpl, attr, mode);
    }
}

//
//
CK_RV counter_validate_attribute(TEMPLATE *tmpl, CK_ATTRIBUTE *attr,
                                 CK_ULONG mode)
{
    switch (attr->type) {
    case CKA_VALUE:
        /* Fall Through */
    case CKA_HAS_RESET:
        /* Fall Through */
    case CKA_RESET_ON_INIT:
        TRACE_ERROR("%s\n", ock_err(ERR_ATTRIBUTE_READ_ONLY));
        return CKR_ATTRIBUTE_READ_ONLY;
    default:
        return hwf_validate_attribute(tmpl, attr, mode);
    }
}


CK_RV clock_set_default_attributes(TEMPLATE *tmpl, CK_ULONG mode)
{
    CK_RV rc;
    CK_ATTRIBUTE *value_attr;

    rc = hwf_object_set_default_attributes(tmpl, mode);
    if (rc != CKR_OK)
        return rc;

    value_attr = (CK_ATTRIBUTE *) malloc(sizeof(CK_ATTRIBUTE));

    if (!value_attr) {
        TRACE_ERROR("%s\n", ock_err(ERR_HOST_MEMORY));
        return CKR_HOST_MEMORY;
    }

    value_attr->type = CKA_VALUE;
    value_attr->ulValueLen = 0;
    value_attr->pValue = NULL;

    rc = template_update_attribute(tmpl, value_attr);
    if (rc != CKR_OK) {
        TRACE_DEVEL("template_update_attribute failed\n");
        free(value_attr);
        return rc;
    }

    return CKR_OK;
}

CK_RV counter_set_default_attributes(TEMPLATE *tmpl, CK_ULONG mode)
{
    CK_RV rc;
    CK_ATTRIBUTE *value_attr;
    CK_ATTRIBUTE *hasreset_attr;
    CK_ATTRIBUTE *resetoninit_attr;

    rc = hwf_object_set_default_attributes(tmpl, mode);
    if (rc != CKR_OK)
        return rc;

    value_attr = (CK_ATTRIBUTE *) malloc(sizeof(CK_ATTRIBUTE));
    hasreset_attr =
        (CK_ATTRIBUTE *) malloc(sizeof(CK_ATTRIBUTE) + sizeof(CK_BBOOL));
    resetoninit_attr =
        (CK_ATTRIBUTE *) malloc(sizeof(CK_ATTRIBUTE) + sizeof(CK_BBOOL));

    if (!value_attr || !hasreset_attr || !resetoninit_attr) {
        TRACE_ERROR("%s\n", ock_err(ERR_HOST_MEMORY));
        rc = CKR_HOST_MEMORY;
        goto error;
    }

    value_attr->type = CKA_VALUE;
    value_attr->ulValueLen = 0;
    value_attr->pValue = NULL;

    hasreset_attr->type = CKA_HAS_RESET;
    hasreset_attr->ulValueLen = sizeof(CK_BBOOL);
    hasreset_attr->pValue = (CK_BYTE *) hasreset_attr + sizeof(CK_ATTRIBUTE);
    *(CK_BBOOL *) hasreset_attr->pValue = FALSE;

    /* Hmm...  Not sure if we should be setting this here. */
    resetoninit_attr->type = CKA_RESET_ON_INIT;
    resetoninit_attr->ulValueLen = sizeof(CK_BBOOL);
    resetoninit_attr->pValue =
        (CK_BYTE *) resetoninit_attr + sizeof(CK_ATTRIBUTE);
    *(CK_BBOOL *) resetoninit_attr->pValue = FALSE;

    rc = template_update_attribute(tmpl, value_attr);
    if (rc != CKR_OK) {
        TRACE_DEVEL("template_update_attribute failed\n");
        goto error;
    }
    value_attr = NULL;
    rc = template_update_attribute(tmpl, hasreset_attr);
    if (rc != CKR_OK) {
        TRACE_DEVEL("template_update_attribute failed\n");
        goto error;
    }
    hasreset_attr = NULL;
    rc = template_update_attribute(tmpl, resetoninit_attr);
    if (rc != CKR_OK) {
        TRACE_DEVEL("template_update_attribute failed\n");
        goto error;
    }
    resetoninit_attr = NULL;

    return CKR_OK;

error:
    if (value_attr)
        free(value_attr);
    if (hasreset_attr)
        free(hasreset_attr);
    if (resetoninit_attr)
        free(resetoninit_attr);

    return rc;
}
