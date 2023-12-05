# dns_updater
Just another dns updater

# "special" functions
e.g. why did i built this?

- ipv6 detection using local interface data
- "solid" c coded build, not as fishy as shell scripts
- integrated webserver with following abilities:
    - nice looking status website, reachable via /state
    - redirect from / to /state
    - health-check on /health-check
    - manual dns update trigger on /dns-update
- ability to read config from file or cli-parameter

# configuration
like said the configuration can be provided via either a configuration file or cli params,

to run the updater, either use either one of:\
`dns_updater file "path/to/file"`\
`dns_updater cli "<configuration_data>"`\
the file must contain the configuration data

## configuration data
configuration data must be provided in a json similar format:

simple value:

```
key: value
```

or structured value:

```
key: {
    subkey1: value,
    subkey2: value
}
```

or a list:

```
key: [
    ... ,
    ... ,
    ...
]
```

before and after values, before and after seperating commas, one may use as many whitespace characters as desired.
whitespace characters are space, tab, \r, \n, and commas(,)

after a value, there must follow any whitespace character

however, do NOT parathrise any values if not specified otherwise, the configuration reader is quite fidely and will not accept stuff like "

### values
values must be provided in the following manner:

#### boolean
simple, true or false

#### string
concatenation of any alphanumeric characters, underlines, lines and commas 

#### dnsEntries
a structured value with the following contents:
- type = string(max 4), type of the dnsEntry, e.g. AAAA
- name = string(max 255), dns-name, e.g. subdomain.domain.com
- provider = string, either cloudflare or dummy(returns dummy data, only use for testing)
- providerData(optional) = depending on provider:
    - dummy: not allowed, will fail if providing providerData
    - cloudflare: string(max 40), the api key(can be received from the cloudflare dashboard)

## configuration options
the following keys are allowed:

- enableHTTPServer = boolean, default: true
- enableIPv4 = boolean, default: true
- enableIPv6 = boolean, default: true
- dnsEntries = a list of dnsEntries

### example configuration

```
enableHTTPServer: true
enableIPv4: true
enableIPv6: true
dnsEntries: [
    {
        name: subdomain.domain.com,
        type: A,
        provider: cloudflare,
        providerData: 0123456789012345678901234567890123456789
    },
    {
        name: subdomain.domain.com,
        type: AAAA,
        provider: cloudflare,
        providerData: abcdef0123abcdef0123abcdef0123abcdef0123
    },
    {
        name: test.com,
        type: A,
        provider: dummy
    }
] 
```


# webserver
yeah, this c code actually includes a webserver\
try the state page at /state, cause why not

but DON'T make this available to the public internet, it does not support https\
also, badass hackers may call /dns-update to dos-attack you

the state page should also not be routed directly to the internet, since there is only one http worker thread
if a bad boy starts a http request and keeps the tcp connection alive, the server can't satisfy any other request

to make the webserver safely available to the internet, use any of the following options:

- reverse-proxy with authorization
- vpn
- any other measure to restrict access to the webserver

# how it works(in short)
the main thread initializes 3 workers: 
- webserver
- timer
- updater

## updater
this thread waits for a trigger from the other threads to start updating.\
when triggered, it retrieves the current ip adresses.\
then all dns_entries are updated according to their provider

## timer
really simple, triggers the updater all 10 Minutes

## webserver
serves the following sites: 

### /
a redirect to /state

### /state
a graphical overview of the state

### /dns-update
triggers the updater after some secs, may be useful for routers when receiving a new ip

### /health-check
simple health-check, returns 200 if all is fine and 500 if there is a error

# installing and running

requirements for compiling:
- gcc
- make

requirements for running:
- curl
- dig

## compiling
in the main directory, simply run `make` to build the executable `./dns_updater`.

### changing webserver port number
default http server listens to port 8008.\
when you require the updater to use another port, compile using the following option:\
`make EXTRA_CFLAGS=-DPORT_NUMBER=<insert_port_number>`

### changing the internet adapter used for IPv6 address
`make EXTRA_CFLAGS=-DIPV6_INTERNET_DEVICE=\\\"<insert_device_eg_eth0>\\\"`

### disabling the logging when nothing changed
`make EXTRA_CFLAGS=-DLOG_UNCHANGED=0`

## running
well, just go for it and provide configuration data as mentioned in "configuration"

### stopping
simply press ctrl + c or send SIGINT otherwise to the process for a clean shutdown \
current known problem: the weberver socket may be unusable for some seconds