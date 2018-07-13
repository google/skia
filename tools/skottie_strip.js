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

function strip_keyframes(keyframes) {
    // TODO
}

function strip_property(prop) {
    delete prop['ix'];
}

function strip_default_prop(obj, prop_name, default_val) {
    if (!obj || !prop_name in obj)
        return;

    var prop = obj[prop_name];
    if (!prop || !'k' in prop)
        return;

    var val = prop['k'];

    if (is_animated_property(prop)) {
        strip_keyframes(val);
        return;
    }

    if (typeof val === typeof default_val) {
        if (val === default_val) {
            delete obj[prop_name];
            return;
        }

        if (Array.isArray(default_val) &&
            val.length >= default_val.length &&
            arrays_equal(default_val, val.slice(0, default_val.length))) {
            delete obj[prop_name];
            return;
        }
    }

    strip_property(prop);
}

function strip_transform(t) {
    if (!t)
        return;

    strip_default_prop(t, 'o', 100);
    strip_default_prop(t, 'r', 0);
    strip_default_prop(t, 'a', [0, 0]);
    strip_default_prop(t, 'p', [0, 0]);
    strip_default_prop(t, 's', [100, 100]);
}

function strip_shape(shape) {
    if (!shape) return;

    var group = shape['it'];
    if (Array.isArray(group))
        group.forEach(strip_shape);

    delete shape['ind'];
    delete shape['mn'];
    delete shape['nm'];

    switch (shape['ty']) {
    case 'fl':
        strip_default_prop(shape, 'o', 100);
        break;
    case 'sh':
        strip_default_prop(shape, 'c', false);
        break;
    case 'st':
// TODO:
//        strip_default_prop(shape, 'ml', 4);
//        strip_default_prop(shape, 'lc', 1);
//        strip_default_prop(shape, 'lj', 1);
        strip_default_prop(shape, 'o', 100);
        strip_default_prop(shape, 'w', 1);
        break;
    case 'tr':
        strip_transform(shape);
        break;
    }
}

function strip_layer(layer) {
    delete layer['nm'];
    delete layer['ddd'];

    strip_transform(layer['ks']);

    (layer['shapes'] || []).forEach(strip_shape);
}

function strip_comp(comp) {
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

            console.log("%s:\t\t%d -> %d\t\t(%d%)", file, orig_size, stripped_size, stripped_size * 100 / orig_size);
        });
    });
});
