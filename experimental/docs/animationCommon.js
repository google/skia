var animationState = {};
animationState.reset = function (engine) {
    if ('string' === typeof engine) {
        this.defaultEngine = engine;
    }
    this.defaults = {};
    this.displayList = [];
    this.displayDict = {};
    this.start = null;
    this.time = 0;
    this.timeline = [];
    this.timelineIndex = 0;
    this.requestID = null;
    this.paused = false;
    this.displayEngine = 'undefined' === typeof engine ? this.defaultEngine : engine;
}

function addActions(frame, timeline) {
    var keyframe = keyframes[frame];
    var len = keyframe.length;
    for (var i = 0; i < len; ++i) {
        var action = keyframe[i];
        loopOver(action, timeline);
    }
}

function animateList(now) {
    if (animationState.paused) {
        return;
    }
    if (animationState.start == null) {
        animationState.start = now - animationState.time;
    }
    animationState.time = now - animationState.start;
    var stillAnimating = false;
    for (var index = animationState.timelineIndex; index < animationState.timeline.length; ++index) {
        var animation = animationState.timeline[index];
        if (animation.time > animationState.time) {
            stillAnimating = true;
            break;
        }
        if (animation.time + animation.duration < animationState.time) {
            if (animation.finalized) {
                continue;
            }
            animation.finalized = true;
        }
        stillAnimating = true;
        var actions = animation.actions;
        for (var aIndex = 0; aIndex < actions.length; ++aIndex) {
            var action = actions[aIndex];
            var hasDraw = 'draw' in action;
            var hasRef = 'ref' in action;
            var displayIndex;
            if (hasDraw) {
                var ref = hasRef ? action.ref : "anonymous_" + index + "_" + aIndex;
                assert('string' == typeof(ref));
                if (ref in animationState.displayDict) {
                    displayIndex = animationState.displayDict[ref];
                } else {
                    assert('string' == typeof(action.draw));
                    var draw = (new Function("return " + action.draw))();
                    assert('object' == typeof(draw));
                    var paint;
                    if ('paint' in action) {
                        assert('string' == typeof(action.paint));
                        paint = (new Function("return " + action.paint))();
                        assert('object' == typeof(paint) && !isArray(paint));
                    } else {
                        paint = animationState.defaults.paint;
                    }
                    displayIndex = animationState.displayList.length;
                    animationState.displayList.push( { "ref":ref, "draw":draw, "paint":paint,
                        "drawSpec":action.draw, "paintSpec":action.paint,
                        "drawCopied":false, "paintCopied":false,
                        "drawDirty":true, "paintDirty":true, "once":false } );
                    animationState.displayDict[ref] = displayIndex;
                }
            } else if (hasRef) {
                assert('string' == typeof(action.ref));
                displayIndex = animationState.displayDict[action.ref];
            } else {
                assert(actions.length == 1);
                for (var prop in action) {
                    if ('paint' == prop) {
                        assert('string' == typeof(action[prop]));
                        var obj = (new Function("return " + action[prop]))();
                        assert('object' == typeof(obj) && !isArray(obj));
                        animationState.defaults[prop] = obj;
                    } else {
                        animationState.defaults[prop] = action[prop];
                    }
                }
                continue;
            }
            var targetSpec = 'target' in action ? action.target : animationState.defaults.target;
            assert(targetSpec);
            assert('string' == typeof(targetSpec));
            assert(displayIndex < animationState.displayList.length);
            var display = animationState.displayList[displayIndex];
            var modDraw = targetSpec.startsWith('draw');
            assert(modDraw || targetSpec.startsWith('paint'));
            var modType = modDraw ? "draw" : "paint";
            var copied = modDraw ? display.drawCopied : action.paintCopied;
            if (!copied) {
                var copy;
                if (!modDraw || display.drawSpec.startsWith("text")) {
                    copy = {};
                    var original = modDraw ? display.draw : display.paint;
                    for (var p in original) {
                        copy[p] = original[p];
                    }
                } else if (display.drawSpec.startsWith("paths")) {
                    copy = [];
                    for (var i = 0; i < display.draw.length; ++i) {
                        var curves = display.draw[i];
                        var curve = Object.keys(curves)[0];
                        copy[i] = {};
                        copy[i][curve] = curves[curve].slice(0);  // clone the array of curves
                    }
                } else {
                    assert(display.drawSpec.startsWith("pictures"));
                    copy = [];
                    for (var i = 0; i < display.draw.length; ++i) {
                        var entry = display.draw[i];
                        copy[i] = { "draw":entry.draw, "paint":entry.paint };
                    }
                }
                display[modType] = copy;
                display[modType + "Copied"] = true;
            }
            var targetField, targetObject, fieldOffset;
            if (targetSpec.endsWith("]")) {
                fieldOffset = targetSpec.lastIndexOf("[");
                assert(fieldOffset >= 0);
                targetField = targetSpec.substring(fieldOffset + 1, targetSpec.length - 1);
                var arrayIndex = +targetField;
                if (!isNaN(arrayIndex) && targetField.length > 0) {
                    targetField = arrayIndex;
                }

            } else {
                fieldOffset = targetSpec.lastIndexOf(".");
                if (fieldOffset >= 0) {
                    targetField = targetSpec.substring(fieldOffset + 1, targetSpec.length);
                } else {
                    targetObject = display;
                    targetField = targetSpec;
                }
            }
            if (fieldOffset >= 0) {
                var sub = targetSpec.substring(0, fieldOffset);
                targetObject = (new Function('display', "return display." + sub))(display);
            }
            assert(null != targetObject[targetField]);
            if (!('start' in action) || action.start < animation.time) {
                for (var p in animationState.defaults) {
                    if ('draw' == p || 'paint' == p || 'ref' == p) {
                        continue;
                    }
                    assert('range' == p || 'target' == p || 'formula' == p || 'params' == p);
                    if (!(p in action)) {
                        action[p] = animationState.defaults[p];
                    }
                }
                if ('number' == typeof(action.formula)) {
                    targetObject[targetField] = action.formula;
                    action.once = true;
                }
                action.start = animation.time;
            }
            if (action.once) {
                continue;
            }
            var value = Math.min(1, (animationState.time - animation.time) / animation.duration);
            var scaled = action.range[0] + (action.range[1] - action.range[0]) * value;
            if ('params' in action) {
                if (!('func' in action)) {
                    if (isArray(action.params)) {
                        action.funcParams = [];
                        var len = action.params.length;
                        for (var i = 0; i < len; ++i) {
                            action.funcParams[i] = 'target' == action.params[i]
                                ? targetObject[targetField]
                                : (new Function("return " + action.params[i]))();
                        }
                    } else {
                        action.funcParams = 'target' == action.params
                                ? targetObject[targetField]
                                : (new Function("return " + action.params))();
                    }
                    assert('formula' in action && 'string' == typeof(action.formula));
                    // evaluate inline function to get value
                    action.func = new Function('value', 'params', "return " + action.formula);
                }
                scaled = action.func(scaled, action.funcParams);
            }
            if (targetObject[targetField] != scaled) {
                if (modDraw) {
                    display.drawDirty = true;
                } else {
                    display.paintDirty = true;
                }
                targetObject[targetField] = scaled;
            }
        }
    }
    displayBackend(animationState.displayEngine, animationState.displayList);

    if (stillAnimating) {
        animationState.requestID = requestAnimationFrame(animateList);
    }
}

function flattenPaint(paint) {
    if (!paint.paint) {
        return;
    }
    var parent = paints[paint.paint];
    flattenPaint(parent);
    for (var prop in parent) {
        if (!(prop in paint)) {
            paint[prop] = parent[prop];
        }
    }
    paint.paint = null;
}

function init(engine, keyframe) {
    animationState.reset(engine);
    setupPaint();
    setupBackend(animationState.displayEngine);
    keyframeInit(keyframe);
}

function keyframeInit(frame) {
    animationState.reset();
    addActions("_default", animationState.timeline);
    addActions(frame, animationState.timeline);
    for (var index = 0; index < animationState.timeline.length; ++index) {
        animationState.timeline[index].position = index;
    }
    animationState.timeline.sort(function(a, b) {
        if (a.time == b.time) {
            return a.position - b.position;
        }
        return a.time - b.time;
    });
    keyframeBackendInit(animationState.displayEngine, animationState.displayList,
            keyframes[frame][0]);
    animationState.requestID = requestAnimationFrame(animateList);
}

function loopAddProp(action, propName) {
    var funcStr = "";
    var prop = action[propName];
    if ('draw' != propName && isArray(prop)) {
        funcStr += '[';
        for (var index = 0; index < prop.length; ++index) {
            funcStr += loopAddProp(prop, index);
            if (index + 1 < prop.length) {
                funcStr += ", ";
            }
        }
        funcStr += ']';
        return funcStr;
    }
    assert("object" != typeof(prop));
    var useString = "string" == typeof(prop) && isAlpha(prop.charCodeAt(0));
    if (useString) {
        funcStr += "'";
    }
    funcStr += prop;
    if (useString) {
        funcStr += "'";
    }
    return funcStr;
}

function loopOver(rec, timeline) {
    var funcStr = "";
    if (rec.for) {
        funcStr += "for (" + rec.for[0] + "; " + rec.for[1] + "; " + rec.for[2] + ") {\n";
    }
    funcStr += "    var time = " + ('time' in rec ? rec.time : 0) + ";\n";
    funcStr += "    var duration = " + ('duration' in rec ? rec.duration : 0) + ";\n";
    funcStr += "    var actions = [];\n";
    var len = rec.actions.length;
    for (var i = 0; i < len; ++i) {
        funcStr += "    var action" + i + " = {\n";
        var action = rec.actions[i];
        for (var p in action) {
            funcStr += "        '" + p + "':";
            funcStr += loopAddProp(action, p);
            funcStr += ",\n";
        }
        funcStr = funcStr.substring(0, funcStr.length - 2);
        funcStr += "\n    };\n";
        funcStr += "    actions.push(action" + i + ");\n";
    }
    funcStr += "    timeline.push( { 'time':time, 'duration':duration, 'actions':actions,"
            + "'finalized':false } );\n";
    if (rec.for) {
        funcStr += "}\n";
    }
    var func = new Function('rec', 'timeline', funcStr);
    func(rec, timeline);
}

function setupPaint() {
    for (var prop in paints) {
        flattenPaint(paints[prop]);
    }
}
