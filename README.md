# os-toolbox

```shell
make
# Build 'trace' program.
```

```shell
sudo ./trace [arg]
# Use 'trace'(root) to analysis a living process or start a new one.
# [arg] needs to be a existing pid or a path to executable file.
# e.g., 'ls', '/home/xxx/Desktop/yyy', '12345' are valid [arg]
# No other arguments for the new process is allowed, i.e., 'ls -al' is not a valid [arg]
```

```shell
make result
# Outputs essentials to 'result' file.
# Alternatively, use the following commands to show everything on terminal.
python3 count.py
```

```shell
source easydump.sh [start] [stop] [file]
# Peek from [start] tp [stop] in [file]
# e.g., source easydump.sh 9adc 10be0 /usr/lib/libxxxx.so
# disassemble libxxxx.so from 0x9adc to 0x10be0 along with the source code and line number info.
```
