Text Properties API
=============
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

```js
// Specifies which propties are wanted in a call to ComputeTextProperties()
//
interface TextPropertiesMasks {
    BidiLevel:      number;  // 63 for the low 6 bits

    GraphemeBreak:  number;  // 1 << 6,
    IntraWordBreak: nubmer;  // 1 << 7,
    WordBreak:      number;  // 1 << 8,
    SoftLineBreak:  number;  // 1 << 9,
    HardLineBreak:  number;  // 1 << 10,

    IsControl:      number;  // 1 << 11,
    IsSpace:        number;  // 1 << 12,
    IsWhiteSpace:   number;  // 1 << 13,
}

// These identify the bit-masks for the outputs of ComputeTextProperties()
//
interface TextPropertiesRequests {
    bidiLevel:     boolean?;

    graphemeBreak: boolean?;
    wordBreak:     boolean?;       // returns Word and IntraWord break properties
    lineBreak:     boolean?;       // returns Soft and Hard linebreak properties

    isControl:     boolean?;
    isSpace:       boolean?;
    isWhiteSpace:  boolean?;
}

// Returns an array the same length as the input string. Each returned value contains the
// bitfield results for the corresponding code-point in the string. For surrogate pairs
// in the input, the results will be in the first output value, and the 2nd output value
// will be zero.
//
// Bitfields that are currently unused, or which correspond to an Option attribute that
// was not requested, will be set to zero.
//
ComputeTextProperties(requests: TextPropertiesRequests,
                      text: string,
                      bcp47: string?): Uint32Array;
```

## Example

```js
const requests = {
    isWhiteSpace: true,
    lineBreak: true,
};

const text = "Because I could not stop for Death\nHe kindly stopped for me";

const results = ComputeTextProperties(requests, text);

// expected results

results[7,9,15,19,24,28,37,44,52,65] --> TextPropertiesMasks.IsWhiteSpace |
                                         TextPropertiesMasks.SoftLineBreak
results[34] --> TextPropertiesMasks.HardLineBreak
```

## Related

Some facilities for characterizing Unicode already exist, either as part of EcmaScript or the Web api. See [intl segmenter](https://github.com/tc39/proposal-intl-segmenter). This
proposal acknowledges these, but suggests that any potential overlap in functionality is OK,
given the design constraint spelled out in the [Ergonomics](#Ergonomics) section.

Similar to the contrast between canvas2d and webgl, this proposal seeks to provide very efficient,
lower level access to unicode propoerties, specifically for sophisticated (possibly native ported to wasm)
frameworks and apps. It is not intended to replace existing facilities (i.e. Segmenter), but rather
to offer an alternative interface more suited to high-performance clients.

We also propose a higher level interface specfically aimed at [Text Shaping](text_overview.md).

## Contributors:
 [mikerreed](https://github.com/mikerreed),
