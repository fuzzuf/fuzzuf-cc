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
 * @file executor.hpp
 * @author Ricerca Security <fuzzuf-dev@ricsec.co.jp>
 */
#ifndef FUZZUF_CC_INCLUDE_UTILS_EXECUTOR_HPP
#define FUZZUF_CC_INCLUDE_UTILS_EXECUTOR_HPP
#include <sys/epoll.h>

#include <array>
#include <boost/range/iterator_range.hpp>
#include <cstddef>
#include <cstdint>
#include <cstdlib>
#include <filesystem>
#include <string>
#include <tuple>
#include <vector>

#include "features/forkserver/config.h"

namespace fs = std::filesystem;

namespace fuzzuf_cc::utils {

/**
 * Wrapper for pipe operation required in executor.
 * This class is intended to make executor code simple.
 */
class Pipe {
 public:
  Pipe();
  Pipe(const Pipe&) = delete;
  Pipe(Pipe&&) = delete;
  Pipe& operator=(const Pipe&) = delete;
  Pipe& operator=(Pipe&&) = delete;
  ~Pipe();
  /**
   * Indicate this pipe is read only for this process.
   */
  void Readonly();
  /**
   * Indicate this pipe is write only for this process.
   */
  void Writeonly();
  /**
   * Close this pipe.
   */
  void CloseBoth();
  /**
   * Bind this pipe to specified file descriptor.
   * Pipe will marked as read only and read data is passed to the file
   * descriptor.
   */
  void PipeToFd(int fd);
  /**
   * Bind this pipe to specified file descriptor.
   * Pipe will marked as write only and incoming data from the file descriptor
   * is passed to the pipe.
   */
  void FdToPipe(int fd);
  /**
   * Get file descriptor of the pipe.
   * readonly() or writeonly() must be called prior to this function.
   */
  int GetFd() const;

 private:
  std::array<int, 2u> fds;
};

/**
 * Simple executor to run PUT compiled in forkserver mode.
 * This executor is intended to use in unit tests.
 */
class Executor {
 public:
  /**
   * Constructor
   * @param args_ arguments of PUT
   * @param env_ environment variables for forkserver process
   */
  Executor(std::vector<std::string>&& args_, std::vector<std::string>&& env_);
  Executor(const Executor&) = delete;
  Executor(Executor&&) = delete;
  Executor& operator=(const Executor&) = delete;
  Executor& operator=(Executor&&) = delete;
  ~Executor();
  /**
   * Run PUT once
   * @param standard_input the value is passed to the PUT by standard input
   * @return tuple of execution result, standard output from the PUT and
   * standard error from the PUT
   */
  std::tuple<ExecutePUTAPIResponse, std::vector<std::byte>,
             std::vector<std::byte> >
  operator()(const std::vector<std::byte>& standard_input);
  template <typename Range>
  auto operator()(const Range& r)
#if __cplusplus >= 202002L
      -> std::tuple<ExecutePUTAPIResponse, std::vector<std::byte>,
                    std::vector<std::byte> >
  requires requires {
    !std::is_same_v<std::remove_cv_t<std::remove_reference_t<Range> >,
                    std::vector<std::byte> >;
  }
#else
      -> std::enable_if_t<
          !std::is_same_v<std::remove_cv_t<std::remove_reference_t<Range> >,
                          std::vector<std::byte> >,
          std::tuple<ExecutePUTAPIResponse, std::vector<std::byte>,
                     std::vector<std::byte> > >
#endif
  { return operator()(std::vector<std::byte>(r.begin(), r.end())); }
  std::tuple<ExecutePUTAPIResponse, std::vector<std::byte>,
             std::vector<std::byte> >
  operator()() {
    return operator()(std::vector<std::byte>{});
  }

 private:
  void OpenStdin();
  void CloseStdin();
  void WriteTestcaseToStdin(const std::vector<std::byte>& testcase);
  void CallForkServerAPI(_ForkServerAPI command);
  std::vector<std::string> args;
  std::vector<char*> argv;
  std::vector<std::string> env;
  std::vector<char*> envp;
  std::uint64_t executor_id = 1u;
  pid_t fork_server_pid = 0;
  Pipe to_fork_server;
  Pipe from_fork_server;
  int target_stdin_fd = -1;
  fs::path target_stdin_path;
  Pipe target_stdout;
  Pipe target_stderr;
  int epoll_fd = -1;
  epoll_event stdout_event;
  epoll_event stderr_event;
  epoll_event from_fork_server_event;
};

}  // namespace fuzzuf_cc::utils
#endif
