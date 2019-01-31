/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include <vector>

#include "Benchmark.h"
#include "SkFontTypes.h"
#include "SkMakeUnique.h"
#include "SkTypeface.h"
#include "SkUTF.h"
#include "SkUtils.h"

// From Project Guttenberg. This is UTF-8 text.
static const char* text[] = {
        "Call me Ishmael.  Some years ago--never mind how",
        "long precisely --having little or no money in my purse, and nothing particular",
        "to interest me on shore, I thought I would sail about a little and see the",
        "watery part of the world.  It is a way I have of driving off the spleen, and",
        "regulating the circulation.  Whenever I find myself growing grim about the",
        "mouth; whenever it is a damp, drizzly November in my soul; whenever I find",
        "myself involuntarily pausing before coffin warehouses, and bringing up the",
        "rear of every funeral I meet; and especially whenever my hypos get such an",
        "upper hand of me, that it requires a strong moral principle to prevent me",
        "from deliberately stepping into the street, and methodically knocking",
        "people's hats off--then, I account it high time to get to sea as soon as I can.",
        "",
        "This is my substitute for pistol and ball.  With a philosophical flourish",
        "Cato throws himself upon his sword; I quietly take to the ship.  There is",
        "nothing surprising in this.  If they but knew it, almost all men in their",
        "degree, some time or other, cherish very nearly the same feelings towards the",
        "ocean with me.  There now is your insular city of the Manhattoes, belted round",
        "by wharves as Indian isles by coral reefs--commerce surrounds it with her surf.",
        "",
        "Right and left, the streets take you waterward.  Its extreme down-town is",
        "the battery, where that noble mole is washed by waves, and cooled by",
        "breezes, which a few hours previous were out of sight of land.  Look at the",
        "crowds of water-gazers there.  Circumambulate the city of a dreamy Sabbath",
        "afternoon.  Go from Corlears Hook to Coenties Slip, and from thence, by",
        "Whitehall northward.  What do you see?--Posted like silent sentinels all",
        "around the town, stand thousands upon thousands of mortal men fixed in ocean",
        "reveries.  Some leaning against the spiles; some seated upon the pier-heads;",
        "some looking over the bulwarks glasses!",
        "of ships from China; some high aloft in the rigging, as if striving to get a",
        "still better seaward peep.  But these are all landsmen; of week days pent up",
        "in lath and plaster--tied to counters, nailed to benches, clinched to desks.",
        "How then is this?  Are the green fields gone?  What do they here?  But look!",
        "here come more crowds, pacing straight for the water, and seemingly bound for",
        "a dive.  Strange!  Nothing will content them but the extremest limit of the",
        "land; loitering under the shady lee of yonder warehouses will not suffice.",
        "No.  They must get just as nigh the water as they possibly can without falling",
        "in.  And there they stand--miles of them--leagues.  Inlanders all, they come",
        "from lanes and alleys, streets and avenues, --north, east, south, and west.",
        "Yet here they all unite.  Tell me, does the magnetic virtue of the needles of",
        "the compasses of all those ships attract them thither?  Once more.  Say, you",
        "are in the country; in some high land of lakes.  Take almost any path you",
        "please, and ten to one it carries you down in a dale, and leaves you there",
        "by a pool in the stream.  There is magic in it.  Let the most absent-minded",
        "of men be plunged in his deepest reveries--stand that man on his legs, set his",
        "feet a-going, and he will infallibly lead you to water, if water there be in",
        "all that region.  Should you ever be athirst in the great American desert,",
        "try this experiment, if your caravan happen to be supplied with a metaphysical",
        "professor.  Yes, as every one knows, meditation and water are wedded for ever.",
        "",
        "But here is an artist.  He desires to paint you the dreamiest, shadiest,",
        "quietest, most enchanting bit of romantic landscape in all the valley of the",
        "Saco.  What is the chief element he employs?  There stand his trees, each with",
        "a hollow trunk, as if a hermit and a crucifix were within; and here sleeps",
        "his meadow, and there sleep his cattle; and up from yonder cottage goes a",
        "sleepy smoke.  Deep into distant woodlands winds a mazy way, reaching to",
        "overlapping spurs of mountains bathed in their hill-side blue.  But though",
        "the picture lies thus tranced, and though this pine-tree shakes down its sighs",
        "like leaves upon this shepherd's head, yet all were vain, unless the",
        "shepherd's eye were fixed upon the magic stream before him.  Go visit the",
        "Prairies in June,",
        "",
        "when for scores on scores of miles you wade knee-deep among Tiger-lilies--what",
        "is the one charm wanting? --Water --there is not a drop of water there!  Were",
        "Niagara but a cataract of sand, would you travel your thousand miles to see",
        "it?  Why did the poor poet of Tennessee, upon suddenly receiving two handfuls",
        "of silver, deliberate whether to buy him a coat, which he sadly needed, or",
        "invest his money in a pedestrian trip to Rockaway Beach?  Why is almost every",
        "robust healthy boy with a robust healthy soul in him, at some time or other",
        "crazy to go to sea?  Why upon your first voyage as a passenger, did you",
        "yourself feel such a mystical vibration, when first told that you and your",
        "ship were now out of sight of land?  Why did the old Persians hold the sea",
        "holy?  Why did the Greeks give it a separate deity, and own brother of Jove?",
        "Surely all this is not without meaning.  And still deeper the meaning of that",
        "story of Narcissus, who because he could not grasp the tormenting, mild image",
        "he saw in the fountain, plunged into it and was drowned.  But that same",
        "image, we ourselves see in all rivers and oceans.  It is the image of the",
        "ungraspable phantom of life; and this is the key to it all.  Now, when I say",
        "that I am in the habit of going to sea whenever I begin to grow hazy about the",
        "eyes, and begin to be over conscious of my lungs, I do not mean to have it",
        "inferred that I ever go to sea as a passenger.  For to go as a passenger you",
        "must needs have a purse, and a purse is but a rag unless you have something",
        "in it.  Besides, passengers get sea-sick --grow quarrelsome --don't sleep of",
        "nights --do not enjoy themselves much, as a general thing; --no, I never go as a",
        "passenger; nor, though I am something of a salt, do I ever go to sea as a",
        "Commodore, or a Captain, or a Cook.  I abandon the glory and distinction of",
        "such offices to those who like them.  For my part, I abominate all honorable",
        "respectable toils, trials, and tribulations of every kind whatsoever.  It is",
        "quite as much as I can do to take care of myself, without taking care of",
        "ships, barques, brigs, schooners, and what not.  And as for going as cook, --",
        "though I confess there is considerable glory in that, a cook being a sort of",
        "officer on ship-board --yet, somehow, I never fancied broiling fowls; --though",
        "once broiled, judiciously buttered, and judgmatically salted and peppered,",
        "there is no one who will",
        "",
        "speak more respectfully, not to say reverentially, of a broiled fowl than I",
        "will.  It is out of the idolatrous dotings of the old Egyptians upon broiled",
        "ibis and roasted river horse, that you see the mummies of those creatures in",
        "their huge bake-houses the pyramids.  No, when I go to sea, I go as a simple",
        "sailor, right before the mast, plumb down into the forecastle, aloft there",
        "to the royal mast-head.  True, they rather order me about some, and make me",
        "jump from spar to spar, like a grasshopper in a May meadow.  And at first,",
        "this sort of thing is unpleasant enough.  It touches one's sense of honor,",
        "particularly if you come of an old established family in the land, the van",
        "Rensselaers, or Randolphs, or Hardicanutes.  And more than all, if just",
        "previous to putting your hand into the tar-pot, you have been lording it as a",
        "country schoolmaster, making the tallest boys stand in awe of you.  The",
        "transition is a keen one, I assure you, from the schoolmaster to a sailor,",
        "and requires a strong decoction of Seneca and the Stoics to enable you to grin",
        "and bear it.  But even this wears off in time.  What of it, if some old hunks",
        "of a sea-captain orders me to get a broom and sweep down the decks?  What does",
        "that indignity amount to, weighed, I mean, in the scales of the New",
        "Testament?  Do you think the archangel Gabriel thinks anything the less of me,",
        "because I promptly and respectfully obey that old hunks in that particular",
        "instance?  Who aint a slave?  Tell me that.  Well, then, however the old",
        "sea-captains may order me about--however they may thump and punch me about, I",
        "have the satisfaction of knowing that it is all right; that everybody else",
        "is one way or other served in much the same way -- either in a physical or",
        "metaphysical point of view, that is; and so the universal thump is passed",
        "round, and all hands should rub each other's shoulder-blades, and be",
        "content.  Again, I always go to sea as a sailor, because they make a point of",
        "paying me for my trouble, whereas they never pay passengers a single penny",
        "that I ever heard of.  On the contrary, passengers themselves must pay.  And",
        "there is all the difference in the world between paying and being paid.  The",
        "act of paying is perhaps the most uncomfortable infliction that the two",
        "orchard thieves entailed upon us."
};

class UtfToGlyph : public Benchmark {
public:
    explicit UtfToGlyph(SkTextEncoding encoding) : fEncoding{encoding} { }

protected:
    const char* onGetName() override {
        switch (fEncoding) {
            case SkTextEncoding::kUTF8:
                return "SkTypefaceUtf8ToGlyphID";
            case SkTextEncoding::kUTF16:
                return "SkTypefaceUtf16ToGlyphID";
            case SkTextEncoding::kUTF32:
                return "SkTypefaceUtf32ToGlyphID";
            default:
                return "oh no!!!";
        }
    }

    bool isSuitableFor(Backend backend) override {
        return backend == kNonRendering_Backend;
    }

    void onDelayedSetup() override {
        int lines = SK_ARRAY_COUNT(text);

        int maxGlyphs = 0;
        for (int i = 0; i < lines; i++) {
            fLines.emplace_back(this->convertLine(i));
            maxGlyphs = std::max(maxGlyphs, fLines.back()->glyphCount);
        }
        fGlyphIds.insert(fGlyphIds.begin(), maxGlyphs, 0);
    }

    void onDraw(int loops, SkCanvas* canvas) override {
        auto tf = SkTypeface::MakeFromName("monospace", SkFontStyle());
        // Do more loops to reduce variance.
        for (int i = 0; i < loops * 3; ++i) {
            for (auto& line : fLines) {
                tf->charsToGlyphs(line->utf.data(),
                                  (SkTypeface::Encoding)fEncoding, fGlyphIds.data(),
                                  line->glyphCount);
            }
        }
    }

private:
    struct Line {
        int glyphCount = 0;
        std::vector<char> utf;
    };

    std::unique_ptr<Line> convertLine(int lineIndex) {
        std::unique_ptr<Line> result = skstd::make_unique<Line>();

        const char* cursor = text[lineIndex];
        size_t len = strlen(cursor);
        const char* end = cursor + len;
        result->glyphCount = SkUTF::CountUTF8(cursor, len);
        std::vector<char>& bytes = result->utf;
        while (cursor < end) {
            SkUnichar u = SkUTF::NextUTF8(&cursor, end);

            switch (fEncoding) {
                case SkTextEncoding::kUTF8: {
                    char buffer[SkUTF::kMaxBytesInUTF8Sequence];
                    size_t count = SkUTF::ToUTF8(u, buffer);
                    result->utf.insert(bytes.end(), buffer, buffer + count);
                    break;
                }
                case SkTextEncoding::kUTF16: {
                    uint16_t buffer[2];
                    size_t count = SkUTF::ToUTF16(u, buffer);
                    result->utf.insert(bytes.end(), (char *)buffer, (char *)buffer + count * 2);
                    break;
                }
                case SkTextEncoding::kUTF32: {
                    result->utf.insert(bytes.end(), (char*) &u, (char*) &u + 4);
                    break;
                }
                default:
                    SK_ABORT("Bad encoding");
            }
        }

        return result;
    }

    SkTextEncoding fEncoding;
    std::vector<std::unique_ptr<Line>> fLines;
    std::vector<SkGlyphID> fGlyphIds;
};

DEF_BENCH( return new UtfToGlyph(SkTextEncoding::kUTF8); )
DEF_BENCH( return new UtfToGlyph(SkTextEncoding::kUTF16); )
DEF_BENCH( return new UtfToGlyph(SkTextEncoding::kUTF32); )
