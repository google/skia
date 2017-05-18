var circle = {
    "center":{ "x":200, "y":200 },
    "radius":100
}

var gradients = {
    "grad1": { "cx":200, "cy":200, "r":300,
        "stops": [
            { "offset":0, "color": argb(76,0,0,255) },
            { "offset":1, "color": argb( 0,0,0,255) }
        ]
    },
    "grad2": { "cx":200, "cy":200, "r":300,
        "stops": [
            { "offset":0, "color": argb(76,0,255,0) },
            { "offset":1, "color": argb( 0,0,255,0) }
        ]
    },
    "grad3": { "cx":200, "cy":200, "r":300,
        "stops": [
            { "offset":0, "color": argb(76,255,0,0) },
            { "offset":1, "color": argb( 0,255,0,0) }
        ]
    },
    "grad4": { "cx":200, "cy":200, "r":300,
        "stops": [
            { "offset":0, "color": argb(76,192,63,192) },
            { "offset":1, "color": argb( 0,192,63,192) }
        ]
    },
    "grad5": { "cx":200, "cy":200, "r":300,
        "stops": [
            { "offset":0, "color": argb(76,127,127,0) },
            { "offset":1, "color": argb( 0,127,127,0) }
        ]
    },
    "grad6": { "cx":200, "cy":200, "r":300,
        "stops": [
            { "offset":0, "color": argb(76,127,0,127) },
            { "offset":1, "color": argb( 0,127,0,127) }
        ]
    },
    "grad7": { "cx":200, "cy":200, "r":300,
        "stops": [
            { "offset":0, "color": argb(76,0,127,127) },
            { "offset":1, "color": argb( 0,0,127,127) }
        ]
    },
    "grad8": { "cx":200, "cy":200, "r":300,
        "stops": [
            { "offset":0, "color": argb(76,63,192,63) },
            { "offset":1, "color": argb( 0,63,192,63) }
        ]
    }
};

var paths = {
    "cubicSegment1": [
        { "cubic": [ 200,200, 200,200, 200,200, 200,200 ] }
    ],
    "cubicSegment2": [
        { "cubic": [ 200,200, 250,200, 300,200, 300,100 ] }
    ],
    "curveSegment1": [
        { "cubic": [ 200,200, 250,200, 300,150, 300,100 ] }
    ],
    "curveSegment2": [
        { "cubic": [ 200,200, 250,200, 300,150, 200,100 ] }
    ],
    "curveSegment3": [
        { "cubic": [ 200,200, 350,200, 250,-150, 170,300 ] }
    ],
    "diagSegment": [
        { "line":  [ 200,200, 100,100 ] }
    ],
    "horzSegment": [
        { "line":  [ 200,200, 341.4,200 ] }
    ],
    "lineSegment": [
        { "line":  [ 200,200, 200 + circle.radius * Math.cos(-22.5 * Math.PI / 180),
                              200 + circle.radius * Math.sin(-22.5 * Math.PI / 180) ] }
    ],
    "span1": [
        { "quad":  [ 200,200, 300,300, 200,300 ] }
    ],
    "span2": [
        { "cubic": [ 200,200, 100,300, 100,400, 200,300 ] }
    ],
    "span3": [
        { "cubic": [ 200,200, 300,100, 100,400, 300,200 ] }
    ],
    "span4": [
        { "quad":  [ 200,200, 300,300, 400,300 ] }
    ],
    "span5": [
        { "quad":  [ 200,200, 280,320, 200,400 ] }
    ],
    "span6": [
        { "quad":  [ 200,200, 60,340, 100,400 ] }
    ],
    "vertSegment": [
        { "line":  [ 200,200, 200,341.4 ] }
    ],
    "wedge1": [
        { "line":  [ 200,200, 500,500 ] },
        { "arcTo": [ 375.74,624.36, 200,624.26, 424.26 ] },
        { "close": null }
    ],
    "wedge2": [
        { "line":  [ 200,200, 200,624.26 ] },
        { "arcTo": [ 24.265,624.26, -100,500, 424.26 ] },
        { "close": null }
    ],
    "wedge3": [
        { "line":  [ 200,200, 500,-100 ] },
        { "arcTo": [ 1138.22,537.70, 240,622.5, 424.26 ] },
        { "close": null }
    ],
    "wedge4": [
        { "line":  [ 200,200, 500,500 ] },
        { "arcTo": [ 530.79,438.42, 579.47,389.74, 424.26 ] },
        { "close": null }
    ],
    "wedge5": [
        { "line":  [ 200,200, 389.74,579.47 ] },
        { "arcTo": [ 284.94,563.441, 200,500, 424.26 ] },
        { "close": null }
    ],
    "wedge6": [
        { "line":  [ 200,200, 10.26,579.47 ] },
        { "arcTo": [ -51.318,548.68, -100,500, 424.26 ] },
        { "close": null }
    ],
    "wedgeXY1": [
        { "line":  [ 200,200, 500,-100 ] },
        { "arcTo": [ 624.26,24.265, 624.26,200, 424.26 ] },
        { "close": null }
    ],
    "wedgeXY2": [
        { "line":  [ 200,200, 200,-175.74 ] },
        { "arcTo": [ 364.83,-196.61, 500,-100, 424.26 ] },
        { "close": null }
    ],
    "wedgeXY3": [
        { "line":  [ 200,200, -100,-100 ] },
        { "arcTo": [ 35.170,-196.61, 200,-175.74, 424.26 ] },
        { "close": null }
    ],
    "wedgeXY4": [
        { "line":  [ 200,200, -175.74,200 ] },
        { "arcTo": [ -196.61,35.170, -100,-100, 424.26 ] },
        { "close": null }
    ],
    "wedgeXY5": [
        { "line":  [ 200,200, -100,500 ] },
        { "arcTo": [ -196.61,364.83, -175.74,200, 424.26 ] },
        { "close": null }
    ],
    "wedgeXY6": [
        { "line":  [ 200,200, -100,500 ] },
        { "arcTo": [ 75.735,500, 200,624.26, 424.26 ] },
        { "close": null }
    ],
    "wedgeXY7": [
        { "line":  [ 200,200, 200,624.26 ] },
        { "arcTo": [ 324.26,500, 500,500, 424.26 ] },
        { "close": null }
    ],
    "wedgeXY8": [
        { "line":  [ 200,200, 500,500 ] },
        { "arcTo": [ 500,324.26, 624.26,200, 424.26 ] },
        { "close": null }
    ],
    "xaxis": [
        { "line":  [ 100,200, 300,200 ] }
    ],
    "yaxis": [
        { "line":  [ 200,100, 200,300 ] }
    ]
};

var text = {
    "curve1d1": {
        "string":"Some curves initially occupy", "x":400, "y":200
    },
    "curve1d2": {
        "string":"one-dimensional sectors, then diverge.", "x":400, "y":240
    },
    "curveMultiple1": {
        "string":"A curve span may cover more", "x":400, "y":200
    },
    "curveMultiple2": {
        "string":"than one sector.", "x":400, "y":240
    },
    "line1DDest1": {
        "string":"Some lines occupy one-dimensional", "x":400, "y":200
    },
    "line1DDest2": {
        "string":"sectors.", "x":400, "y":240
    },
    "lineSingle": {
        "string":"Line spans are contained by a single sector.", "x":400, "y":200
    },
    "sector1": {
        "string":"A sector is a wedge of a circle", "x":400, "y":200
    },
    "sector2": {
        "string":"containing a range of points.", "x":400, "y":240
    },
    "sectorXY1": {
        "string":"X > 0   Y < 0   -Y < X", "x":500, "y":460
    },
    "sectorXY2": {
        "string":"X > 0   Y < 0   -Y > X", "x":500, "y":460
    },
    "sectorXY3": {
        "string":"X < 0   Y < 0    Y < X", "x":500, "y":460
    },
    "sectorXY4": {
        "string":"X < 0   Y < 0    Y > X", "x":500, "y":460
    },
    "sectorXY5": {
        "string":"X < 0   Y > 0   -Y > X", "x":500, "y":460
    },
    "sectorXY6": {
        "string":"X < 0   Y > 0   -Y < X", "x":500, "y":460
    },
    "sectorXY7": {
        "string":"X > 0   Y > 0    Y > X", "x":500, "y":460
    },
    "sectorXY8": {
        "string":"X > 0   Y > 0    Y < X", "x":500, "y":460
    },
    "sectorXY9": {
        "string":"X > 0   Y == 0", "x":500, "y":460
    },
    "sectorXY10": {
        "string":"Y > 0   0 == X", "x":500, "y":460
    },
    "sectorXY11": {
        "string":"X < 0   Y == X", "x":500, "y":460
    },
    "sectorXYA": {
        "string":"X > 0   Y > 0    Y < X", "x":500, "y":310
    },
    "sectorXYB": {
        "string":"X < 0   Y > 0   -Y < X", "x":500, "y":360
    },
    "sectorXYC": {
        "string":"X < 0   Y < 0    Y < X", "x":500, "y":410
    },
    "spanWedge": {
        "string":"All spans are contained by a wedge", "x":400, "y":200
    },
    "trivialWedge1": {
        "string":"Wedges that don't overlap can be", "x":400, "y":200
    },
    "trivialWedge2": {
        "string":"easily sorted.", "x":400, "y":240
    },
    "xaxis1": {
        "string":"-X", "x":100, "y":220
    },
    "xaxis2": {
        "string":"+X", "x":300, "y":220
    },
    "yaxis1": {
        "string":"-Y", "x":205, "y":100
    },
    "yaxis2": {
        "string":"+Y", "x":205, "y":300
    }
};

var typefaces = {
    "description": { "style":"normal", "family":"Helvetica,Arial" }
};

var paints = {
    "axisStroke":    { "style":"stroke",   "color":rgb(191,191,191) },
    "axisTextDesc":  { "paint":"textBase", "color":rgb(191,191,191) },
    "axisTextRight": { "paint":"axisTextDesc", "textAlign":"right" },
    "axisTextTop":   { "paint":"axisTextDesc", "textBaseline":"hanging" },
    "diagSegment":   { "style":"stroke",    "color":rgb(127,63,127), "strokeWidth":2 },
    "gradient1":     { "style":"fill",      "gradient":"gradients.grad1", "color":alpha(255) },
    "gradient2":     { "paint":"gradient1", "gradient":"gradients.grad2" },
    "gradient3":     { "paint":"gradient1", "gradient":"gradients.grad3" },
    "gradient4":     { "paint":"gradient1", "gradient":"gradients.grad4" },
    "gradient5":     { "paint":"gradient1", "gradient":"gradients.grad5" },
    "gradient6":     { "paint":"gradient1", "gradient":"gradients.grad6" },
    "gradient7":     { "paint":"gradient1", "gradient":"gradients.grad7" },
    "gradient8":     { "paint":"gradient1", "gradient":"gradients.grad8" },
    "horzSegment":   { "paint":"diagSegment", "color":rgb(192,92,31) },
    "picture":       { "color":alpha(255) },
    "sectorADesc":   { "paint":"textBase", "color":rgb(0,0,255) },
    "sectorBDesc":   { "paint":"textBase", "color":rgb(0,127,0) },
    "sectorCDesc":   { "paint":"textBase", "color":rgb(255,0,0) },
    "sectorXY1":     { "paint":"textBase", "color":rgb(192,63,192) },
    "sectorXY2":     { "paint":"textBase", "color":rgb(127,127,0) },
    "sectorXY3":     { "paint":"textBase", "color":rgb(255,0,0) },
    "sectorXY4":     { "paint":"textBase", "color":rgb(127,0,127) },
    "sectorXY5":     { "paint":"textBase", "color":rgb(0,127,127) },
    "sectorXY6":     { "paint":"textBase", "color":rgb(0,127,0) },
    "sectorXY7":     { "paint":"textBase", "color":rgb(63,192,63) },
    "sectorXY8":     { "paint":"textBase", "color":rgb(0,0,255) },
    "sectorXY9":     { "paint":"textBase", "color":rgb(192,92,31) },
    "sectorXY10":    { "paint":"textBase", "color":rgb(31,92,192) },
    "sectorXY11":    { "paint":"textBase", "color":rgb(127,63,127) },

    "stroke":        { "style":"stroke",   "color":rgb(0,0,0) },
    "textBase":      { "style":"fill",     "color":rgb(0,0,0), "typeface":"description",
            "textSize":"1.3rem" },
    "vertSegment":   { "paint":"diagSegment", "color":rgb(31,92,192) },
};

var pictures = {
     "curve1DDestText": [
        { "draw":"text.curve1d1", "paint":"paints.textBase" },
        { "draw":"text.curve1d2", "paint":"paints.textBase" }
    ],
     "curveMultipleText": [
        { "draw":"text.curveMultiple1", "paint":"paints.textBase" },
        { "draw":"text.curveMultiple2", "paint":"paints.textBase" }
    ],
    "line1DDestText": [
        { "draw":"text.line1DDest1", "paint":"paints.textBase" },
        { "draw":"text.line1DDest2", "paint":"paints.textBase" }
    ],
    "sectorXYA": [
        { "draw":"text.sectorXYA", "paint":"paints.sectorADesc" },
        { "draw":"paths.wedgeXY8", "paint":"paints.gradient1" }
    ],
    "sectorXYB": [
        { "draw":"text.sectorXYB", "paint":"paints.sectorBDesc" },
        { "draw":"paths.wedgeXY6", "paint":"paints.gradient2" }
    ],
    "sectorXYC": [
        { "draw":"text.sectorXYC", "paint":"paints.sectorCDesc" },
        { "draw":"paths.wedgeXY3", "paint":"paints.gradient3" }
    ],
    "sectorText": [
        { "draw":"text.sector1", "paint":"paints.textBase" },
        { "draw":"text.sector2", "paint":"paints.textBase" }
    ],
    "trivialWedgeSpans": [
        { "draw":"paths.span4", "paint":"paints.stroke" },
        { "draw":"paths.wedge4", "paint":"paints.gradient4" },
        { "draw":"paths.span5", "paint":"paints.stroke" },
        { "draw":"paths.wedge5", "paint":"paints.gradient5" },
        { "draw":"paths.span6", "paint":"paints.stroke" },
        { "draw":"paths.wedge6", "paint":"paints.gradient6" }
    ],
    "trivialWedgeText": [
        { "draw":"text.trivialWedge1", "paint":"paints.textBase" },
        { "draw":"text.trivialWedge2", "paint":"paints.textBase" }
    ],
    "xaxis": [
        { "draw":"paths.xaxis", "paint":"paints.axisStroke" },
        { "draw":"text.xaxis1", "paint":"paints.axisTextDesc" },
        { "draw":"text.xaxis2", "paint":"paints.axisTextRight" }
    ],
    "yaxis": [
        { "draw":"paths.yaxis", "paint":"paints.axisStroke" },
        { "draw":"text.yaxis1", "paint":"paints.axisTextTop" },
        { "draw":"text.yaxis2", "paint":"paints.axisTextDesc" }
    ],
    "axes": [
        { "draw":"pictures.xaxis", "paint":"paints.picture" },
        { "draw":"pictures.yaxis", "paint":"paints.picture" }
    ]
};

var gradientLookup = [
    0, 4, 5, 3, 6, 7, 2, 8, 1
];

var keyframes = {
    "_default": [
        { "actions": [
            { "range":[0,255], "paint":"paints.picture", "target":"paint.color",
                    "params":"target", "formula":"alpha(value, params)" }
        ]}
    ],
    "keyframe1": [
        { "time":   0, "duration":1000, "canvas":"clear", "actions": [
            { "draw":"text.spanWedge", "paint":"paints.textBase" }
        ]},
        { "time":1000, "duration":1000, "actions": [
            { "ref":"span1", "draw":"paths.span1", "paint":"paints.stroke" }
        ]},
        { "time":1500, "duration":1500, "actions": [
            { "ref":"wedge1", "draw":"paths.wedge1", "paint":"paints.gradient1" }
        ]},
        { "time":3500, "duration": 500, "actions": [
            { "ref":"span1", "range":[255,0] },
            { "ref":"wedge1", "range":[255,0] }
        ]},
        { "time":4000, "duration":1000, "actions": [
            { "ref":"span2", "draw":"paths.span2", "paint":"paints.stroke" }
        ]},
        { "time":4500, "duration":1500, "actions": [
            { "ref":"wedge2", "draw":"paths.wedge2", "paint":"paints.gradient2" }
        ]},
        { "time":6500, "duration": 500, "actions": [
            { "ref":"span2", "range":[255,0] },
            { "ref":"wedge2", "range":[255,0] }
        ]},
        { "time":7000, "duration":1000, "actions": [
            { "draw":"paths.span3", "paint":"paints.stroke" }
        ]},
        { "time":7500, "duration":1500, "actions": [
            { "draw":"paths.wedge3", "paint":"paints.gradient3" }
        ]}
    ],
    "keyframe2": [
        { "time":   0, "duration":1000, "canvas":"clear", "actions": [
            { "draw":"pictures.trivialWedgeText", "paint":"paints.picture" }
        ]},
        { "time":2000, "duration":1500, "actions": [
            { "draw":"pictures.trivialWedgeSpans", "paint":"paints.picture" }
        ]}
    ],
    "keyframe3": [
        { "time":   0, "duration":1000, "canvas":"clear", "actions": [
            { "draw":"pictures.sectorText" },
            { "draw":"pictures.xaxis" }
        ]},
        { "time": 500, "duration":1000, "actions": [
            { "draw":"pictures.yaxis" }
        ]},
        { "time":2000, "duration":1500, "actions": [
            { "draw":"pictures.sectorXYA" }
        ]},
        { "time":3000, "duration":1500, "actions": [
            { "draw":"pictures.sectorXYB" }
        ]},
        { "time":4000, "duration":1500, "actions": [
            { "draw":"pictures.sectorXYC" }
        ]}
    ],
    "keyframe4": [
        { "time":   0, "duration":1000, "canvas":"clear", "actions": [
            { "draw":"text.lineSingle", "paint":"paints.textBase" },
            { "draw":"pictures.axes" }
        ]},
        { "time":1000, "duration":1000, "actions": [
            { "ref":"line", "draw":"paths.lineSegment", "paint":"paints.stroke" }
        ]},
        { "time":1850, "duration":1000, "actions": [
            { "ref":"sectorXY1", "draw":"text.sectorXY1", "paint":"paints.sectorXY1" },
            { "ref":"sectorXY1", "target":"draw.y", "formula":260 },
            { "ref":"wedgeXY1", "draw":"paths.wedgeXY1", "paint":"paints.gradient4" }
        ]},
        { "time":3000, "duration":4000, "actions": [
            { "ref":"line", "target":"draw[0].line[2]",
                "range":[-22.5 * Math.PI / 180, (-22.5 - 360) * Math.PI / 180], "params":"circle",
                "formula":"params.center.x + params.radius * Math.cos(value)"
            },
            { "ref":"line", "target":"draw[0].line[3]",
                "range":[-22.5 * Math.PI / 180, (-22.5 - 360) * Math.PI / 180], "params":"circle",
                "formula":"params.center.y + params.radius * Math.sin(value)"
            }
        ]},
        { "for":["i=2", "i<=8", "++i"], "time":"2250 + 500 * i", "duration":100, "actions": [
            { "ref":"'sectorXY' + i", "draw":"'text.sectorXY' + i",
                    "paint":"'paints.sectorXY' + i" },
            { "ref":"'sectorXY' + i", "target":"draw.y", "formula":260 },
            { "ref":"'wedgeXY' + i", "draw":"'paths.wedgeXY' + i",
                    "paint":"'paints.gradient' + gradientLookup[i]" },
            { "ref":"'sectorXY' + (i - 1)", "range":[255,0] },
            { "ref":"'wedgeXY' + (i - 1)", "range":[255,0] }
        ]},
        { "time":2250 + 500 * 9, "duration":100, "actions": [
            { "ref":"sectorXY1" },
            { "ref":"wedgeXY1" },
            { "ref":"sectorXY8", "range":[255,0] },
            { "ref":"wedgeXY8", "range":[255,0] }
        ]}
    ],
    "keyframe5": [
        { "time":   0, "duration":1000, "canvas":"clear", "actions": [
            { "draw":"pictures.curveMultipleText" },
            { "draw":"pictures.axes" }
        ]},
        { "time":1000, "duration":1000, "actions": [
            { "ref":"curve", "draw":"paths.curveSegment1", "paint":"paints.stroke" }
        ]},
        { "time":2000, "duration":1000, "actions": [
            { "draw":"text.sectorXY1", "paint":"paints.sectorXY1",
                    "target":"draw.y", "formula":260 + 1 * 25},
            { "draw":"paths.wedgeXY1", "paint":"paints.gradient4" }
        ]},
        { "time":3000, "duration":1000, "actions": [
            { "ref":"curve", "range":[0,1], "target":"draw",
                "params":["paths.curveSegment1","paths.curveSegment2"],
                "formula":"interp_paths(value, params)"
            }
        ]},
        { "time":4000, "duration":1000, "actions": [
            { "draw":"text.sectorXY2", "paint":"paints.sectorXY2",
                    "target":"draw.y", "formula":260 + 2 * 25},
            { "draw":"paths.wedgeXY2", "paint":"paints.gradient5" }
        ]},
        { "time":5000, "duration":1000, "actions": [
            { "ref":"curve", "range":[0,1], "target":"draw",
                "params":["paths.curveSegment2","paths.curveSegment3"],
                "formula":"interp_paths(value, params)"
            }
        ]},
        { "for":["i=3", "i<=6", "++i"], "time":"6000", "actions": [
            { "ref":"'text' + i", "draw":"'text.sectorXY' + i", "paint":"'paints.sectorXY' + i",
                    "target":"draw.y", "formula":"260 + i * 25" },
        ]},
        { "for":["i=3", "i<=6", "++i"], "time":"6000", "duration":1000, "actions": [
            { "ref":"'text' + i" },
        ]},
        { "time":6000, "duration":1000, "actions": [
            { "draw":"paths.wedgeXY3", "paint":"paints.gradient3" },
            { "draw":"paths.wedgeXY4", "paint":"paints.gradient6" },
            { "draw":"paths.wedgeXY5", "paint":"paints.gradient7" },
            { "draw":"paths.wedgeXY6", "paint":"paints.gradient2" },
        ]}
    ],
    "keyframe6": [
        { "time":   0, "duration":1000, "canvas":"clear", "actions": [
            { "draw":"pictures.line1DDestText" },
            { "draw":"pictures.axes" }
        ]},
        { "time":2000, "duration":1000, "actions": [
            { "ref":"xy9", "draw":"text.sectorXY9", "paint":"paints.sectorXY9" },
            { "ref":"xy9", "target":"draw.y", "formula":260 + 25},
            { "draw":"paths.horzSegment", "paint":"paints.horzSegment" }
        ]},
        { "time":3000, "duration":1000, "actions": [
            { "ref":"xy10", "draw":"text.sectorXY10", "paint":"paints.sectorXY10" },
            { "ref":"xy10", "target":"draw.y", "formula":260 + 50 },
            { "draw":"paths.vertSegment", "paint":"paints.vertSegment" }
        ]},
        { "time":4000, "duration":1000, "actions": [
            { "ref":"xy11", "draw":"text.sectorXY11", "paint":"paints.sectorXY11" },
            { "ref":"xy11", "target":"draw.y", "formula":260 + 75 },
            { "draw":"paths.diagSegment", "paint":"paints.diagSegment" }
        ]}
    ],
    "keyframe7": [
        { "time":   0, "duration":1000, "canvas":"clear", "actions": [
            { "draw":"pictures.curve1DDestText" },
            { "draw":"pictures.axes" }
        ]},
        { "time":2000, "duration":1000, "actions": [
            { "ref":"cubic", "draw":"paths.cubicSegment1", "paint":"paints.stroke" },
            { "ref":"cubic", "range":[0,1], "target":"draw",
                "params":"paths.cubicSegment2", "formula":"path_partial(value, params)" },
            { "ref":"xy9", "draw":"text.sectorXY9", "paint":"paints.sectorXY9" },
            { "ref":"xy9", "target":"draw.y", "formula":260 + 25},
            { "draw":"paths.horzSegment", "paint":"paints.horzSegment" }
        ]},
        { "time":3000, "duration":1000, "actions": [
            { "ref":"xy1", "draw":"text.sectorXY1", "paint":"paints.sectorXY1" },
            { "ref":"xy1", "target":"draw.y", "formula":260 + 60},
            { "draw":"paths.wedgeXY1", "paint":"paints.gradient4" }
        ]},
    ]
};
