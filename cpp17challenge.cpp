/*
*  Write a command line tool that takes in a CSV file, overwrites all the data
*  of a given column by a given value, and outputs the results into a new CSV
*  file.
*
*  More specifically, this command line tool should accept the following
*  arguments:
*
*  - the filename of a CSV file,
*  - the name of the column to overwrite in that file,
*  - the string that will be used as a replacement for that column,
*  - the filename where the output will be written.
*
*  if the input file is empty, the program should write “input file missing” to
*  the console.
*  if the input file does not contain the specified column, the program should
*  write “column name doesn’t exists in the input file” to the console.
*  In both cases, there shouldn’t be any output file generated.
*
*  And if the program succeeds but there is already a file having the name
*  specified for output, the program should overwrite this file.
*
*/

#include <stdio.h>
#include <cstdlib>
#include <errno.h>
#include <fcntl.h>
#include <iostream>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sstream>
#include <unistd.h>

int main(int argc, char** argv) {
  if (argc != 5) {
    exit(1);
  }
  auto in_file = argv[1];
  auto out_file = argv[4];
  std::string col_name = argv[2];
  std::string col_val = argv[3];

  // Read from input file.
  errno = 0;
  auto in_fd = open(in_file, O_RDONLY);
  int saved_errno = errno;
  if (in_fd == -1) {
   if (saved_errno == ENOENT) {
     std::cerr << "input file missing" << std::endl;
   } else {
     std::cerr << "unexpected error: " << strerror(saved_errno) << std::endl;
   }
   exit(1);
  }

  // Open output file for writing.
  errno = 0;
  int out_fd = open(out_file, O_WRONLY | O_CREAT | O_TRUNC);
  saved_errno = errno;
  if (out_fd == -1) {
    std::cerr << "error in opening output file: " << strerror(saved_errno) << std::endl;
    exit(1);
  }
  // Start with reading the full file.
  std::stringstream file_contents;
  const auto BUF_SIZE = 4096;
  char buf[BUF_SIZE] = {0};
  while (true) {
    errno = 0;
    ssize_t read_bytes = read(in_fd, buf, BUF_SIZE);
    int saved_errno = errno;
    if (read_bytes == -1) {
      std::cerr << "failed while reading input file; error: "
                << strerror(saved_errno) << std::endl;
      exit(1);
    } else if (read_bytes == 0) {
      // Full file has been read.
      break;
    }
    file_contents.write(buf, read_bytes);
  }
  // Parse file.
  std::string contents = file_contents.str();
  auto pos = contents.find("\n");
  if (pos == std::string::npos) {
    std::cerr << "file is incorrectly formatted" << std::endl;
    exit(1);
  }
  auto col_pos = contents.find(col_name);
  if (col_pos == std::string::npos or col_pos > pos) {
    std::cerr << "column name doesn't exist in the input file" << std::endl;
    exit(1);
  }
  size_t cols = 0;
  int count = 0;  // To store column number.
  while ((cols = contents.find_first_of(",", cols)) < col_pos) {
    cols += 1;
    count++;
  }
  std::stringstream output_stream;
  char * dup = strdup(contents.c_str());
  // Write first row.
  char * data = strtok(dup, "\n");
  output_stream << data << "\n";
  // Write rest.
  std::string curr_delim = ",";
  int found = 1;
  while ((data = strtok(nullptr, curr_delim.c_str())) != nullptr) {
    found += 1;
    if (found == count + 2) {
      output_stream << col_val << curr_delim;
      curr_delim = "\n";
      found = 0;
      continue;
    }
    output_stream << data << curr_delim;
    curr_delim = ",";
  }
  // Write to output file.
  while (true) {
    auto read_bytes = output_stream.readsome(buf, BUF_SIZE);
    if (read_bytes == 0) {
      break;
    }
    errno = 0;
    if (write(out_fd, buf, read_bytes) == -1) {
      const int saved_errno = errno;
      std::cout << "failed when writing to output file: " << strerror(saved_errno) << std::endl;
    }
  }
  return 0;
}
