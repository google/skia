---
title: 'Text Properties API'
linkTitle: 'Text Properties API'
---

A variety of (internationally correct) text processing requires know the *properties* of unicode characters.
For example, where in a string are the word boundaries (needed for line-breaking), which need to be ordered
right-to-left or left-to-right?

We propose a batch call that **characterizes** the code-points in a string. The method will return an array
of bitfields packed in a 32bit unsigned long, containing the results of all of the Options.

## Functional Requirement

One measure of the value/completeness of the API is the following:

*For sophisticated apps or frameworks (e.g. Flutter or Lottie) that need...
- text shaping
- line breaking
- word and grapheme boundaries

Certainly this API could include **more** than is strictly required for those use cases, but it is important that it include **at least** enough to allow them to function without increasing their (WASM) download size
by having to include an copy of ICU (or its equivalent).

## Ergonomics

Associated with the above Function Requirements, another driver for the shape of the API is efficiency, esp. when called by **WASM** clients. There is a real cost for each JS <--> WASM call, more than the equivalent
sequence between JS and the Browser.
- Minimize # calls needed for a block of text
- Homogenous arrays rather than sequence of objects

Given this, implementations are encourged to use **Uint32Array** typed array buffer for the result.

```WebIDL
//  Bulk call to characterize the code-points in a string.
//  This can return a number of different properties per code-point, so to maximize performance,
//  it will only compute the requested properties requested (see optional boolean request fields).
//
interface TextProperties {
    const unsigned long BidiLevelMask   = 31,       // 0..31 bidi level

    const unsigned long GraphemeBreak   = 1 << 5,
    const unsigned long IntraWordBreak  = 1 << 6,
    const unsigned long WordBreak       = 1 << 7,
    const unsigned long SoftLineBreak   = 1 << 8,
    const unsigned long HardLineBreak   = 1 << 9,

    const unsigned long IsControl       = 1 << 10,
    const unsigned long IsSpace         = 1 << 11,
    const unsigned long IsWhiteSpace    = 1 << 12,

    attribute boolean bidiLevel?;
    attribute boolean graphemeBreak?;
    attribute boolean wordBreak?;       // returns Word and IntraWord break properties
    attribute boolean lineBreak?;       // returns Soft and Hard linebreak properties

    attribute boolean isControl?;
    attribute boolean isSpace?;
    attribute boolean isWhiteSpace?;

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
const properties = {
    isWhiteSpace: true,
    lineBreak: true,
};

const text = "Because I could not stop for Death\nHe kindly stopped for me";

const results = properties.characterize(text);

// expected results

results[7,9,15,19,24,28,37,44,52,65] --> IsWhiteSpace | SoftLineBreak
results[34] --> HardLineBreak
```

## Related

Some facilities for characterizing Unicode already exist, either as part of EcmaScript or the Web api. See [intl segmenter](https://github.com/tc39/proposal-intl-segmenter). This
proposal acknowledges these, but suggests that any potential overlap in functionality is OK,
given the design constraint spelled out in the [Ergonomics](#ergonomics) section.

Similar to the contrast between canvas2d and webgl, this proposal seeks to provide very efficient,
lower level access to unicode propoerties, specifically for sophisticated (possibly native ported to wasm)
frameworks and apps. It is not intended to replace existing facilities (i.e. Segmenter), but rather
to offer an alternative interface more suited to high-performance clients.

We also propose a higher level interface specfically aimed at [Text Shaping](/docs/dev/design/text_overview).

## Contributors:
 [mikerreed](https://github.com/mikerreed),
