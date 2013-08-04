/**
 * This file is part of cComplete (https://bitbucket.org/blue-dev/ccomplete).
 *
 * @copyright 2013, Robin Dietrich
 * @license Apache 2.0 <http://www.apache.org/licenses/LICENSE-2.0>
 * @license Public Domain <>
 */

"use strict";

var Plugin = require("../cloud9.core/plugin");
var util = require("util");
var Clang = require("./ccomplete.node");

var name = "ccomplete";

module.exports = function setup(options, imports, register) {
    imports.ide.register(name, ccompletePlugin, register);
};

var ccompletePlugin = function(ide) {
    this.ide = ide;
    this.hooks = ["command"];
    this.name = "ccomplete";
    this.clang = new Clang.ccomplete();
};

util.inherits(ccompletePlugin, Plugin);

(function() {
    this.command = function(user, message, client) {
        if (message.command != "ccomplete")
            return false;

        var _self = this;
        
        // run command
        switch (message.subcommand) {
            case "version":
                _self.sendResult(0, "version", {
                    code: 0,
                    err: 0,
                    type: "version",
                    version: _self.clang.Version(),
                    out: null
                });
                break;
            case "complete":
                var matches = [];
                try {
                    matches = _self.clang.Complete(message.file, message.line, message.col);
                } catch (e) {
                    // just ignore, probably a type error, return empty set
                }
                
                _self.sendResult(0, "complete", {
                    code: 0,
                    err: 0,
                    type: "complete",
                    matches: matches,
                    out: null
                });
                break;
            case "diagnose":
                var matches = [];
                try {
                    matches = _self.clang.Diagnose(message.file);
                } catch (e) {
                    // just ignore, probably a type error, return empty set
                }
                
                _self.sendResult(0, "diagnose", {
                    code : 0,
                    err: 0,
                    type: "diagnose",
                    matches: matches,
                    out: null
                });
                break;
            case "args":
                if (message.args && message.args !== null)
                    _self.clang.args = message.args.split(",");
                
                _self.sendResult(0, "args", {
                   code: 0,
                   err: 0,
                   type: "args",
                   args: _self.clang.args.splice(","),
                   out: null
                });                    
                break;
            case "cachesize":
                if (message.cachesize && message.cachesize !== null)
                    _self.clang.cachesize = message.cachesize;
                    
                _self.sendResult(0, "cachesize", {
                    code: 0,
                    err: 0,
                    type: "cachesize",
                    cachesize: _self.clang.cachesize,
                    out: null
                });
                break;
            default:
                console.log("ccomplete: Wrong command " + message.subcommand);
                break;
        }
        
        return true;
    };
}).call(ccompletePlugin.prototype);
