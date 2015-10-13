#!/bin/bash

dir=`mktemp --directory`

hb_shape=$1
shift
fontfile=$1
shift
hb_shape="$hb_shape $@"
unicodes=`./hb-unicode-decode`
text=`./hb-unicode-encode "$unicodes"`
glyphs=`echo "$text" | $hb_shape "$fontfile"`

cp "$fontfile" "$dir/font.ttf"
pyftsubset \
	--glyph-names \
	"$dir/font.ttf" \
	--text="$text"
if ! test -s "$dir/font.ttf.subset"; then
	echo "Subsetter didn't produce nonempty subset font in $dir/font.ttf.subset" >&2
	exit 2
fi

# Verify that subset font produces same glyphs!
glyphs_subset=`echo "$text" | $hb_shape "$dir/font.ttf.subset"`

if ! test "x$glyphs" = "x$glyphs_subset"; then
	echo "Subset font produced different glyphs!" >&2
	echo "Perhaps font doesn't have glyph names; checking visually..." >&2
	hb_view=${hb_shape/shape/view}
	echo "$text" | $hb_view "$dir/font.ttf" --output-format=png --output-file="$dir/orig.png"
	echo "$text" | $hb_view "$dir/font.ttf.subset" --output-format=png --output-file="$dir/subset.png"
	if ! cmp "$dir/orig.png" "$dir/subset.png"; then
		echo "Images differ.  Please inspect $dir/*.png." >&2
		echo "$glyphs"
		echo "$glyphs_subset"
		exit 2
	fi
	echo "Yep; all good." >&2
	rm -f "$dir/orig.png"
	rm -f "$dir/subset.png"
	glyphs=$glyphs_subset
fi

sha1sum=`sha1sum "$dir/font.ttf.subset" | cut -d' ' -f1`
subset="fonts/sha1sum/$sha1sum.ttf"
mv "$dir/font.ttf.subset" "$subset"

echo "$subset:$unicodes:$glyphs"

rm -f "$dir/font.ttf"
rmdir "$dir"
