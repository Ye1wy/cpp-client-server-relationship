#include "../include/client.h"

chain::Client::Client()
    : data_(),
      client_socket_(),
      mtx_(),
      cv_(),
      status_send_data_(false),
      is_connection_made_(false) {};

void chain::Client::Start() {
  std::thread input_thread(&Client::InputThread, this);
  std::thread processing_thread(&Client::ProcessingThread, this);

  input_thread.join();
  processing_thread.join();
}

bool chain::Client::Connect() {
  if (is_connection_made_) {
    std::cout << "Client: you already connected!" << std::endl;

  } else if (client_socket_.Connect()) {
    is_connection_made_ = true;
    return true;
  }

  return false;
}

void chain::Client::Send(std::string data) {
  if (!status_send_data_) {
    throw std::runtime_error("Somthing went wrong! Thread handling ERROR");
  }

  if (!is_connection_made_) {
    std::cout
        << "You don't connected! Type *reconnect* and try sand data again!"
        << std::endl;
    return;
  }

  std::string send_status = client_socket_.Send(data);

  char buffer[1024] = {0};
  int bytes_read =
      recv(client_socket_.get_file_descriptor(), buffer, sizeof(buffer), 0);

  if (bytes_read > 0) {
    std::cout << "Client: " << send_status << std::endl;

  } else if (bytes_read == 0) {
    std::cout << "Client: Server is unreacheble! Reconnect to server"
              << std::endl;

  } else if (bytes_read == -1) {
    perror("Client: Read error");
    std::cout << "Error code: " << errno << std::endl;
  }
}

void chain::Client::Reconnect() {
  while (!is_connection_made_) {
    try {
      std::cout << "Client: Attemting to reconnect..." << std::endl;
      if (Connect()) {
        is_connection_made_ = true;
        std::cout << "Client: Reconnected successfully" << std::endl;
        std::cout << std::endl;

      } else {
        break;
      }

    } catch (const std::exception &e) {
      std::cerr << "Client: Reconnect failed: " << e.what() << std::endl;
      std::this_thread::sleep_for(std::chrono::seconds(RECONNECT_INTERVAL));
    }
  }
}

void chain::Client::AttemptReconnect() {
  if (!is_connection_made_) {
    client_socket_.Stop();
    client_socket_.SocketUp();
    Reconnect();
  }
}

void chain::Client::InputThread() {
  std::string tmp_data;

  while (true) {
    std::cin >> tmp_data;

    if (tmp_data == "reconnect") {
      std::cout << "Reconnect initialized..." << std::endl;
      is_connection_made_ = false;
      AttemptReconnect();
      continue;
    }

    if (!Valid(tmp_data)) {
      std::cout << "Client: You input not valide string" << std::endl;
      continue;
    }

    PrimaryProcessing(tmp_data);

    {
      std::lock_guard<std::mutex> lock(mtx_);
      data_ = tmp_data;
      status_send_data_ = false;
    }

    cv_.notify_one();
  }
}

void chain::Client::ProcessingThread() {
  while (true) {
    std::unique_lock<std::mutex> lock(mtx_);
    cv_.wait(lock);

    status_send_data_ = true;

    int sanding_data = SecondaryProcessing();

    Send(std::to_string(sanding_data));
  }
}

bool chain::Client::Valid(const std::string verifiable) const noexcept {
  if (verifiable.length() > MAX_LENGHT) {
    return false;
  }

  if (!IsDigits(verifiable)) {
    return false;
  }

  return true;
}

bool chain::Client::IsDigits(const std::string &str) const {
  for (char ch : str) {
    if (!isdigit(ch)) {
      return false;
    }
  }

  return true;
}

void chain::Client::PrimaryProcessing(std::string &data) noexcept {
  std::sort(data.begin(), data.end());
  std::cout << "Client: Data sorted" << std::endl;
  int digit;
  std::string result;
  bool is_even_sequence = false;

  for (char ch : data) {
    digit = ch - '0';

    if (digit % 2 == 0) {
      if (!is_even_sequence) {
        is_even_sequence = true;
        result += "KB";
      }

    } else {
      is_even_sequence = false;
      result += ch;
    }
  }

  data = std::move(result);
}

int chain::Client::SecondaryProcessing() noexcept {
  std::cout << "Client: Processed data: " << data_ << std::endl;
  int digit = 0, sum_of_data = 0;
  std::string handling_data = std::move(data_);

  for (char ch : handling_data) {
    digit = ch - '0';

    if (ch >= 'A' and ch <= 'Z') {
      continue;
    }

    sum_of_data += digit;
  }

  return sum_of_data;
}