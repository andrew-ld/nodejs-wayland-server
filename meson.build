project(
    'nodejs-wayland',
    'cpp',
    version: '1.0.0',
    default_options: ['cpp_std=c++17'],
)

node_inc = include_directories('node_modules/node-addon-api')

weston_opts = [
    'backend-wayland=true',
    'backend-x11=false',
    'renderer-gl=true',
    'shell-desktop=false',
    'xwayland=true',
    'backend-drm=false',
    'backend-headless=false',
    'backend-pipewire=false',
    'backend-rdp=false',
    'backend-vnc=false',
    'renderer-vulkan=false',
    'remoting=false',
    'pipewire=false',
    'shell-ivi=false',
    'shell-kiosk=true',
    'shell-lua=false',
    'color-management-lcms=false',
    'image-jpeg=false',
    'image-webp=false',
    'demo-clients=false',
    'tools=[]',
    'tests=false',
    'doc=false',
    'perfetto=false',
    'backend-default=wayland',
    'default_library=static',
    'systemd=false',
    'test-junit-xml=false',
    'desktop-shell-client-default=kiosk',
    'simple-clients=[]',
]

weston_sp = subproject('weston-fork', default_options: weston_opts)
libexec_weston_dep = weston_sp.get_variable('dep_libexec_weston')

shared_module(
    'nodejs_wayland',
    'src/wrapper.cpp',
    name_suffix: 'node',
    name_prefix: '',
    include_directories: [node_inc],
    dependencies: [libexec_weston_dep],
    install: true,
    install_rpath: '$ORIGIN',
)
