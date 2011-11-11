
// DO NOT EDIT THIS FILE - it is machine generated -*- c++ -*-

#ifndef __gnu_xml_pipeline_TeeConsumer__
#define __gnu_xml_pipeline_TeeConsumer__

#pragma interface

#include <java/lang/Object.h>
#include <gcj/array.h>

extern "Java"
{
  namespace gnu
  {
    namespace xml
    {
      namespace pipeline
      {
          class EventConsumer;
          class TeeConsumer;
      }
    }
  }
  namespace org
  {
    namespace xml
    {
      namespace sax
      {
          class Attributes;
          class ContentHandler;
          class DTDHandler;
          class ErrorHandler;
          class Locator;
        namespace ext
        {
            class DeclHandler;
            class LexicalHandler;
        }
      }
    }
  }
}

class gnu::xml::pipeline::TeeConsumer : public ::java::lang::Object
{

public:
  TeeConsumer(::gnu::xml::pipeline::EventConsumer *, ::gnu::xml::pipeline::EventConsumer *);
  ::gnu::xml::pipeline::EventConsumer * getFirst();
  ::gnu::xml::pipeline::EventConsumer * getRest();
  ::org::xml::sax::ContentHandler * getContentHandler();
  ::org::xml::sax::DTDHandler * getDTDHandler();
  ::java::lang::Object * getProperty(::java::lang::String *);
  void setErrorHandler(::org::xml::sax::ErrorHandler *);
  void setDocumentLocator(::org::xml::sax::Locator *);
  void startDocument();
  void endDocument();
  void startPrefixMapping(::java::lang::String *, ::java::lang::String *);
  void endPrefixMapping(::java::lang::String *);
  void skippedEntity(::java::lang::String *);
  void startElement(::java::lang::String *, ::java::lang::String *, ::java::lang::String *, ::org::xml::sax::Attributes *);
  void endElement(::java::lang::String *, ::java::lang::String *, ::java::lang::String *);
  void processingInstruction(::java::lang::String *, ::java::lang::String *);
  void characters(JArray< jchar > *, jint, jint);
  void ignorableWhitespace(JArray< jchar > *, jint, jint);
  void notationDecl(::java::lang::String *, ::java::lang::String *, ::java::lang::String *);
  void unparsedEntityDecl(::java::lang::String *, ::java::lang::String *, ::java::lang::String *, ::java::lang::String *);
  void attributeDecl(::java::lang::String *, ::java::lang::String *, ::java::lang::String *, ::java::lang::String *, ::java::lang::String *);
  void elementDecl(::java::lang::String *, ::java::lang::String *);
  void externalEntityDecl(::java::lang::String *, ::java::lang::String *, ::java::lang::String *);
  void internalEntityDecl(::java::lang::String *, ::java::lang::String *);
  void comment(JArray< jchar > *, jint, jint);
  void startCDATA();
  void endCDATA();
  void startEntity(::java::lang::String *);
  void endEntity(::java::lang::String *);
  void startDTD(::java::lang::String *, ::java::lang::String *, ::java::lang::String *);
  void endDTD();
private:
  ::gnu::xml::pipeline::EventConsumer * __attribute__((aligned(__alignof__( ::java::lang::Object)))) first;
  ::gnu::xml::pipeline::EventConsumer * rest;
  ::org::xml::sax::ContentHandler * docFirst;
  ::org::xml::sax::ContentHandler * docRest;
  ::org::xml::sax::ext::DeclHandler * declFirst;
  ::org::xml::sax::ext::DeclHandler * declRest;
  ::org::xml::sax::ext::LexicalHandler * lexFirst;
  ::org::xml::sax::ext::LexicalHandler * lexRest;
public:
  static ::java::lang::Class class$;
};

#endif // __gnu_xml_pipeline_TeeConsumer__