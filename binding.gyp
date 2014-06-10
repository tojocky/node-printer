{
  'targets': [
    {
      'target_name': 'node_printer',
      'sources': [ 'src/node_printer.cc',
        'src/node_printer_posix.cc',
        'src/node_printer_win.cc'
      ],
      'conditions': [
        ['OS == "win"', {
          'sources!': [
            # posixx-only; exclude on other platforms.
            'src/node_printer_posix.cc'
          ]
        }, { # != win32
          'sources!': [
            # Linux-only; exclude on other platforms.
            'src/node_printer_win.cc'
          ],
          'cflags':[
            #run: 'cups-config --cflags'
          ],
          'ldflags':[
            #run: 'cups-config --libs'
            '-lcups -lgssapi_krb5 -lkrb5 -lk5crypto -lcom_err -lz -lpthread -lm -lcrypt -lz'
          ]
        }]
      ]
    }
  ]
}
