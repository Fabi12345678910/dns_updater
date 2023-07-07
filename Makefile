CFLAGS += -std=c11 -Wall -Wextra -pedantic -pthread -g -O3
LDFLAGS += -pthread

# Add the names of your executables here
TARGETS = dns_updater

//PROVIDERS = cloudflare

.PHONY: all
all: $(TARGETS)

.PHONY: clean
clean:
	$(RM) $(TARGETS) *.o

dns_updater: dns_updater.o implementation_testing.o configuration_reader.o linked_list.o configuration_reader_common.o cloudflare.o
	gcc -o dns_updater $(CFLAGS) dns_updater.o implementation_testing.o configuration_reader.o linked_list.o configuration_reader_common.o cloudflare.o

dns_updater.o: dns_updater.c
	gcc -c $(CFLAGS) dns_updater.c

implementation_testing.o: implementation_testing.c
	gcc -c $(CFLAGS) implementation_testing.c

configuration_reader.o: helper_functions/configuration_reader.c
	gcc -c $(CFLAGS) helper_functions/configuration_reader.c

linked_list.o: helper_functions/linked_list.c
	gcc -c $(CFLAGS) helper_functions/linked_list.c

configuration_reader_common.o: helper_functions/configuration_reader_common.c
	gcc -c $(CFLAGS) helper_functions/configuration_reader_common.c

cloudflare.o: helper_functions/providers/cloudflare.c
	gcc -c $(CFLAGS) helper_functions/providers/cloudflare.c