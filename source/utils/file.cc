/**
 * This file is part of the "plotfx" project
 *   Copyright (c) 2014 Paul Asmuth, Google Inc.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 * 
 * * Redistributions of source code must retain the above copyright notice, this
 *   list of conditions and the following disclaimer.
 * 
 * * Redistributions in binary form must reproduce the above copyright notice,
 *   this list of conditions and the following disclaimer in the documentation
 *   and/or other materials provided with the distribution.
 * 
 * * Neither the name of the copyright holder nor the names of its
 *   contributors may be used to endorse or promote products derived from
 *   this software without specific prior written permission.
 * 
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */
#include "buffer.h"
#include "exception.h"
#include "file.h"
#include <sys/fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

namespace plotfx {

File File::openFile(
    const std::string& filename,
    int flags) {
  int open_flags = 0;

  switch (flags & (O_READ | O_WRITE)) {
    case O_READ:
      open_flags = O_RDONLY;
      break;

    case O_WRITE:
      open_flags = O_WRONLY;
      break;

    case (O_READ | O_WRITE):
      open_flags = O_RDWR;
      break;

    default:
      RAISE(
          kIllegalArgumentError,
          "openFile('%s'): at least one of O_READ | O_WRITE must be set",
          filename.c_str());
  }

  if ((flags & O_CREATEOROPEN) > 0) {
    open_flags |= O_CREAT;
  } else if ((flags & O_CREATE) > 0) {
    open_flags |= O_CREAT;
    open_flags |= O_EXCL;
  }

  if ((flags & O_TRUNCATE) > 0) {
    open_flags |= O_TRUNC;
  }

  int fd = open(filename.c_str(), open_flags, S_IRUSR | S_IWUSR);
  if (fd < 0) {
    RAISE_ERRNO(kIOError, "openFile('%s'): open() failed", filename.c_str());
  }

  if ((flags & O_AUTODELETE) > 0) {
    unlink(filename.c_str());
  }

  return File(fd, flags);
}

File::File(int fd, int flags) : fd_(fd), flags_(flags) {}

File::~File() {
  if (fd_ >= 0) {
    close(fd_);
  }
}

File::File(File&& other) : fd_(other.fd_), flags_(other.flags_) {
  other.fd_ = -1;
}

int File::fd() const {
  return fd_;
}

bool File::isWritable() const {
  return (flags_ & O_WRITE) == O_WRITE;
}

size_t File::size() const {
  struct stat fd_stat;

  if (fstat(fd_, &fd_stat) < 0) {
    RAISE_ERRNO(kIOError, "fstat(%i) failed", fd_);
  }

  return fd_stat.st_size;
}

void File::seekTo(size_t pos) {
  if (lseek(fd_, pos, SEEK_SET) < 0) {
    RAISE_ERRNO(kIOError, "lseek(%i) failed", fd_);
  }
}

size_t File::read(void* buf, size_t buf_len) {
  int res = ::read(fd_, buf, buf_len);

  if (res < 0) {
    RAISE_ERRNO(kIOError, "read(%i) failed", fd_);
  }

  return res;
}

size_t File::read(Buffer* buf) {
  return read(buf->data(), buf->size());
}

void File::write(const void* buf, size_t buf_len) {
  int res = ::write(fd_, buf, buf_len);

  if (res < 0 || res != buf_len) {
    RAISE_ERRNO(kIOError, "write(%i) failed", fd_);
  }
}

void File::write(const Buffer& buf) {
  write(buf.data(), buf.size());
}

File File::clone() const {
  int new_fd = dup(fd_);

  if (new_fd < 0) {
    RAISE_ERRNO(kIOError, "dup(%i) failed", fd_);
  }

  return File(new_fd, flags_);
}

void File::truncate(size_t new_size) {
  if (ftruncate(fd_, new_size) < 0) {
    RAISE_ERRNO(kIOError, "ftruncate(%i) failed", fd_);
  }
}

}
