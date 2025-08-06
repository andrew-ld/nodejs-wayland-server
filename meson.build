project(
    'nodejs-wayland',
    'cpp',
    version: '1.0.0',
    default_options: ['cpp_std=c++17'],
)

napi_inc_path = get_option('napi_include_path')
node_inc_path = get_option('node_include_path')

napi_inc = include_directories(napi_inc_path)
node_inc = include_directories(node_inc_path)

weston_dep = dependency(
    'weston-fork',
    default_options: ['default_library=shared'],
)

shared_module(
    'my_nodelib',
    'src/wrapper.cpp',
    name_suffix: 'node',
    name_prefix: '',
    include_directories: [napi_inc, node_inc],
    dependencies: [weston_dep],
    install: true,
    install_rpath: '$ORIGIN',
)
