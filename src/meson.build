gtkminimodem_sources = [
  'main.c',
  'gtkminimodem-window.c',
]

gtkminimodem_deps = [
  dependency('gio-2.0', version: '>= 2.50'),
  dependency('gtk+-3.0', version: '>= 3.22'),
]

gnome = import('gnome')

gtkminimodem_sources += gnome.compile_resources('gtkminimodem-resources',
  'gtkminimodem.gresource.xml',
  c_name: 'gtkminimodem'
)

executable('gtkminimodem', gtkminimodem_sources,
  dependencies: [ gtkminimodem_deps ],
  install: true,
)
