/*
 * fuzzuf-cc
 * Copyright (C) 2022-2023 Ricerca Security
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Affero General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Affero General Public License for more details.
 *
 * You should have received a copy of the GNU Affero General Public License
 * along with this program.  If not, see http://www.gnu.org/licenses/.
 */
/**
 * @file executor.cpp
 * @author Ricerca Security <fuzzuf-dev@ricsec.co.jp>
 */
#include "fuzzuf_cc/utils/executor.hpp"

#include <sys/epoll.h>
#include <sys/fcntl.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/types.h>
#include <sys/wait.h>

#include <algorithm>
#include <array>
#include <boost/range/iterator_range.hpp>
#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <sstream>
#include <string>

#include "fuzzuf_cc/utils/create_shared_memory.hpp"
#include "fuzzuf_cc/utils/errno_to_system_error.hpp"
#include "fuzzuf_cc/utils/shared_range.hpp"

namespace fuzzuf_cc::utils {

Pipe::Pipe() {
  if (pipe(fds.data())) {
    throw fuzzuf_cc::utils::errno_to_system_error(
        errno, "Pipe::Pipe : Unable to create pipe");
  }
}
Pipe::~Pipe() { CloseBoth(); }
void Pipe::Readonly() {
  if (fds[1] >= 0) {
    close(fds[1]);
    fds[1] = -1;
  }
}
void Pipe::Writeonly() {
  if (fds[0] >= 0) {
    close(fds[0]);
    fds[0] = -1;
  }
}
void Pipe::CloseBoth() {
  Readonly();
  Writeonly();
}
void Pipe::PipeToFd(int fd) {
  if (fds[0] >= 0) {
    if (dup2(fds[0], fd) < 0) {
      throw fuzzuf_cc::utils::errno_to_system_error(
          errno, "Pipe::PipeToFd : dup2 failed");
    }
  }
  Readonly();
}
void Pipe::FdToPipe(int fd) {
  if (fds[1] >= 0) {
    if (dup2(fds[1], fd) < 0) {
      throw fuzzuf_cc::utils::errno_to_system_error(
          errno, "Pipe::FdToPipe : dup2 failed");
    }
  }
  Writeonly();
}
int Pipe::GetFd() const {
  if (fds[0] >= 0)
    return fds[0];
  else
    return fds[1];
}

Executor::Executor(std::vector<std::string>&& args_,
                   std::vector<std::string>&& env_)
    : args(std::move(args_)), env(std::move(env_)) {
  argv.reserve(args.size() + 1u);
  std::transform(args.begin(), args.end(), std::back_inserter(argv),
                 [](auto& v) { return v.data(); });
  argv.push_back(nullptr);
  envp.reserve(env.size() + 1u);
  std::transform(env.begin(), env.end(), std::back_inserter(envp),
                 [](auto& v) { return v.data(); });
  envp.push_back(nullptr);

  fork_server_pid = fork();
  if (fork_server_pid < 0) {
    throw fuzzuf_cc::utils::errno_to_system_error(
        errno, "Executor::Executor : fork failed");
  }
  if (fork_server_pid == 0) {
    to_fork_server.PipeToFd(FORKSRV_FD_READ);
    from_fork_server.FdToPipe(FORKSRV_FD_WRITE);
    target_stdout.FdToPipe(1);
    target_stderr.FdToPipe(2);
    execve(argv.front(), argv.data(), envp.data());
    throw fuzzuf_cc::utils::errno_to_system_error(
        errno, "Executor::Executor : execve failed");
  }

  to_fork_server.Writeonly();
  from_fork_server.Readonly();
  target_stdout.Readonly();
  target_stderr.Readonly();
  auto to_fork_server_fd = to_fork_server.GetFd();
  auto from_fork_server_fd = from_fork_server.GetFd();

  if (write(to_fork_server_fd, &executor_id, sizeof(executor_id)) <
      (int)sizeof(executor_id)) {
    throw fuzzuf_cc::utils::errno_to_system_error(
        errno, "Executor::Executor : Unable to set executor id");
  }

  const int read_pid_bytes =
      read(from_fork_server_fd, &fork_server_pid, sizeof(fork_server_pid));
  if (read_pid_bytes < 0) {
    throw fuzzuf_cc::utils::errno_to_system_error(
        errno, "Executor::Executor : Unable to get fork server pid");
  } else if (read_pid_bytes < (int)sizeof(fork_server_pid)) {
    throw fuzzuf_cc::utils::errno_to_system_error(
        EIO, "Executor::Executor : Fork server closed connection unexpectedly");
  }

  OpenStdin();

  epoll_fd = epoll_create(1);
  stdout_event.data.fd = target_stdout.GetFd();
  stdout_event.events = EPOLLIN | EPOLLRDHUP;
  if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, target_stdout.GetFd(), &stdout_event) <
      0) {
    throw fuzzuf_cc::utils::errno_to_system_error(
        errno, "Executor::Executor : Unable to attach stdout to epoll");
  }

  stderr_event.data.fd = target_stderr.GetFd();
  stderr_event.events = EPOLLIN | EPOLLRDHUP;
  if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, target_stderr.GetFd(), &stderr_event) <
      0) {
    throw fuzzuf_cc::utils::errno_to_system_error(
        errno, "Executor::Executor : Unable to attach stderr to epoll");
  }

  from_fork_server_event.data.fd = from_fork_server.GetFd();
  from_fork_server_event.events = EPOLLIN | EPOLLRDHUP;
  if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, from_fork_server.GetFd(),
                &from_fork_server_event) < 0) {
    throw fuzzuf_cc::utils::errno_to_system_error(
        errno,
        "Executor::Executor : Unable to attach from_fork_server to epoll");
  }
}
Executor::~Executor() {
  CloseStdin();
  target_stdout.CloseBoth();
  target_stderr.CloseBoth();
  to_fork_server.CloseBoth();
  from_fork_server.CloseBoth();
  int wstatus = 0u;
  if (waitpid(fork_server_pid, &wstatus, 0) < 0) {
    std::abort();
  }
}
std::tuple<ExecutePUTAPIResponse, std::vector<std::byte>,
           std::vector<std::byte> >
Executor::operator()(const std::vector<std::byte>& standard_input) {
  WriteTestcaseToStdin(standard_input);
  CallForkServerAPI(_ForkServerAPI::ExecutePUTCommand);

  constexpr std::size_t chunk_size = 512u;
  std::vector<std::byte> standard_output;
  std::vector<std::byte> standard_error;
  ExecutePUTAPIResponse response;
  response.error = NoResponseError;
  epoll_event event;
  int wait_for = -1;
  while (1) {
    auto event_count = epoll_wait(epoll_fd, &event, 1, wait_for);
    if (event_count < 0) {
      int e = errno;
      if (e != EINTR)
        throw fuzzuf_cc::utils::errno_to_system_error(
            e, "epoll_wait failed during the execution");
    } else if (event_count == 0) {
      break;
    } else {
      if (event.events & EPOLLIN) {
        if (event.data.fd == target_stdout.GetFd()) {
          const auto old_size = standard_output.size();
          standard_output.resize(old_size + chunk_size);
          int read_size = 0u;
          if ((read_size = read(target_stdout.GetFd(),
                                std::next(standard_output.data(), old_size),
                                chunk_size)) < 0) {
            throw fuzzuf_cc::utils::errno_to_system_error(
                errno, "Executor::operator() : Unable to read stdout");
          }
          standard_output.resize(old_size + read_size);
        } else if (event.data.fd == target_stderr.GetFd()) {
          const auto old_size = standard_error.size();
          standard_error.resize(old_size + chunk_size);
          int read_size = 0u;
          if ((read_size = read(target_stderr.GetFd(),
                                std::next(standard_error.data(), old_size),
                                chunk_size)) < 0) {
            throw fuzzuf_cc::utils::errno_to_system_error(
                errno, "Executor::operator() : Unable to read stderr");
          }
          standard_error.resize(old_size + read_size);
        } else if (event.data.fd == from_fork_server.GetFd()) {
          if (read(from_fork_server.GetFd(), &response,
                   sizeof(ExecutePUTAPIResponse)) < 0) {
            throw fuzzuf_cc::utils::errno_to_system_error(
                errno,
                "Executor::operator() : Unable to retrive execution result");
          }
          // Successfully retrieved the execution results. Exit event loop
          // immediately.
          wait_for = 0;
        }
      } else if (event.events == EPOLLHUP || event.events == EPOLLERR) {
        if (epoll_ctl(epoll_fd, EPOLL_CTL_DEL, event.data.fd, nullptr) < 0) {
          throw fuzzuf_cc::utils::errno_to_system_error(
              errno, "Executor::operator() : Unable to detach closed fd");
        }
      }
    }
  }

  return std::make_tuple(response, std::move(standard_output),
                         std::move(standard_error));
}

void Executor::OpenStdin() {
  std::stringstream ss;
  ss << "/dev/shm/fuzzuf-cc.forkserver.executor_id-" << executor_id << ".stdin";
  target_stdin_path = ss.str();

  target_stdin_fd =
      open(target_stdin_path.c_str(), O_WRONLY | O_CREAT | O_CLOEXEC, 0600);
  if (target_stdin_fd == -1) {
    throw fuzzuf_cc::utils::errno_to_system_error(
        errno, "Executor::OpenStdin : open failed");
  }

  CallForkServerAPI(_ForkServerAPI::ReadStdinCommand);
}
void Executor::CloseStdin() {
  if (target_stdin_fd != -1) {
    close(target_stdin_fd);
  }

  if (remove(target_stdin_path.c_str()) == -1) {
    throw fuzzuf_cc::utils::errno_to_system_error(
        errno, "Executor::CloseStdin : remove failed");
  }
}
void Executor::WriteTestcaseToStdin(const std::vector<std::byte>& testcase) {
  if (lseek(target_stdin_fd, 0, SEEK_SET) == -1) {
    throw fuzzuf_cc::utils::errno_to_system_error(
        errno, "Executor::WriteTestcaseToStdin : Unable to lseek stdin");
  }

  if (write(target_stdin_fd, testcase.data(), testcase.size()) == -1) {
    throw fuzzuf_cc::utils::errno_to_system_error(
        errno, "Executor::WriteTestcaseToStdin : Unable to write stdin");
  }

  if (ftruncate(target_stdin_fd, testcase.size()) == -1) {
    throw fuzzuf_cc::utils::errno_to_system_error(
        errno, "Executor::WriteTestcaseToStdin : Unable to ftruncate stdin");
  }
}
void Executor::CallForkServerAPI(_ForkServerAPI command) {
  int count =
      write(to_fork_server.GetFd(), (void*)&command, sizeof(_ForkServerAPI));
  if (count < 0) {
    throw fuzzuf_cc::utils::errno_to_system_error(
        errno, "Executor::CallForkServerAPI : Unable to write command");
  }
}

}  // namespace fuzzuf_cc::utils
