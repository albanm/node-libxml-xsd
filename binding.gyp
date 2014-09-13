{
  'variables': {
    'target_arch%': 'ia32', # build for a 32-bit CPU by default
    'xmljs_include_dirs%': [],
    'xmljs_libraries%': [],
  },
  "targets": [
    {
      "target_name": "node-libxml-xsd",
      "sources": [ "src/schema.cc", "src/node_libxml_xsd.cc" ],
      "include_dirs": [
      	"<!(node -e \"require('nan')\")",
      	'<@(xmljs_include_dirs)'
      ],
      'link_settings': {
        'libraries': [
          '<@(xmljs_libraries)',
        ]
      }
    }
  ]
}