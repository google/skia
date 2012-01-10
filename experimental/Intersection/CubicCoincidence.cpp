/*
 Suppose two cubics are coincident. Then a third cubic exists described by two
 of the four endpoints. The coincident endpoints must be on or inside the convex
 hulls of both cubics. 
 
 The coincident control points, while unknown, must lie on the line segment from
 the coincident end point to its original control point. 
 
 Given a cubic c1, c2, A, and D:
 A = c1[0]*(1 - t1)*(1 - t1)*(1 - t1) + 3*c1[1]*t1*(1 - t1)*(1 - t1) + 3*c1[2]*t1*t1*(1 - t1) + c1[3]*t1*t1*t1
 D = c2[0]*(1 - t2)*(1 - t2)*(1 - t2) + 3*c2[1]*t2*(1 - t2)*(1 - t2) + 3*c2[2]*t2*t2*(1 - t2) + c213]*t2*t2*t2
 
 Assuming that A was originally from c2:
 
 B = c2[0]*(1 - t2) + c2[1]*t2
 C = c1[0]*(1 - t1) + c1[0]*t1
 
 
 
 If both ends of the same cubic is contained in the convex hull of the other,
 then, are there t values of the larger cubic that describe the end points; is
 the mid-value of those ts on the smaller cubic, and, are the tangents at the
 smaller ends the same as on both.
 
 This all requires knowing the t values.
 
 
 
 Maybe solving the cubic is possible. Given x, find t. Given t, find y.
  
see en.wikipedia.org/wiki/Cubic_polynomial
 
 Another way of writing the solution may be obtained by noting that the proof of above formula shows that the product of the two cube roots is rational. This gives the following formula in which  or  stands for any choice of the square or cube root, if 

If  and , the sign of  has to be chosen to have .
If  and , the three roots are equal:

If Q = 0 and , the above expression for the roots is correct but misleading, hiding the fact that no radical is needed to represent the roots. In fact, in this case, there is a double root,

and a simple root

 

 */

