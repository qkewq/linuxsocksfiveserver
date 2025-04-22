Work in progress

SOCKS5 server for linux  
I have no idea what I'm doing


SOCKS5 Linux Proxy Server Checklist

- Parse config file
  - Get methods
  - Get method priorities
  - Get listener IP and port
  - Support IPv4 and IPv6
- Establish listener
- Establish inbound connection
- SOCKS5 handshake
  - Support IPv4 and IPv6 request
  - Support connect command
- Establish outbound connection
- SOCKS5 reply
  - Reply proper error code on error
  - Kill connection within 10 seconds on error
- Start passing data
  - epoll
- Make systemd compatible
  - Service file
  - Service user
  - Install shell script

- Optional
  - Support user/pass method
  - Incorporate TLS
  - Support domain names
  - Support BIND command
  - Support UDP ASSOCIATE command

- Double Optional
  - Make code readable