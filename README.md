# mssg
Mart's Static Site Generator written in Rust (v2018)

## Using cargo to install the program
* First use rustup and [install Rust](https://www.rust-lang.org/tools/install):
  * macOS, Linux, or other UNIX-like OS: `curl https://sh.rustup.rs -sSf | sh`
* Download the latest release or clone this mssg repository
* Then just execute: `cargo install --path .`
  * Cargo will sort out the dependencies, compiling, and installation for you at once

### Dependencies used:
```
yansi = "0.5.0"
pulldown-cmark = "0.6.0"
toml = "0.5"
image = "0.22.2"
```

## TODO
* Use CommonMark/Markdown (pulldown-cmark)
* toml for configuration (toml)
* Templating
* Static blogging feature
* Translated page version
* Gallery feature
  * Thumbnail generator (image)
* Generate better error messages (yansi for colors)
