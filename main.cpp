
#include <stdio.h>
#include <cstddef>
#include <string>
#include <errno.h>
#include <fcntl.h>
#include <unistd.h>
#include <iostream>


using uint64 = unsigned int64_t;

int main(int argc, char** argv) {
  std::string mode = argv[1];
  uint64 addr = std::stoull(argv[3], nullptr, 16);
  uint64 size = 0x1000;
  
  if(argc == 4) { 
    size = std::stoull(argv[4], nullptr, 10);
  }
  char cmd[0x200];
  const char* fname = (std::string("/proc/") + argv[2] + "/mem").c_str();
  
  char* buf = new char[size];
  if(mode == "write") { 
    int fd = open(fname, O_WRONLY);
    lseek64(fd, addr, SEEK_SET);
    read(STDIN_FILENO, buf, size);
    write(fd, buf, size);
    return 0;
  }
  if(mode == "read") { 
    int fd = open(fname, O_RDONLY);
    lseek64(fd, addr, SEEK_SET);
    read(fd, buf, size);
    write(STDIN_FILENO, buf, size); 
    return 0;
  }

  


}

