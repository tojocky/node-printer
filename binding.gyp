{
  'targets': [
    {
      'target_name': 'node_printer',
      'sources': [ 'src/node_printer.cc',
        'src/node_printer_posix.cc',
        'src/node_printer_win.cc'
      ],
      'conditions': [
        ['OS != "win"', {
          'sources!': [
            # Linux-only; exclude on other platforms.
            'src/node_printer_win.cc'
          ]}
        ],
        ['OS != "posix"', {
          'sources!': [
            # posixx-only; exclude on other platforms.
            'src/node_printer_posix.cc'
          ]}
        ]
      ]
    }
  ]
}
