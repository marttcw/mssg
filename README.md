# mssg
Mart's Static Site Generator written in C (C99)

* [Releases](https://github.com/marttcw/mssg/releases)
* Development version: v0.0.2
* Current version: v0.0.1 - 2019-12-29 - Alpha

## Description
A lightweight and fast static site generator that have simple templating functions and optional/extended features

## Dependencies
* **Currently there is no dependencies at all as of v0.0.1. However the following listed dependencies are planned to happen as the development goes.**
* There is no mandatory dependencies, however you will be missing out on thumbnail generation, general image resize, and commonmark conversion if making mssg without the optional dependencies.
### Future/TODO Dependencies (Not actual current optional dependencies)
* [ImageMagick](https://imagemagick.org/api/resize.php#ThumbnailImage) - MagickCore API - Used for making thumbnail for the gallery and resizing images in general
* [libcmark](https://github.com/commonmark/cmark) - commonmark - Converts commonmark/markdown files to HTML

## Compiling
* `make`

## Installation
* `make install`

## Uninstallation
* `make uninstall`

## Features
### Templating
* Variables types
  * string
  * integer
  * float
* for loop
* base of another file
### Extra
* Minify

## Template commands
```
base: Set following file as the base file of the current file, however the rest of the variables are set before going back to the base file.
{% base src/dir/foo.html %}
parse: Parse through the file given
{% parse src/dir/foo.html %}

setblock: Set a block to be put when called
{% setblock content %}{% end %}
putblock: Put the block given
{% putblock content %}

set: Set a variable
{% set foo bar %} - String
{% set foo 1 %} - Integer
{% set foo 1.1 %} - Float
{% set foo aaa bbb ccc %} - List of strings
{% set foo .num 15 .stuff 20 %} - Dictionary (TODO)

Get variable
{{ foo }}

link: Set a link string derived from the source+destination root path
{% link /foo.html %} - Produce just the link
{% link /foo.html Foo %} - Produce a href link tag

loop: For loop: EX: i start on foo to 5
{% loop i foo 5 %}
{% end %}
```

### Pre-defined variables
#### #blog
Blog for loop:

```
{% for post in #blog %}
  {{ post.date }}
  {{ post.link }}
  {{ post.name }}
{% endfor %}
```

### Configuration file only
Set your text editor for editing blog files
```
blog editor vim
```

## TODO
* Static blog
* config
  * `set list one two 3 four five` (list)
  * `setdir dirlist /blog/` (root of source directory)

## TODO (Old)
* Rework
* Templating - Doing
  * for loop - Done
* Hashmap implementation - Done
* Static blogging feature - Doing
  * `blog new test` - Create a new blog post - Done
  * `blog edit 2020-05-05 test` - Edit a blog post - Done
  * `blog list` - List your posts - Doing
  * `blog delete 20190921_01` - Delete a blog post - Doing
  * Templating - blog list - Done
* Translated page version
* Gallery feature
  * Thumbnail generator - Use external program
* For loop a list of strings - Done
* For loop a list of dictionary - Done
* Point to another variable (EX: `{% list fruit $apple $banana $other %}`) - Done? Need further testing
* Minify CSS + JS

## Changelogs
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

