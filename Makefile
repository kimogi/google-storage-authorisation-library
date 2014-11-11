#root Makefile

export CC:=gcc
export CFLAGS += -g
export LIBS := -lrt -lcurl -lcrypto -lm

SUBDIRS = prop_tool google_storage oauth
.PHONY: subdirs $(SUBDIRS) clean all

all: google_storage_utils

subdirs: $(SUBDIRS)	
$(SUBDIRS):	
		$(MAKE) -C $@

utils.o: utils.c
	$(CC) $(CFLAGS) $(LDFLAGS) -c utils.c $(LIBS)

google_storage_utils: subdirs utils.o
	$(CC) $(CFLAGS) $(LDFLAGS) -o google_storage_utils utils.o google_storage/google_storage_tool.o oauth/oauth_tool.o oauth/base64url.o oauth/rsa.o prop_tool/prop.o $(LIBS)

clean:
	rm -rf *.o
	rm -rf google_storage_utils
	for d in $(SUBDIRS); do (cd $$d; $(MAKE) clean ); done
