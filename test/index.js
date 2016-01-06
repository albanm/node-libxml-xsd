var fs = require('fs');

var should = require('should');

var xsd = require('../index');
var libxmljs = xsd.libxmljs;

describe('node-libxml-xsd', function() {
	var schemaSource;
	before(function(callback) {
		fs.readFile('./test/resources/chapter04ord1.xsd', 'utf8', function(err, data) {
			schemaSource = data;
			callback(err);
		});
	});
	var docSource;
	before(function(callback) {
		fs.readFile('./test/resources/chapter04.xml', 'utf8', function(err, data) {
			docSource = data;
			callback(err);
		});
	});

	var schema;
	describe('synchronous parse function', function() {
		it('should parse a schema from a libxmljs xml document', function() {
			var schemaDoc = libxmljs.parseXml(schemaSource);
			schema = xsd.parse(schemaDoc);
			schema.should.be.type('object');
		});
		it('should parse a schema from a xml string', function() {
			schema = xsd.parse(schemaSource);
			schema.should.be.type('object');
		});
		it('should throw an error when parsing invalid schema', function() {
			(function() {
				xsd.parse('this is not a schema!');
			}).should.throw();
		});
	});

	describe('parseFile function', function() {
		it('should parse a schema from a file', function(callback) {
			xsd.parseFile('./test/resources/chapter04ord1.xsd', function(err, schema) {
				schema.should.be.type('object');
				callback(err);
			});
		});
	});

	describe('asynchronous parse function', function() {
		it('should parse a schema from a libxmljs xml document', function(callback) {
			var schemaDoc = libxmljs.parseXml(schemaSource);
			xsd.parse(schemaDoc, function(err, schema) {
				schema.should.be.type('object');
				callback(err);
			});
		});
		it('should parse a schema from a xml string', function(callback) {
			xsd.parse(schemaSource, function(err, schema) {
				schema.should.be.type('object');
				callback(err);
			});
		});
		it('should return an error when parsing invalid schema', function(callback) {
			xsd.parse('this is not a schema!', function(err) {
				should.exist(err);
				callback();
			});
		});
	});


	describe('synchronous validate function', function() {
		it('should validate a schema to a libxmljs xml document', function() {
			var doc = libxmljs.parseXml(docSource);
			var validationErrors = schema.validate(doc);
			should.not.exist(validationErrors);
		});
		it('should validate a schema to a xml string', function() {
			var validationErrors = schema.validate(docSource);
			should.not.exist(validationErrors);
		});
		it('should throw an error if given a bad xml', function() {
			(function() {
				schema.validate('this is not valid!');
			}).should.throw();
		});
		it('should return validation errors if content is good xml but not valid', function() {
			var validationErrors = schema.validate('<test>this is not valid!</test>');
			validationErrors.should.be.type('object');
			validationErrors.should.have.lengthOf(1);
		});
	});


	describe('asynchronous validate function', function() {
		it('should validate a schema to a libxmljs xml document', function(callback) {
			var doc = libxmljs.parseXml(docSource);
			schema.validate(doc, function(err, validationErrors) {
				should.not.exist(err);
				should.not.exist(validationErrors);
				callback();
			});
		});
		it('should validate a schema to a xml string', function(callback) {
			schema.validate(docSource, function(err, validationErrors) {
				should.not.exist(err);
				should.not.exist(validationErrors);
				callback();
			});
		});
		it('should return an error if given a bad xml', function(callback) {
			schema.validate('this is not valid!', function(err, validationErrors) {
				should.exist(err);
				callback();
			});
		});
		it('should return validation errors if content is good xml but not valid', function(callback) {
			schema.validate('<test>this is not valid!<a></a></test>', function(err, validationErrors) {
				should.not.exist(err);
				validationErrors.should.have.lengthOf(1);
				callback();
			});
		});
	});

	describe('validateFile function', function() {
		it('should validate a schema to a xml file', function(callback) {
			schema.validateFile('./test/resources/chapter04.xml', function(err, validationErrors) {
				should.not.exist(err);
				should.not.exist(validationErrors);
				callback();
			});
		});
	});

});
