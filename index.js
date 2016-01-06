/**
 * Node.js bindings for XSD validation from libxml
 * @module libxml-xsd
 */

var fs = require('fs');
var libxmljs = require('libxmljs-mt');
var binding = require('bindings')('node-libxml-xsd');

/**
 * The libxmljs module. Prevents the need for a user's code to require it a second time. Also prevent weird bugs.
 */
exports.libxmljs = libxmljs;

/**
 * A compiled schema. Do not call this constructor, instead use parse or parseFile.
 *
 * store both the source document and the parsed schema
 * if we don't store the schema doc it will be deleted by garbage collector and it will result in segfaults.
 *
 * @constructor
 * @param {Document} schemaDoc - XML document source of the schema
 * @param {Document} schemaObj - Simple wrapper of a XSD schema
 */
var Schema = function(schemaDoc, schemaObj){
	this.schemaDoc = schemaDoc;
	this.schemaObj = schemaObj;
};

/**
 * Parse a XSD schema
 *
 * If no callback is given the function will run synchronously and return the result or throw an error.
 *
 * @param {string|Document} source - The content of the schema as a string or a [libxmljs document]{@link https://github.com/polotek/libxmljs/wiki/Document}
 * @param {parseCallback} [callback] - The callback that handles the response. Expects err and Schema object.
 * @return {Schema} Only if no callback is given.
 */
exports.parse = function(source, callback) {
	// schema can be given as a string or a pre-parsed xml document
	if (typeof source === 'string') {
		try {
			source = libxmljs.parseXml(source);
		} catch (err) {
			if (callback) return callback(err);
			throw err;
		}
	}

	if (callback) {
		binding.schemaAsync(source, function(err, schema){
			if (err) return callback(err);
			callback(null, new Schema(source, schema));
		});
	} else {
		return new Schema(source, binding.schemaSync(source));
	}
};
/**
 * Callback to the parse function
 * @callback parseCallback
 * @param {error} [err]
 * @param {Schema} [schema]
 */

/**
 * Parse a XSD schema
 *
 * @param {stringPath} sourcePath - The path of the file
 * @param {parseFileCallback} callback - The callback that handles the response. Expects err and Schema object.
 */
exports.parseFile = function(sourcePath, callback) {
	fs.readFile(sourcePath, 'utf8', function(err, data){
		if (err) return callback(err);
		exports.parse(data, callback);
	});
};
/**
 * Callback to the parseFile function
 * @callback parseFileCallback
 * @param {error} [err]
 * @param {Schema} [schema]
 */

/**
 * Validate a XML document over a schema
 *
 * If no callback is given the function will run synchronously and return the result or throw an error.
 *
 * @param {string|Document} source - The XML content to validate with the schema, to be given as a string or a [libxmljs document]{@link https://github.com/polotek/libxmljs/wiki/Document}
 * @param {Schema~validateCallback} [callback] - The callback that handles the response. Expects err and an array of validation errors, null if none.
 * @return {string|Document} Only if no callback is given. An array of validation errors, null if none.
 */
Schema.prototype.validate = function(source, callback) {

	// xml can be given as a string or a pre-parsed xml document
	var outputString = false;
	if (typeof source === 'string') {
		try {
			source = libxmljs.parseXml(source);
		} catch (err) {
			if (callback) return callback(err);
			throw err;
		}
		outputString = true;
	}

	if (callback) {
		binding.validateAsync(this.schemaObj, source, function(err, validationErrors){
			if (err) return callback(err);
			return callback(null, validationErrors.length > 0 ? validationErrors : null);
		});
	} else {
		var validationErrors = binding.validateSync(this.schemaObj, source);
		return validationErrors.length > 0 ? validationErrors : null;
	}
};
/**
 * Callback to the Schema.validate function
 * @callback Schema~validateCallback
 * @param {error} [err] - Error when attempting to validate (not a validation error).
 * @param {array} [validationErrors] - A array of errors from validating the schema, null if none.
 */

/**
 * Apply a schema to a XML file
 *
 * @param {string} sourcePath - The path of the file to read
 * @param {Schema~validateFileCallback} callback The callback that handles the response. Expects err and an array of validation errors null if none.
 */
Schema.prototype.validateFile = function(sourcePath, callback) {
	var that = this;
	fs.readFile(sourcePath, 'utf8', function(err, data){
		if (err) return callback(err);
		that.validate(data, callback);
	});
};
/**
 * Callback to the Schema.validateFile function
 * @callback Schema~validateFileCallback
 * @param {error} [err] - Error when attempting to validate (not a validation error).
 * @param {array} [validationErrors] - A array of errors from validating the schema, null if none.
 */
