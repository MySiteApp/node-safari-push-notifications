{
  "targets": [
    {
      "target_name": "pkcs7",
      "sources": [
          "src/pkcs7.cc"
      ],
      'conditions': [
        ['OS=="win"', {
          'conditions': [
            # "openssl_root" is the directory on Windows of the OpenSSL files
            ['target_arch=="x64"', {
              'variables': {
                'openssl_root%': 'C:/Program Files/OpenSSL-Win64'
              },
            }, {
              'variables': {
                'openssl_root%': 'C:/Program Files (x86)/OpenSSL-Win32'
              },
            }],
          ],
          'defines': [
            'uint=unsigned int',
          ],
          'libraries': [
            '-L<(openssl_root)/lib',
          ],
          'include_dirs': [
            '<(openssl_root)/include'
          ],
        }, {  # OS!="win"
          'include_dirs': [
            '<(node_root_dir)/deps/openssl/openssl/include'
          ],
        }],
      ],

      'include_dirs': [
        "<!(node -e \"require('nan')\")"
      ]
    }
  ]
}
