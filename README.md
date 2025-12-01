# Interior Sergeant-At-Arms

Interior Sergeant-At-Arms(ISAA) is a Free Software simple singleton process manager. It's job is to make sure only one process of a certain class is running at a time. It is broken up into two programs: `isaa`, a cli tool to attempt to run porcesses with; `isaad`, a daemon to govern whether to terminate a process or whether attempts to run a process should be allowed or not.

<!-- markdown-toc start - Don't edit this section. Run M-x markdown-toc-refresh-toc -->
**Table of Contents**

- [Interior Sergeant-At-Arms](#interior-sergeant-at-arms)
  - [Why?](#why)
  - [Installation](#installation)
  - [Usage](#usage)
    - [Examples](#examples)
  - [Logging](#logging)
  - [Contributing](#contributing)
    - [TODO](#todo)
  - [License](#license)

<!-- markdown-toc end -->


## Why?

For me personally, ISAA improved the feel of my GNU/Linux system. For example toggling my app launcher whenever I do the hotkey again, like MacOS. Also, making sure only one widget exists in the same general window position. For the first example, there were shell commands I could use to do this, but those would kill all of my wofi instances and a program is more portable as a whole. For the second example, there are probably ways of working with your compositor to do this, but ISAA attempts to follow the Unix philosophy, allowing it to once again be more portable.

## Installation

clone or download a release and cd into it.

Then use `make`. 

Compile.

```
make
```

And install.

```
sudo make install
```

We have a simple systemd unit you can use for `isaad`.

```
cp systemd/isaad.service ~/.config/systemd/user/isaad.service
```

Then enable and run.

```
systemctl --user enable --now isaad.service
```

Check to make sure there are no errors.

```
systemctl --user status isaad.service
```

You now have `isaa`.

## Usage

If you need help, you can run ```isaa --help``` and a menu will appear with common guidance.

ISAA commands are split into two parts, seperated by the first instance of ` -- `.

```
isaa [OPTIONS ...] -- [COMMAND ...]
```

There are three options that define what `isaa` should tell `isaad`. Based on these `isaad` may kill processes in the background, and will tell `isaa` whether it should run the command process or not.

**ignore**: If a process is running with the class name from ignore, `isaad` will instruct `isaa` not to run.

```isaa --ignore foo -- process1```

**toggle**: If a process is running with the class name from toggle, `isaad` will instruct `isaa` not to run and will kill that running process.

```isaa --toggle bar -- process2```

**replace**: If a process is running with the class name from replace, `isaad` will instruct `isaa` to run, and will kill that running process.

```isaa --replace foobar -- process3```

In all these cases, if there are no conflicts, then `isaad` will instruct `isaa` to run.

**IMPORTANT**: If no options were provided, `isaad` will instruct `isaa` not to run.

You may chain multiple together in any order.

```isaa -i alsoIgnore --ignore side -r alsoReplace --replace top -t alsoToggle --toggle right -- process4```

Events are resolved in order.

1. First, ignore: If there is any conflict in ignore, then the process will be ignored, and no replacing or toggling will happen.
2. Then, toggle: If there was no conflict with ignore, but there is conflict with toggle, then it will toggle **ALL** the conflicts, and no replacing will happen.
3. Finally, replace: If there was no conflict with ignor or toggle, but there is a conflict with replace, then it will replace **ALL** the conflicts,. This means `isaad` will terminate them, and `isaa` will run the command.
4. If no conflicts then `isaa` will run the command.

### Examples

Here are real examples I use in my current setup.

```
isaa --toggle launcher --replace side -- wofi --show drun
```

Like MacOS, I have my application launcher set to `SUPER+Space`. Now, having it set to this allows me to get a nice toggle mechanism, instead of having `wofi` launch over itself.

```
isaa --replace side -- nmgui
```

In my setup I have a variety of applications appear on my right hand side, including my application launcher `wofi`. This pattern allows my launcher and other side widgets to replace each other.

## Logging

`isaa` and `isaad` both use syslog.

```
journalctl -f
```

To watch it log.

## Contributing

I am open to contributions either via [email](mailto:isaa@robertismo.com) or Github. I would love to hear your use case and how we can improve ISAA.

compile with debug logging and flags

```
make debug
```

You can also use isaa as a library.

```
make lib
```

### TODO

- release 1.0.0
- better hash implementation

## License

ISAA is Free Software. All the code in this repository is licensed under the GNU Affero General Public License.
