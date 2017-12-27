Two-point Conical Gradient
====

<script type="text/x-mathjax-config">
MathJax.Hub.Config({
    tex2jax: {
        inlineMath: [['$','$'], ['\\(','\\)']]
    }
});
</script>

<script src='https://cdnjs.cloudflare.com/ajax/libs/mathjax/2.7.2/MathJax.js?config=TeX-MML-AM_CHTML'></script>

We present a fast but not-so-straightforward shader algorithm (compared to bruteforcely solving
the quadratic equation) for computing the two-point conical gradient (see
[spec](https://html.spec.whatwg.org/multipage/canvas.html#dom-context-2d-createradialgradient)).

## Problem Statement and Setup

Let two circles be $C_0, r_0$ and $C_1, r_1$ where $C$ is the center and $r$ is the radius. For any
point $P = (x, y)$ we want the shader to quickly compute a gradient $t \in \mathbb R$ such that $p$
is on the linearly interpolated circle with center $C_t = (1-t) \cdot C_0 + t \cdot C_1$ and radius
$r_t = (1-t) \cdot r_0 + t \cdot r_1 \geq 0$ (note that radius $r_t$ has to be *nonnegative*). If
there are multiple (at most 2) solutions of $t$, choose the bigger one.

There are two degenerated cases:

1. $C_0 = C_1$ so the gradient is essentially a simple radial gradient.
2. $r_0 = r_1$ so the gradient is a single strip with bandwidth $2 r_0 = 2 r_1$.

<!-- TODO maybe add some fiddle or images here to illustrate the two degenerated cases -->

They are easy to handle so we won't cover them here. From now on, we assume $C_0 \neq C_1$ and $r_0
\neq r_1$.

As $r_0 \neq r_1$, we can find a focal point $C_f = (1-f) \cdot C_0 + f \cdot C_1$ where its
corresponding linearly interpolated radius $r_f = (1-f) \cdot r_0 + f \cdot r_1 = 0$.
Solving the latter equation gets us $f = r_0 / (r_0 - r_1)$.

As $C_0 \neq C_1$, focal point $C_f$ is different from $C_1$ unless $r_1 = 0$. If $r_1 = 0$, we can
swap $C_0, r_0$ with $C_1, r_1$, compute swapped gradient $t_s$ as if $r_1 \neq 0$, and finally set
$t = 1 - t_s$. The only catch here is that with multiple solutions of $t_s$, we shall choose the
smaller one (so $t$ could be the bigger one).

Assuming that we've done swapping if necessary so $C_1 \neq C_f$, we can then do a linear
transformation to map $C_f, C_1$ to $(0, 0), (1, 0)$. After the transformation:

1. All centers $C_t = (x_t, 0)$ must be on the $x$ axis
2. The radius $r_t$ is $x_t r_1$.
3. Given $x_t$ , we can derive $t = f + (1 - f) x_t$

From now on, we'll focus on how to quickly computes $x_t$. Note that $r_t \geq 0$ so we're only
interested nonnegative solution $x_t$. Again, if there are multiple $x_t$ solutions, we may want to
find the bigger one if $1 - f > 0$, and smaller one if $1 - f < 0$, so the corresponding $t$ is
always the bigger one (note that $f \neq 1$, otherwise we'll swap $c_0, r_0$ with $c_1, r_1$).

## Solving $x_t$

**Lemma 1.** Draw a ray from $C_f = (0, 0)$ to $P = (x, y)$. For every
intersection points $P_1$ between that ray and circle $C_1 = (1, 0), r_1$, there exists an $x_t$
that equals to the length of segment $C_f P$ over length of segment $C_f P_1$. That is,
$x_t = || C_f P || / ||C_f P_1||$

*Proof.* Draw a line from $P$ that's parallel to $C_1 P_1$. Let it intersect with $x$-axis on point
$C = (x', y')$.

<img src="conical/lemma1.svg"/>

Triangle $\triangle C_f C P$ is similar to triangle $\triangle C_f C_1 P_1$.
Therefore $||P C|| = ||P_1 C_1|| \cdot (||C_f C|| / ||C_f C_1||) = r_1 x'$. Thus $x'$ is a solution
to $x_t$. Because triangle $\triangle C_f C P$ and triangle $\triangle C_f C_1 P_1$ are similar, $x'
= ||C_f C_1|| \cdot (||C_f P|| / ||C_f P_1||) = ||C_f P|| / ||C_f P_1||$. $\square$

**Lemma 2.** For every solution $x_t$, if we extend/shrink segment $C_f P$ to $C_f P_1$ with ratio
$1 / x_t$ (i.e., find $P_1$ on ray $C_f P$ such that $||C_f P_1|| / ||C_f P|| = 1 / x_t$), then
$P_1$ must be on circle $C_1, r_1$.

*Proof.* Let $C_t = (x_t, 0)$. Triangle $\triangle C_f C_t P$ is similar to $C_f C_1 P_1$. Therefore
$||C_1 P_1|| = r_1$ and $P_1$ is on circle $C_1, r_1$. $\square$

**Corollary 1.** By lemma 1. and 2., we conclude that the number of solutions $x_t$ is equal to the
number of intersections between ray $C_f P$ and circle $C_1, r_1$. Therefore

* when $r_1 > 1$, there's always one unique intersection/solution; we call this "well-behaved"; this
  was previously known as the "inside" case;
* when $r_1 = 1$, there's either one or zero intersection/solution (excluding $C_f$ which is always
  on the circle); we call this "focal-on-circle"; this was previously known as the "edge" case;

<img src="conical/corollary2.2.1.svg"/>
<img src="conical/corollary2.2.2.svg"/>

* when $r_1 < 1$, there may be $0, 1$, or $2$ solutions; this was also previously as the "outside"
  case.

<img src="conical/corollary2.3.1.svg" width="30%"/>
<img src="conical/corollary2.3.2.svg" width="30%"/>
<img src="conical/corollary2.3.3.svg" width="30%"/>

**Lemma 3.** When solution exists, one such solution is
$$
    x_t = {|| C_f P || \over ||C_f P_1||} = \frac{x^2 + y^2}{x + \sqrt{(r_1^2 - 1) y^2 + r_1^2 x^2}}
$$

*Proof.* As $C_f = (0, 0), P = (x, y)$, we have $||C_f P|| = \sqrt(x^2 + y^2)$. So we'll mainly
focus on how to compute $||C_f P_1||$.

**When $x \geq 0$:**

<img src="conical/lemma3.1.svg"/>

Let $X_P = (x, 0)$ and $H$ be a point on $C_f P_1$ such that $C_1 H$ is perpendicular to $C_1
P_1$. Triangle $\triangle C_1 H C_f$ is similar to triangle $\triangle P X_P C_f$. Thus
$$||C_f H|| = ||C_f C_1|| \cdot (||C_f X_P|| / ||C_f P||) = x / \sqrt{x^2 + y^2}$$
$$||C_1 H|| = ||C_f C_1|| \cdot (||P X_P|| / ||C_f P||) = y / \sqrt{x^2 + y^2}$$

Triangle $\triangle C_1 H P_1$ is a right triangle with hypotenuse $r_1$. Hence
$$ ||H P_1|| = \sqrt{r_1^2 - ||C_1 H||^2} = \sqrt{r_1^2 - y^2 / (x^2 + y^2)} $$

We have
\begin{align}
    ||C_f P_1|| &= ||C_f H|| + ||H P_1|| \\\\\\
        &= x / \sqrt{x^2 + y^2} + \sqrt{r_1^2 - y^2 / (x^2 + y^2)} \\\\\\
        &= \frac{x + \sqrt{r_1^2 (x^2 + y^2) - y^2}}{\sqrt{x^2 + y^2}} \\\\\\
        &= \frac{x + \sqrt{(r_1^2 - 1) y^2 + r_1^2 x^2}}{\sqrt{x^2 + y^2}}
\end{align}

**When $x < 0$:**

Define $X_P$ and $H$ similarly as before except that now $H$ is on ray $P_1 C_f$ instead of
$C_f P_1$.

<img src="conical/lemma3.2.svg"/>

As before, triangle $\triangle C_1 H C_f$ is similar to triangle $\triangle P X_P C_f$, and triangle
$\triangle C_1 H P_1$ is a right triangle, so we have
$$||C_f H|| = ||C_f C_1|| \cdot (||C_f X_P|| / ||C_f P||) = -x / \sqrt{x^2 + y^2}$$
$$||C_1 H|| = ||C_f C_1|| \cdot (||P X_P|| / ||C_f P||) = y / \sqrt{x^2 + y^2}$$
$$ ||H P_1|| = \sqrt{r_1^2 - ||C_1 H||^2} = \sqrt{r_1^2 - y^2 / (x^2 + y^2)} $$

Note that the only difference is changing $x$ to $-x$ because $x$ is negative.

Also note that now $||C_f P_1|| = -||C_f H|| + ||H P_1||$ and we have $-||C_f H||$ instead of
$||C_f H||$. That negation cancels out the negation of $-x$ so we get the same equation
of $||C_f P_1||$ for both $x \geq 0$ and $x < 0$ cases:

$$
    ||C_f P_1|| = \frac{x + \sqrt{(r_1^2 - 1) y^2 + r_1^2 x^2}}{\sqrt{x^2 + y^2}}
$$

Finally
$$
    x_t = \frac{||C_f P||}{||C_f P_1||} = \frac{\sqrt{x^2 + y^2}}{||C_f P_1||}
        = \frac{x^2 + y^2}{x + \sqrt{(r_1^2 - 1) y^2 + r_1^2 x^2}}
$$ $\square$

**Corollary 2.** If $r_1 = 1$, $x_t = \frac{x^2 + y^2}{(1 + r_1) x}$, and the solution exists
(i.e., $x_t = \frac{x^2 + y^2}{(1 + r_1) x} \geq 0$) iff $x \geq 0$.

*Proof.* Simply plug $r_1 = 1$ into the formula of Lemma 3. $\square$

**Corollary 3.** If $r_1 > 1$, $x_t = (\sqrt{(r_1^2 - 1) y ^2 + r_1^2 x^2}  - x) / (r^2 - 1)$.

## Accelerations
