node-libxml-xsd
===============

[![Build status](https://travis-ci.org/albanm/node-libxml-xsd.svg)](https://travis-ci.org/albanm/node-libxml-xsd)
[![Code Climate](https://codeclimate.com/github/albanm/node-libxml-xsd/badges/gpa.svg)](https://codeclimate.com/github/albanm/node-libxml-xsd)
[![NPM version](https://badge.fury.io/js/libxml-xsd.svg)](http://badge.fury.io/js/libxml-xsd)

*XSD validation for node.js using [libxml](http://xmlsoft.org/)*

Installation
------------

	npm install libxml-xsd

Basic usage
-----------

```js
var xsd = require('libxml-xsd');

xsd.parseFile(schemaPath, function(err, schema){
  schema.validate(documentString, function(err, validationErrors){
    // err contains any technical error
    // validationError is an array, null if the validation is ok
  });  
});
```

Libxmljs integration
--------------------

Node-libxml-xsd depends on [libxmljs](https://github.com/polotek/libxmljs/issues/226) that bundles libxml.

The libxmljs module required by node-libxml-xsd is exposed as ```require('libxml-xsd').libxmljs```. This prevents depending on libxmljs twice which is not optimal and source of weird bugs.

It is possible to work with libxmljs documents instead of strings as inputs to the *parse()* and *validate()* functions.

Imports and includes
--------------------

XSD includes are supported but relative paths must be given from the execution directory, usually the root of the project.

Includes are resolved when parsing the schema. Therefore the parsing task becomes IO bound, which is why you should not use synchronous parsing when you expect some includes.

Sync or async
-------------

The same *parse()* and *validate()* functions can be used in synchronous mode simply by removing the callback parameter.
In this case if a technical error occurs it will be thrown and validation errors will be returned.

```js
var xsd = require('libxml-xsd');

var schema = xsd.parse(schemaString);

var validationErrors = schema.validate(documentString);

```

The asynchronous functions use the [libuv work queue](http://nikhilm.github.io/uvbook/threads.html#libuv-work-queue)
to provide parallelized computation in node.js worker threads. This makes it non-blocking for the main event loop of node.js.

Note that libxmljs parsing doesn't use the work queue, so only a part of the process is actually parallelized.

The tasks being mostly CPU bound and very fast it is not a big problem to use synchronous mode.
But remember that if you use some includes the parsing task becomes IO bound.

Environment compatibility
-------------------------

For now 64bits linux and 32bits windows are confirmed. Other environments are probably ok, but not checked. Please report an issue if you encounter some difficulties.

Node-libxml-xsd depends on [node-gyp](https://github.com/TooTallNate/node-gyp), you will need to meet its requirements. This can be a bit painful mostly for windows users. The node-gyp version bundled in your npm will have to be greater than 0.13.0, so you might have to follow [these instructions to upgrade](https://github.com/TooTallNate/node-gyp/wiki/Updating-npm's-bundled-node-gyp). There is no system dependancy otherwise as libxml is bundled by libxmljs.

API Reference
=============
Node.js bindings for XSD validation from libxml

**Members**

* [libxml-xsd](#module_libxml-xsd)
  * [libxml-xsd.parse(source, [callback])](#module_libxml-xsd.parse)
  * [libxml-xsd.parseFile(sourcePath, callback)](#module_libxml-xsd.parseFile)
  * [class: libxml-xsd~Schema](#module_libxml-xsd..Schema)
    * [new libxml-xsd~Schema(schemaDoc, schemaObj)](#new_module_libxml-xsd..Schema)
    * [schema.validate(source, [callback])](#module_libxml-xsd..Schema#validate)
    * [schema.validateFile(sourcePath, callback)](#module_libxml-xsd..Schema#validateFile)

<a name="module_libxml-xsd.parse"></a>
##libxml-xsd.parse(source, [callback])
Parse a XSD schema

If no callback is given the function will run synchronously and return the result or throw an error.

**Params**

- source `string` | `Document` - The content of the schema as a string or a [libxmljs document](https://github.com/polotek/libxmljs/wiki/Document)  
- \[callback\] <code>[parseCallback](#parseCallback)</code> - The callback that handles the response. Expects err and Schema object.  

**Returns**: `Schema` - Only if no callback is given.  
<a name="module_libxml-xsd.parseFile"></a>
##libxml-xsd.parseFile(sourcePath, callback)
Parse a XSD schema

**Params**

- sourcePath `stringPath` - The path of the file  
- callback <code>[parseFileCallback](#parseFileCallback)</code> - The callback that handles the response. Expects err and Schema object.  

<a name="module_libxml-xsd..Schema"></a>
##class: libxml-xsd~Schema
**Members**

* [class: libxml-xsd~Schema](#module_libxml-xsd..Schema)
  * [new libxml-xsd~Schema(schemaDoc, schemaObj)](#new_module_libxml-xsd..Schema)
  * [schema.validate(source, [callback])](#module_libxml-xsd..Schema#validate)
  * [schema.validateFile(sourcePath, callback)](#module_libxml-xsd..Schema#validateFile)

<a name="new_module_libxml-xsd..Schema"></a>
###new libxml-xsd~Schema(schemaDoc, schemaObj)
A compiled schema. Do not call this constructor, instead use parse or parseFile.

store both the source document and the parsed schema
if we don't store the schema doc it will be deleted by garbage collector and it will result in segfaults.

**Params**

- schemaDoc `Document` - XML document source of the schema  
- schemaObj `Document` - Simple wrapper of a XSD schema  

**Scope**: inner class of [libxml-xsd](#module_libxml-xsd)  
<a name="module_libxml-xsd..Schema#validate"></a>
###schema.validate(source, [callback])
Validate a XML document over a schema

If no callback is given the function will run synchronously and return the result or throw an error.

**Params**

- source `string` | `Document` - The XML content to validate with the schema, to be given as a string or a [libxmljs document](https://github.com/polotek/libxmljs/wiki/Document)  
- \[callback\] <code>[validateCallback](#Schema..validateCallback)</code> - The callback that handles the response. Expects err and an array of validation errors, null if none.  

**Returns**: `string` | `Document` - Only if no callback is given. An array of validation errors, null if none.  
<a name="module_libxml-xsd..Schema#validateFile"></a>
###schema.validateFile(sourcePath, callback)
Apply a schema to a XML file

**Params**

- sourcePath `string` - The path of the file to read  
- callback <code>[validateFileCallback](#Schema..validateFileCallback)</code> - The callback that handles the response. Expects err and an array of validation errors null if none.  

*documented by [jsdoc-to-markdown](https://github.com/75lb/jsdoc-to-markdown)*.
