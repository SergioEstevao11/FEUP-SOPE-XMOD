# FEUP-SOPE-XMOD
**Description** - UNIX's 'chmod' command replica

**Course** - Operating Systems

## Project Overview

XMOD is a UNIX command line tool that replicates the functionality of the 'chmod' command. It runs in user-space and allows users to change the permissions of files and directories on a UNIX-based system.


## Usage

To run the XMOD, simply compile the source code using the provided Makefile and then execute the resulting binary.
XMOD supports both octal and symbolic inputs.
<br/>
Supports the flags: -R (recursive); -v (verbose); -c (compact).
<br/>
The commands to run are:

``` ./xmod [OPTIONS] [FILEPATH] ```


## Examples:

- **Octal**

``` ./xmod 0777 ficheiro.txt ```

- **Symbolic**

``` ./xmod u=rwx ficheiro.txt ```
<br/>
``` ./xmod g=rwx ficheiro.txt ```
<br/>
``` ./xmod o=rwx ficheiro.txt ```
