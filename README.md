# riddle

A toy imperative Pascal/C compiled language.

## How to run

### Requirements

1. Install [meson](https://mesonbuild.com/Getting-meson.html)

```
pip3 install --user meson
```

2. Install [ninja](https://ninja-build.org) build system

3. Run the following in terminal:

```
meson builddir
```

### On each compilation/execution

```
meson compile -C builddir
cd builddir
./lexer_demo
```

### Test iteration

```
meson test -v -C builddir
```

Add these commands to your text editor for fast access.

For vim, you can add these options to .vimrc or .localvimrc
```
nmap <leader>cc meson compile -C builddir
nmap <leader>ct meson test -C builddir
```
