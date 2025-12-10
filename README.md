# Linux Kernel Producer–Consumer Module (Zombie Process Monitor)

This project implements a **producer–consumer system inside the Linux kernel** using kernel threads, semaphores, a circular buffer, and process traversal.  
The module scans for zombie processes belonging to a specific UID and logs their information using the kernel ring buffer.

This demonstrates understanding of **Linux kernel internals**, **thread synchronization**, **semaphores**, **process management**, and **low-level memory handling**.

---

## 🚀 Features

### 👨‍💻 Producer Thread(s)
- Iterates through every process in the system using `for_each_process()`
- Detects zombie processes (`EXIT_ZOMBIE`)
- Matches them against a user-specified UID
- Inserts them into a shared circular buffer
- Signals consumers using a semaphore (`up(&full)`)

### 🧵 Consumer Thread(s)
- Waits for the `full` semaphore
- Removes zombie entries from the buffer
- Logs PID and parent PID to kernel logs (`printk`)
- Signals producers using `up(&empty)`

### 🔧 Synchronization
- Fixed-size circular buffer  
- `empty` semaphore controls buffer availability  
- `full` semaphore controls consumption  
- Proper kernel-thread cleanup with `kthread_stop()`  
- Safe memory allocation with `kmalloc()`  

---
