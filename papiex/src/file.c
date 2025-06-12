static inline const char* open_flags2str(int flags) {
  char ret_str[PATH_MAX];
  ret_str[0] = '\0';
  if (flags & O_RDONLY) strcat (ret_str, "O_RDONLY|");
  if (flags & O_WRONLY) strcat (ret_str, "O_WRONLY|");
  if (flags & O_RDWR) strcat (ret_str, "O_RDWR|");
  if (flags & O_CREAT) strcat (ret_str, "O_CREAT|");
  if (flags & O_EXCL) strcat (ret_str, "O_EXCL|");
  if (flags & O_NOCTTY) strcat (ret_str, "O_NOCTTY|");
  if (flags & O_TRUNC) strcat (ret_str, "O_TRUNC|");
  if (flags & O_APPEND) strcat (ret_str, "O_APPEND|");
  if (flags & O_NONBLOCK) strcat (ret_str, "O_NONBLOCK|");
  if (flags & O_NDELAY) strcat (ret_str, "O_NDELAY|");
  if (flags & O_SYNC) strcat (ret_str, "O_SYNC|");
  if (flags & O_FSYNC) strcat (ret_str, "O_FSYNC|");
  if (flags & O_ASYNC) strcat (ret_str, "O_ASYNC|");
  int len = strlen(ret_str);
  if (len >0) ret_str[len-1] = '\0';
  return (strdup(ret_str));
}

static inline const char* mode2string(int mode) {
  char ret_str[PATH_MAX];
  ret_str[0] = '\0';
  if (mode & S_IRWXU) strcat (ret_str, "S_IRWXU | ");
  if (mode & S_IRUSR) strcat (ret_str, "S_IRUSR | ");
  if (mode & S_IXUSR) strcat (ret_str, "S_IXUSR | ");
  if (mode & S_IRWXG) strcat (ret_str, "S_IRWXG | ");
  if (mode & S_IRGRP) strcat (ret_str, "S_IRGRP | ");
  if (mode & S_IWGRP) strcat (ret_str, "S_IWGRP | ");
  if (mode & S_IXGRP) strcat (ret_str, "S_IXGRP | ");
  if (mode & S_IRWXO) strcat (ret_str, "S_IRWXO | ");
  if (mode & S_IROTH) strcat (ret_str, "S_IROTH | ");
  if (mode & S_IWOTH) strcat (ret_str, "S_IWOTH | ");
  if (mode & S_IXOTH) strcat (ret_str, "S_IXOTH | ");
  return (strdup(ret_str));
}
