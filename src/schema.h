// Very simple v8 wrapper for xslt stylesheet, see "Wrapping C++ objects" section here http://nodejs.org/api/addons.html

#ifndef SRC_SCHEMA_H_
#define SRC_SCHEMA_H_

#include <libxml/xmlschemas.h>

class Schema : public Nan::ObjectWrap {
	public:
	    static void Init(v8::Handle<v8::Object> exports);
	    static v8::Local<v8::Object> New(xmlSchemaPtr schema);
	    xmlSchemaPtr schema_obj;

	private:
	    explicit Schema(xmlSchemaPtr schema);
	    ~Schema();
	    static Nan::Persistent<v8::Function> constructor;
};

#endif  // SRC_SCHEMA_H_