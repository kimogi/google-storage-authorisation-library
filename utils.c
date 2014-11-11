#include <stdarg.h>
#include "google_storage/google_storage_tool.h"

#define FILE_KEY "-f"
#define CONFIG_KEY "-c"

#define SUCCESS 0
#define FAILURE -1
#define DEFAULT_PROP_FILE "/etc/blix/blix_google.config"

void send_file(char *file_path, char *prop_file_path);
void print_help();
void print_usage_error(const char *format, ...);
int gain_props(char *prop_file_path, struct google_storage_props **props);
void free_props(struct google_storage_props *props);

int main(int argc, char **argv) {
	if(1 >= argc) {
		print_usage_error("Unexpected number of arguments");
		return EXIT_SUCCESS;
	}
	
	char *prop_file_path = DEFAULT_PROP_FILE;
	char *file_path;
  int c_argc = argc - 1;
  char **c_argv = argv + 1;

	if(2 <= c_argc && 0 == strcmp(*(c_argv), CONFIG_KEY)) {
		prop_file_path = *(c_argv + 1);
		c_argc -= 2;
		c_argv += 2;
	}
	
	if(2 <= c_argc && 0 == strcmp(*c_argv, FILE_KEY)) {
    file_path = *(c_argv + 1);  
    send_file(file_path, prop_file_path);

	} else {
		print_usage_error("Illigal number of arguments");
	}

	return EXIT_SUCCESS;
}

void send_file(char *file_path, char *prop_file_path) {
	struct google_storage_props *props = NULL;

	if(FAILURE == gain_props(prop_file_path, &props)) {
    puts("Failed to load kinesis props");
    return;
  }

  upload_file(file_path, props);

	free_props(props);
}

void print_usage_error(const char *format, ...) {
	va_list arg;

	va_start(arg, format);
	puts("ERROR : ");
	vfprintf(stdout, format, arg);
   	va_end (arg);
	puts("\n");
	
	print_help();
}

void print_help() {
	printf("\nUsage :\n	google_storage_utils [-c path to config file] -f <file name>\n");
}

int gain_props(char *prop_file_path, struct google_storage_props **props) {
  *props = (struct google_storage_props *)calloc(1, sizeof(struct google_storage_props));

  int res;

  if(FAILURE == (res = get_prop(prop_file_path, GOOGLE_STORAGE_CLIENT_ID_PROP, &((*props)->client_id), GOOGLE_STORAGE_CLIENT_ID_LEN))) {
    printf("No such property found : %s\n", GOOGLE_STORAGE_CLIENT_ID_PROP);
  }
  if(FAILURE == (res = get_prop(prop_file_path, GOOGLE_STORAGE_EMAIL_ADDRESS_PROP, &((*props)->email_address), GOOGLE_STORAGE_EMAIL_ADDRESS_LEN))) {
    printf("No such property found : %s\n", GOOGLE_STORAGE_EMAIL_ADDRESS_PROP);
  }
  if(FAILURE == (res = get_prop(prop_file_path, GOOGLE_STORAGE_PRIVATE_KEY_PATH_PROP, &((*props)->private_key_path), GOOGLE_STORAGE_PRIVATE_KEY_PATH_LEN))) {
    printf("No such property found : %s\n", GOOGLE_STORAGE_PRIVATE_KEY_PATH_PROP);
  }
  if(FAILURE == (res = get_prop(prop_file_path, GOOGLE_STORAGE_BUCKET_NAME_PROP, &((*props)->bucket_name), GOOGLE_STORAGE_BUCKET_NAME_LEN))) {
    printf("No such property found : %s\n", GOOGLE_STORAGE_BUCKET_NAME_PROP);
  }
  if(FAILURE == (res = get_prop(prop_file_path, GOOGLE_STORAGE_UPLOAD_TYPE_PROP, &((*props)->upload_type), GOOGLE_STORAGE_UPLOAD_TYPE_LEN))) {
    printf("No such property found : %s\n", GOOGLE_STORAGE_UPLOAD_TYPE_PROP);
  }
  if(FAILURE == (res = get_prop(prop_file_path, GOOGLE_STORAGE_CONTENT_TYPE_PROP, &((*props)->content_type), GOOGLE_STORAGE_CONTENT_TYPE_LEN))) {
    printf("No such property found : %s\n", GOOGLE_STORAGE_CONTENT_TYPE_PROP);
  }

  get_prop(prop_file_path, GOOGLE_STORAGE_CONNECTION_TIMEOUT_PROP, &((*props)->connection_timeout), GOOGLE_STORAGE_CONNECTION_TIMEOUT_LEN);
  get_prop(prop_file_path, GOOGLE_STORAGE_TIMEOUT_PROP, &((*props)->timeout), GOOGLE_STORAGE_TIMEOUT_LEN); 
 
  return res;
}

void free_props(struct google_storage_props *props) {
  if(NULL != props) {
    if(NULL != props->client_id)
      free(props->client_id);
    if(NULL != props->email_address)
      free(props->email_address);
    if(NULL != props->private_key_path)
      free(props->private_key_path);
    if(NULL != props->bucket_name)
      free(props->bucket_name);
    if(NULL != props->upload_type)
      free(props->upload_type);
    if(NULL != props->content_type)
      free(props->content_type);
    if(NULL != props->connection_timeout)
      free(props->connection_timeout);
    if(NULL != props->timeout)
      free(props->timeout);

    free(props);
  }
}
