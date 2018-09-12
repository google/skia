/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

var fs = require('fs');

function arrays_equal(a, b) {
  if (a === b) return true;
  if (a == null || b == null) return false;
  if (a.length != b.length) return false;

  for (var i = 0; i < a.length; ++i) {
    if (a[i] !== b[i]) return false;
  }
  return true;
}

function is_animated_property(prop) {
    if ('a' in prop) return prop['a'] === 1;

    var k = prop['k'];

    return Array.isArray(k) && k.length > 0 && typeof k[0] === 'object';
}

function strip_property(prop) {
    delete prop['ix'];
    delete prop['hd'];
}

function is_default(val, default_val) {
    if (typeof val !== typeof default_val)
        return false;

    if (val === default_val)
        return true;

    return Array.isArray(default_val) &&
           val.length >= default_val.length &&
           arrays_equal(default_val, val.slice(0, default_val.length));
}

function strip_animatable_defaults(obj, defaults) {
    if (!obj) return;

    for (var prop_name in defaults) {
        if (!prop_name in obj) continue;

        var prop = obj[prop_name];
        if (!prop || !'k' in prop) continue;

        var val = prop['k'];

        if (is_animated_property(prop)) {
            strip_keyframes(val);
            continue;
        }

        if (is_default(val, defaults[prop_name])) {
            delete obj[prop_name];
            continue;
        }

        strip_property(prop);

        delete prop["a"];
    }
}

function strip_static_defaults(obj, defaults) {
    if (!obj) return;

    for (var prop_name in defaults) {
        if (!prop_name in obj) continue;

        var prop = obj[prop_name];
        if (!prop) continue;

        if (is_default(prop, defaults[prop_name])) {
            delete obj[prop_name];
            continue;
        }

        strip_property(prop);
    }
}

function strip_keyframes(keyframes) {
    keyframes.forEach(function (kf) {
        delete kf['n'];

        strip_static_defaults(kf,
            {
                'ti': [0, 0, 0],
                'to': [0, 0, 0]
            });
    });
}

function strip_transform(t) {
    strip_animatable_defaults(t,
        {
            'o': 100,
            'r':   0,
            'sk':  0,
            'sa':  0,
            'a': [  0,   0],
            'p': [  0,   0],
            's': [100, 100]
        });
}

function strip_shape(shape) {
    if (!shape) return;

    var group = shape['it'];
    if (Array.isArray(group))
        group.forEach(strip_shape);

    delete shape['cix'];
    delete shape['cl'];
    delete shape['hd'];
    delete shape['ind'];
    delete shape['ix'];
    delete shape['mn'];
    delete shape['nm'];
    delete shape['np'];

    switch (shape['ty']) {
    case 'fl':
        strip_animatable_defaults(shape, { 'o': 100, 'c': [0, 0, 0, 1] });
        break;
    case 'sh':
        strip_static_defaults(shape, { 'c': false });
        strip_static_defaults(shape, { 'closed': false });

        if (ks = shape['ks']) {
            if (is_animated_property(ks))
                strip_keyframes(ks['k']);
            strip_property(ks);
        }

        break;
    case 'st':
        strip_animatable_defaults(shape, { 'o': 100, 'w': 1, 'c': [0, 0, 0, 1] });
        strip_static_defaults(shape, { 'lc': 1, 'lj': 1, 'ml': 4 });
        break;
    case 'tr':
        strip_transform(shape);
        break;
    }
}

function strip_layer(layer) {
    delete layer['cl'];
    delete layer['ddd'];
    delete layer['nm'];

    strip_transform(layer['ks']);

    strip_static_defaults(layer,
        {
            'st': 0,
            'sr': 1
        });

    (layer['shapes'] || []).forEach(strip_shape);
}

function strip_comp(comp) {
    delete comp['ddd'];
    delete comp['nm'];

    (comp['layers'] || []).forEach(strip_layer);
}

function strip_root(root) {
    strip_comp(root);

    (root['assets'] || []).forEach(strip_comp);
}

process.argv.slice(2).forEach(function(file) {
    if (!file.match(/\.*.json$/)) {
        console.log('!!! ' + file + " doesn't look like a Lottie JSON file - skipping.");
        return;
    }

    fs.readFile(file, function(err, data) {
        if (err) throw err;

        var root = JSON.parse(data);
        strip_root(root);

        var stripped_file = file.replace(/.json$/, '_min.json');
        fs.writeFile(stripped_file, JSON.stringify(root), function(err) {
            if (err) throw err;

            var     orig_size = fs.statSync(file).size;
            var stripped_size = fs.statSync(stripped_file).size;

            function pad(v, n) {
                var str = '' + v;
                return str.padStart(n);
            }

            console.log("%s%s -> %s\t(%d%)", file.padEnd(48), pad(orig_size, 8),
                        pad(stripped_size, 8),
                        (stripped_size * 100 / orig_size).toFixed(3));
        });
    });
});
