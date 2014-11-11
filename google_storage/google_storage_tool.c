#include "google_storage_tool.h"

const char *SCOPE = "https://www.googleapis.com/auth/devstorage.read_write";
const char *AUTH_TOKEN_NEEDLE = "\"access_token\" : \"";
const char *UPLOAD_URL_FORMAT = "https://www.googleapis.com/upload/storage/v1/b/%s/o?uploadType=%s&name=%s";

struct google_storage_props *props;

size_t parse_auth_response_callback(char *contents, size_t length, size_t nmemb, void *userp);
int authorized_upload_file(char *file_path, char *auth_token, long connection_timeout, long timeout);
void mine_file_name(char *file_path, char *file_name);

void upload_file(char *file_path, struct google_storage_props *google_props) { 
  props = google_props;
  request_auth_token(props, SCOPE, props->private_key_path, &parse_auth_response_callback, (void *)file_path);
}

size_t parse_auth_response_callback(char *contents, size_t length, size_t nmemb, void *userp) {  
  size_t real_size = length * nmemb;
  
  char *content_from_auth_token_needle = strstr(contents, AUTH_TOKEN_NEEDLE);
  char *content_from_auth_token = (char *)calloc(strlen(content_from_auth_token_needle) - strlen(AUTH_TOKEN_NEEDLE) + 1, sizeof(char));
  memmove(content_from_auth_token, content_from_auth_token_needle + strlen(AUTH_TOKEN_NEEDLE), strlen(content_from_auth_token_needle) - strlen(AUTH_TOKEN_NEEDLE));
  
  char *auth_token_termination_pos = strchr(content_from_auth_token, '"');   
  int auth_token_len = (int)(auth_token_termination_pos - content_from_auth_token);
  
  char *auth_token = (char *)calloc(auth_token_len + 1, sizeof(char));
  memcpy(auth_token, content_from_auth_token, auth_token_len);

  long connection_timeout = props->connection_timeout != NULL ? (long)atoi(props->connection_timeout) : 0;
  long timeout = props->timeout != NULL ? (long)atoi(props->timeout) : 0;

  if (-1 == authorized_upload_file((char *)userp, auth_token, connection_timeout, timeout)) {
    perror("Failed to upload file");
  }
  
  free(content_from_auth_token);
  return real_size;
}

int authorized_upload_file(char *file_path, char *auth_token, long connection_timeout, long timeout) {
  CURL *curl;
  CURLcode response;

  struct curl_httppost *formpost = NULL;

  curl = curl_easy_init();
  if(!curl) {
    perror("Failed to get curl handle");
    return -1;
  }

  char file_name[256];
  mine_file_name(file_path, file_name); 

  FILE *file = fopen(file_path,"rb");
  if(file == NULL) {
    perror("Error while opening the file");
    return -1;
  }

  struct stat file_info;
  if(fstat(fileno(file), &file_info) != 0) {
    perror("Invalid file to load");
    return -1;
  }

  char *upload_url = (char *)calloc(strlen(UPLOAD_URL_FORMAT) + strlen(props->upload_type) + strlen(props->bucket_name) + strlen(file_name) + 1, sizeof(char));
  sprintf(upload_url, UPLOAD_URL_FORMAT, props->bucket_name, props->upload_type, file_name);

  curl_easy_setopt(curl, CURLOPT_URL, upload_url);
  curl_easy_setopt(curl, CURLOPT_POST, 1L);
  curl_easy_setopt(curl, CURLOPT_READDATA, file);
  curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE, file_info.st_size);
  curl_easy_setopt(curl, CURLOPT_TCP_NODELAY, 1L);
  curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 0);
  curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0);
  curl_easy_setopt(curl, CURLOPT_VERBOSE, 1L);
  curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT, connection_timeout);
  curl_easy_setopt(curl, CURLOPT_TIMEOUT, timeout);

  char *auth_header = (char *)calloc(strlen(auth_token) + strlen("Authorization: Bearer "), sizeof(char));
  sprintf(auth_header, "Authorization: Bearer %s", auth_token);

  char *content_length_header = (char *)calloc(strlen("Content-Length: ") + 10 + 1, sizeof(char));
  sprintf(content_length_header, "Content-Length: %d", file_info.st_size);

  char *content_type_header = (char *)calloc(strlen("Content-Type: ") + strlen(props->content_type) + 1, sizeof(char));
  sprintf(content_type_header, "Content-Type: %s", props->content_type);

  struct curl_slist *curl_headers = NULL;
  curl_headers = curl_slist_append(curl_headers, content_type_header);
  curl_headers = curl_slist_append(curl_headers, content_length_header);  
  curl_headers = curl_slist_append(curl_headers, auth_header);
  curl_headers = curl_slist_append(curl_headers, "Expect: ");
  curl_easy_setopt(curl, CURLOPT_HTTPHEADER, curl_headers); 

  response = curl_easy_perform(curl);

  fclose(file);
  free(upload_url);
  free(content_length_header);
  free(content_type_header);
  free(auth_header);
  curl_slist_free_all(curl_headers);
  curl_easy_cleanup(curl);
  curl_global_cleanup();

  if(response != CURLE_OK) {
    fprintf(stderr, "curl_easy_perform() failed with code %d : %s\n", response, curl_easy_strerror(response));
    return -1;
  }
  return 0;
}

void mine_file_name(char *file_path, char *file_name) {
  char *token_pointer = strtok(file_path, "/");
  char *last_token = token_pointer;
  while (token_pointer != NULL) {
    last_token = token_pointer;
    token_pointer = strtok(NULL, "/");
  }
  memcpy(file_name, last_token, strlen(last_token));
  memset(file_name + strlen(last_token), '\0', 1);
}
