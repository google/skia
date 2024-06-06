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
page will render by visiting the [Gerrit issue](https://skia-review.googlesource.com/c/skia/+/862957/####)
and clicking the eye icon to the left of the file:

<img src="../eye_icon.png" style="display: inline-block;" />

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

Bootstrap contains many classes that allow you to avoid setting styles via CSS.
For example, just using classes, we can change the font, the padding, and the
color:

```html
<p class="font-monospace p-2 text-danger">This is in monospace</p>
```

Which renders as:

<p class="font-monospace p-2 text-danger">This is in monospace</p>

## Diagrams

[Mermaid](https://mermaid-js.github.io/mermaid/#/) diagrams are enabled, so
this:

````markdown
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

## Code Snippets

To get syntax highlighting in code snippets you need to specify the language,
which is specified after the first code fence, for example this is how you would
show HTML markup:

````
```html
<p class="font-monospace p-2 text-danger">This is in monospace</p>
```
````

## Configuration

The Hugo configuration file is `config.toml` located in the site directory.
