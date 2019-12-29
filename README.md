# mssg
Mart's Static Site Generator written in C (C99)

* [Releases](https://github.com/marttcw/mssg/releases)
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
mssg requires [meson](https://mesonbuild.com/) and [ninja](https://ninja-build.org/) to be installed in order to compile, install, and run the project.

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

## Template commands
```
base: Set the following file as the base file of the current file, the rest of the current file will be added once sub_content was used in the base file.
{% base src/dir/foo.html %}
extends: Set following file as the base file of the current file, however the rest of the variables are set before going back to the base file.
{% extends src/dir/foo.html %}
sub_content: Parse through the file on top of the base file, considered file is used via the base command
{% sub_content %}
content: Parse through the file given
{% content src/dir/foo.html %}

string: Set a string variable
{% string foo "hello world" %}
block: Set a string variable, but whatever in between block and endblock are put into the variable
{% block foo %} ... {% endblock %}
list: Set a list
{% list foo apple banana carrot %}
dict: Set a dictionary
{% dict fruit name:apple texture:crunchy %}
EX: $variable: A pointer to another variable already defined
{% string pointer $foo %}

link: Set a link string derived from the source path
{% link src/dir/foo.css %}

For loop: Loop through a list variable
{% for item in list %}
  {{ item }}
{% endfor %}

For loop optionals:
{% for item in list (range) (increment) (start) %}
{% for item in list -1 1 0 %}
Where range of -1 = to maximum range of the list

copy: Filenames which can just be simply copied over to the build side
{% copy style.css rss.xml %}
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

