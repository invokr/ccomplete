# cComplete - Code9 C / C++ Code Completion
## About

This plugin enables C / C++ code completion and diagnostics using libclang.

 - node_ccomplete -> Native (limited) Node.js libclang bindings
 - cloud9.ccomplete -> Server extension providing complete / diagnostic functions for the client
 - ext.ccomplete -> Client extension for parsing server results and adding settings / menu items
 - ext.language -> Modification of the original ext.language/complete.js file 

## License

This code is licenses under the Public Domain license. If this license is not available in your
jurisdiction the Apache 2.0 licenses applies (see LICENSE).

## Screenshots

 - complete1.png -> Completion of a non-native container with descriptions shown
 - complete2.png -> Diagnostics dumped in the console: missing ";" after the struct and incomplete statment

## Features
 
 - C / C++ Code Completion
 - C / C++ Error reporting
 - Translation unit caching for a 50% speed increase in subsequent completions

Parsing of large files (e.g. include all boost libraries at once) takes about 4.0 seconds. My own projects
take between 0.5 and 1.5 seconds to parse.

## Drawbacks

 - Error reporting can only be invoked via the console using the command `diagnose`
 - Can only be installed on self-hosted Cloud9 instances

Due to the way Cloud9 separates completion and error reporting from the rest of the IDE, I have no 
idea how to pass the data from the server extension over to the worker thread.

In addition to that, even if I knew how to overcome that issue, the worker thread uses pre-generated
files that are only regenerated when running `make worker`. This makes it impossible to add custom
language handlers on the fly without access to the Cloud9 instance. 

## Install

### node_ccomplete

Requirements: libclang; llvm; nodejs (0.8.8+)

    cd ccomplete/native
    node-waf configure
    node-waf
    cp build/Release/ccomplete.node ../cloud9.ccomplete
 
If there are any linking errors when building the extension, edit the file `ccomplete/native/wscript`.
Adjust the `LINKFLAGS` on line 7 to point to your clang / llvm location.

You can try if the extension works by running the demo/demo.js file which completes functions for
demo/demo.cpp.

### cloud9.ccomplete

Requirements: node_ccomplete

    cd ccomplete
    cp -R cloud9.ccomplete /cloud9/plugins-server
    
To enable the extension, add it to the to the config object in /cloud9/config/default.js on line
162:

    [...]
    },
    "./cloud9.ccomplete",
    "./cloud9.socket",
    [...]
    
Restart your Cloud9 instance. If it's starting up without any errors you're done installing. 
If the server crashes with a "ccomplete.node not found" error, you haven't installed node_ccomplete. 
If you do have it installed but still get this error, open /cloud9/plugins-server/ccomplete.js and set 
the path to it manually on line 5.

    var Clang = require("/my/path/to/ccomplete.node");

### ext.ccomplete

Requirements: node_ccomplete cloud9.ccomplete

    cd ccomplete
    cp -R ext.ccomplete /cloud9/plugins-client
    
To enable the extension, add it to the clientPlugins array in /cloud9/config/default.js between 
"/ext/codecomplete/codecomplete" and "/ext/vim/vim" on line 130:

    clientPlugins: [
        [...]
        "ext/codecomplete/codecomplete",
        "ext/ccomplete/ccomplete",           <- (!)
        "ext/vim/vim",
        [...]
    ]

The extension is working when you see the C / C++ settings category in the preferences
panel and the Tools -> C++ Menu entry.

### ext.language

Requirements: node_ccomplete cloud9.ccomplete ext.ccomplete

This will enable the actual code completion when pressing strg+space in a "c_cpp"-language file.
Make sure the other plugins work as far as you can tell before replacing the complete.js file.

    cd ccomplete/ext.language
    cp /cloud9/plugins-client/ext.language/language.js language.js.orig
    rm /cloud9/plugins-client/ext.language/language.js
    cp language.js /cloud9/plugins-client/ext.language/

Restart Cloud9. When pressing Strg+Space you should now see completion results. If there is an insane
amount of syntactic errors in your file and clang wont be able to build a translation unit, try running
`diagnose` in the console to see if that gives you any result. Be sure to test the plugin with a simple
no-includes-required C / C++ file completing a local struct.