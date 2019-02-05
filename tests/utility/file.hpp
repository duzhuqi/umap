//////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2019, Lawrence Livermore National Security, LLC.
// Produced at the Lawrence Livermore National Laboratory
//
// Created by Marty McFadden, 'mcfadden8 at llnl dot gov'
// LLNL-CODE-733797
//
// All rights reserved.
//
// This file is part of UMAP.
//
// For details, see https://github.com/LLNL/umap
// Please also see the COPYRIGHT and LICENSE files for LGPL license.
//////////////////////////////////////////////////////////////////////////////
#ifndef LIB_UTILITY_FILE_HPP
#define LIB_UTILITY_FILE_HPP

#include <sys/stat.h>
#include <sys/time.h>
#include <sys/resource.h>
#include <fcntl.h>
#include <unistd.h>

#include <iostream>
#include <string>

namespace utility {
void extend_file_size_manually(const int fd, const ssize_t file_size) {
  auto buffer = new unsigned char[4096];
  for (off_t i = 0; i < file_size / 4096; ++i) {
    ::pwrite(fd, buffer, 4096, i * 4096);
  }
  const size_t remained_size = file_size % 4096;
  if (remained_size > 0)
    ::pwrite(fd, buffer, remained_size, file_size - remained_size);

  ::sync();
  delete[] buffer;
}

bool extend_file_size(const int fd, const size_t file_size) {
  /// -----  extend the file if its size is smaller than that of mapped area ----- ///
#ifdef __linux__
  struct stat statbuf;
  if (::fstat(fd, &statbuf) == -1) {
    ::perror("fstat");
    std::cerr << "errno: " << errno << std::endl;
    return false;
  }
  if (::llabs(statbuf.st_size) < static_cast<ssize_t>(file_size)) {
    if (::ftruncate(fd, file_size) == -1) {
      ::perror("ftruncate");
      std::cerr << "errno: " << errno << std::endl;
      return false;
    }
  }
#else
#warning "Manually extend file size"
  extend_file_size_manually(fd, file_size);
#endif
  return true;
}

bool extend_file_size(const std::string &file_name, const size_t file_size) {
  const int fd = ::open(file_name.c_str(), O_RDWR);
  if (fd == -1) {
    ::perror("open");
    std::cerr << "errno: " << errno << std::endl;
    return false;
  }

  if (!extend_file_size(fd, file_size)) return false;
  ::close(fd);

  return true;
}

bool create_file(const std::string &file_name) {
  const int fd = ::open(file_name.c_str(), O_RDWR | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR);
  if (fd == -1) {
    ::perror("open");
    std::cerr << "errno: " << errno << std::endl;
    return false;
  }
  ::close(fd);

  return true;
}
}  // namespace utility

#endif //LIB_UTILITY_FILE_HPP
