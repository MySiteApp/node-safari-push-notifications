{
  "targets": [
    {
      "target_name": "pkcs7",
      "sources": [
          "src/openssl.cc",
          "src/pkcs7.cc"
      ],
      "libraries": [ "-lssl" ],

      'conditions': [
        [ 'OS=="win"', {
          'conditions': [
            # "openssl_root" is the directory on Windows of the OpenSSL files
            ['target_arch=="x64"', {
              'variables': {
                'openssl_root%': 'C:/OpenSSL-Win64'
              },
            }, {
              'variables': {
                'openssl_root%': 'C:/OpenSSL-Win32'
              },
            }],
          ],
          'defines': [
            'uint=unsigned int',
          ],
          'libraries': [
            '-l<(openssl_root)/lib/libeay32.lib',
          ],
          'include_dirs': [
            '<(openssl_root)/include',
          ],
        }, { # OS!="win"
          'include_dirs': [
            # use node's bundled openssl headers on Unix platforms
            '<(node_root_dir)/deps/openssl/openssl/include'
          ],
        }],
      ]
    }
  ]
}
