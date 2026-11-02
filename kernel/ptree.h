struct ptreeinfo {
    int pid;
    int ppid;
    int state;
    uint64 memsize;   // số byte bộ nhớ user của tiến trình
    char name[16];
};
