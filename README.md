# SocketTest

Socket Server &amp; Client test program

Within the “user-mode” (not kernel-mode) of the MS Windows environment,
write a C++ program to capture all file system events and using an
asynchronous thread, write the following record to an SSL socket (port
5055):-

- Total byte-length of this record
- Fixed value: QZW-0001-0009
- Date and time of event
- Process ID initiating file access
- Process name
- Parent Process ID
- Parent Process name
- File system activity (read/write/delete/etc.)
- File path and name
- File size
- Read data length
- Write data length
- Snapshot of the process memory block(s)

Fields should be delimited with value 0x00FF00 (3 bytes)

Additionally, write a separate socket listener program for port 5055 to
read the stream and write it to a file using the filename format: Fixed
Value (field 1) _ IP Address

# Question & Answers

- The client should be stop at this point or continue to capture the next events?
  - =>  Continue.

- If the client should go on, the server will repeat to write the contents to the same file (named as "fixed value_ip address"), and the file would keep only the last one. Do you agree this?
  - =>  No. New data should be appended to the file. However, the file could be split into hourly files using the file name as "fixed value_ip address_yyyyddmm_hh"

- If the client should go on, it would take substantial time to write 2MB of memory dump to the file. Do you agree this?
  - => Perhaps. We may add a filter and not write every event, depending on the process name, if it is well known - we are looking to log data for unknown/abnormal processes, but we will add this filter later.

- If the client append the contents to the log file, this file would become larger and larger. Do you agree this?
  - => Agree; see above response - we will split into hourly files.



- "just sending the data to my own webserver which will log the data in a file."
  - What do you mean?
  - Don't you want the server?
  - Or do you want my server to upload the log file to your web server?
  - In this case, all of the file system events can't be treated.
  - The processing overload would be very big.


  - => The webserver (or any remote server) is just a location where the log data can be sent and stored for later processing
  - => Agree; see above response - there may be too many events, so will add some filtering.
  - => For dev/test purposes, to keep the amount of data manageable, we can just sample the file system events every 5 seconds to reduce the data collected.


PS.
There are 3 method to monitor file system events.
1. Use kernel mode driver and cooperate with it in user mode. But this can't be used by your suggestion. And this requires a lot of time and budget (In this mode, microsoft made process explorer works.)
2. Use API hook. This is very desireable. But Windows doesn't support API hook anymore, if we try to hook an API, windows would block the program suspecting it as a malicious virus.
3. Use File System Watcher. This is the method I can use at the moment. But this does not support process information.
The last one is only available way, now. I want your suggestion.
Obviously, it is impossible to get process information.

- => ok, I understand the situation. I suggest that we use option 3.

- => Regarding processes, I would like you to write a separate function that watches for new processes being started (probably by polling the active process list and maintaining a list in memory to compare one against the other, to determine what processes are new and what has terminated). Again, we want to take a snapshot of their memory, but we need to filter this so that we don't snapshot every new process, only the ones we are interested in, such as processes that are new and unknown to us. Against, for now, limit the data by using a 5 second increment between taking a snapshot.

- => Its not perfect, but it will do for now, while I determine how we will apply a filter.

- => The goal at the moment is to have a working program that can capture some file and process activity and send it to a remote server where it writes the data to a log file.
