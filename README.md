# mssg
Mart's Static Site Generator written in C (C99)

* Current version: v0.0.1 - 2019-12-29 - Alpha

## Description
A lightweight and fast static site generator that have simple templating functions and optional/extended features

## Dependencies
* **Currently there is no dependencies at all, however the following listed dependencies are planned to happen as the development goes**
* There is no mandatory dependencies, however you will be missing out on thumbnail generation, general image resize, and commonmark conversion if making mssg without the optional dependencies.
### Optional, but recommended
* [ImageMagick](https://imagemagick.org/api/resize.php#ThumbnailImage) - MagickCore API - Used for making thumbnail for the gallery and resizing images in general
* [libcmark](https://github.com/commonmark/cmark) - commonmark - Converts commonmark/markdown files to HTML

## Compiling
* `meson build`
* `cd build`
* `ninja`

## Installation
* `ninja install`

## Uninstallation
* `ninja uninstall`

## Templating Features
* Variables types
  * string
  * list
  * dictionary
* for loop list
* extends/base of another file
* `{% for post in #blog -1 %}`
  * `{{ post.date }}`, `{{ post.title }}`, `{{ post.link }}`

## TODO
* Templating - Doing
  * for loop - Done?
* Hashmap implementation - Done
* Static blogging feature - Doing
  * `blog new test` - Create a new blog post - Done
  * `blog edit 2020-05-05 test` - Edit a blog post - Done
  * `blog list` - List your posts - Doing
  * `blog delete 20190921_01` - Delete a blog post - Doing
  * Templating - blog list - Done
* Translated page version
* Gallery feature
  * Thumbnail generator
* For loop a list of strings - Done
* For loop a list of dictionary - Done
* Point to another variable (EX: `{% list fruit $apple $banana $other %}`) - Done? Need further testing

## Changelogs
### 2019-12-29: v0.0.1 - Alpha
* **Not released yet**
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

