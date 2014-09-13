#define BUILDING_NODE_EXTENSION
#include <iostream>
#include <list>
#include <node.h>
#include <nan.h>

// includes from libxmljs
#include <xml_syntax_error.h>
#include <xml_document.h>

#include "./node_libxml_xsd.h"
#include "./schema.h"

using namespace v8;

NAN_METHOD(SchemaSync) {
  	NanScope();

    libxmljs::XmlDocument* doc = node::ObjectWrap::Unwrap<libxmljs::XmlDocument>(args[0]->ToObject());
    
    xmlSchemaParserCtxtPtr parser_ctxt = xmlSchemaNewDocParserCtxt(doc->xml_obj);
    if (parser_ctxt == NULL) {
        return NanThrowError("Could not create context for schema parser");
    }
    xmlSchemaPtr schema = xmlSchemaParse(parser_ctxt);
    if (schema == NULL) {
        return NanThrowError("Invalid XSD schema");
    }

    Local<Object> schemaWrapper = Schema::New(schema);
  	NanReturnValue(schemaWrapper);
}

// for memory the segfault i previously fixed were due to xml documents being deleted
// by garbage collector before their associated schema.
class SchemaWorker : public NanAsyncWorker {
 public:
  SchemaWorker(libxmljs::XmlDocument* doc, NanCallback *callback)
    : NanAsyncWorker(callback), doc(doc) {}
  ~SchemaWorker() {}

  // Executed inside the worker-thread.
  // It is not safe to access V8, or V8 data structures
  // here, so everything we need for input and output
  // should go on `this`.
  void Execute () {
  	parser_ctxt = xmlSchemaNewDocParserCtxt(doc->xml_obj);
    if (parser_ctxt != NULL) {
        result = xmlSchemaParse(parser_ctxt);
    }
  }

  // Executed when the async work is complete
  // this function will be run inside the main event loop
  // so it is safe to use V8 again
  void HandleOKCallback () {
    NanScope();
    if (parser_ctxt == NULL) {
		Local<Value> argv[] = { NanError("Could not create context for schema parser") };
        callback->Call(2, argv);
    } else if (result == NULL) {
        Local<Value> argv[] = { NanError("Invalid XSD schema") };
        callback->Call(2, argv);
    } else {
        Local<Object> resultWrapper = Schema::New(result);
        Local<Value> argv[] = { NanNull(), resultWrapper };
        callback->Call(2, argv);
    }
  };

 private:
  libxmljs::XmlDocument* doc;
  xmlSchemaParserCtxtPtr parser_ctxt;
  xmlSchemaPtr result;
};

NAN_METHOD(SchemaAsync) {
    NanScope();

    libxmljs::XmlDocument* doc = node::ObjectWrap::Unwrap<libxmljs::XmlDocument>(args[0]->ToObject());
    NanCallback *callback = new NanCallback(args[1].As<Function>());
    NanAsyncQueueWorker(new SchemaWorker(doc, callback));
    NanReturnUndefined();
}

NAN_METHOD(ValidateSync) {
    NanScope();

    // Prepare the array of errors to be filled by validation
    v8::Local<v8::Array> errors = NanNew<v8::Array>();
    xmlResetLastError();
    xmlSetStructuredErrorFunc(reinterpret_cast<void *>(&errors),
            libxmljs::XmlSyntaxError::PushToArray);

    // Extract schema an document to validate from their wrappers
    Schema* schema = node::ObjectWrap::Unwrap<Schema>(args[0]->ToObject());
    libxmljs::XmlDocument* doc = node::ObjectWrap::Unwrap<libxmljs::XmlDocument>(args[1]->ToObject());
	
    // Actual validation
    xmlSchemaValidCtxtPtr valid_ctxt = xmlSchemaNewValidCtxt(schema->schema_obj);
    if (valid_ctxt == NULL) {
        return NanThrowError("Unable to create a validation context for the schema");
    }
    xmlSchemaValidateDoc(valid_ctxt, doc->xml_obj);

	  xmlSetStructuredErrorFunc(NULL, NULL);

    // Don't return the boolean result, instead return array of validation errors
    // will be empty if validation is ok
    NanReturnValue(errors);
}

// Used by async validate to store a list or validation errors
// required as libxmljs::XmlSyntaxError::PushToArray uses V8 objects which are not available
// in a async worker using libuv queue
void PushErrorsToList(void* errs, xmlError* error) {
  std::list<xmlError*>* errors = *reinterpret_cast<std::list<xmlError*>**>(errs);
  errors->push_back(error);
}


// for memory the segfault i previously fixed were due to xml documents being deleted
// by garbage collector before their associated schema.
class ValidateWorker : public NanAsyncWorker {
 public:
  ValidateWorker(Schema* schema, libxmljs::XmlDocument* doc, NanCallback *callback)
    : NanAsyncWorker(callback), schema(schema), doc(doc) {
      errors = new std::list<xmlError*>;
    }
  ~ValidateWorker() {
    delete errors;
  }

  // Executed inside the worker-thread.
  // It is not safe to access V8, or V8 data structures
  // here, so everything we need for input and output
  // should go on `this`.
  void Execute () {
    xmlResetLastError();

    xmlSetStructuredErrorFunc(reinterpret_cast<void *>(&errors), PushErrorsToList);
    
    // Actual validation
    valid_ctxt = xmlSchemaNewValidCtxt(schema->schema_obj);
    
    xmlSchemaValidateDoc(valid_ctxt, doc->xml_obj);

    xmlSetStructuredErrorFunc(NULL, NULL);
  }

  // Executed when the async work is complete
  // this function will be run inside the main event loop
  // so it is safe to use V8 again
  void HandleOKCallback () {
    NanScope();

    if (valid_ctxt == NULL) {
        Local<Value> argv[] = { NanError("Unable to create a validation context for the schema") };
        callback->Call(2, argv);
    } else {
        // Wrap validation errors in V8 objects
        v8::Local<v8::Array> v8_errors = NanNew<v8::Array>();
        // push method for array
        v8::Local<v8::Function> push = v8::Local<v8::Function>::Cast(v8_errors->Get(NanNew<v8::String>("push")));

        for (std::list<xmlError*>::iterator it = errors->begin(); it != errors->end(); it++) {
          v8::Local<v8::Value> argv[1] = { libxmljs::XmlSyntaxError::BuildSyntaxError(*it) };
          push->Call(v8_errors, 1, argv);
        }
        
        Local<Value> argv[] = { NanNull(), v8_errors };
        callback->Call(2, argv);
    }
  };

  private:
    Schema* schema;
    libxmljs::XmlDocument* doc;
    xmlSchemaValidCtxtPtr valid_ctxt;
    std::list<xmlError*>* errors;
};

NAN_METHOD(ValidateAsync) {
    NanScope();

    Schema* schema = node::ObjectWrap::Unwrap<Schema>(args[0]->ToObject());
    libxmljs::XmlDocument* doc = node::ObjectWrap::Unwrap<libxmljs::XmlDocument>(args[1]->ToObject());
    NanCallback *callback = new NanCallback(args[2].As<Function>());

    NanAsyncQueueWorker(new ValidateWorker(schema, doc, callback));
    NanReturnUndefined();
}

// Compose the module by assigning the methods previously prepared
void InitAll(Handle<Object> exports) {
  	Schema::Init(exports);
  	exports->Set(NanNew<String>("schemaSync"), NanNew<FunctionTemplate>(SchemaSync)->GetFunction());
    exports->Set(NanNew<String>("schemaAsync"), NanNew<FunctionTemplate>(SchemaAsync)->GetFunction());
  	exports->Set(NanNew<String>("validateSync"), NanNew<FunctionTemplate>(ValidateSync)->GetFunction());
    exports->Set(NanNew<String>("validateAsync"), NanNew<FunctionTemplate>(ValidateAsync)->GetFunction());
}
NODE_MODULE(node_libxml_xsd, InitAll);