/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include <vector>

#include "bench/Benchmark.h"
#include "include/core/SkFont.h"
#include "include/core/SkTypeface.h"
#include "src/core/SkUtils.h"
#include "src/utils/SkUTF.h"

// From Project Guttenberg. This is UTF-8 text.
static const char* atext[] = {
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

static const char* ctext[] = {
        "第一回",
        "胡秀才告狀鳴冤　施賢臣得夢訪案",
        "",
        "　　話說江都縣有一秀才，姓胡，名登舉。他的父母為人所殺，頭顱不見。胡登舉合家",
        "嚇得膽裂魂飛，慌忙出門，去稟縣主。",
        "",
        "　　跑到縣衙，正遇升堂，就進去喊冤。走至堂上，打了一躬，手舉呈詞，口稱：「父",
        "師在上，門生禍從天降。叩稟老父師，即賜嚴拿。」說著，將呈詞遞上。書吏接過，鋪",
        "在公案。施公靜心細閱。上寫：具呈生員胡登舉，祖居江都縣。生父曾作翰林，告老家",
        "居，廣行善事，憐恤窮苦，並無苛刻待人之事。不意於某日夜間，生父母閉戶安眠。至",
        "天曉，生往請安，父母俱不言語。生情急，踢開門戶，見父母屍身俱在牀上，兩個人頭",
        "，並沒蹤影。生忝居學校，父母如此死法，何以身列校庠對雙親而無愧乎？為此具呈，",
        "嚎叩老父師大人恩准，速賜拿獲兇手，庶生冤仇得雪。感戴無既。沾仁。上呈。",
        "",
        "　　施公看罷，不由點頭，暗暗吃驚，想道：「夤夜入院，非奸即盜。胡翰林夫婦年老",
        "被殺，而不竊去財物，且將人頭拿去，其中情由，顯係仇謀。此宗無題文章，令人如何",
        "做法？」為難良久，說道：「即委捕廳四老爺，前去驗屍。你只管入殮，自有頭緒結斷",
        "。」胡秀才一聽，只得含淚下堂，出衙回家，伺候驗屍。",
        "",
        "　　且說施公吩咐速去知會四衙，往胡家驗屍呈報，把呈詞收入袖內，吩咐退堂。進內",
        "書房坐下，長隨送茶畢，用過了飯，把呈詞取出，鋪在案上翻閱。低頭細想，此案難結",
        "。欠身伸手，在書架上拿了古書一部，係《拍案稱奇》，放在桌上要看；對證此案，即",
        "日好斷這沒頭之事。將《拍案稱奇》，自頭至尾看完，又取了一部，係海瑞參拿嚴嵩的",
        "故事。不覺困倦，放下書本，伏於書案之上，朦朧打睡。夢中看見外邊牆頭之下，有群",
        "黃雀兒九隻，點頭搖尾，唧哩喳啦，不住亂叫。施公一見，心中甚驚。又聽見地上哼哼",
        "唧唧的豬叫；原來是油光兒的七個小豬兒，望著賢臣亂叫。施公夢中稱奇，方要去細看",
        "，那九隻黃雀兒，一齊飛下牆來，與地下七個小豬兒，點頭亂噪。那七個小豬兒，站起",
        "身來，望黃雀拱抓，口內哼哼亂叫。雀噪豬叫，偶然起了一陣怪風，把豬雀都裹了去了",
        "。施公夢中一聲驚覺，大叫說：「奇怪的事！」施安在旁邊站立，見主人如此驚叫，不",
        "知何故，連忙叫：「老爺醒來！醒來！」施公聽言，抬頭睜眼，沉吟多時。想夢中之事",
        "，說：「奇哉！怪哉！」就問施安這天有多時了。施安答道：「日色西沉了。」施公點",
        "頭，又問：「方才你可見些什麼東西沒有？」施安說：「並沒見什麼東西，倒有一陣風",
        "刮過牆去。」施公聞言，心中細想，這九隻黃雀、七個小豬奇怪，想來內有曲情。將書",
        "擱在架上，前思後想，一夜未睡。直到天明，淨面整衣，吩咐傳梆升堂。坐下，抽籤叫",
        "快頭英公然、張子仁上來。二人走至堂上，跪下叩頭。施公就將昨日夢見九隻黃雀、七",
        "個小豬為題出簽差人，說：「限你二人五日之期，將九黃、七豬拿來，如若遲延，重責",
        "不饒。」將簽遞於二人。二人跪趴半步，口稱：「老爺容稟：小的們請個示來。",
        "",
        "　　這九黃、七豬，是兩個人名，還是兩個物名，現在何處？求老爺吩咐明白，小的們",
        "好去訪拿。」言罷叩頭。施公一聽，說道：「無用奴才，連個九黃、七豬都不知道，還",
        "在本縣應役麼？分明偷閒躲懶，安心抗差玩法。」吩咐：「給我拉下去打！」兩邊發喊",
        "按倒，每人打了十五板。二人跪下叩頭，復又討示，叫聲：「老爺，究竟吩咐明白，待",
        "小的們好去拿人。」施公聞言，心中不由大怒，說：「好大膽的奴才！本縣深知你二人",
        "久慣應役，極會搪塞，如敢再行囉唣，定加重責！」二人聞言，萬分無奈，站起退下去",
        "，訪拿九黃、七豬而去。施公也隨退堂。",
        "",
        "　　施公一連五日，假裝有恙，並未升堂。到了第六日，一早吩咐點鼓升堂，坐下。衙",
        "役人等伺候。只見一人走至公堂案下，手捧呈詞，口稱：「父師，門生胡登舉父母被殺",
        "之冤，求父師明鑒。倘遲久不獲，兇犯走脫難捉。且生員讀書一場，豈不有愧？如門生",
        "另去投呈伸冤，老父台那時休怨！」言罷一躬，將呈遞上。施公帶笑道：「賢契不必急",
        "躁。本縣已經差人明捕暗訪，專拿形跡可疑之人，審得自然替你申冤。」胡登舉無奈，",
        "說道：「父台！速替門生伸冤，感恩不盡！」施公說：「賢契請回，催呈留下。」胡登",
        "舉打躬下堂，出衙回家。且說施公為難多會，方要提胡宅管家的審問，只見公差英公然",
        "、張子仁上堂，跪下回稟：「小的二人，並訪不著九黃、七豬，求老爺寬限。」",
        "",
        "　　施公聞言，激惱成怒，喝叫左右拉下，每人打十五大板。不容分說，只打的哀求不",
        "止，鮮血直流。打完提褲，戰戰兢兢，跪在地下，口尊：「老爺，叩討明示，以便好去",
        "捉人。」施公聞言無奈，硬著心腸說道：「再寬你們三日限期，如其再不捉拿兇犯，定",
        "行處死！」二差聞言，篩糠打戰，只是磕頭，如雞食碎米一般。施公又說：「你們不必",
        "多說，快快去捕要緊。」施公想二役兩次受刑，亦覺心中不忍，退堂進內。可憐二人還",
        "在下面叩頭，大叫：「老爺，可憐小的們性命罷！」言畢，又是咚咚的叩頭。縣堂上未",
        "散的三班六房之人，見二人這樣，個個兔死狐悲，歎惜不止，一齊說：「罷呀！起來罷",
        "！老爺進去了，還求那個？」二人聞言，抬頭不看見老爺，忍氣站起，腿帶棒傷，身形",
        "晃亂。旁邊上來四個人，用手挽架下堂。"};


class UtfToGlyph : public Benchmark {
public:
    UtfToGlyph(SkTextEncoding encoding, const char* (*text), int lineCount, const char* name)
        : fEncoding{encoding}
        , fText{text}
        , fLineCount{lineCount}
        , fName{name} { }

protected:
    const char* onGetName() override {
        return fName;
    }

    bool isSuitableFor(Backend backend) override {
        return backend == kNonRendering_Backend;
    }

    void onDelayedSetup() override {
        int maxGlyphs = 0;
        for (int i = 0; i < fLineCount; i++) {
            fLines.emplace_back(this->convertLine(i));
            maxGlyphs = std::max(maxGlyphs, fLines.back()->glyphCount);
        }
        fGlyphIds.insert(fGlyphIds.begin(), maxGlyphs, 0);
        fTypeface = SkTypeface::MakeFromName("monospace", SkFontStyle());
    }

    void onDraw(int loops, SkCanvas* canvas) override {
        SkFont font(fTypeface);
        // Do more loops to reduce variance.
        for (int i = 0; i < loops * 3; ++i) {
            for (auto& line : fLines) {
                font.textToGlyphs(line->utf.data(), line->utf.size(), fEncoding,
                                  fGlyphIds.data(), line->glyphCount);
            }
        }
    }

private:
    struct Line {
        int glyphCount = 0;
        std::vector<char> utf;
    };

    std::unique_ptr<Line> convertLine(int lineIndex) {
        std::unique_ptr<Line> result = std::make_unique<Line>();

        const char* cursor = fText[lineIndex];
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
    sk_sp<SkTypeface> fTypeface;
    const char** fText;
    int fLineCount;
    const char* fName;
};

DEF_BENCH(return new UtfToGlyph(SkTextEncoding::kUTF32, ctext, SK_ARRAY_COUNT(ctext),
                                "SkTypefaceUTF32ToGlyphCN");)
DEF_BENCH(return new UtfToGlyph(SkTextEncoding::kUTF16, ctext, SK_ARRAY_COUNT(ctext),
                                "SkTypefaceUTF16ToGlyphCN");)
DEF_BENCH(return new UtfToGlyph(SkTextEncoding::kUTF8, ctext, SK_ARRAY_COUNT(ctext),
                                "SkTypefaceUTF8ToGlyphCN");)


DEF_BENCH(return new UtfToGlyph(SkTextEncoding::kUTF32, atext, SK_ARRAY_COUNT(atext),
                                "SkTypefaceUTF32ToGlyphAscii");)
DEF_BENCH(return new UtfToGlyph(SkTextEncoding::kUTF16, atext, SK_ARRAY_COUNT(atext),
                                "SkTypefaceUTF16ToGlyphAscii");)
DEF_BENCH(return new UtfToGlyph(SkTextEncoding::kUTF8, atext, SK_ARRAY_COUNT(atext),
                                "SkTypefaceUTF8ToGlyphAscii");)



