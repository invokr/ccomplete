#ifndef __MAIN_CCOMPLETE_HPP_
#define __MAIN_CCOMPLETE_HPP_

/**
 * This file is part of cComplete (https://bitbucket.org/blue-dev/ccomplete).
 *
 * @copyright 2013, Robin Dietrich
 * @license Apache 2.0 <http://www.apache.org/licenses/LICENSE-2.0>
 * @license Public Domain <>
 */

#include <string>
#include <vector>

#include <v8.h>
#include <node.h>

#include "clang/Index.h"
#include "lru.hpp"

using namespace v8;

/** code completion module for nodejs based on clang */
class ccomplete : public node::ObjectWrap {
    protected:
        /** list of arguments to pass to clang */
        std::vector<std::string> args;
        /** set of translation units */
        CXIndex cIndex;
        /** translation unit cache */
        lru<std::string, CXTranslationUnit> cCache;
    
        /** constructor, initializes index*/
        ccomplete();        
        /** destructor, frees index */
        ~ccomplete();
        /** called when a new instance is created */
        static Handle<Value> New(const Arguments& args);        
    public:
        static Persistent<FunctionTemplate> constructor;
        
        /** initialization function for node */
        static void Init(Handle<Object> target);
        /** returns module version */
        static Handle<Value> Version(const Arguments& args);
        /** returns current arguments supplied to clang */
        static Handle<Value> GetArgs(Local<String> property, const AccessorInfo& info); 
        /** sets arguments supploed to clang */
        static void SetArgs(Local<String> property, Local<Value> value, const AccessorInfo& info);
        /** returns maximum number of cache entries */
        static Handle<Value> GetCache(Local<String> property, const AccessorInfo& info); 
        /** sets maximum number of cache entries */
        static void SetCache(Local<String> property, Local<Value> value, const AccessorInfo& info);
        /** code completion function, [filename|row|col] */
        static Handle<Value> Complete(const Arguments& args);
        /** code diagnostics function, [filename] */
        static Handle<Value> Diagnose(const Arguments& args);
        
        /** checks if we should complete the given cursor */
        bool completeCursor(CXCursorKind c);
        /** gets return type of comletion function */
        std::string returnType(CXCursorKind ck);
};

#endif /* __MAIN_CCOMPLETE_HPP_ */