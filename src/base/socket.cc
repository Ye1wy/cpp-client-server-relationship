#include "include/socket.h"

chain::Socket::Socket(FileDescriptorType fd, AddressFamily address_family,
                      SocketType socket_type, ProtocolType protocol) {
  fd_ = socket(address_family, socket_type, protocol);

  if (fd_ < 0) {
    throw "Socket failed";
  }

  int status;
  status = inet_pton(address_family, "127.0.0.1", &address_f_.sin_addr);

  if (status == 0) {
    throw "Invalide address";
  }

  address_f_.sin_family = address_family;
  address_f_.sin_port = htons(PORT);
  addrlen = sizeof(address_f_);
}

void chain::Socket::Bind() {
  int status =
      bind(fd_, reinterpret_cast<sockaddr*>(&address_f_), sizeof(address_f_));

  if (status < 0) {
    throw "Bind failed";
  }
}

void chain::Socket::Listen() {
  int status = listen(fd_, 3);

  if (status < 0) {
    throw "Listen failed";
  }
}

chain::Socket chain::Socket::Accept() {
  Socket new_socket;
  new_socket.fd_ =
      accept(fd_, reinterpret_cast<sockaddr*>(&address_f_), &addrlen);

  if (new_socket.fd_ < 0) {
    throw "Accept failed";
  }

  return new_socket;
}

void chain::Socket::Connect() {
  int status = connect(fd_, reinterpret_cast<sockaddr*>(&address_f_), addrlen);

  if (status < 0) {
    throw "Connection failed";
  }
}

void chain::Socket::Close() { close(fd_); }

chain::FileDescriptorType chain::Socket::get_file_descriptor() const noexcept {
  return fd_;
}

chain::Address chain::Socket::get_address_f() const noexcept {
  return address_f_;
}