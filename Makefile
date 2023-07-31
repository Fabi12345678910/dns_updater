CFLAGS += -std=c11 -Wall -Wextra -pedantic -pthread -g -O3
LDFLAGS += -pthread

# Add the names of your executables here
TARGETS = dns_updater

#PROVIDERS = cloudflare

THREAD_FUNCTIONS_O = updater.o timer.o http_server.o

GETTERS_O = local_interface_data.o whats_my_ip.o

.PHONY: all
all: $(TARGETS)

.PHONY: clean
clean:
	$(RM) $(TARGETS) *.o

dns_updater: dns_updater.o implementation_testing.o configuration_reader.o linked_list.o circular_array.o configuration_reader_common.o cloudflare.o call_curl.o $(THREAD_FUNCTIONS_O) $(GETTERS_O)
	gcc -o dns_updater $(CFLAGS) dns_updater.o implementation_testing.o configuration_reader.o linked_list.o circular_array.o configuration_reader_common.o cloudflare.o call_curl.o $(THREAD_FUNCTIONS_O) $(GETTERS_O)

dns_updater.o: dns_updater.c
	gcc -c $(CFLAGS) dns_updater.c

implementation_testing.o: implementation_testing.c
	gcc -c $(CFLAGS) implementation_testing.c

configuration_reader.o: helper_functions/configuration_reader.c
	gcc -c $(CFLAGS) helper_functions/configuration_reader.c

linked_list.o: helper_functions/linked_list.c
	gcc -c $(CFLAGS) helper_functions/linked_list.c

circular_array.o: helper_functions/circular_array.c
	gcc -c $(CFLAGS) helper_functions/circular_array.c

call_curl.o: helper_functions/call_curl.c
	gcc -c $(CFLAGS) helper_functions/call_curl.c

configuration_reader_common.o: helper_functions/configuration_reader_common.c
	gcc -c $(CFLAGS) helper_functions/configuration_reader_common.c

cloudflare.o: helper_functions/providers/cloudflare.c
	gcc -c $(CFLAGS) helper_functions/providers/cloudflare.c

updater.o: thread_functions/updater.c
	gcc -c $(CFLAGS) thread_functions/updater.c

timer.o: thread_functions/timer.c
	gcc -c $(CFLAGS) thread_functions/timer.c

http_server.o: thread_functions/http_server.c
	gcc -c $(CFLAGS) thread_functions/http_server.c

local_interface_data.o: helper_functions/ipv6_getters/local_interface_data.c
	gcc -c $(CFLAGS) helper_functions/ipv6_getters/local_interface_data.c

whats_my_ip.o: helper_functions/ipv4_getters/whats_my_ip.c
	gcc -c $(CFLAGS) helper_functions/ipv4_getters/whats_my_ip.c