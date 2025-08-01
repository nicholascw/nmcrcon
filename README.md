# Nicholas' Minecraft RCON Client / `nmcrcon`
Yet another Minecraft RCON client that tries to go somewhere<sup>[1](https://github.com/Tiiffi/mcrcon/issues/45#issuecomment-997283909) [2](https://github.com/Tiiffi/mcrcon/issues/58)</sup>.

## Overview
This is just my overnight attempt of recreating the mcrcon client from scratch after investigating [Tiiffi/mcrcon](https://github.com/Tiiffi/mcrcon) for some minutes and deteremined a pull request may be much more work than just starting from scratch. I am doing this with the goal of making a usable one that will never have the invalid packet size issue that [Tiiffi/mcrcon](https://github.com/Tiiffi/mcrcon) had for years, and also get rid of the weird design decisions such as must enter the password in the command line when invoking it, unable to use the arrow keys when editing, etc.

**This project is in early development. Contributions over PRs are welcome.**

## Build
#### Arch Linux
There is `PKGBUILD` supplied in this repository as well as in [AUR](https://aur.archlinux.org/packages/nmcrcon-git).

#### Other Linux distros
You can use the provided `Makefile` to build `nmcrcon`. Note that you have to install `libbestline` for your system. Build `nmcrcon` simply with `make`.

#### Windows, macOS, *BSD, etc.
As it's just an overnight effort, for now, I don't know. The current compatibility guarantee is not higher than "works on my machine" but I will try to help. Try by yourself and let me know if it works or not. Windows users can simply use WSL or try build with Cygwin.

## Usage

#### Invocation

Current version of `nmcrcon` supports the following options:

```
Usage: nmcrcon [OPTION...] [HOST[:PORT]] [COMMAND]

  -H, --host=HOST            Server host
  -p, --pass=PASSWORD        RCON Password
  -P, --port=PORT            Server port
  -q, -s, --quiet, --silent  Don't produce any output
  -v, --verbose              Output more information
  -w, --wait=SECONDS         Wait duration in between each command
  -?, --help                 Give this help list
      --usage                Give a short usage message
  -V, --version              Print program version
```

However, the `[COMMAND]` and the following options has not been implemented yet:

```
  -q, -s, --quiet, --silent  Don't produce any output
  -v, --verbose              Output more information
  -w, --wait=SECONDS         Wait duration in between each command
```

Current version of `nmcrcon` is still fully compatible with the early version invocation, but `-H` and `-P` must not present. They would have a higher priority than `[HOST[:PORT]]` so that `[COMMAND]` without host or port specified in `[HOST[:PORT]]` can be correctly supported.

<details>
<summary>Early Version Invocation</summary>

Prior to the introduction of `Argp`, `nmcrcon` has no configuration file, no envrionment variables, and no command line options. The only command line arguments it takes is "`[SERVER_ADDRESS:[RCON_PORT]]`" which is also optional. If you don't provide the server address or the RCON port, it defaults to `localhost` and `25575`. See examples below:

```
$ nmcrcon
$ nmcrcon 123.123.123.123
$ nmcrcon 123.123.123.123:25576
$ nmcrcon mc.example.com
$ nmcrcon mc.example.com:25576
```

Above invocation would likely remain fully unchanged, but be aware of the future plans that may change the way `nmcrcon` treats command line arguments. Refer to [Roadmap](#roadmap) for more details.

</details>

#### Environment Variables

You may also specify server host, port, and password in environment variables `NMCRCON_HOST`, `NMCRCON_PORT`, `NMCRCON_PASS`. When `NMCRCON_` environment variables are not set, nmcrcon will fall back to `MCRCON_` variables for compatibility with Tiiffi/mcrcon. **Please note that while the `NMCRCON_` variables would have higher priorities than `MCRCON_` ones, using `NMCRCON_` variables does not automatically mute other `MCRCON_` variables, so a mixed use of `NMCRCON_` and `MCRCON_` variables is also possible.**


#### The console shortcuts

As `nmcrcon` is built with [jart/bestline](https://github.com/jart/bestline), it brings you a better console experience with shortcuts support. See a full list of shortcuts supported listed below grabbed from `bestline`'s README.

<details>
<summary>Shortcuts</summary>

```
CTRL-E         END
CTRL-A         START
CTRL-B         BACK
CTRL-F         FORWARD
CTRL-L         CLEAR
CTRL-H         BACKSPACE
CTRL-D         DELETE
CTRL-Y         YANK
CTRL-D         EOF (IF EMPTY)
CTRL-N         NEXT HISTORY
CTRL-P         PREVIOUS HISTORY
CTRL-R         SEARCH HISTORY
CTRL-G         CANCEL SEARCH
ALT-<          BEGINNING OF HISTORY
ALT->          END OF HISTORY
ALT-F          FORWARD WORD
ALT-B          BACKWARD WORD
CTRL-ALT-F     FORWARD EXPR
CTRL-ALT-B     BACKWARD EXPR
ALT-RIGHT      FORWARD EXPR
ALT-LEFT       BACKWARD EXPR
CTRL-K         KILL LINE FORWARDS
CTRL-U         KILL LINE BACKWARDS
ALT-H          KILL WORD BACKWARDS
CTRL-W         KILL WORD BACKWARDS
CTRL-ALT-H     KILL WORD BACKWARDS
ALT-D          KILL WORD FORWARDS
ALT-Y          ROTATE KILL RING AND YANK AGAIN
ALT-\          SQUEEZE ADJACENT WHITESPACE
CTRL-T         TRANSPOSE
ALT-T          TRANSPOSE WORD
ALT-U          UPPERCASE WORD
ALT-L          LOWERCASE WORD
ALT-C          CAPITALIZE WORD
CTRL-Z         SUSPEND PROCESS
CTRL-\         QUIT PROCESS
CTRL-S         PAUSE OUTPUT
CTRL-Q         UNPAUSE OUTPUT (IF PAUSED)
CTRL-Q         ESCAPED INSERT
CTRL-SPACE     SET MARK
CTRL-X CTRL-X  GOTO MARK
CTRL-Z         SUSPEND PROCESS
```
</details>

#### Built-in Commands

Like Linux shells, `nmcrcon` supports built-in commands and is designed to be easily expandable. Below is the exhaustive list of all built-in commands for the current version, and may expand in future versions. Note that built-in commands has a higher priority than RCON commands in case of a conflict. 

| Command | Description                           |
| :-----: | :------------------------------------ |
| `auth`  | Re-invoke the authentication process. |
| `clear` | Clears the screen.                    |
| `exit`  | Exits the `nmcrcon` shell.            |

## Roadmap
I have a list of plans in case I have time and interest to continue investing in this project, which may introduce significant breaking changes in near future. Therefore, currently there is no versioning plans in place for now. Any who uses `nmcrcon` or interested in contributing to this project should be aware of the potential changes in future versions. The following are current TODOs, which will likely be implemented in the given order.

- [x] **`Ctrl-C` behavior** - Alter the behavior so that it only interrupts the current command and allows the user to continue interacting with the shell.
- [x] **Compability with [Tiiffi/mcrcon](https://github.com/Tiiffi/mcrcon)** - Provide command line options and environment variables that is compatible with `mcrcon`.
- [ ] **Configuration** - Support configuration files for persistent settings.
- [ ] **Customizable prompt** - Allow users to customize the shell prompt.
- [ ] **Command history** - Implement command history.
- [ ] **Tab completion** - Add tab completion support.
- [ ] **More built-in commands** - Prepare for scripting functionality, add commands such as `sleep`,`connect`,`disconnect`.
- [ ] **Scripting** - Allow users to write scripts using `nmcrcon` commands.
- [ ] **`man` page** - Create a man page for `nmcrcon`.

Versioning or releases may come by the time about half of these are ticked, but I'm not sure yet. I may also work on a potential pre-built executable distribution with GitHub Actions, as well as potential Windows / macOS binaries, at some point.

## Issues
You may report any bugs or feature requests by creating an issue on [GitHub Issues](https://github.com/nicholascw/nmcrcon/issues/new) and I would be happy to help with troubleshooing. However, be fully aware that **`nmcrcon` is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.**

## Contributing
If you're interested in contributing to `nmcrcon`, feel free to fork the repository and submit a pull request. Please make sure to follow the existing code style and add appropriate tests for any new features or bug fixes.

## Sponsor
If you find `nmcrcon` useful and would like to support its development, consider sponsoring it on [GitHub Sponsors](https://github.com/sponsors/nicholascw).

## License
This project is licensed under the GPLv3 license. For more information, please refer to [LICENSE](LICENSE).
