/**
 * This file is part of cComplete (https://bitbucket.org/blue-dev/ccomplete).
 *
 * @copyright 2013, Robin Dietrich
 * @license Apache 2.0 <http://www.apache.org/licenses/LICENSE-2.0>
 * @license Public Domain <>
 */

#include <algorithm>

#include "ccomplete.hpp"

Persistent<FunctionTemplate> ccomplete::constructor;

ccomplete::ccomplete() : ObjectWrap(), args(), cIndex(nullptr), cCache() {
    cIndex = clang_createIndex(1, 1);
    cCache.setCallback([](CXTranslationUnit u) {
        clang_disposeTranslationUnit(u);
    });
}

ccomplete::~ccomplete() {
    clang_disposeIndex(cIndex);
}

Handle<Value> ccomplete::New(const Arguments &args) {
    HandleScope scope;
    ccomplete *c = new ccomplete();
    c->Wrap(args.This());            
    return args.This();
}

void ccomplete::Init(Handle<Object> target) {
    HandleScope scope;
    
    // wrap new and make it persistent
    Local<FunctionTemplate> local_function_template = FunctionTemplate::New(New); 
    ccomplete::constructor = Persistent<FunctionTemplate>::New(local_function_template);
        
    ccomplete::constructor->InstanceTemplate()->SetInternalFieldCount(1);
    ccomplete::constructor->SetClassName(String::NewSymbol("ccomplete"));        
    
    // accessors for args, cachesize
    ccomplete::constructor->InstanceTemplate()->SetAccessor(String::New("args"), GetArgs, SetArgs);
    ccomplete::constructor->InstanceTemplate()->SetAccessor(String::New("cachesize"), GetCache, SetCache);
    
  
    // make methods available for node
    NODE_SET_PROTOTYPE_METHOD(ccomplete::constructor, "Version", Version);      
    NODE_SET_PROTOTYPE_METHOD(ccomplete::constructor, "Complete", Complete);
    NODE_SET_PROTOTYPE_METHOD(ccomplete::constructor, "Diagnose", Diagnose);
    
    target->Set(String::NewSymbol("ccomplete"), ccomplete::constructor->GetFunction());
}

Handle<Value> ccomplete::Version(const Arguments& args) {
    HandleScope scope;    
    return scope.Close(v8::String::New("V1.0"));
}

Handle<Value> ccomplete::GetArgs(Local<String> property, const AccessorInfo& info) {
    HandleScope scope;  
    ccomplete* instance = node::ObjectWrap::Unwrap<ccomplete>(info.Holder());
      
    Handle<Array> res = Array::New(instance->args.size());
    for (std::size_t i = 0; i < instance->args.size(); ++i)
        res->Set(i, String::New(instance->args[i].c_str()));
        
    return scope.Close(res);
}   

void ccomplete::SetArgs(Local<String> property, Local<Value> value, const AccessorInfo& info) {
    ccomplete* instance = node::ObjectWrap::Unwrap<ccomplete>(info.Holder());
    instance->args.clear();
    
    if (value->IsArray()) {
        Handle<Array> arr = Handle<Array>::Cast(value);
        for (std::size_t i = 0; i < arr->Length(); ++i) {
            String::Utf8Value str(arr->Get(i));            
            instance->args.push_back( *str );
        }
    } else if (value->IsString()) {
        String::Utf8Value str(value);
        instance->args.push_back(*str);
    } else {
        ThrowException(
            Exception::TypeError(String::New("First argument must be a String or an Array"))
        );
    }
}

Handle<Value> ccomplete::GetCache(Local<String> property, const AccessorInfo& info) {
    HandleScope scope;  
    ccomplete* instance = node::ObjectWrap::Unwrap<ccomplete>(info.Holder());
    
    // !fix! might overflow when cache entry max is > 32 bit
    Handle<Integer> res = Integer::New(instance->cCache.getSize());        
    return scope.Close(res);
}   

void ccomplete::SetCache(Local<String> property, Local<Value> value, const AccessorInfo& info) {
    ccomplete* instance = node::ObjectWrap::Unwrap<ccomplete>(info.Holder());
    
    if (value->IsUint32()) {
        instance->cCache.setSize(value->ToUint32()->Value());
    } else {
        ThrowException(
            Exception::TypeError(String::New("First argument must be an Integer"))
        );
    }
}

Handle<Value> ccomplete::Complete(const Arguments& args) {
    // check if the fuction is called correctly
    if (args.Length() != 3) 
         return ThrowException(
            Exception::SyntaxError(String::New("Usage: filename, row, collumn"))
        );
    
    if (!args[0]->IsString())
        return ThrowException(
            Exception::TypeError(String::New("First argument must be a String"))
        );
        
    if (!args[1]->IsUint32())
        return ThrowException(
            Exception::TypeError(String::New("Second argument must be an Integer"))
        );
    
    if (!args[2]->IsUint32())
        return ThrowException(
            Exception::TypeError(String::New("Third argument must be an Integer"))
        );
    
    HandleScope scope;  
    ccomplete* instance = node::ObjectWrap::Unwrap<ccomplete>(args.This());    
    
    // convert string vector to const char* vector for clang
    std::vector<const char*> cArgs;
    std::transform(
        instance->args.begin(),
        instance->args.end(),
        std::back_inserter(cArgs),
        [](const std::string &s) -> const char* {
            return s.c_str();
        }
    );
      
    // complete code using clang
    Handle<Array> ret = Array::New(); 
    String::Utf8Value file(args[0]);
    uint32_t row = args[1]->ToUint32()->Value();
    uint32_t col = args[2]->ToUint32()->Value();
    
    unsigned options = CXTranslationUnit_SkipFunctionBodies | CXTranslationUnit_PrecompiledPreamble;
        
    CXTranslationUnit trans;
    if (instance->cCache.has(*file)) 
        trans = instance->cCache.retrieve(*file);
    else 
        trans = instance->cCache.insert(*file, clang_parseTranslationUnit(instance->cIndex, *file, &cArgs[0], cArgs.size(), NULL, 0, options));
    
    if (!trans) 
        return ThrowException(Exception::Error(String::New("Unable to build translation unit")));
    
    CXCodeCompleteResults *res = clang_codeCompleteAt(trans, *file, row, col, NULL, 0, 0);
    
    uint32_t j = 0;
    for (unsigned i = 0; i < res->NumResults; ++i) {
        // skip unessecary completion results
        if (clang_getCompletionAvailability(res->Results[i].CompletionString) == CXAvailability_NotAccessible || !instance->completeCursor(res->Results[i].CursorKind))
            continue;
            
        std::string desc = "";
        std::string returnType = instance->returnType(res->Results[i].CursorKind);
        uint32_t results = clang_getNumCompletionChunks(res->Results[i].CompletionString);
        
        // build an informative string from the completion results
        for (uint32_t k = 0; k < results; ++k) {
            CXCompletionChunkKind cKind = clang_getCompletionChunkKind(res->Results[i].CompletionString, k);
            CXString cText = clang_getCompletionChunkText(res->Results[i].CompletionString, k);
            
            const char *text = clang_getCString(cText);
            if (!text) text = "";
            
            if (cKind == CXCompletionChunk_ResultType) {
                returnType = text;
            } else {
                desc += std::string("|||")+text;
            }
            
            clang_disposeString(cText);
        }
        
        desc = returnType + desc;
            
        ret->Set(j++, String::New(desc.c_str()));
    }

    clang_disposeCodeCompleteResults(res);  
    return scope.Close(ret);
}

Handle<Value> ccomplete::Diagnose(const Arguments& args) {
    // check if the fuction is called correctly
    if (args.Length() != 1) 
         return ThrowException(
            Exception::SyntaxError(String::New("Usage: filename"))
        );
    
    if (!args[0]->IsString())
        return ThrowException(
            Exception::TypeError(String::New("First argument must be a String"))
        );
    
    HandleScope scope;  
    ccomplete* instance = node::ObjectWrap::Unwrap<ccomplete>(args.This());    
    
    // convert string vector to const char* vector for clang
    std::vector<const char*> cArgs;
    std::transform(
        instance->args.begin(),
        instance->args.end(),
        std::back_inserter(cArgs),
        [](const std::string &s) -> const char* {
            return s.c_str();
        }
    );
      
    Handle<Array> ret = Array::New(); 
    String::Utf8Value file(args[0]);
    
    // dont skip function bodies when error checking and always overwrite the current unit if any
    unsigned options = CXTranslationUnit_PrecompiledPreamble;        
    CXTranslationUnit trans = instance->cCache.insert(*file, clang_parseTranslationUnit(instance->cIndex, *file, &cArgs[0], cArgs.size(), NULL, 0, options));
    
    if (!trans) 
        return ThrowException(Exception::Error(String::New("Unable to build translation unit")));
    
    int dOpt = CXDiagnostic_DisplaySourceLocation | CXDiagnostic_DisplayColumn;
    
    // iterate through diagnostics
    uint32_t j = 0;
    for (uint32_t i = 0; i < clang_getNumDiagnostics(trans); ++i) {
        CXDiagnostic d = clang_getDiagnostic(trans, i);
        CXString str = clang_formatDiagnostic(d, dOpt);
        std::string r(clang_getCString(str));
        
        ret->Set(j++, String::New(r.c_str()));
        
        clang_disposeDiagnostic(d);
    }
    
    return scope.Close(ret);
}

bool ccomplete::completeCursor(CXCursorKind c) {
    switch (c) {
        case CXCursor_CXXMethod:
        //case CXCursor_NotImplemented:
        case CXCursor_FieldDecl:
        case CXCursor_ObjCPropertyDecl:
        case CXCursor_ObjCClassMethodDecl:
        case CXCursor_ObjCInstanceMethodDecl:
        case CXCursor_ObjCIvarDecl:
        case CXCursor_FunctionTemplate:
        //case CXCursor_TypedefDecl:
        case CXCursor_Namespace:
            return true;
        default:
            return false;
    }
}

std::string ccomplete::returnType(CXCursorKind ck) {
    switch (ck) {
        case CXCursor_ObjCInterfaceDecl: 
        case CXCursor_ClassTemplate:     
        case CXCursor_ClassDecl:         
            return "class";
        case CXCursor_EnumDecl:
            return "enum";
        case CXCursor_StructDecl:
            return "struct";
        case CXCursor_MacroDefinition:
            return "macro";
        case CXCursor_NamespaceAlias:    
        case CXCursor_Namespace:
            return "namespace";
        case CXCursor_TypedefDecl:
            return "typedef";
        case CXCursor_Constructor:
            return "constructor";        
        case CXCursor_Destructor:
            return "destructor";
        case CXCursor_UnionDecl: 
            return "union";
        default:
            return "";
    }
}

extern "C" {
    void RegisterModule(Handle<Object> target) {
        ccomplete::Init(target);
    }
}

NODE_MODULE(ccomplete, RegisterModule);