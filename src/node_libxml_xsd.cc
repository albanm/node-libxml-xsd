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

void none(void *ctx, const char *msg, ...) {
  // do nothing
  return;
}

NAN_METHOD(SchemaSync) {
  	Nan::HandleScope scope;

    libxmljs::XmlDocument* doc = Nan::ObjectWrap::Unwrap<libxmljs::XmlDocument>(info[0]->ToObject());

    xmlSchemaParserCtxtPtr parser_ctxt = xmlSchemaNewDocParserCtxt(doc->xml_obj);
    if (parser_ctxt == NULL) {
        return Nan::ThrowError("Could not create context for schema parser");
    }
    xmlSchemaValidityErrorFunc err;
    xmlSchemaValidityWarningFunc warn;
    void* ctx;
    xmlSchemaGetParserErrors(parser_ctxt, &err, &warn, &ctx);
    xmlSchemaSetParserErrors(parser_ctxt, err, (xmlSchemaValidityWarningFunc) none, ctx);
    xmlSchemaPtr schema = xmlSchemaParse(parser_ctxt);
    if (schema == NULL) {
        return Nan::ThrowError("Invalid XSD schema");
    }

    Local<Object> schemaWrapper = Schema::New(schema);
  	info.GetReturnValue().Set(schemaWrapper);
}

// for memory the segfault i previously fixed were due to xml documents being deleted
// by garbage collector before their associated schema.
class SchemaWorker : public Nan::AsyncWorker {
 public:
  SchemaWorker(libxmljs::XmlDocument* doc, Nan::Callback *callback)
    : Nan::AsyncWorker(callback), doc(doc) {}
  ~SchemaWorker() {}

  // Executed inside the worker-thread.
  // It is not safe to access V8, or V8 data structures
  // here, so everything we need for input and output
  // should go on `this`.
  void Execute () {
    libxmljs::WorkerSentinel workerSentinel(workerParent);
  	parser_ctxt = xmlSchemaNewDocParserCtxt(doc->xml_obj);
    if (parser_ctxt != NULL) {
        xmlSchemaValidityErrorFunc err;
        xmlSchemaValidityWarningFunc warn;
        void* ctx;
        xmlSchemaGetParserErrors(parser_ctxt, &err, &warn, &ctx);
        xmlSchemaSetParserErrors(parser_ctxt, err, (xmlSchemaValidityWarningFunc) none, ctx);
        result = xmlSchemaParse(parser_ctxt);
    }
  }

  // Executed when the async work is complete
  // this function will be run inside the main event loop
  // so it is safe to use V8 again
  void HandleOKCallback () {
    Nan::HandleScope scope;
    if (parser_ctxt == NULL) {
		Local<Value> argv[] = { Nan::Error("Could not create context for schema parser") };
        callback->Call(2, argv);
    } else if (result == NULL) {
        Local<Value> argv[] = { Nan::Error("Invalid XSD schema") };
        callback->Call(2, argv);
    } else {
        Local<Object> resultWrapper = Schema::New(result);
        Local<Value> argv[] = { Nan::Null(), resultWrapper };
        callback->Call(2, argv);
    }
  };

 private:
  libxmljs::WorkerParent workerParent;
  libxmljs::XmlDocument* doc;
  xmlSchemaParserCtxtPtr parser_ctxt;
  xmlSchemaPtr result;
};

NAN_METHOD(SchemaAsync) {
    Nan::HandleScope scope;

    libxmljs::XmlDocument* doc = Nan::ObjectWrap::Unwrap<libxmljs::XmlDocument>(info[0]->ToObject());
    Nan::Callback *callback = new Nan::Callback(info[1].As<Function>());
    Nan::AsyncQueueWorker(new SchemaWorker(doc, callback));
    return;
}

NAN_METHOD(ValidateSync) {
    Nan::HandleScope scope;

    // Prepare the array of errors to be filled by validation
    //v8::Local<v8::Array> errors = Nan::New<v8::Array>();
    //xmlResetLastError();
    //xmlSetStructuredErrorFunc(reinterpret_cast<void *>(&errors),
    //        libxmljs::XmlSyntaxError::PushToArray);
    libxmljs::XmlSyntaxErrorsSync errors;

    // Extract schema an document to validate from their wrappers
    Schema* schema = Nan::ObjectWrap::Unwrap<Schema>(info[0]->ToObject());
    libxmljs::XmlDocument* doc = Nan::ObjectWrap::Unwrap<libxmljs::XmlDocument>(info[1]->ToObject());

    // Actual validation
    xmlSchemaValidCtxtPtr valid_ctxt = xmlSchemaNewValidCtxt(schema->schema_obj);
    if (valid_ctxt == NULL) {
        return Nan::ThrowError("Unable to create a validation context for the schema");
    }
    xmlSchemaValidateDoc(valid_ctxt, doc->xml_obj);

	  xmlSetStructuredErrorFunc(NULL, NULL);

    // Don't return the boolean result, instead return array of validation errors
    // will be empty if validation is ok
    info.GetReturnValue().Set(errors.ToArray());
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
class ValidateWorker : public Nan::AsyncWorker {
 public:
  ValidateWorker(Schema* schema, libxmljs::XmlDocument* doc, Nan::Callback *callback)
    : Nan::AsyncWorker(callback), schema(schema), doc(doc) {}

  libxmljs::XmlSyntaxErrorsStore errors;

  // Executed inside the worker-thread.
  // It is not safe to access V8, or V8 data structures
  // here, so everything we need for input and output
  // should go on `this`.
  void Execute () {
    libxmljs::WorkerSentinel workerSentinel(workerParent);
    libxmljs::XmlSyntaxErrorsAsync errorsSentinel(errors);

    // Actual validation
    valid_ctxt = xmlSchemaNewValidCtxt(schema->schema_obj);

    xmlSchemaValidateDoc(valid_ctxt, doc->xml_obj);
  }

  // Executed when the async work is complete
  // this function will be run inside the main event loop
  // so it is safe to use V8 again
  void HandleOKCallback () {
    Nan::HandleScope scope;

    if (valid_ctxt == NULL) {
        Local<Value> argv[] = { Nan::Error("Unable to create a validation context for the schema") };
        callback->Call(2, argv);
    } else {
        Local<Value> argv[] = { Nan::Null(), errors.ToArray() };
        callback->Call(2, argv);
    }
  };

  private:
    libxmljs::WorkerParent workerParent;
    Schema* schema;
    libxmljs::XmlDocument* doc;
    xmlSchemaValidCtxtPtr valid_ctxt;
};

NAN_METHOD(ValidateAsync) {
    Nan::HandleScope scope;

    Schema* schema = Nan::ObjectWrap::Unwrap<Schema>(info[0]->ToObject());
    libxmljs::XmlDocument* doc = Nan::ObjectWrap::Unwrap<libxmljs::XmlDocument>(info[1]->ToObject());
    Nan::Callback *callback = new Nan::Callback(info[2].As<Function>());

    Nan::AsyncQueueWorker(new ValidateWorker(schema, doc, callback));
    return;
}

// Compose the module by assigning the methods previously prepared
void InitAll(Handle<Object> exports) {
  	Schema::Init(exports);
  	exports->Set(Nan::New<String>("schemaSync").ToLocalChecked(), Nan::New<FunctionTemplate>(SchemaSync)->GetFunction());
    exports->Set(Nan::New<String>("schemaAsync").ToLocalChecked(), Nan::New<FunctionTemplate>(SchemaAsync)->GetFunction());
  	exports->Set(Nan::New<String>("validateSync").ToLocalChecked(), Nan::New<FunctionTemplate>(ValidateSync)->GetFunction());
    exports->Set(Nan::New<String>("validateAsync").ToLocalChecked(), Nan::New<FunctionTemplate>(ValidateAsync)->GetFunction());
}
NODE_MODULE(node_libxml_xsd, InitAll);
