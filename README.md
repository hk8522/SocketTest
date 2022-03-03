# SocketTest
Socket Server &amp; Client test program

Within the “user-mode” (not kernel-mode) of the MS Windows environment,
write a C++ program to capture all file system events and using an
asynchronous thread, write the following record to an SSL socket (port
5055):-

-        Total byte-length of this record
-        Fixed value: QZW-0001-0009
-        Date and time of event
-        Process ID initiating file access
-        Process name
-        Parent Process ID
-        Parent Process name
-        File system activity (read/write/delete/etc.)
-        File path and name
-        File size
-        Read data length
-        Write data length
-        Snapshot of the process memory block(s)

Fields should be delimited with value 0x00FF00 (3 bytes)

Additionally, write a separate socket listener program for port 5055 to
read the stream and write it to a file using the filename format: Fixed
Value (field 1) _ IP Address

