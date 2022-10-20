project('spi-pipe-ng', 'c', version: '0.1', meson_version: '>=0.53.0')
add_global_arguments('-DPROJ_VER="' + meson.project_version() + '"', language : 'c')

cc = meson.get_compiler('c')
cc.has_header_symbol('limits.h', 'PATH_MAX', required : true)

static = get_option('static').enabled()

if static
    static_arg = [ '-static' ]
else
    static_arg = []
endif

version_src = vcs_tag(input: 'version.c.in', output: 'version.c', fallback: '(unknown)')

executable('spi-pipe-ng', 'spi-pipe-ng.c', version_src, install: true, link_args: static_arg)
