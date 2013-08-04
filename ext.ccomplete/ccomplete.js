/**
 * This file is part of cComplete (https://bitbucket.org/blue-dev/ccomplete).
 *
 * @copyright 2013, Robin Dietrich
 * @license Apache 2.0 <http://www.apache.org/licenses/LICENSE-2.0>
 * @license Public Domain <>
 */

define(function(require, exports, module) {
    var ide = require("core/ide");
    var ext = require("core/ext");
    var util = require("core/util");
    var commands = require("ext/commands/commands");
    var menus = require("ext/menus/menus");
    var editors = require("ext/editors/editors");
    var logger = require("ext/console/logger");             
    
    var settings = require("ext/settings/settings");
    var cSettings = require("text!ext/ccomplete/ccomplete.xml");
    
    module.exports = ext.register("ext/ccomplete/ccomplete", {
        name       : "cComplete",
        dev        : "Robin Dietrich",
        alone      : true,
        type       : ext.GENERAL,
        offline    : false,
        deps       : [editors],
        nodes      : [ ],        
        extensions : [ ],
                
        getFullPath : function() {
            var node = ide.getActivePage().$doc.getNode();
            var path = node.getAttribute("path");
            if (path.indexOf("/workspace/" >= 0))
                path = path.substr(11);
        
            path = ide.workspaceDir + "/" + path;
            
            return path;
        },
        
        getPos : function() {
             var editor = editors.currentEditor.amlEditor.$editor;
             return editor.getCursorPosition();
        },
        
        getParsedResult : function(matches) {
            matches = matches.map(function(m) {
                var chunks = m.split("|||");
                
                if (chunks.length > 1) {
                    var ret = chunks[0];
                    var func = chunks[1];
                    var dec = "";
                    
                    for (var i = 2; i < chunks.length; ++i) {                    
                        if (chunks[i] != ')') {
                            dec = dec+chunks[i];
                        } else {
                            dec = dec+")";
                            break;
                        }
                    }
                    
                    var doc = "";
                    var icon = "";
                    var replace = "";
                    var priority = 0;
                    
                    if (dec !== "") {
                        doc = "Name: "+func+"<br />Parameters:"+dec+"<br />Returns: "+ret;
                        icon = "method";
                        replace = func+"(^^)";
                        priority = 1;                        
                    } else {
                        doc = "Name: "+func+"<br />Returns: "+ret;
                        icon = "property";
                        replace = func+"^^";
                        priority = 2;
                    }                        
                    
                    return {
                        name:func, 
                        replaceText:replace, 
                        doc:doc, 
                        icon:icon, 
                        meta:ret, 
                        priority:priority
                    };
                }
            });
        
            matches.sort(function (a, b) {
                if (a.priority == b.priority)
                    return a.name > b.name;
                else 
                    return a.priority < b.priority;
            });
            
            return matches;
        },
        
        isCpp : function() {
            var parts = this.getFullPath().split(".");
            var fext = parts[parts.length-1];            
            var is = (this.extensions.indexOf(fext) != -1);
            
            return is;
        },
            
        hook : function() {
            var _self = this;
            
            settings.addSettings("C / C++", cSettings);
            
            ide.addEventListener("settings.load", function(e){
                settings.setDefaults("ext/ccomplete", [
                    ["ccomplete", "false"],
                    ["cachesize", 10],
                    ["args", "-I/usr/include,-I/usr/local/include"],
                    ["extensions", "c,h,cpp,hpp,cc,hh,cxx"]
                ]);
                
                _self.updateSettings();
            });
            
            _self.nodes.push(
                menus.addItemByPath("Tools/~", new apf.divider(), 90000)  
            );
            
            _self.nodes.push(
                menus.addItemByPath("Tools/C++", null, 90001)
            );
            
            _self.nodes.push(
                menus.addItemByPath("Tools/C++/Diagnose", new apf.item({
                    caption : "Diagnose",
                    hint: "analyzes current file using clang",
                    command : "diagnose"
                }), 200)
            );
            
            commands.addCommand({
                name: "diagnose",
                bindKey: null,
                isAvailable : function(editor){
                    return editor && editor.amlEditor;
                },
                exec: function() {
                    _self.diagnose();
                }
            });
            
            ide.addEventListener("socketMessage", this.onMessage.bind(this));
        },
        
        complete : function() {
            // send to ccomplete
            var pos  = this.getPos();
            var data = {
                command : "ccomplete",
                subcommand : "complete",
                file : this.getFullPath(),
                line: (pos.row+1),
                col: (pos.column+1)
            };
    
            ide.dispatchEvent("track_action", {type: "ccomplete", cmd: "ccomplete", subcommand: data.subcommand});
            if (!ide.onLine) {
                util.alert(
                    "Currently Offline",
                    "Currently Offline",
                    "This operation could not be completed because you are offline."
                );
            } else {
                ide.send(data);
            }
        },
        
        diagnose : function(file) {
            var _self = this;
            
            if (!file)
                file = _self.getFullPath();
            
            var data = {
                command : "ccomplete",
                subcommand : "diagnose",
                file: file,
                hash : null
            };
            
            ide.dispatchEvent("track_action", {type: "ccomplete", cmd: "ccomplete", subcommand: data.subcommand});
            if (!ide.onLine) {
                util.alert(
                    "Currently Offline",
                    "Currently Offline",
                    "This operation could not be completed because you are offline."
                );
            } else {
                ide.send(data);
            }
        },
        
        updateSettings : function(x) {
            this.enabled = apf.isTrue(settings.model.queryValue("ext/ccomplete/@ccomplete"));
            this.extensions = settings.model.queryValue("ext/ccomplete/@extensions").split(",");
            this.updateArgs(settings.model.queryValue("ext/ccomplete/@args"));
            this.updateCachesize(parseInt(settings.model.queryValue("ext/ccomplete/@cachesize")));
        },
        
        updateArgs : function(args) {
            var data = {
                command : "ccomplete",
                subcommand : "args",
                args : args,
                hash : null
            };
            
            ide.dispatchEvent("track_action", {type: "ccomplete", cmd: "ccomplete", subcommand: data.subcommand});
            if (!ide.onLine) {
                util.alert(
                    "Currently Offline",
                    "Currently Offline",
                    "This operation could not be completed because you are offline."
                );
            } else {
                ide.send(data);
            }
        },
        
        updateCachesize : function(cachesize) {
            var data = {
                command : "ccomplete",
                subcommand : "cachesize",
                cachesize : cachesize,
                hash : null
            };
            
            ide.dispatchEvent("track_action", {type: "ccomplete", cmd: "ccomplete", subcommand: data.subcommand});
            if (!ide.onLine) {
                util.alert(
                    "Currently Offline",
                    "Currently Offline",
                    "This operation could not be completed because you are offline."
                );
            } else {
                ide.send(data);
            }
        },
        
        onMessage : function(e) {
            var msg = e.message;
            
            if (msg.type != "result" && msg.subtype != "ccomplete")
                return;
    
            if (msg.body.err) {
                util.alert(
                    "Error",
                    "There was an error returned from the server:",
                    msg.body.err
                );    
                return;
            }
            
            switch(msg.body.type) {
                case "args":
                    settings.model.setQueryValue("ext/ccomplete/@args", msg.body.args);
                    break;
                case "cachesize":
                    settings.model.setQueryValue("ext/ccomplete/@cachesize", msg.body.cachesize);
                    break;
                case "diagnose":
                    for (var i = 0; i < msg.body.matches.length; ++i) {
                        logger.log(msg.body.matches[i]);
                    }
                    break;
                default:
                    break;
            }
        }
    })
});
