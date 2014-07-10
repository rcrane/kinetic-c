/*
 * kinetic-c-client
 * Copyright (C) 2014 Seagate Technology.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 *
 */

/*
 * Generated by the protocol buffer compiler protoc-c.
 * Generated wtih: protobuf-c 1.0.0-rc1, libprotoc 2.5.0
 * Generated from: kinetic.proto
 */

#ifndef _KINETIC_PROTO_H
#define _KINETIC_PROTO_H

#include <protobuf-c/protobuf-c.h>

PROTOBUF_C__BEGIN_DECLS

#if PROTOBUF_C_VERSION_NUMBER < 1000000
# error This file was generated by a newer version of protoc-c which is incompatible with your libprotobuf-c headers. Please update your headers.
#elif 1000000 < PROTOBUF_C_MIN_COMPILER_VERSION
# error This file was generated by an older version of protoc-c which is incompatible with your libprotobuf-c headers. Please regenerate this file with a newer version of protoc-c.
#endif

typedef struct _KineticProto KineticProto;
typedef struct _KineticProto_Command KineticProto_Command;
typedef struct _KineticProto_Header KineticProto_Header;
typedef struct _KineticProto_Body KineticProto_Body;
typedef struct _KineticProto_Status KineticProto_Status;
typedef struct _KineticProto_KeyValue KineticProto_KeyValue;
typedef struct _KineticProto_Range KineticProto_Range;
typedef struct _KineticProto_Setup KineticProto_Setup;
typedef struct _KineticProto_P2POperation KineticProto_P2POperation;
typedef struct _KineticProto_P2POperation__Operation KineticProto_P2POperation__Operation;
typedef struct _KineticProto_P2POperation__Peer KineticProto_P2POperation__Peer;
typedef struct _KineticProto_GetLog KineticProto_GetLog;
typedef struct _KineticProto_GetLog__Utilization KineticProto_GetLog__Utilization;
typedef struct _KineticProto_GetLog__Temperature KineticProto_GetLog__Temperature;
typedef struct _KineticProto_GetLog__Capacity KineticProto_GetLog__Capacity;
typedef struct _KineticProto_GetLog__Configuration KineticProto_GetLog__Configuration;
typedef struct _KineticProto_GetLog__Configuration__Interface KineticProto_GetLog__Configuration__Interface;
typedef struct _KineticProto_GetLog__Statistics KineticProto_GetLog__Statistics;
typedef struct _KineticProto_GetLog__Limits KineticProto_GetLog__Limits;
typedef struct _KineticProto_Security KineticProto_Security;
typedef struct _KineticProto_Security__ACL KineticProto_Security__ACL;
typedef struct _KineticProto_Security__ACL__Scope KineticProto_Security__ACL__Scope;

/* --- enums --- */

typedef enum _KineticProto_Status__StatusCode
{
    KINETIC_PROTO_STATUS__STATUS_CODE__INVALID_STATUS_CODE = -1,
    KINETIC_PROTO_STATUS__STATUS_CODE__NOT_ATTEMPTED = 0,
    KINETIC_PROTO_STATUS__STATUS_CODE__SUCCESS = 1,
    KINETIC_PROTO_STATUS__STATUS_CODE__HMAC_FAILURE = 2,
    KINETIC_PROTO_STATUS__STATUS_CODE__NOT_AUTHORIZED = 3,
    KINETIC_PROTO_STATUS__STATUS_CODE__VERSION_FAILURE = 4,
    KINETIC_PROTO_STATUS__STATUS_CODE__INTERNAL_ERROR = 5,
    KINETIC_PROTO_STATUS__STATUS_CODE__HEADER_REQUIRED = 6,
    KINETIC_PROTO_STATUS__STATUS_CODE__NOT_FOUND = 7,
    KINETIC_PROTO_STATUS__STATUS_CODE__VERSION_MISMATCH = 8,
    KINETIC_PROTO_STATUS__STATUS_CODE__SERVICE_BUSY = 9,
    KINETIC_PROTO_STATUS__STATUS_CODE__EXPIRED = 10,
    KINETIC_PROTO_STATUS__STATUS_CODE__DATA_ERROR = 11,
    KINETIC_PROTO_STATUS__STATUS_CODE__PERM_DATA_ERROR = 12,
    KINETIC_PROTO_STATUS__STATUS_CODE__REMOTE_CONNECTION_ERROR = 13,
    KINETIC_PROTO_STATUS__STATUS_CODE__NO_SPACE = 14,
    KINETIC_PROTO_STATUS__STATUS_CODE__NO_SUCH_HMAC_ALGORITHM = 15,
    KINETIC_PROTO_STATUS__STATUS_CODE__INVALID_REQUEST = 16,
    KINETIC_PROTO_STATUS__STATUS_CODE__NESTED_OPERATION_ERRORS = 17
    PROTOBUF_C__FORCE_ENUM_TO_BE_INT_SIZE(KINETIC_PROTO_STATUS__STATUS_CODE)
} KineticProto_Status__StatusCode;
typedef enum _KineticProto_GetLog__Type
{
    KINETIC_PROTO_GET_LOG__TYPE__INVALID_TYPE = -1,
    KINETIC_PROTO_GET_LOG__TYPE__UTILIZATIONS = 0,
    KINETIC_PROTO_GET_LOG__TYPE__TEMPERATURES = 1,
    KINETIC_PROTO_GET_LOG__TYPE__CAPACITIES = 2,
    KINETIC_PROTO_GET_LOG__TYPE__CONFIGURATION = 3,
    KINETIC_PROTO_GET_LOG__TYPE__STATISTICS = 4,
    KINETIC_PROTO_GET_LOG__TYPE__MESSAGES = 5,
    KINETIC_PROTO_GET_LOG__TYPE__LIMITS = 6
    PROTOBUF_C__FORCE_ENUM_TO_BE_INT_SIZE(KINETIC_PROTO_GET_LOG__TYPE)
} KineticProto_GetLog__Type;
typedef enum _KineticProto_Security__ACL__HMACAlgorithm
{
    KINETIC_PROTO_SECURITY__ACL__HMACALGORITHM__INVALID_HMAC_ALGORITHM = -1,
    KINETIC_PROTO_SECURITY__ACL__HMACALGORITHM__HmacSHA1 = 1
    PROTOBUF_C__FORCE_ENUM_TO_BE_INT_SIZE(KINETIC_PROTO_SECURITY__ACL__HMACALGORITHM)
} KineticProto_Security__ACL__HMACAlgorithm;
typedef enum _KineticProto_Security__ACL__Permission
{
    KINETIC_PROTO_SECURITY__ACL__PERMISSION__INVALID_PERMISSION = -1,
    KINETIC_PROTO_SECURITY__ACL__PERMISSION__READ = 0,
    KINETIC_PROTO_SECURITY__ACL__PERMISSION__WRITE = 1,
    KINETIC_PROTO_SECURITY__ACL__PERMISSION__DELETE = 2,
    KINETIC_PROTO_SECURITY__ACL__PERMISSION__RANGE = 3,
    KINETIC_PROTO_SECURITY__ACL__PERMISSION__SETUP = 4,
    KINETIC_PROTO_SECURITY__ACL__PERMISSION__P2POP = 5,
    KINETIC_PROTO_SECURITY__ACL__PERMISSION__GETLOG = 7,
    KINETIC_PROTO_SECURITY__ACL__PERMISSION__SECURITY = 8
    PROTOBUF_C__FORCE_ENUM_TO_BE_INT_SIZE(KINETIC_PROTO_SECURITY__ACL__PERMISSION)
} KineticProto_Security__ACL__Permission;
typedef enum _KineticProto_Synchronization
{
    KINETIC_PROTO_SYNCHRONIZATION__INVALID_SYNCHRONIZATION = -1,
    KINETIC_PROTO_SYNCHRONIZATION__WRITETHROUGH = 1,
    KINETIC_PROTO_SYNCHRONIZATION__WRITEBACK = 2,
    KINETIC_PROTO_SYNCHRONIZATION__FLUSH = 3
    PROTOBUF_C__FORCE_ENUM_TO_BE_INT_SIZE(KINETIC_PROTO_SYNCHRONIZATION)
} KineticProto_Synchronization;
typedef enum _KineticProto_Algorithm
{
    KINETIC_PROTO_ALGORITHM__INVALID_ALGORITHM = -1,
    KINETIC_PROTO_ALGORITHM__SHA1 = 1,
    KINETIC_PROTO_ALGORITHM__SHA2 = 2,
    KINETIC_PROTO_ALGORITHM__SHA3 = 3,
    KINETIC_PROTO_ALGORITHM__CRC32 = 4,
    KINETIC_PROTO_ALGORITHM__CRC64 = 5
    PROTOBUF_C__FORCE_ENUM_TO_BE_INT_SIZE(KINETIC_PROTO_ALGORITHM)
} KineticProto_Algorithm;
typedef enum _KineticProto_MessageType
{
    KINETIC_PROTO_MESSAGE_TYPE__INVALID_MESSAGE_TYPE = -1,
    KINETIC_PROTO_MESSAGE_TYPE__GET = 2,
    KINETIC_PROTO_MESSAGE_TYPE__GET_RESPONSE = 1,
    KINETIC_PROTO_MESSAGE_TYPE__PUT = 4,
    KINETIC_PROTO_MESSAGE_TYPE__PUT_RESPONSE = 3,
    KINETIC_PROTO_MESSAGE_TYPE__DELETE = 6,
    KINETIC_PROTO_MESSAGE_TYPE__DELETE_RESPONSE = 5,
    KINETIC_PROTO_MESSAGE_TYPE__GETNEXT = 8,
    KINETIC_PROTO_MESSAGE_TYPE__GETNEXT_RESPONSE = 7,
    KINETIC_PROTO_MESSAGE_TYPE__GETPREVIOUS = 10,
    KINETIC_PROTO_MESSAGE_TYPE__GETPREVIOUS_RESPONSE = 9,
    KINETIC_PROTO_MESSAGE_TYPE__GETKEYRANGE = 12,
    KINETIC_PROTO_MESSAGE_TYPE__GETKEYRANGE_RESPONSE = 11,
    KINETIC_PROTO_MESSAGE_TYPE__GETVERSION = 16,
    KINETIC_PROTO_MESSAGE_TYPE__GETVERSION_RESPONSE = 15,
    KINETIC_PROTO_MESSAGE_TYPE__SETUP = 22,
    KINETIC_PROTO_MESSAGE_TYPE__SETUP_RESPONSE = 21,
    KINETIC_PROTO_MESSAGE_TYPE__GETLOG = 24,
    KINETIC_PROTO_MESSAGE_TYPE__GETLOG_RESPONSE = 23,
    KINETIC_PROTO_MESSAGE_TYPE__SECURITY = 26,
    KINETIC_PROTO_MESSAGE_TYPE__SECURITY_RESPONSE = 25,
    KINETIC_PROTO_MESSAGE_TYPE__PEER2PEERPUSH = 28,
    KINETIC_PROTO_MESSAGE_TYPE__PEER2PEERPUSH_RESPONSE = 27,
    KINETIC_PROTO_MESSAGE_TYPE__NOOP = 30,
    KINETIC_PROTO_MESSAGE_TYPE__NOOP_RESPONSE = 29,
    KINETIC_PROTO_MESSAGE_TYPE__FLUSHALLDATA = 32,
    KINETIC_PROTO_MESSAGE_TYPE__FLUSHALLDATA_RESPONSE = 31
    PROTOBUF_C__FORCE_ENUM_TO_BE_INT_SIZE(KINETIC_PROTO_MESSAGE_TYPE)
} KineticProto_MessageType;

/* --- messages --- */

struct _KineticProto_Command
{
    ProtobufCMessage base;
    KineticProto_Header* header;
    KineticProto_Body* body;
    KineticProto_Status* status;
};
#define KINETIC_PROTO_COMMAND__INIT \
 { PROTOBUF_C_MESSAGE_INIT (&KineticProto_command__descriptor) \
    , NULL, NULL, NULL }

struct _KineticProto_Header
{
    ProtobufCMessage base;
    protobuf_c_boolean has_clusterversion;
    int64_t clusterversion;
    protobuf_c_boolean has_identity;
    int64_t identity;
    protobuf_c_boolean has_connectionid;
    int64_t connectionid;
    protobuf_c_boolean has_sequence;
    int64_t sequence;
    protobuf_c_boolean has_acksequence;
    int64_t acksequence;
    protobuf_c_boolean has_messagetype;
    KineticProto_MessageType messagetype;
    protobuf_c_boolean has_timeout;
    int64_t timeout;
    protobuf_c_boolean has_earlyexit;
    protobuf_c_boolean earlyexit;
    protobuf_c_boolean has_backgroundscan;
    protobuf_c_boolean backgroundscan;
};
#define KINETIC_PROTO_HEADER__INIT \
 { PROTOBUF_C_MESSAGE_INIT (&KineticProto_header__descriptor) \
    , 0,0, 0,0, 0,0, 0,0, 0,0, 0,0, 0,0, 0,0, 0,0 }

struct _KineticProto_Body
{
    ProtobufCMessage base;
    KineticProto_KeyValue* keyvalue;
    KineticProto_Range* range;
    KineticProto_Setup* setup;
    KineticProto_P2POperation* p2poperation;
    KineticProto_GetLog* getlog;
    KineticProto_Security* security;
};
#define KINETIC_PROTO_BODY__INIT \
 { PROTOBUF_C_MESSAGE_INIT (&KineticProto_body__descriptor) \
    , NULL, NULL, NULL, NULL, NULL, NULL }

struct _KineticProto_Status
{
    ProtobufCMessage base;
    protobuf_c_boolean has_code;
    KineticProto_Status__StatusCode code;
    char* statusmessage;
    protobuf_c_boolean has_detailedmessage;
    ProtobufCBinaryData detailedmessage;
};
#define KINETIC_PROTO_STATUS__INIT \
 { PROTOBUF_C_MESSAGE_INIT (&KineticProto_status__descriptor) \
    , 0,0, NULL, 0,{0,NULL} }

struct _KineticProto_KeyValue
{
    ProtobufCMessage base;
    protobuf_c_boolean has_newversion;
    ProtobufCBinaryData newversion;
    protobuf_c_boolean has_force;
    protobuf_c_boolean force;
    protobuf_c_boolean has_key;
    ProtobufCBinaryData key;
    protobuf_c_boolean has_dbversion;
    ProtobufCBinaryData dbversion;
    protobuf_c_boolean has_tag;
    ProtobufCBinaryData tag;
    protobuf_c_boolean has_algorithm;
    KineticProto_Algorithm algorithm;
    protobuf_c_boolean has_metadataonly;
    protobuf_c_boolean metadataonly;
    protobuf_c_boolean has_synchronization;
    KineticProto_Synchronization synchronization;
};
#define KINETIC_PROTO_KEY_VALUE__INIT \
 { PROTOBUF_C_MESSAGE_INIT (&KineticProto_key_value__descriptor) \
    , 0,{0,NULL}, 0,0, 0,{0,NULL}, 0,{0,NULL}, 0,{0,NULL}, 0,0, 0,0, 0,0 }

struct _KineticProto_Range
{
    ProtobufCMessage base;
    protobuf_c_boolean has_startkey;
    ProtobufCBinaryData startkey;
    protobuf_c_boolean has_endkey;
    ProtobufCBinaryData endkey;
    protobuf_c_boolean has_startkeyinclusive;
    protobuf_c_boolean startkeyinclusive;
    protobuf_c_boolean has_endkeyinclusive;
    protobuf_c_boolean endkeyinclusive;
    protobuf_c_boolean has_maxreturned;
    int32_t maxreturned;
    protobuf_c_boolean has_reverse;
    protobuf_c_boolean reverse;
    size_t n_key;
    ProtobufCBinaryData* key;
};
#define KINETIC_PROTO_RANGE__INIT \
 { PROTOBUF_C_MESSAGE_INIT (&KineticProto_range__descriptor) \
    , 0,{0,NULL}, 0,{0,NULL}, 0,0, 0,0, 0,0, 0,0, 0,NULL }

struct _KineticProto_Setup
{
    ProtobufCMessage base;
    protobuf_c_boolean has_newclusterversion;
    int64_t newclusterversion;
    protobuf_c_boolean has_instantsecureerase;
    protobuf_c_boolean instantsecureerase;
    protobuf_c_boolean has_setpin;
    ProtobufCBinaryData setpin;
    protobuf_c_boolean has_pin;
    ProtobufCBinaryData pin;
    protobuf_c_boolean has_firmwaredownload;
    protobuf_c_boolean firmwaredownload;
};
#define KINETIC_PROTO_SETUP__INIT \
 { PROTOBUF_C_MESSAGE_INIT (&KineticProto_setup__descriptor) \
    , 0,0, 0,0, 0,{0,NULL}, 0,{0,NULL}, 0,0 }

struct _KineticProto_P2POperation__Operation
{
    ProtobufCMessage base;
    protobuf_c_boolean has_key;
    ProtobufCBinaryData key;
    protobuf_c_boolean has_version;
    ProtobufCBinaryData version;
    protobuf_c_boolean has_newkey;
    ProtobufCBinaryData newkey;
    protobuf_c_boolean has_force;
    protobuf_c_boolean force;
    KineticProto_Status* status;
    KineticProto_P2POperation* p2pop;
};
#define KINETIC_PROTO_P2_POPERATION__OPERATION__INIT \
 { PROTOBUF_C_MESSAGE_INIT (&KineticProto_p2_poperation__operation__descriptor) \
    , 0,{0,NULL}, 0,{0,NULL}, 0,{0,NULL}, 0,0, NULL, NULL }

struct _KineticProto_P2POperation__Peer
{
    ProtobufCMessage base;
    char* hostname;
    protobuf_c_boolean has_port;
    int32_t port;
    protobuf_c_boolean has_tls;
    protobuf_c_boolean tls;
};
#define KINETIC_PROTO_P2_POPERATION__PEER__INIT \
 { PROTOBUF_C_MESSAGE_INIT (&KineticProto_p2_poperation__peer__descriptor) \
    , NULL, 0,0, 0,0 }

struct _KineticProto_P2POperation
{
    ProtobufCMessage base;
    KineticProto_P2POperation__Peer* peer;
    size_t n_operation;
    KineticProto_P2POperation__Operation** operation;
    protobuf_c_boolean has_allchildoperationssucceeded;
    protobuf_c_boolean allchildoperationssucceeded;
};
#define KINETIC_PROTO_P2_POPERATION__INIT \
 { PROTOBUF_C_MESSAGE_INIT (&KineticProto_p2_poperation__descriptor) \
    , NULL, 0,NULL, 0,0 }

struct _KineticProto_GetLog__Utilization
{
    ProtobufCMessage base;
    char* name;
    protobuf_c_boolean has_value;
    float value;
};
#define KINETIC_PROTO_GET_LOG__UTILIZATION__INIT \
 { PROTOBUF_C_MESSAGE_INIT (&KineticProto_get_log__utilization__descriptor) \
    , NULL, 0,0 }

struct _KineticProto_GetLog__Temperature
{
    ProtobufCMessage base;
    char* name;
    protobuf_c_boolean has_current;
    float current;
    protobuf_c_boolean has_minimum;
    float minimum;
    protobuf_c_boolean has_maximum;
    float maximum;
    protobuf_c_boolean has_target;
    float target;
};
#define KINETIC_PROTO_GET_LOG__TEMPERATURE__INIT \
 { PROTOBUF_C_MESSAGE_INIT (&KineticProto_get_log__temperature__descriptor) \
    , NULL, 0,0, 0,0, 0,0, 0,0 }

struct _KineticProto_GetLog__Capacity
{
    ProtobufCMessage base;
    protobuf_c_boolean has_nominalcapacityinbytes;
    uint64_t nominalcapacityinbytes;
    protobuf_c_boolean has_portionfull;
    float portionfull;
};
#define KINETIC_PROTO_GET_LOG__CAPACITY__INIT \
 { PROTOBUF_C_MESSAGE_INIT (&KineticProto_get_log__capacity__descriptor) \
    , 0,0, 0,0 }

struct _KineticProto_GetLog__Configuration__Interface
{
    ProtobufCMessage base;
    char* name;
    protobuf_c_boolean has_mac;
    ProtobufCBinaryData mac;
    protobuf_c_boolean has_ipv4address;
    ProtobufCBinaryData ipv4address;
    protobuf_c_boolean has_ipv6address;
    ProtobufCBinaryData ipv6address;
};
#define KINETIC_PROTO_GET_LOG__CONFIGURATION__INTERFACE__INIT \
 { PROTOBUF_C_MESSAGE_INIT (&KineticProto_get_log__configuration__interface__descriptor) \
    , NULL, 0,{0,NULL}, 0,{0,NULL}, 0,{0,NULL} }

struct _KineticProto_GetLog__Configuration
{
    ProtobufCMessage base;
    char* vendor;
    char* model;
    protobuf_c_boolean has_serialnumber;
    ProtobufCBinaryData serialnumber;
    protobuf_c_boolean has_worldwidename;
    ProtobufCBinaryData worldwidename;
    char* version;
    char* compilationdate;
    char* sourcehash;
    char* protocolversion;
    char* protocolcompilationdate;
    char* protocolsourcehash;
    size_t n_interface;
    KineticProto_GetLog__Configuration__Interface** interface;
    protobuf_c_boolean has_port;
    int32_t port;
    protobuf_c_boolean has_tlsport;
    int32_t tlsport;
};
#define KINETIC_PROTO_GET_LOG__CONFIGURATION__INIT \
 { PROTOBUF_C_MESSAGE_INIT (&KineticProto_get_log__configuration__descriptor) \
    , NULL, NULL, 0,{0,NULL}, 0,{0,NULL}, NULL, NULL, NULL, NULL, NULL, NULL, 0,NULL, 0,0, 0,0 }

struct _KineticProto_GetLog__Statistics
{
    ProtobufCMessage base;
    protobuf_c_boolean has_messagetype;
    KineticProto_MessageType messagetype;
    protobuf_c_boolean has_count;
    uint64_t count;
    protobuf_c_boolean has_bytes;
    uint64_t bytes;
};
#define KINETIC_PROTO_GET_LOG__STATISTICS__INIT \
 { PROTOBUF_C_MESSAGE_INIT (&KineticProto_get_log__statistics__descriptor) \
    , 0,0, 0,0, 0,0 }

struct _KineticProto_GetLog__Limits
{
    ProtobufCMessage base;
    protobuf_c_boolean has_maxkeysize;
    uint32_t maxkeysize;
    protobuf_c_boolean has_maxvaluesize;
    uint32_t maxvaluesize;
    protobuf_c_boolean has_maxversionsize;
    uint32_t maxversionsize;
    protobuf_c_boolean has_maxtagsize;
    uint32_t maxtagsize;
    protobuf_c_boolean has_maxconnections;
    uint32_t maxconnections;
    protobuf_c_boolean has_maxoutstandingreadrequests;
    uint32_t maxoutstandingreadrequests;
    protobuf_c_boolean has_maxoutstandingwriterequests;
    uint32_t maxoutstandingwriterequests;
    protobuf_c_boolean has_maxmessagesize;
    uint32_t maxmessagesize;
    protobuf_c_boolean has_maxkeyrangecount;
    uint32_t maxkeyrangecount;
};
#define KINETIC_PROTO_GET_LOG__LIMITS__INIT \
 { PROTOBUF_C_MESSAGE_INIT (&KineticProto_get_log__limits__descriptor) \
    , 0,0, 0,0, 0,0, 0,0, 0,0, 0,0, 0,0, 0,0, 0,0 }

struct _KineticProto_GetLog
{
    ProtobufCMessage base;
    size_t n_type;
    KineticProto_GetLog__Type* type;
    size_t n_utilization;
    KineticProto_GetLog__Utilization** utilization;
    size_t n_temperature;
    KineticProto_GetLog__Temperature** temperature;
    KineticProto_GetLog__Capacity* capacity;
    KineticProto_GetLog__Configuration* configuration;
    size_t n_statistics;
    KineticProto_GetLog__Statistics** statistics;
    protobuf_c_boolean has_messages;
    ProtobufCBinaryData messages;
    KineticProto_GetLog__Limits* limits;
};
#define KINETIC_PROTO_GET_LOG__INIT \
 { PROTOBUF_C_MESSAGE_INIT (&KineticProto_get_log__descriptor) \
    , 0,NULL, 0,NULL, 0,NULL, NULL, NULL, 0,NULL, 0,{0,NULL}, NULL }

struct _KineticProto_Security__ACL__Scope
{
    ProtobufCMessage base;
    protobuf_c_boolean has_offset;
    int64_t offset;
    protobuf_c_boolean has_value;
    ProtobufCBinaryData value;
    size_t n_permission;
    KineticProto_Security__ACL__Permission* permission;
    protobuf_c_boolean has_tlsrequired;
    protobuf_c_boolean tlsrequired;
};
#define KINETIC_PROTO_SECURITY__ACL__SCOPE__INIT \
 { PROTOBUF_C_MESSAGE_INIT (&KineticProto_security__acl__scope__descriptor) \
    , 0,0, 0,{0,NULL}, 0,NULL, 0,0 }

struct _KineticProto_Security__ACL
{
    ProtobufCMessage base;
    protobuf_c_boolean has_identity;
    int64_t identity;
    protobuf_c_boolean has_key;
    ProtobufCBinaryData key;
    protobuf_c_boolean has_hmacalgorithm;
    KineticProto_Security__ACL__HMACAlgorithm hmacalgorithm;
    size_t n_scope;
    KineticProto_Security__ACL__Scope** scope;
};
#define KINETIC_PROTO_SECURITY__ACL__INIT \
 { PROTOBUF_C_MESSAGE_INIT (&KineticProto_security__acl__descriptor) \
    , 0,0, 0,{0,NULL}, 0,0, 0,NULL }

struct _KineticProto_Security
{
    ProtobufCMessage base;
    size_t n_acl;
    KineticProto_Security__ACL** acl;
};
#define KINETIC_PROTO_SECURITY__INIT \
 { PROTOBUF_C_MESSAGE_INIT (&KineticProto_security__descriptor) \
    , 0,NULL }

struct _KineticProto
{
    ProtobufCMessage base;
    KineticProto_Command* command;
    protobuf_c_boolean has_hmac;
    ProtobufCBinaryData hmac;
};
#define KINETIC_PROTO_INIT \
 { PROTOBUF_C_MESSAGE_INIT (&KineticProto_descriptor) \
    , NULL, 0,{0,NULL} }

/* KineticProto_Command methods */
void KineticProto_command__init(KineticProto_Command* message);
/* KineticProto_Header methods */
void KineticProto_header__init(KineticProto_Header* message);
/* KineticProto_Body methods */
void KineticProto_body__init(KineticProto_Body* message);
/* KineticProto_Status methods */
void KineticProto_status__init(KineticProto_Status* message);
/* KineticProto_KeyValue methods */
void KineticProto_key_value__init(KineticProto_KeyValue* message);
/* KineticProto_Range methods */
void KineticProto_range__init(KineticProto_Range* message);
/* KineticProto_Setup methods */
void KineticProto_setup__init(KineticProto_Setup* message);
/* KineticProto_P2POperation__Operation methods */
void KineticProto_p2_poperation__operation__init(KineticProto_P2POperation__Operation* message);
/* KineticProto_P2POperation__Peer methods */
void KineticProto_p2_poperation__peer__init(KineticProto_P2POperation__Peer* message);
/* KineticProto_P2POperation methods */
void KineticProto_p2_poperation__init(KineticProto_P2POperation* message);
/* KineticProto_GetLog__Utilization methods */
void KineticProto_get_log__utilization__init(KineticProto_GetLog__Utilization* message);
/* KineticProto_GetLog__Temperature methods */
void KineticProto_get_log__temperature__init(KineticProto_GetLog__Temperature* message);
/* KineticProto_GetLog__Capacity methods */
void KineticProto_get_log__capacity__init(KineticProto_GetLog__Capacity* message);
/* KineticProto_GetLog__Configuration__Interface methods */
void KineticProto_get_log__configuration__interface__init(KineticProto_GetLog__Configuration__Interface* message);
/* KineticProto_GetLog__Configuration methods */
void KineticProto_get_log__configuration__init(KineticProto_GetLog__Configuration* message);
/* KineticProto_GetLog__Statistics methods */
void KineticProto_get_log__statistics__init(KineticProto_GetLog__Statistics* message);
/* KineticProto_GetLog__Limits methods */
void KineticProto_get_log__limits__init(KineticProto_GetLog__Limits* message);
/* KineticProto_GetLog methods */
void KineticProto_get_log__init(KineticProto_GetLog* message);
/* KineticProto_Security__ACL__Scope methods */
void KineticProto_security__acl__scope__init(KineticProto_Security__ACL__Scope* message);
/* KineticProto_Security__ACL methods */
void KineticProto_security__acl__init(KineticProto_Security__ACL* message);
/* KineticProto_Security methods */
void KineticProto_security__init(KineticProto_Security* message);
/* KineticProto methods */
void KineticProto_init(KineticProto* message);
size_t KineticProto_get_packed_size(const KineticProto* message);
size_t KineticProto_pack(const KineticProto* message, uint8_t* out);
size_t KineticProto_pack_to_buffer(const KineticProto* message, ProtobufCBuffer* buffer);
KineticProto* KineticProto_unpack(ProtobufCAllocator* allocator, size_t len, const uint8_t* data);
void KineticProto_free_unpacked(KineticProto* message, ProtobufCAllocator* allocator);
/* --- per-message closures --- */

typedef void (*KineticProto_Command_Closure)(const KineticProto_Command* message, void* closure_data);
typedef void (*KineticProto_Header_Closure)(const KineticProto_Header* message, void* closure_data);
typedef void (*KineticProto_Body_Closure)(const KineticProto_Body* message, void* closure_data);
typedef void (*KineticProto_Status_Closure)(const KineticProto_Status* message, void* closure_data);
typedef void (*KineticProto_KeyValue_Closure)(const KineticProto_KeyValue* message, void* closure_data);
typedef void (*KineticProto_Range_Closure)(const KineticProto_Range* message, void* closure_data);
typedef void (*KineticProto_Setup_Closure)(const KineticProto_Setup* message, void* closure_data);
typedef void (*KineticProto_P2POperation__Operation_Closure)(const KineticProto_P2POperation__Operation* message, void* closure_data);
typedef void (*KineticProto_P2POperation__Peer_Closure)(const KineticProto_P2POperation__Peer* message, void* closure_data);
typedef void (*KineticProto_P2POperation_Closure)(const KineticProto_P2POperation* message, void* closure_data);
typedef void (*KineticProto_GetLog__Utilization_Closure)(const KineticProto_GetLog__Utilization* message, void* closure_data);
typedef void (*KineticProto_GetLog__Temperature_Closure)(const KineticProto_GetLog__Temperature* message, void* closure_data);
typedef void (*KineticProto_GetLog__Capacity_Closure)(const KineticProto_GetLog__Capacity* message, void* closure_data);
typedef void (*KineticProto_GetLog__Configuration__Interface_Closure)(const KineticProto_GetLog__Configuration__Interface* message, void* closure_data);
typedef void (*KineticProto_GetLog__Configuration_Closure)(const KineticProto_GetLog__Configuration* message, void* closure_data);
typedef void (*KineticProto_GetLog__Statistics_Closure)(const KineticProto_GetLog__Statistics* message, void* closure_data);
typedef void (*KineticProto_GetLog__Limits_Closure)(const KineticProto_GetLog__Limits* message, void* closure_data);
typedef void (*KineticProto_GetLog_Closure)(const KineticProto_GetLog* message, void* closure_data);
typedef void (*KineticProto_Security__ACL__Scope_Closure)(const KineticProto_Security__ACL__Scope* message, void* closure_data);
typedef void (*KineticProto_Security__ACL_Closure)(const KineticProto_Security__ACL* message, void* closure_data);
typedef void (*KineticProto_Security_Closure)(const KineticProto_Security* message, void* closure_data);
typedef void (*KineticProto_Closure)(const KineticProto* message, void* closure_data);

/* --- services --- */

/* --- descriptors --- */

extern const ProtobufCMessageDescriptor KineticProto_descriptor;
extern const ProtobufCMessageDescriptor KineticProto_command__descriptor;
extern const ProtobufCMessageDescriptor KineticProto_header__descriptor;
extern const ProtobufCMessageDescriptor KineticProto_body__descriptor;
extern const ProtobufCMessageDescriptor KineticProto_status__descriptor;
extern const ProtobufCEnumDescriptor    KineticProto_status__status_code__descriptor;
extern const ProtobufCMessageDescriptor KineticProto_key_value__descriptor;
extern const ProtobufCMessageDescriptor KineticProto_range__descriptor;
extern const ProtobufCMessageDescriptor KineticProto_setup__descriptor;
extern const ProtobufCMessageDescriptor KineticProto_p2_poperation__descriptor;
extern const ProtobufCMessageDescriptor KineticProto_p2_poperation__operation__descriptor;
extern const ProtobufCMessageDescriptor KineticProto_p2_poperation__peer__descriptor;
extern const ProtobufCMessageDescriptor KineticProto_get_log__descriptor;
extern const ProtobufCMessageDescriptor KineticProto_get_log__utilization__descriptor;
extern const ProtobufCMessageDescriptor KineticProto_get_log__temperature__descriptor;
extern const ProtobufCMessageDescriptor KineticProto_get_log__capacity__descriptor;
extern const ProtobufCMessageDescriptor KineticProto_get_log__configuration__descriptor;
extern const ProtobufCMessageDescriptor KineticProto_get_log__configuration__interface__descriptor;
extern const ProtobufCMessageDescriptor KineticProto_get_log__statistics__descriptor;
extern const ProtobufCMessageDescriptor KineticProto_get_log__limits__descriptor;
extern const ProtobufCEnumDescriptor    KineticProto_get_log__type__descriptor;
extern const ProtobufCMessageDescriptor KineticProto_security__descriptor;
extern const ProtobufCMessageDescriptor KineticProto_security__acl__descriptor;
extern const ProtobufCMessageDescriptor KineticProto_security__acl__scope__descriptor;
extern const ProtobufCEnumDescriptor    KineticProto_security__acl__hmacalgorithm__descriptor;
extern const ProtobufCEnumDescriptor    KineticProto_security__acl__permission__descriptor;
extern const ProtobufCEnumDescriptor    KineticProto_synchronization__descriptor;
extern const ProtobufCEnumDescriptor    KineticProto_algorithm__descriptor;
extern const ProtobufCEnumDescriptor    KineticProto_message_type__descriptor;

PROTOBUF_C__END_DECLS

#endif  /* _KINETIC_PROTO_H */