# Failures
## Exceptions
- Handled where needed.
	 + Ultimately handled at some level, i.e. should not lead to crashes.
	 + Handled where [parts of] state can be recovered.
	 + Methods may throw or propagate exceptions.
- STL is welcome.

## Crashes
- Must not come from unhandled exceptions.
- May come from invalid pointers, division by zero, or other OS violation for which exceptions are not thrown.

__PENDING QUESTIONS__
1. When a process gets restarted:
	+ Should its state be zeroed out, i.e. should the process start over?
	+ Should the state upto a certain checkpoint be preserved, i.e. should the process resume from that checkpoint?
	+ Should the process be allowed to decide what to do with the state (which may be corrupt)?

## Hardware
- The heartbeat of the processing device is lost. 

## Power
- Out of scope.

# Recovery
## Principles
- Different types of processes have different crash expectations. A process that does a lot of work is expected to crash. And vice versa, a process that is not expected to crash must not do a lot of work.
- State is owned by a parent proces (not necessarily the immediate parent). That way, the parent process can re-create a crashed process and eventually restore its previous state.
- Starting a child process by forking its parent has a low cost.

__PENDING QUESTIONS__
1. How to replicate state across devices?
2. Up to what point should process start on the backup device?


# Process Types
## Root
### Summary
- Multiplicity = 1
- Crash expectation = Never
- Threads = 1
- Lifespan = Until shut down

### Functionality
- Preallocates memory for each daemon, and forks itself for each daemon.
- Upon signal about a daemon crah, forks itself to restart the crashed daemon. 

## Daemon
### Summary
- Multiplicity = 1..N
- Crash expectation = Very rarely
- Threads = 1
- Lifespan = Until root gets shut down

### Functionality
- Communicates with the other daemons through the state owned by their common parent, the root.
- Whenever there is a task that may cause a crash, starts a transaction for it by forking itself.
- When a transaction crashes, restarts it by forking itself.

## Transaction
### Summary
- Multiplicity = 0..N
- Crash expectation = Occasionally
- Threads = 1..N
- Lifespan = When a task is needed until it is done.

### Functionality
- Performs a specific task.
- May spin up threads within the same process.

__PENDING QUESTIONS__
1. May a transaction spin up child transactions?
	+ What is the benefit? Checkpoints?


# State
- Each daemon declares the max amount of memory it (and its transactions) may need at any given time.
- The root allocates and maps the desired memory by each daemon to its own address space (before forking).
- After forking, the root marks other daemons' states as read-only.
- Absolutely no additional memory allocation takes place in any process.


# Lib
## Pool
## Process
## Log
## Mutex and Locks
## Ring
## Isolation
## Trace
## Test

---
# v0.8
## Done
- Add a base `_stream<Stream>` that exposes the state getters.
  - Implement a single set of stream state verification methods.
  - Derive `http_` streams from the base `_stream`.
  - Derive `table_` and `line_` from the base `_stream`.
- Test log lines.
## To Do
- Make `line_ostream` use `_sb` instead of directly `_buffer`.
- Make the inheritance from `std::ostream`/`std::istream` public, and don't implement `good()`, `bad()`, etc.
- Convert `LogPtr` to `Log*` for all classes.
- Convert `LineStream` to `Line`.
- Convert `FilterPtr` to `Filter`. 
- `timelimited_file_streambuf`
- `sizelimited_file_streambuf`
- Tag.
- README
  - Try It - step-by-step instructions
  - Use It - step-by-step instructions
  - Organize class reference - Media, Streams, Utilities
- Improve style

# v1.8
## Done
## To Do
- Samples

# v1.9
## Done
## To Do
- WebSocket (SHA-1, base64)

# v1.10
## Done
## To Do
- Internationalization
- ring - single writer, multiple readers
- file-backed buffer

---

- async
	+ future_state::continuations
	+ future::then
	+ queue
	+ async::start(func)
	+ async::start_at(time_point, func)
	+ async::start_after(duration, func)
	+ async::start_when(bool, func)
	+ async::start_then(future, func)


- conf
- flight (killbit)
- uuid (request/correlation)
- usage
- ring
- then


PID PPID PS
ps -el | sed -E -e 's/^. +. +([[:digit:]]+) +([[:digit:]]+) +.+ +([[:alpha:]]+)$/\1 \2 \3/'