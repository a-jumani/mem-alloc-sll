# mem-alloc-sll
A memory allocator with simpler design. It uses a singly-linked list of free memory chunks to allocate and de-allocate memory regions in virtual address space of a process.

**Development Environment:** C (gcc 5.4.0)

**OS:** Ubuntu 16.04 LTS

**Features:**
1. ``Mem_Init`` must be used to specify total heap memory a program can use (including overhead of the allocator itself). This region (obtained using ``mmap()``) is unmapped automatically when the process terminates.
2. The allocator can be linked to programs as a shared library. It is important to include the path to library in environment variable ``$LD_LIBRARY_PATH``. That is, you can execute ``export LD_LIBRARY_PATH=<path>:$LD_LIBRARY_PATH``.
3. Only a pointer is used as a global variable.
