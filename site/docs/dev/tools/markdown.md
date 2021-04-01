---
title: 'Markdown'
linkTitle: 'Markdown'
---

This site is build with [Hugo](https://gohugo.io/) and
[Docsy](https://www.docsy.dev/).

Any file you put under `/site/` that has the extension `.md` will be processed
as Markdown. All other files will be served directly. For example, images can be
added and they will be served correctly and referenced from within Markdown
files.

When preparing for a code review of site docs you can get a preview of how the
page will render by visiting the skia.org site and add a query parameter `cl`
with the value of the Reitveld issue id:

    https://skia.org/path/to/markdown-file?cl=REITVELD_ISSUE_NUMBER

See the [Docsy documentation](https://www.docsy.dev/docs/) for more details on
how to configure and use docsy. For example the
[Navigation](https://www.docsy.dev/docs/adding-content/navigation/) section
explains what frontmatter needs to be added to a page to get it to appear in the
top navigation bar.

## Frontmatter

Each page needs a frontmatter section that provides information on that page.
For example:

```
---
title: 'Markdown'
linkTitle: 'Markdown'
---
```

This is true for both Markdown and HTML pages. See
[the Docsy documentation on frontmatter](https://www.docsy.dev/docs/adding-content/content/#page-frontmatter)
for more details.

## Styling And Icons

Docsy supports both
[Bootstrap](https://getbootstrap.com/docs/5.0/getting-started/introduction/) and
[Font-Awesome](https://fontawesome.com/). Check out their documentation for what
they offer.

Bootstrap is huge and powerful, you will need to really read the documentation,
but here are a few examples of what's possible:

[Accordian](https://getbootstrap.com/docs/5.0/components/accordion/)

<div class="accordion" id="accordionExample">
  <div class="accordion-item">
    <h2 class="accordion-header" id="headingOne">
      <button class="accordion-button" type="button" data-bs-toggle="collapse" data-bs-target="#collapseOne" aria-expanded="true" aria-controls="collapseOne">
        Accordion Item #1
      </button>
    </h2>
    <div id="collapseOne" class="accordion-collapse collapse show" aria-labelledby="headingOne" data-bs-parent="#accordionExample">
      <div class="accordion-body">
        <strong>This is the first item's accordion body.</strong>
      </div>
    </div>
  </div>
  <div class="accordion-item">
    <h2 class="accordion-header" id="headingTwo">
      <button class="accordion-button collapsed" type="button" data-bs-toggle="collapse" data-bs-target="#collapseTwo" aria-expanded="false" aria-controls="collapseTwo">
        Accordion Item #2
      </button>
    </h2>
    <div id="collapseTwo" class="accordion-collapse collapse" aria-labelledby="headingTwo" data-bs-parent="#accordionExample">
      <div class="accordion-body">
        <strong>This is the second item's accordion body.</strong>
      </div>
    </div>
  </div>
</div>

[Definition Lists](https://getbootstrap.com/docs/5.0/content/typography/#description-list-alignment)

<dl class="row">
  <dt class="col-sm-3">Description lists</dt>
  <dd class="col-sm-9">A description list is perfect for defining terms.</dd>

  <dt class="col-sm-3">Term</dt>
  <dd class="col-sm-9">
    <p>Definition for the term.</p>
    <p>And some more placeholder definition text.</p>
  </dd>

  <dt class="col-sm-3">Another term</dt>
  <dd class="col-sm-9">This definition is short, so no extra paragraphs or anything.</dd>
</dl>

## Diagrams

[Mermaid](https://mermaid-js.github.io/mermaid/#/) diagrams are enabled, so
this:

````
```mermaid
graph TD;
    A-->B;
    A-->C;
    B-->D;
    C-->D;
```
````

Gets rendered as:

```mermaid
graph TD;
    A-->B;
    A-->C;
    B-->D;
    C-->D;
```

## Configuration

The Hugo configuration file is [config.toml](../../../config.toml) in the site
directory.
