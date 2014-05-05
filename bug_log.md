BUG LOG
=======

2014-05-05 
----------
- Fix permissions for video memory
  - Prior to this any user-level program could write to video memory
    without repercussion, arguably not notable, but an issue nonetheless
  - Fixed by refactoring functions to allow flags

- Fix issues related to halting the last process in the OS
  - Exiting from the last shell would cause a loop of pagefaults from
    user memory
  - Fixed by spawning a new shell after the last process closes

- Fix index out of bounds that caused memory issues related to fish on
  multiple terminals
  - Fish would appear on other terminals and in general wreak havock if
    more than one ran at once
  - Fixed by fixing an array index that often appeared out of bounds

- Fix issues related to killing processes with crtl-c
  - Processes would sometimes get removed from the scheduling queue when
    they shouldn't
  - Fixed by refactoring a number of commands to directly accept
    pointers to pcb

- Fix sys\_halt\_internal implementation and issues with RTCs closing when
  they shouldn't
  - A bug in rtc\_close often removed the wrong rtc from the global rtc
    chain, or otherwised caused other process's rtcs to stop updating

- Fix up some code related to sys\_halt
  - Refactored sys\_halt to be useable inside the kernel, which fixed a
    wrong return code issue when a program was squashed after generating
    an exception


2014-05-04 
----------

- Fix some rtc bugs, halt when last shell is closed
- Fix page fault on switching terminals
- Fix issue where switching terminals didn't actually switch the correct terminal
- Fix issue where we didn't switch to a page directory containing fake video memory before attempting to restore the screen


2014-05-03
----------

- Fix issue where terminal cursor didn't wrap to the previous line when backspacing


2014-05-02
----------

- Fix forward declarations
- Fix issue with mapping user video memory
- Fix scheduling by removing actively running process from queues.


2014-05-01
----------

- Fix issue where schedule() is only called once
- Fix use of uninitialized variable in scheduler
- Minor fixes
- Fix permissions
- Fix last issue where screen cursor doesn't sync to global cursor
- Fix issues related to improperly assigning video memory


2014-04-27 
----------

- Fix #11, where backspacing after clearing the screen caused a page fault."
- Fix header issue where static inlines needed global variables
- Fix issues relating to running the OS on real hardware.


2014-04-22
----------

- Fix issue with virtual rtc where using frequency macros resulted in incorrect frequencies
- Fix bug where virtual rtc would tick immediately upon being opened


2014-04-21
----------

- Fix issue with clearing/setting interrupts


2014-04-20
----------

- Fix issues that arose from custom filesys\_img, copy script from class directory that does it for us properly.


2014-04-19
----------

- Fix keyboard initialization on real hardware
- Remove unused code and fix character
- Fix keyboard buffer handling when we're not in a term\_read


2014-04-18
----------

- Fix bugs related to spawning new proceeses and address calculation
- Fix bug in term.c where keys are echoed to the screen even when they're not added to the key queue


2014-04-17 
----------

- Fix '{' bug in PIC
- Fix bug where userspace address was not properly checked; user programs now squashed on raised exception
- Fix term\_read bug related to uninitialized variables
- Fix read\_data, hopefully for the last time
- Temporarily fix issues caused by reading blocksize of 2^n in read\_data
- Fix small style issues in syscall\_impl.c and a bug where vidmapping didn't actually load pages for video memory
- Fix paging bug where certain page tabes were RO when they should have been RW
- Fix rtc driver implementation so that takes ptr to int instead of int
- Fix issues related to compiling sample programs
- Fix bug where syscall number bounds are not properly validated
- Fix read\_dentry\_by\_name for real this time
- Fix issues with dir\_read implementation


2014-04-16
----------

- Validate fd before calling read/write/close. Fixes #6.
- Fix a bug where attempting to execute a non-executable would leave the system in a state where no further executables could run
- Fix issue in read\_dentry\_by\_name where we'd match filenames longer than the given string
- Fix typo
- Fix bug in read\_data, where there were instances of offset that were not multiplied by db\_first
- Fix exec/halt implementation. Second process can now run more than once without crashing.


2014-04-14
----------

- Fix issues brought up by last commit
- Fix read\_data
- Fix proc.h misc issues
- Fix file\_loader
- Fix additional page directories


2014-04-13
----------

- Fix compile errors


2014-04-02
----------

- Fix linking issue with gcc by substituting ld
- Fix dumb mistake where ports are not the same size
- Fix portability issue related to using GCC for linking that caused the filesystem image to appear too high in memory


2014-04-01
----------

- Fix test index
- Fix read test
- Fix Filename errors
- Fix implementation of term\_write so that it writes the number of bytes passed


2014-03-23
----------

- Fix email script so that it rewrites by name as well.
- Add script to fix emails in history. Dangerous. Do not use.


2014-03-20 
----------

- Fix small spelling error
- Fix authors.txt for my email
- Fix incomplete variable name changes


2014-03-18
----------

- Fix RTC
- Fix PIC init, reenable RTC, fix PIC EOI, implement IRQ handlers


2014-03-17 
----------

- Fix spelling error
- Fix exception handling; initialize IDT; define halt; halt on unhandled exceptions
- Fix i8259\_init


2014-03-16
----------

- Fix bug related to limit on GDT

