# mssg
Mart's Static Site Generator written in C (C99)

## Description
A lightweight and fast static site generator that have simple templating functions and optional/extended features

## Dependencies
* There is no mandatory dependencies, however you will be missing out on thumbnail generation, general image resize, and commonmark conversion if making mssg without the optional dependencies.
### Optional, but recommended
* [ImageMagick](https://imagemagick.org/api/resize.php#ThumbnailImage) - MagickCore API - Used for making thumbnail for the gallery and resizing images in general
* [libcmark](https://github.com/commonmark/cmark) - commonmark - Converts commonmark/markdown files to HTML

## TODO
* Templating
* Static blogging feature
  * `blog post` - Create a new blog post
  * `blog edit 20190921_01` - Edit a blog post
  * `blog list` - List your posts
  * `blog delete 20190921_01` - Delete a blog post
* Translated page version
* Gallery feature
  * Thumbnail generator

## Changelogs
### 2019-12-15: v0.0.1
* **Not released yet**
* Files templating `base and content` works EX: `{% base src/dir/foo.html %}`
* Text variables and placements EX: `{% string foo "hello world" %}` and `{{ foo }}`
* Block variable EX: `{% block foo %} hello world {% endblock %}`
* Build via files with `index.` in it
* Link file EX: `{% link src/dir/style.css %}`
* Link file by variable EX: `{% link_var foo %}` (TODO)
* Configuration file, uses same options as files templating

