# 3430 A2
### Name: Tristan Montilla
### ID: 7818872
### Section: A01

# Question 1
## Description
- Assumes that 'cat' and 'dog' means **any instance** that matches the word including substrings.
 - 'cats' and 'dogs' are examples of this. As well as 'catherine' and 'doggo'.
 - 'c at' and such are not included however.
 - basically *anything* that has the word 'cat' and 'dog' are taken into consideration.
- Seems to meet all criterias.
 - fork/execvp working.
 - target program is launched by child process.
 - anonymous pipe is used and configured correctly. stdout of child is written to stdout of parent.
 - parent is able to alter data based on what signals it has received. defaulting to cat-to-dog. all is working.
 - objectively high quality.
## Notes
Program works as intended and defensive programming and design by contract are applied.
- *Checked* for errors in handling the program like no args, pipe error, fork error, etc.
- Proper closing of files
- Checks for EINTR which helps in handling interrupts.

# Question 2
## Description
For my signal implementation, I used a stripped down program of what I did with Q1 and mixed in some code strips from lecture and the internet.
- Have different output. both for signal and sigaction.

## Level1: Signal handler implementation
- Implemented signal handlers for signals:
 * SIGUSR1
 * SIGUSR2
 * SIGALRM

##Level2: Signal interrupt and no human interaction
Evaluation for throwing multiple signals at the same time (interrupting previous signals):
 - Signal gets interrupted by other signals.
  - Order does not matter.
  - Signals handlers (at least the user ones) are not interrupted if we throw the same signal. (e.g. SIGUSR1 and then SIGUSR1 again)
 - Develops weird *unexpected behaviors* when signal handlers are *interrupted*.
  - parent and child communication is disturbed. and system state is in disarray.

No human interaction needed. Child and parent send and receive signals.
 - Did a bit of send and pass. Changing states and behavior for each SIGUSR1, SIGUSR2, and SIGALRM sent.

##Level3: Signal and sigaction
 - Sigaction is a newer and more powerful version of signal.
  - more actions and more information obtained.
  - ***blocks*** other signals from *interrupting* current signal.
 - signal has vulnerabilities that sigaction keeps in check.
  - Time between detection for signal is bad. Dumps core by default which is a security issue.
 - Signal behavior varies on different systems. **Sigaction is standardized**

##Notes
###async-signal-safe functions
 - Use of write on signal hanlders to *prevent handlers from doing weird behavior when interrupts occur*.
  - async-signal-safe functions are functions that are safe to use on handlers and async situations.
