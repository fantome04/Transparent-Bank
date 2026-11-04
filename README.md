# Transparent Bank

A C++ implementation of a simple multi-process banking system using **POSIX shared memory** and **semaphores**.

The project models a transparent bank in which multiple clients can access and modify shared bank accounts while preserving data consistency.

## Features

Each bank account contains:

- current balance;
- minimum allowed balance;
- maximum allowed balance;
- frozen/unfrozen state.

The following invariant is maintained:

```text
minimum balance <= current balance <= maximum balance
```

The client supports:

- viewing the current balance of an account;
- viewing the minimum balance;
- viewing the maximum balance;
- freezing an account;
- unfreezing an account;
- transferring money between accounts;
- adding the same amount to all accounts;
- subtracting the same amount from all accounts;
- changing the minimum allowed balance;
- changing the maximum allowed balance.

Invalid operations are rejected without modifying the bank state.

## Implementation

The bank is stored in **POSIX shared memory**, allowing several processes to access the same bank state.

Access to the shared bank is synchronized using a **named POSIX semaphore**. A client locks the semaphore before executing an operation and releases it afterwards. This prevents multiple clients from modifying the bank simultaneously and protects the consistency of transactions.

Operations involving several accounts are handled atomically. For example, a transfer succeeds only when both the source and destination accounts can perform the operation. If an operation cannot be completed, previous changes are rolled back.

The project uses:

- C++;
- POSIX shared memory (`shm_open`, `mmap`, `shm_unlink`);
- POSIX named semaphores (`sem_open`, `sem_wait`, `sem_post`);
- Make;
- Valgrind for testing/debugging.

## Project Structure

```text
.
├── bank.cpp
├── bank.h
├── bank_cell.cpp
├── bank_cell.h
├── client.cpp
├── config.h
├── destroy.cpp
├── init.cpp
├── testing.cpp
├── makefile
└── README.md
```

### Main files

| File | Description |
| --- | --- |
| `bank_cell.h/.cpp` | Implementation of an individual bank account |
| `bank.h/.cpp` | Bank operations such as transfers and operations on all accounts |
| `init.cpp` | Creates and initializes the bank in shared memory |
| `client.cpp` | Interactive client used to execute bank commands |
| `destroy.cpp` | Removes the shared memory object and semaphore |
| `config.h` | Shared configuration |
| `testing.cpp` | Tests for the supported operations |
| `makefile` | Build, run, test, and cleanup commands |

## Initial State

The current configuration creates **10 accounts**.

Each account is initialized with:

```text
current balance = 0
minimum balance = 0
maximum balance = 1000
frozen = false
```

The number of accounts can be changed in `config.h`.

## Building

The project is intended for a Unix/Linux environment with POSIX shared-memory support.

Compile the programs with:

```bash
make init client destroy
```

Alternatively, the complete interactive workflow can be started with:

```bash
make run
```

This initializes the bank, starts the client, and destroys the bank after the client exits.

## Running Manually

First initialize the shared bank:

```bash
./init
```

Then start a client:

```bash
./client
```

Several clients can be started from different terminals:

```bash
./client
```

They will access the same shared bank.

When finished, remove the shared resources:

```bash
./destroy
```

## Client Commands

In the current client interface, accounts are numbered from `1` to `10`.

### Show current balance

```text
current A
```

Example:

```text
current 1
```

### Show minimum balance

```text
minimum A
```

Example:

```text
minimum 1
```

### Show maximum balance

```text
maximum A
```

Example:

```text
maximum 1
```

### Freeze an account

```text
freeze A
```

Example:

```text
freeze 2
```

### Unfreeze an account

```text
unfreeze A
```

Example:

```text
unfreeze 2
```

### Transfer money

```text
transfer A B X
```

Transfers `X` units from account `A` to account `B`.

Example:

```text
transfer 1 2 100
```

The transfer is rejected if either account is frozen or if the resulting balance would violate the minimum or maximum balance constraints.

### Add money to all accounts

```text
addall X
```

Example:

```text
addall 50
```

The operation succeeds only if it can be applied to every account.

### Subtract money from all accounts

```text
suball X
```

Example:

```text
suball 20
```

The operation succeeds only if it can be applied to every account.

### Set minimum balance

```text
setmin A X
```

Example:

```text
setmin 1 -100
```

The new minimum must be compatible with the current and maximum balances, and the account must not be frozen.

### Set maximum balance

```text
setmax A X
```

Example:

```text
setmax 1 2000
```

The new maximum must be compatible with the current and minimum balances, and the account must not be frozen.

### Exit the client

```text
exit
```

## Example Session

```text
$ ./client

current 1
Current balance for cell number 1: 0

maximum 1
Maximum balance for cell number 1: 1000

addall 100
Successfully added 100 to all cells

current 1
Current balance for cell number 1: 100

transfer 1 2 30
Successfully transferred 30 from 1 to 2

current 1
Current balance for cell number 1: 70

current 2
Current balance for cell number 2: 130

freeze 1
Cell number 1 successfully frozen

transfer 1 2 10
invalid operation

unfreeze 1
Cell number 1 successfully unfrozen

exit
```

## Testing

A test program is provided in `testing.cpp`.

Build the tests with:

```bash
make testing
```

Before running the test executable, initialize the bank:

```bash
./init
./testing
./destroy
```

The tests cover the main client operations, including:

- querying balances;
- freezing and unfreezing accounts;
- adding to all accounts;
- subtracting from all accounts;
- setting minimum and maximum balances;
- transferring funds.

### Valgrind

The Makefile also provides:

```bash
make debug_valgrind
```

This builds the debug version, initializes the shared bank, executes the test suite under Valgrind, and destroys the shared resources afterwards.

## Cleaning

Remove generated binaries and object files with:

```bash
make clean
```

## Synchronization and Transaction Safety

All client requests are executed while holding the shared semaphore.

This ensures that two clients cannot modify the bank at the same time.

For multi-account operations, the implementation also avoids leaving the bank in a partially modified state. For example, if money can be withdrawn from the source account but cannot be deposited into the destination account, the withdrawal is reverted.

Similarly, operations such as `addall` and `suball` revert already completed changes if the operation fails on one of the accounts.

This preserves the consistency of the shared bank even when multiple client processes are running.
