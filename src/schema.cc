#include <node.h>
#include <nan.h>

#include "./schema.h"

using namespace v8;

Persistent<Function> Schema::constructor;

Schema::Schema(xmlSchemaPtr schemaPtr) : schema_obj(schemaPtr) {}

Schema::~Schema()
{
    // TODO, potential memory leak here ?
    // We can't free the schema as the xml doc inside was probably
    // already deleted by garbage collector and this results in segfaults
}

void Schema::Init(Handle<Object> exports) {
	 // Prepare constructor template
    Local<FunctionTemplate> tpl = NanNew<FunctionTemplate>();
    tpl->SetClassName(NanNew<String>("Schema"));
  	tpl->InstanceTemplate()->SetInternalFieldCount(1);
  	
    NanAssignPersistent(constructor, tpl->GetFunction());
}

// not called from node, private api
Local<Object> Schema::New(xmlSchemaPtr schemaPtr) {
    NanEscapableScope();
    Local<Object> wrapper = NanNew(constructor)->NewInstance();
    Schema* schema = new Schema(schemaPtr);
    schema->Wrap(wrapper);
    return NanEscapeScope(wrapper);
}
