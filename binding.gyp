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
            ['target_arch=="x64"', {
              'variables': {
                'openssl_root1%': 'C:/Program Files/OpenSSL-Win64',
                'openssl_root2%': 'C:/OpenSSL-Win64'
              },
            }, {
              'variables': {
                'openssl_root1%': 'C:/Program Files (x86)/OpenSSL-Win32',
                'openssl_root1%': 'C:/OpenSSL-Win32'
              },
            }],
          ],
          'defines': [
            'uint=unsigned int',
          ],
          'include_dirs': [
            '<(openssl_root1)/include',
            '<(openssl_root2)/include',
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
