project('mssg'
  , 'c'
  , default_options : ['warning_level=3', 'c_std=c99', 'optimization=3', 'werror=false', 'debug=false']
  , version : '0.0.1'
)

cc = meson.get_compiler('c')

src_inc = include_directories('./src/')

src = [
    './src/main.c'
  , './src/states.c'
  , './src/template.c'
  , './src/files.c'
  , './src/hashmap.c'
  , './src/blog.c'
  , './src/extra.c'
]

executable('mssg'
  , sources : src
  , include_directories : [src_inc]
  , install : true
  , install_dir : get_option('sbindir')
)

