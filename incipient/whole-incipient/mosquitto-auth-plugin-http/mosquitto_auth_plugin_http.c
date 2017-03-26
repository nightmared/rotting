#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>

#include <curl/curl.h>
#include <json.h>
#include <mosquitto.h>
#include <mosquitto_plugin.h>

static char *http_user_uri = "http://localhost:8080/doors/check";
static char *http_acl_uri = "http://localhost:8080/doors/login";

int mosquitto_auth_plugin_version(void) {
  return MOSQ_AUTH_PLUGIN_VERSION;
}

int mosquitto_auth_plugin_init(void **user_data, struct mosquitto_auth_opt *auth_opts, int auth_opt_count) {
  for (int i = 0; i < auth_opt_count; i++) {
    if (strncmp(auth_opts[i].key, "http_user_uri", 13) == 0)
      http_user_uri = auth_opts[i].value;

    if (strncmp(auth_opts[i].key, "http_acl_uri", 12) == 0)
      http_acl_uri = auth_opts[i].value;
  }
  mosquitto_log_printf(MOSQ_LOG_INFO, "mosquitto_auth_plugin_http started: http_user_uri = %s, http_acl_uri = %s", http_user_uri, http_acl_uri);
  return MOSQ_ERR_SUCCESS;
}

int mosquitto_auth_plugin_cleanup(void *user_data, struct mosquitto_auth_opt *auth_opts, int auth_opt_count) {
  return MOSQ_ERR_SUCCESS;
}

int mosquitto_auth_security_init(void *user_data, struct mosquitto_auth_opt *auth_opts, int auth_opt_count, bool reload) {
  return MOSQ_ERR_SUCCESS;
}

int mosquitto_auth_security_cleanup(void *user_data, struct mosquitto_auth_opt *auth_opts, int auth_opt_count, bool reload) {
  return MOSQ_ERR_SUCCESS;
}

const char* generate_auth_tuple(const char* user, const char* pass) {
  json_object *query = json_object_new_object();
  json_object_object_add(query, "id", json_object_new_int(atoi(user)));
  json_object_object_add(query, "password", json_object_new_string(pass));
  const char* post_data = json_object_to_json_string(query);
  free(query);
  return post_data;
}

const char* generate_topic_tuple(const char* user, const char* topic) {
  json_object *query = json_object_new_object();
  json_object_object_add(query, "id", json_object_new_int(atoi(user)));
  json_object_object_add(query, "topic", json_object_new_string(topic));
  const char* post_data = json_object_to_json_string(query);
  free(query);
  return post_data;
}

int mosquitto_auth_unpwd_check(void *user_data, const char *username, const char *password) {
  if (username == NULL || password == NULL) {
    return MOSQ_ERR_AUTH;
  }

  CURL *curl = curl_easy_init();

  if (curl == NULL) {
    mosquitto_log_printf(MOSQ_LOG_WARNING, "failed to initialize curl (curl_easy_init AUTH): %s", strerror(errno));
    return MOSQ_ERR_AUTH;
  }

  curl_easy_setopt(curl, CURLOPT_URL, http_user_uri);
  curl_easy_setopt(curl, CURLOPT_POSTFIELDS, generate_auth_tuple(username, password));

  int http_status = -1;
  if (curl_easy_perform(curl) == CURLE_OK)
    curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &http_status);

  curl_easy_cleanup(curl);
  mosquitto_log_printf(MOSQ_LOG_DEBUG, "http_status=%i", http_status);
  return (http_status == 200 ? MOSQ_ERR_SUCCESS : MOSQ_ERR_AUTH);
}

int mosquitto_auth_acl_check(void *user_data, const char *clientid, const char *username, const char *topic, int access) {

  CURL *curl = curl_easy_init();

  if (curl == NULL) {
    mosquitto_log_printf(MOSQ_LOG_WARNING, "failed to initialize curl (curl_easy_init AUTH): %s", strerror(errno));
    return MOSQ_ERR_AUTH;
  }

  curl_easy_setopt(curl, CURLOPT_URL, http_acl_uri);
  curl_easy_setopt(curl, CURLOPT_POSTFIELDS, generate_topic_tuple(username, topic));

  int http_status = -1;
  if (curl_easy_perform(curl) == CURLE_OK)
    curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &http_status);

  curl_easy_cleanup(curl);
  mosquitto_log_printf(MOSQ_LOG_DEBUG, "http_status=%i", http_status);
  return (http_status == 200 ? MOSQ_ERR_SUCCESS : MOSQ_ERR_AUTH);
}

int mosquitto_auth_psk_key_get(void *user_data, const char *hint, const char *identity, char *key, int max_key_len) {
  return MOSQ_ERR_AUTH;
}
