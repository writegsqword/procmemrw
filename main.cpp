
#include <stdio.h>
#include <cstddef>
#include <string>
#include <errno.h>
#include <fcntl.h>
#include <unistd.h>
#include <iostream>
#include <fstream>
#include "argparse.hpp"

using uint64 = unsigned int64_t;
using byte = unsigned int8_t;

int fd_is_valid(int fd)
{
    return fcntl(fd, F_GETFD) != -1 || errno != EBADF;
}

// not very efficient but eh
std::vector<byte> read_until_eof()
{

    std::vector<byte> result;
    // FILE *f = freopen(argv[1], "rb", stdin);
    unsigned int b;

    while ((b = getchar_unlocked()) != EOF)
    {
        result.push_back(b);
    }
    return result;
}

int main(int argc, char **argv)
{

    uint64 addr, size = 0, pid;
    std::string filename = "";
    bool mode_read, mode_write;

    argparse::ArgumentParser args("Process Memory Dumper");
    auto &opmode = args.add_mutually_exclusive_group(true);
    args.add_argument("-a", "--address")
        .help("The address to read/write memory to(hex)")
        .scan<'x', uint64>()
        .required();
        //.store_into(addr);

    opmode.add_argument("-r", "--read")
        .help("Use read mode(exclusive with --write)")
        .flag()
        .store_into(mode_read);

    opmode.add_argument("-w", "--write")
        .help("Use write mode(exclusive with --read)")
        .flag()
        .store_into(mode_write);

    args.add_argument("-s", "--size")
        .help("The amount of bytes to read/write, defaults to 1024 bytes for read and size of input buffer for write")
        //.required()
        .scan<'u', uint64>()
        .store_into(size);

    args.add_argument("-f", "--file")
        .help("Name of file to read/write dump to(defaults to stdin/stdout if unspecified)")
        .default_value("")
        .store_into(filename);

    args.add_argument("-p", "--pid")
        .help("PID of process to read/write memory from")
        .required()
        .scan<'u', uint64>()
        .store_into(pid);

    try
    {
        args.parse_args(argc, argv);
    }
    catch (const std::exception &err)
    {
        std::cerr << err.what() << std::endl;
        std::cerr << args;
        std::exit(1);
    }
    addr = args.get<uint64>("-a");
    // size = args.get<uint64>("-s");

    bool default_size = false;
    if (size == 0)
    {
        size = 1024;
        default_size = true;
    }

    std::string proc_fname = "/proc/" + std::to_string(pid) + "/mem";
    int memory_fd;
    if (mode_read)
    {
        memory_fd = open(proc_fname.c_str(), O_RDONLY);
    }
    else if (mode_write)
    {
        memory_fd = open(proc_fname.c_str(), O_WRONLY);
    }

    if (!fd_is_valid(memory_fd))
    {
        std::cerr << "Invalid memory file descriptor, try running in sudo\n";
        std::exit(1);
    }
    lseek64(memory_fd, addr, SEEK_SET);
    std::vector<byte> buf(size);
    if (mode_read)
    {
        if (filename.length() != 0)
        {
            freopen(filename.c_str(), "wb", stdout);
        }
        // if(default_size)
        read(memory_fd, buf.data(), size);
        write(STDOUT_FILENO, buf.data(), size);
        return 0;
    }

    if (mode_write)
    {
        if (filename.length() != 0)
        {
            freopen(filename.c_str(), "rb", stdin);
        }
        if (default_size)
        {
            buf = read_until_eof();
        }
        else
        {
            read(STDIN_FILENO, buf.data(), size);
        }
        write(memory_fd, buf.data(), buf.size());
        return 0;
    }
}