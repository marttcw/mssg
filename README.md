# mssg
Simple Static Site Generator written in Rust

* [Releases](https://github.com/mtcw99/mssg/releases)
* Development version: v0.0.3
* Current version: v0.0.1 - 2019-12-29 - Alpha

## Description
A lightweight and fast static site generator that have simple templating functions and optional/extended features

## Instructions
### Compiling and Installation
* `cargo install --path .`
### Uninstallation
* `cargo uninstall`

## Usage
* Refer to `mssg -h`

## TODO
* Rework to Rust
* Aim for feature parity close to [ssg](https://www.romanzolotarev.com/ssg.html)

## Changelogs
### 2021-03-03: v0.0.3 - Rework 2
* Changed to Rust
### 2020-08-30: v0.0.2 - Rework
* Templating completely changed
* Parsing different
* Minify HTML works
### 2020-08-01: v0.0.2 - Rework
* Make replaced ninja for compiling, etc...
* Rework starts
### 2020-01-XX: v0.0.2 - Alpha
* Execute external programs
* Blog list now in latest blog order
### 2019-12-29: v0.0.1 - Alpha
* Files templating `base and content` works EX: `{% base src/dir/foo.html %}`
* Text variables and placements EX: `{% string foo "hello world" %}` and `{{ foo }}`
* Block variable EX: `{% block foo %} hello world {% endblock %}`
* Build via files with `index.` in it
* Link file EX: `{% link src/dir/style.css %}`
* Configuration file, uses same options as files templating
* Blog new and edit
  * config: EX: `blog editor nvim`
  * templating: for loop `#blog`
* Pointers: EX: `{% string foo $other_foo %}`
* File copy parameter: Config EX: `copy style.css rss.xml`
  * Files with those filenames just get a simple copy over to the build equivalent

