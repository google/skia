Text Characterization API
=============
A variety of (internationally correct) text processing requires know the *properties* of unicode characters.
For example, where in a string are the word boundaries (needed for line-breaking), which need to be ordered
right-to-left or left-to-right?

We propose a batch call that **characterizes** the code-points in a string. The method will return an array
of bitfields packed in a 32bit unsigned long, containing the results of all of the Options.

## Functional Requirement

One measure of the value/completeness of the API is the following:

*For sophisticated apps or frameworks (e.g. Flutter or Skottie) that need...
- text shaping
- line breaking
- word and grapheme boundaries
- *whatever else to implement rich, editable text*

... can they, by using this API, avoid their own copy of ICU?*

Certainly this API could include **more** than is strictly required for those use cases, but it is important that it include **at least** enough to allow them to function without increasing their (WASM) download size.

## Ergonomics

Associated with the above Function Requirements, another driver for the shape of the API is efficiency, esp. when called by **WASM** clients. There is a real cost for each JS <--> WASM call, more than the equivalent
sequence between JS and the Browser.
- Minimize # calls needed for a block of text
- Homogenous arrays rather than sequence of objects

Given this, implementations are encourged to use **Uint32Array** array buffer for the result.

```WebIDL
interface TextCharacterization {
    const unsigned long BiDiLevelMask        = 63, // 6 bits reserved for bidi level

    const unsigned long WhiteSpaceBreak      = 1 << 6,
    const unsigned long SoftLineBreak        = 1 << 7,
    const unsigned long HardLineBreak        = 1 << 8,
    const unsigned long WordBreak            = 1 << 9,
    const unsigned long SentenceBreak        = 1 << 10,
    const unsigned long PartOfIntraWordBreak = 1 << 11,

    const unsigned long GraphemeStart        = 1 << 12,
    // room for future expansion

    attribute boolean bidiLevel?;
    attribute boolean whiteSpaceBreak?;
    attribute boolean softLineBreak?;
    attribute boolean hardLineBreak?;
    attribute boolean wordBreak;
    attribute boolean sentenceBreak;
    attribute boolean partOfIntraWordBreak?;
    attribute boolean graphemeStart?;

    // Returns an array the same length as the input string. Each returned value contains the
    // bitfield results for the corresponding code-point in the string. For surrogate pairs
    // in the input, the results will be in the first output value, and the 2nd output value
    // will be zero.
    //
    // Bitfields that are currently unused, or which correspond to an Option attribute that
    // was not requested, will be set to zero.
    //
    sequence<unsigned long> characterize(DOMString inputString,
                                         DOMString bcp47?);
}
```

## Example

```js
const options = {
    whiteSpaceBreak: true,
    hardLineBreak: true,
};

const text = "Because I could not stop for Death\nHe kindly stopped for me";

const results = options.characterize(text);

// expected results

results[7,9,15,19,24,28,37,44,52,65] --> WhiteSpaceBreak | WordBreak
results[34] --> HardLineBreak
```

## Related

Some facilities for characterizing Unicode already exist, either as part of EcmaScript or the Web api. See [intl segmenter](https://github.com/tc39/proposal-intl-segmenter). This
proposal acknowledges this, but suggests that any potential overlap in functionality may be OK,
given the design constraint spelled out in the [Ergonomics](#Ergonomics) section.



## Contributors:
 [mikerreed](https://github.com/mikerreed),
