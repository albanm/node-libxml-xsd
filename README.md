XSD validation for node.js using libxml
=======================================

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
    * [schema.validateToFile(sourcePath, callback)](#module_libxml-xsd..Schema#validateToFile)

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
  * [schema.validateToFile(sourcePath, callback)](#module_libxml-xsd..Schema#validateToFile)

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
- \[callback\] <code>[validateCallback](#Schema..validateCallback)</code> - The callback that handles the response. Expects err an array of validation errors.  

**Returns**: `string` | `Document` - Only if no callback is given. An array of validation errors, empty if ok.  
<a name="module_libxml-xsd..Schema#validateToFile"></a>
###schema.validateToFile(sourcePath, callback)
Apply a schema to a XML file

**Params**

- sourcePath `string` - The path of the file to read  
- callback <code>[validateToFileCallback](#Schema..validateToFileCallback)</code> - The callback that handles the response. Expects err and an array of validation errors.  

*documented by [jsdoc-to-markdown](https://github.com/75lb/jsdoc-to-markdown)*.