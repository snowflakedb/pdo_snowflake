/*
* Copyright (c) 2018-2019 Snowflake Computing
*/

#ifndef PROJECT_SF_OCSP_TELEMETRY_DATA_H
#define PROJECT_SF_OCSP_TELEMETRY_DATA_H

typedef struct ocsp_telemetry_data
{
  char event_type[20];
  char sfc_peer_host[1024];
  char cert_id[1024];
  char ocsp_req_b64[4096];
  char ocsp_responder_url[1024];
  char error_msg[4096];
  int insecure_mode;
  int failopen_mode;
  int cache_enabled;
  int cache_hit;
}SF_OTD;

#endif //PROJECT_SF_OCSP_TELEMETRY_DATA_H
