/* Modified for use with yasm by Peter Johnson. */
/*
------------------------------------------------------------------------------
perfect.c: code to generate code for a hash for perfect hashing.
(c) Bob Jenkins, September 1996, December 1999
You may use this code in any way you wish, and it is free.  No warranty.
I hereby place this in the public domain.
Source is http://burtleburtle.net/bob/c/perfect.c

This generates a minimal perfect hash function.  That means, given a
set of n keys, this determines a hash function that maps each of
those keys into a value in 0..n-1 with no collisions.

The perfect hash function first uses a normal hash function on the key
to determine (a,b) such that the pair (a,b) is distinct for all
keys, then it computes a^scramble[tab[b]] to get the final perfect hash.
tab[] is an array of 1-byte values and scramble[] is a 256-term array of 
2-byte or 4-byte values.  If there are n keys, the length of tab[] is a 
power of two between n/3 and n.

I found the idea of computing distinct (a,b) values in "Practical minimal 
perfect hash functions for large databases", Fox, Heath, Chen, and Daoud, 
Communications of the ACM, January 1992.  They found the idea in Chichelli 
(CACM Jan 1980).  Beyond that, our methods differ.

The key is hashed to a pair (a,b) where a in 0..*alen*-1 and b in
0..*blen*-1.  A fast hash function determines both a and b
simultaneously.  Any decent hash function is likely to produce
hashes so that (a,b) is distinct for all pairs.  I try the hash
using different values of *salt* until all pairs are distinct.

The final hash is (a XOR scramble[tab[b]]).  *scramble* is a
predetermined mapping of 0..255 into 0..smax-1.  *tab* is an
array that we fill in in such a way as to make the hash perfect.

First we fill in all values of *tab* that are used by more than one
key.  We try all possible values for each position until one works.

This leaves m unmapped keys and m values that something could hash to.
If you treat unmapped keys as lefthand nodes and unused hash values
as righthand nodes, and draw a line connecting each key to each hash
value it could map to, you get a bipartite graph.  We attempt to
find a perfect matching in this graph.  If we succeed, we have
determined a perfect hash for the whole set of keys.

*scramble* is used because (a^tab[i]) clusters keys around *a*.
------------------------------------------------------------------------------
*/

#include <string.h>
#include "tools/genperf/standard.h"
#include "libyasm/coretype.h"
#include "libyasm/phash.h"
#include "tools/genperf/perfect.h"

#define CHECKSTATE 8

/*
------------------------------------------------------------------------------
Find the mapping that will produce a perfect hash
------------------------------------------------------------------------------
*/

/* return the ceiling of the log (base 2) of val */
ub4  phash_log2(val)
ub4  val;
{
  ub4 i;
  for (i=0; ((ub4)1<<i) < val; ++i)
    ;
  return i;
}

/* compute p(x), where p is a permutation of 0..(1<<nbits)-1 */
/* permute(0)=0.  This is intended and useful. */
static ub4  permute(
    ub4 x,                                   /* input, a value in some range */
    ub4 nbits)                             /* input, number of bits in range */
{
  int i;
  int mask   = ((ub4)1<<nbits)-1;                                /* all ones */
  int const2 = 1+nbits/2;
  int const3 = 1+nbits/3;
  int const4 = 1+nbits/4;
  int const5 = 1+nbits/5;
  for (i=0; i<20; ++i)
  {
    x = (x+(x<<const2)) & mask; 
    x = (x^(x>>const3));
    x = (x+(x<<const4)) & mask;
    x = (x^(x>>const5));
  }
  return x;
}

/* initialize scramble[] with distinct random values in 0..smax-1 */
static void scrambleinit(
    ub4      *scramble,                        /* hash is a^scramble[tab[b]] */
    ub4       smax)                /* scramble values should be in 0..smax-1 */
{
  ub4 i;

  /* fill scramble[] with distinct random integers in 0..smax-1 */
  for (i=0; i<SCRAMBLE_LEN; ++i)
  {
    scramble[i] = permute(i, phash_log2(smax));
  }
}

/* 
 * Check if key1 and key2 are the same. 
 * We already checked (a,b) are the same.
 */
static void checkdup(
    key      *key1,
    key      *key2,
    hashform *form)
{
  switch(form->hashtype)
  {
  case STRING_HT:
    if ((key1->len_k == key2->len_k) &&
        !memcmp(key1->name_k, key2->name_k, (size_t)key1->len_k))
    {
      fprintf(stderr, "perfect.c: Duplicates keys!  %.*s\n",
              (int)key1->len_k, key1->name_k);
      exit(EXIT_FAILURE);
    }
    break;
  case INT_HT:
    if (key1->hash_k == key2->hash_k)
    {
      fprintf(stderr, "perfect.c: Duplicate keys!  %.8lx\n", key1->hash_k);
      exit(EXIT_FAILURE);
    }
    break;
  case AB_HT:
    fprintf(stderr, "perfect.c: Duplicate keys!  %.8lx %.8lx\n",
            key1->a_k, key1->b_k);
    exit(EXIT_FAILURE);
    break;
  default:
    fprintf(stderr, "perfect.c: Illegal hash type %ld\n", (ub4)form->hashtype);
    exit(EXIT_FAILURE);
    break;
  }
}


/* 
 * put keys in tabb according to key->b_k
 * check if the initial hash might work 
 */
static int inittab(
    bstuff   *tabb,                 /* output, list of keys with b for (a,b) */
    ub4       blen,                                        /* length of tabb */
    key      *keys,                           /* list of keys already hashed */
    hashform *form,                                       /* user directives */
    int       complete)    /* TRUE means to complete init despite collisions */
{
  int  nocollision = TRUE;
  key *mykey;

  memset((void *)tabb, 0, (size_t)(sizeof(bstuff)*blen));

  /* Two keys with the same (a,b) guarantees a collision */
  for (mykey=keys; mykey; mykey=mykey->next_k)
  {
    key *otherkey;

    for (otherkey=tabb[mykey->b_k].list_b; 
         otherkey; 
         otherkey=otherkey->nextb_k)
    {
      if (mykey->a_k == otherkey->a_k)
      {
        nocollision = FALSE;
        checkdup(mykey, otherkey, form);
        if (!complete)
          return FALSE;
      }
    }
    ++tabb[mykey->b_k].listlen_b;
    mykey->nextb_k = tabb[mykey->b_k].list_b;
    tabb[mykey->b_k].list_b = mykey;
  }

  /* no two keys have the same (a,b) pair */
  return nocollision;
}


/* Do the initial hash for normal mode (use lookup and checksum) */
static void initnorm(
    key      *keys,                                      /* list of all keys */
    ub4       alen,                /* (a,b) has a in 0..alen-1, a power of 2 */
    ub4       blen,                /* (a,b) has b in 0..blen-1, a power of 2 */
    ub4       smax,               /* maximum range of computable hash values */
    ub4       salt,                 /* used to initialize the hash function */
    gencode  *final)                      /* output, code for the final hash */
{
  key *mykey;
  if (phash_log2(alen)+phash_log2(blen) > UB4BITS)
  {
    ub4 initlev = (salt*0x9e3779b9)&0xffffffff;  /* the golden ratio; an arbitrary value */

    for (mykey=keys; mykey; mykey=mykey->next_k)
    {
      ub4 i, state[CHECKSTATE];
      for (i=0; i<CHECKSTATE; ++i) state[i] = initlev;
      phash_checksum( mykey->name_k, mykey->len_k, state);
      mykey->a_k = state[0]&(alen-1);
      mykey->b_k = state[1]&(blen-1);
    }
    final->used = 4;
    sprintf(final->line[0], 
            "  unsigned long i,state[CHECKSTATE],rsl;\n");
    sprintf(final->line[1], 
            "  for (i=0; i<CHECKSTATE; ++i) state[i]=0x%lx;\n",initlev);
    sprintf(final->line[2],
            "  phash_checksum(key, len, state);\n");
    sprintf(final->line[3], 
            "  rsl = ((state[0]&0x%lx)^scramble[tab[state[1]&0x%lx]]);\n",
            alen-1, blen-1);
  }
  else
  {
    ub4 loga = phash_log2(alen);                            /* log based 2 of blen */
    ub4 initlev = (salt*0x9e3779b9)&0xffffffff;  /* the golden ratio; an arbitrary value */

    for (mykey=keys; mykey; mykey=mykey->next_k)
    {
      ub4 hash = phash_lookup(mykey->name_k, mykey->len_k, initlev);
      mykey->a_k = (loga > 0) ? hash>>(UB4BITS-loga) : 0;
      mykey->b_k = (blen > 1) ? hash&(blen-1) : 0;
    }
    final->used = 2;
    sprintf(final->line[0], 
            "  unsigned long rsl, val = phash_lookup(key, len, 0x%lxUL);\n", initlev);
    if (smax <= 1)
    {
      sprintf(final->line[1], "  rsl = 0;\n");
    }
    else if (blen < USE_SCRAMBLE)
    {
      sprintf(final->line[1], "  rsl = ((val>>%ld)^tab[val&0x%lx]);\n",
              UB4BITS-phash_log2(alen), blen-1);
    }
    else
    {
      sprintf(final->line[1], "  rsl = ((val>>%ld)^scramble[tab[val&0x%lx]]);\n",
              UB4BITS-phash_log2(alen), blen-1);
    }
  }
}



/* Do initial hash for inline mode */
static void initinl(
    key      *keys,                                      /* list of all keys */
    ub4       alen,                /* (a,b) has a in 0..alen-1, a power of 2 */
    ub4       blen,                /* (a,b) has b in 0..blen-1, a power of 2 */
    ub4       smax,                       /* range of computable hash values */
    ub4       salt,                  /* used to initialize the hash function */
    gencode  *final)                        /* generated code for final hash */
{
  key *mykey;
  ub4  amask = alen-1;
  ub4  blog  = phash_log2(blen);
  ub4  initval = salt*0x9e3779b9;    /* the golden ratio; an arbitrary value */

  /* It's more important to have b uniform than a, so b is the low bits */
  for (mykey = keys;  mykey != (key *)0;  mykey = mykey->next_k)
  {
    ub4   hash = initval;
    ub4   i;
    for (i=0; i<mykey->len_k; ++i)
    {
      hash = ((ub1)mykey->name_k[i] ^ hash) + ((hash<<(UB4BITS-6))+(hash>>6));
    }
    mykey->hash_k = hash;
    mykey->a_k = (alen > 1) ? (hash & amask) : 0;
    mykey->b_k = (blen > 1) ? (hash >> (UB4BITS-blog)) : 0;
  }
  final->used = 1;
  if (smax <= 1)
  {
    sprintf(final->line[0], "  unsigned long rsl = 0;\n");
  }
  else if (blen < USE_SCRAMBLE)
  {
    sprintf(final->line[0], "  unsigned long rsl = ((val & 0x%lx) ^ tab[val >> %ld]);\n",
            amask, UB4BITS-blog);
  }
  else
  {
    sprintf(final->line[0], "  unsigned long rsl = ((val & 0x%lx) ^ scramble[tab[val >> %ld]]);\n",
            amask, UB4BITS-blog);
  }
}


/* 
 * Run a hash function on the key to get a and b 
 * Returns:
 *   0: didn't find distinct (a,b) for all keys
 *   1: found distinct (a,b) for all keys, put keys in tabb[]
 *   2: found a perfect hash, no need to do any more work
 */
static ub4 initkey(
    key      *keys,                                      /* list of all keys */
    ub4       nkeys,                                 /* total number of keys */
    bstuff   *tabb,                                    /* stuff indexed by b */
    ub4       alen,                /* (a,b) has a in 0..alen-1, a power of 2 */
    ub4       blen,                /* (a,b) has b in 0..blen-1, a power of 2 */
    ub4       smax,                       /* range of computable hash values */
    ub4       salt,                 /* used to initialize the hash function */
    hashform *form,                                       /* user directives */
    gencode  *final)                                  /* code for final hash */
{
  /* Do the initial hash of the keys */
  switch(form->mode)
  {
  case NORMAL_HM:
    initnorm(keys, alen, blen, smax, salt, final);
    break;
  case INLINE_HM:
    initinl(keys, alen, blen, smax, salt, final);
    break;
#if 0
  case HEX_HM:
  case DECIMAL_HM:
    finished = inithex(keys, nkeys, alen, blen, smax, salt, final, form); 
    if (finished) return 2;
    break;
#endif
  default:
    fprintf(stderr, "fatal error: illegal mode\n"); 
    exit(1);
  }

  if (nkeys <= 1)
  {
    final->used = 1;
    sprintf(final->line[0], "  unsigned long rsl = 0;\n");
    return 2;
  }

  return inittab(tabb, blen, keys, form, FALSE);
}

/* Print an error message and exit if there are duplicates */
static void duplicates(
    bstuff   *tabb,                /* array of lists of keys with the same b */
    ub4       blen,                          /* length of tabb, a power of 2 */
    key      *keys,
    hashform *form)                                       /* user directives */
{
  ub4  i;
  key *key1;
  key *key2;

  (void)inittab(tabb, blen, keys, form, TRUE);

  /* for each b, do nested loops through key list looking for duplicates */
  for (i=0; i<blen; ++i)
    for (key1=tabb[i].list_b; key1; key1=key1->nextb_k)
      for (key2=key1->nextb_k; key2; key2=key2->nextb_k)
        checkdup(key1, key2, form);
}


/* Try to apply an augmenting list */
static int apply(
    bstuff *tabb,
    hstuff *tabh,
    qstuff *tabq,
    ub4     blen,
    ub4    *scramble,
    ub4     tail,
    int     rollback)      /* FALSE applies augmenting path, TRUE rolls back */
{
  ub4     hash;
  key    *mykey;
  bstuff *pb;
  ub4     child;
  ub4     parent;
  ub4     stabb;                                         /* scramble[tab[b]] */

  /* walk from child to parent */
  for (child=tail-1; child; child=parent)
  {
    parent = tabq[child].parent_q;                    /* find child's parent */
    pb     = tabq[parent].b_q;             /* find parent's list of siblings */

    /* erase old hash values */
    stabb = scramble[pb->val_b];
    for (mykey=pb->list_b; mykey; mykey=mykey->nextb_k)
    {
      hash = mykey->a_k^stabb;
      if (mykey == tabh[hash].key_h)
      {                            /* erase hash for all of child's siblings */
        tabh[hash].key_h = (key *)0;
      }
    }

    /* change pb->val_b, which will change the hashes of all parent siblings */
    pb->val_b = (rollback ? tabq[child].oldval_q : tabq[child].newval_q);

    /* set new hash values */
    stabb = scramble[pb->val_b];
    for (mykey=pb->list_b; mykey; mykey=mykey->nextb_k)
    {
      hash = mykey->a_k^stabb;
      if (rollback)
      {
        if (parent == 0) continue;                  /* root never had a hash */
      }
      else if (tabh[hash].key_h)
      {
        /* very rare: roll back any changes */
        apply(tabb, tabh, tabq, blen, scramble, tail, TRUE);
        return FALSE;                                  /* failure, collision */
      }
      tabh[hash].key_h = mykey;
    }
  }
  return TRUE;
}


/*
-------------------------------------------------------------------------------
augment(): Add item to the mapping.

Construct a spanning tree of *b*s with *item* as root, where each
parent can have all its hashes changed (by some new val_b) with 
at most one collision, and each child is the b of that collision.

I got this from Tarjan's "Data Structures and Network Algorithms".  The
path from *item* to a *b* that can be remapped with no collision is 
an "augmenting path".  Change values of tab[b] along the path so that 
the unmapped key gets mapped and the unused hash value gets used.

Assuming 1 key per b, if m out of n hash values are still unused, 
you should expect the transitive closure to cover n/m nodes before 
an unused node is found.  Sum(i=1..n)(n/i) is about nlogn, so expect
this approach to take about nlogn time to map all single-key b's.
-------------------------------------------------------------------------------
*/
static int augment(
    bstuff   *tabb,                                    /* stuff indexed by b */
    hstuff   *tabh,  /* which key is associated with which hash, indexed by hash */
    qstuff   *tabq,        /* queue of *b* values, this is the spanning tree */
    ub4       blen,                                        /* length of tabb */
    ub4      *scramble,                  /* final hash is a^scramble[tab[b]] */
    ub4       smax,                             /* highest value in scramble */
    bstuff   *item,                       /* &tabb[b] for the b to be mapped */
    ub4       nkeys,                     /* final hash must be in 0..nkeys-1 */
    ub4       highwater,    /* a value higher than any now in tabb[].water_b */
    hashform *form)           /* TRUE if we should do a minimal perfect hash */
{
  ub4  q;                      /* current position walking through the queue */
  ub4  tail;              /* tail of the queue.  0 is the head of the queue. */
  ub4  limit=((blen < USE_SCRAMBLE) ? smax : UB1MAXVAL+1);
  ub4  highhash = ((form->perfect == MINIMAL_HP) ? nkeys : smax);
  int  trans = (form->speed == SLOW_HS || form->perfect == MINIMAL_HP);

  /* initialize the root of the spanning tree */
  tabq[0].b_q = item;
  tail = 1;

  /* construct the spanning tree by walking the queue, add children to tail */
  for (q=0; q<tail; ++q)
  {
    bstuff *myb = tabq[q].b_q;                        /* the b for this node */
    ub4     i;                              /* possible value for myb->val_b */

    if (!trans && (q == 1)) 
      break;                                  /* don't do transitive closure */

    for (i=0; i<limit; ++i)
    {
      bstuff *childb = (bstuff *)0;             /* the b that this i maps to */
      key    *mykey;                       /* for walking through myb's keys */

      for (mykey = myb->list_b; mykey; mykey=mykey->nextb_k)
      {
        key    *childkey;
        ub4 hash = mykey->a_k^scramble[i];

        if (hash >= highhash) break;                        /* out of bounds */
        childkey = tabh[hash].key_h;

        if (childkey)
        {
          bstuff *hitb = &tabb[childkey->b_k];

          if (childb)
          {
            if (childb != hitb) break;            /* hit at most one child b */
          }
          else
          {
            childb = hitb;                        /* remember this as childb */
            if (childb->water_b == highwater) break;     /* already explored */
          }
        }
      }
      if (mykey) continue;             /* myb with i has multiple collisions */

      /* add childb to the queue of reachable things */
      if (childb) childb->water_b = highwater;
      tabq[tail].b_q      = childb;
      tabq[tail].newval_q = (ub2)i; /* how to make parent (myb) use this hash */
      tabq[tail].oldval_q = myb->val_b;            /* need this for rollback */
      tabq[tail].parent_q = q;
      ++tail;

      if (!childb)
      {                                  /* found an *i* with no collisions? */
        /* try to apply the augmenting path */
        if (apply(tabb, tabh, tabq, blen, scramble, tail, FALSE))
          return TRUE;        /* success, item was added to the perfect hash */

        --tail;                    /* don't know how to handle such a child! */
      }
    }
  }
  return FALSE;
}


/* find a mapping that makes this a perfect hash */
static int perfect(
    bstuff   *tabb,
    hstuff   *tabh,
    qstuff   *tabq,
    ub4       blen,
    ub4       smax,
    ub4      *scramble,
    ub4       nkeys,
    hashform *form)
{
  ub4 maxkeys;                           /* maximum number of keys for any b */
  ub4 i, j;

  /* clear any state from previous attempts */
  memset((void *)tabh, 0, 
         (size_t)(sizeof(hstuff)*
                  ((form->perfect == MINIMAL_HP) ? nkeys : smax)));
  memset((void *)tabq, 0, (size_t)(sizeof(qstuff)*(blen+1)));

  for (maxkeys=0,i=0; i<blen; ++i) 
    if (tabb[i].listlen_b > maxkeys) 
      maxkeys = tabb[i].listlen_b;

  /* In descending order by number of keys, map all *b*s */
  for (j=maxkeys; j>0; --j)
    for (i=0; i<blen; ++i)
      if (tabb[i].listlen_b == j)
        if (!augment(tabb, tabh, tabq, blen, scramble, smax, &tabb[i], nkeys, 
                     i+1, form))
        {
          fprintf(stderr, "fail to map group of size %ld for tab size %ld\n", j, blen);
          return FALSE;
        }

  /* Success!  We found a perfect hash of all keys into 0..nkeys-1. */
  return TRUE;
}


/*
 * Simple case: user gave (a,b).  No more mixing, no guessing alen or blen. 
 * This assumes a,b reside in (key->a_k, key->b_k), and final->form == AB_HK.
 */
static void hash_ab(
    bstuff  **tabb,       /* output, tab[] of the perfect hash, length *blen */
    ub4      *alen,             /* output, 0..alen-1 is range for a of (a,b) */
    ub4      *blen,             /* output, 0..blen-1 is range for b of (a,b) */
    ub4      *salt,                     /* output, initializes initial hash */
    gencode  *final,                                  /* code for final hash */
    ub4      *scramble,                  /* input, hash = a^scramble[tab[b]] */
    ub4      *smax,                       /* input, scramble[i] in 0..smax-1 */
    key      *keys,                                   /* input, keys to hash */
    ub4       nkeys,                   /* input, number of keys being hashed */
    hashform *form)                                       /* user directives */
{
  hstuff *tabh;
  qstuff *tabq;
  key    *mykey;
  ub4     i;
  int     used_tab;

  /* initially make smax the first power of two bigger than nkeys */
  *smax = ((ub4)1<<phash_log2(nkeys));
  scrambleinit(scramble, *smax);

  /* set *alen and *blen based on max A and B from user */
  *alen = 1;
  *blen = 1;
  for (mykey = keys;  mykey != (key *)0;  mykey = mykey->next_k)
  {
    while (*alen <= mykey->a_k) *alen *= 2;
    while (*blen <= mykey->b_k) *blen *= 2;
  }
  if (*alen > 2**smax)
  {
    fprintf(stderr,
      "perfect.c: Can't deal with (A,B) having A bigger than twice \n");
    fprintf(stderr,
      "  the smallest power of two greater or equal to any legal hash.\n");
    exit(EXIT_FAILURE);
  }

  /* allocate working memory */
  *tabb = (bstuff *)yasm_xmalloc((size_t)(sizeof(bstuff)*(*blen)));
  tabq  = (qstuff *)yasm_xmalloc(sizeof(qstuff)*(*blen+1));
  tabh  = (hstuff *)yasm_xmalloc(sizeof(hstuff)*(form->perfect == MINIMAL_HP ? 
                                             nkeys : *smax));

  /* check that (a,b) are distinct and put them in tabb indexed by b */
  (void)inittab(*tabb, *blen, keys, form, FALSE);

  /* try with smax */
  if (!perfect(*tabb, tabh, tabq, *blen, *smax, scramble, nkeys, form))
  {
    if (form->perfect == MINIMAL_HP)
    {
      fprintf(stderr, "fatal error: Cannot find perfect hash for user (A,B) pairs\n");
      exit(EXIT_FAILURE);
    }
    else
    {
      /* try with 2*smax */
      free((void *)tabh);
      *smax = *smax * 2;
      scrambleinit(scramble, *smax);
      tabh = (hstuff *)yasm_xmalloc(sizeof(hstuff)*(form->perfect == MINIMAL_HP ?
                                                nkeys : *smax));
      if (!perfect(*tabb, tabh, tabq, *blen, *smax, scramble, nkeys, form))
      {
        fprintf(stderr, "fatal error: Cannot find perfect hash for user (A,B) pairs\n");
        exit(EXIT_FAILURE);
      }
    }
  }

  /* check if tab[] was really needed */
  for (i=0; i<*blen; ++i)
  {
    if ((*tabb)[i].val_b != 0) break;            /* assumes permute(0) == 0 */
  }
  used_tab = (i < *blen);

  /* write the code for the perfect hash */
  *salt = 1;
  final->used = 1;
  if (!used_tab)
  {
    sprintf(final->line[0], "  unsigned long rsl = a;\n");
  }
  else if (*blen < USE_SCRAMBLE)
  {
    sprintf(final->line[0], "  unsigned long rsl = (a ^ tab[b]);\n");
  }
  else
  {
    sprintf(final->line[0], "  unsigned long rsl = (a ^ scramble[tab[b]]);\n");
  }

  free((void *)tabq);
  free((void *)tabh);
}


/* guess initial values for alen and blen */
static void initalen(
    ub4      *alen,                                  /* output, initial alen */
    ub4      *blen,                                  /* output, initial blen */
    ub4      *smax,/* input, power of two greater or equal to max hash value */
    ub4       nkeys,                          /* number of keys being hashed */
    hashform *form)                                       /* user directives */
{
  /*
   * Find initial *alen, *blen
   * Initial alen and blen values were found empirically.  Some factors:
   *
   * If smax<256 there is no scramble, so tab[b] needs to cover 0..smax-1.
   *
   * alen and blen must be powers of 2 because the values in 0..alen-1 and
   * 0..blen-1 are produced by applying a bitmask to the initial hash function.
   *
   * alen must be less than smax, in fact less than nkeys, because otherwise
   * there would often be no i such that a^scramble[i] is in 0..nkeys-1 for
   * all the *a*s associated with a given *b*, so there would be no legal
   * value to assign to tab[b].  This only matters when we're doing a minimal
   * perfect hash.
   *
   * It takes around 800 trials to find distinct (a,b) with nkey=smax*(5/8)
   * and alen*blen = smax*smax/32.
   *
   * Values of blen less than smax/4 never work, and smax/2 always works.
   *
   * We want blen as small as possible because it is the number of bytes in
   * the huge array we must create for the perfect hash.
   *
   * When nkey <= smax*(5/8), blen=smax/4 works much more often with 
   * alen=smax/8 than with alen=smax/4.  Above smax*(5/8), blen=smax/4
   * doesn't seem to care whether alen=smax/8 or alen=smax/4.  I think it
   * has something to do with 5/8 = 1/8 * 5.  For example examine 80000, 
   * 85000, and 90000 keys with different values of alen.  This only matters
   * if we're doing a minimal perfect hash.
   *
   * When alen*blen <= 1<<UB4BITS, the initial hash must produce one integer.
   * Bigger than that it must produce two integers, which increases the
   * cost of the hash per character hashed.
   */
  if (form->perfect == NORMAL_HP)
  {
    if ((form->speed == FAST_HS) && (nkeys > *smax*0.8))
    {
      *smax = *smax * 2;
    }

    *alen = ((form->hashtype==INT_HT) && *smax>131072) ? 
      ((ub4)1<<(UB4BITS-phash_log2(*blen))) :   /* distinct keys => distinct (A,B) */
      *smax;                         /* no reason to restrict alen to smax/2 */
    if ((form->hashtype == INT_HT) && *smax < 32)
      *blen = *smax;                      /* go for function speed not space */
    else if (*smax/4 <= (1<<14))
      *blen = ((nkeys <= *smax*0.56) ? *smax/32 :
               (nkeys <= *smax*0.74) ? *smax/16 : *smax/8);
    else
      *blen = ((nkeys <= *smax*0.6) ? *smax/16 : 
               (nkeys <= *smax*0.8) ? *smax/8 : *smax/4);

    if ((form->speed == FAST_HS) && (*blen < *smax/8))
      *blen = *smax/8;

    if (*alen < 1) *alen = 1;
    if (*blen < 1) *blen = 1;
  }
  else
  {
    switch(phash_log2(*smax))
    {
    case 0:
      *alen = 1;
      *blen = 1;
    case 1: case 2: case 3: case 4: case 5: case 6: case 7: case 8:
      *alen = (form->perfect == NORMAL_HP) ? *smax : *smax/2;
      *blen = *smax/2;
      break;
    case 9:
    case 10:
    case 11:
    case 12:
    case 13:
    case 14:
    case 15:
    case 16:
    case 17:
      if (form->speed == FAST_HS)
      {
        *alen = *smax/2;
        *blen = *smax/4;
      }
      else if (*smax/4 < USE_SCRAMBLE)
      {
        *alen = ((nkeys <= *smax*0.52) ? *smax/8 : *smax/4);
        *blen = ((nkeys <= *smax*0.52) ? *smax/8 : *smax/4);
      }
      else
      {
        *alen = ((nkeys <= *smax*(5.0/8.0)) ? *smax/8 : 
                 (nkeys <= *smax*(3.0/4.0)) ? *smax/4 : *smax/2);
        *blen = *smax/4;                /* always give the small size a shot */
      }
      break;
    case 18:
      if (form->speed == FAST_HS)
      {
        *alen = *smax/2;
        *blen = *smax/2;
      }
      else
      {
        *alen = *smax/8;                 /* never require the multiword hash */
        *blen = (nkeys <= *smax*(5.0/8.0)) ? *smax/4 : *smax/2;
      }
      break;
    case 19:
    case 20:
      *alen = (nkeys <= *smax*(5.0/8.0)) ? *smax/8 : *smax/2;
      *blen = (nkeys <= *smax*(5.0/8.0)) ? *smax/4 : *smax/2;
      break;
    default:
      *alen = *smax/2;              /* just find a hash as quick as possible */
      *blen = *smax/2;     /* we'll be thrashing virtual memory at this size */
      break;
    }
  }
}

/* 
** Try to find a perfect hash function.  
** Return the successful initializer for the initial hash. 
** Return 0 if no perfect hash could be found.
*/
void findhash(
    bstuff  **tabb,       /* output, tab[] of the perfect hash, length *blen */
    hstuff  **tabh,       /* output, table of keys indexed by hash value */
    ub4      *alen,             /* output, 0..alen-1 is range for a of (a,b) */
    ub4      *blen,             /* output, 0..blen-1 is range for b of (a,b) */
    ub4      *salt,                     /* output, initializes initial hash */
    gencode  *final,                                  /* code for final hash */
    ub4      *scramble,                  /* input, hash = a^scramble[tab[b]] */
    ub4      *smax,                       /* input, scramble[i] in 0..smax-1 */
    key      *keys,                                   /* input, keys to hash */
    ub4       nkeys,                   /* input, number of keys being hashed */
    hashform *form)                                       /* user directives */
{
  ub4 bad_initkey;                       /* how many times did initkey fail? */
  ub4 bad_perfect;                       /* how many times did perfect fail? */
  ub4 trysalt;                        /* trial initializer for initial hash */
  ub4 maxalen;
  qstuff *tabq;    /* table of stuff indexed by queue value, used by augment */

  /* The case of (A,B) supplied by the user is a special case */
  if (form->hashtype == AB_HT)
  {
    hash_ab(tabb, alen, blen, salt, final, 
            scramble, smax, keys, nkeys, form);
    return;
  }

  /* guess initial values for smax, alen and blen */
  *smax = ((ub4)1<<phash_log2(nkeys));
  initalen(alen, blen, smax, nkeys, form);

  scrambleinit(scramble, *smax);

  maxalen = (form->perfect == MINIMAL_HP) ? *smax/2 : *smax;

  /* allocate working memory */
  *tabb = (bstuff *)yasm_xmalloc((size_t)(sizeof(bstuff)*(*blen)));
  tabq  = (qstuff *)yasm_xmalloc(sizeof(qstuff)*(*blen+1));
  *tabh  = (hstuff *)yasm_xmalloc(sizeof(hstuff)*(form->perfect == MINIMAL_HP ? 
                                             nkeys : *smax));

  /* Actually find the perfect hash */
  *salt = 0;
  bad_initkey = 0;
  bad_perfect = 0;
  for (trysalt=1; ; ++trysalt)
  {
    ub4 rslinit;
    /* Try to find distinct (A,B) for all keys */
    
    rslinit = initkey(keys, nkeys, *tabb, *alen, *blen, *smax, trysalt,
                      form, final);

    if (rslinit == 2)
    {      /* initkey actually found a perfect hash, not just distinct (a,b) */
      *salt = 1;
      *blen = 0;
      break;
    }
    else if (rslinit == 0)
    {
      /* didn't find distinct (a,b) */
      if (++bad_initkey >= RETRY_INITKEY)
      {
        /* Try to put more bits in (A,B) to make distinct (A,B) more likely */
        if (*alen < maxalen)
        {
          *alen *= 2;
        } 
        else if (*blen < *smax)
        {
          *blen *= 2;
          free(tabq);
          free(*tabb);
          *tabb  = (bstuff *)yasm_xmalloc((size_t)(sizeof(bstuff)*(*blen)));
          tabq  = (qstuff *)yasm_xmalloc((size_t)(sizeof(qstuff)*(*blen+1)));
        }
        else
        {
          duplicates(*tabb, *blen, keys, form);      /* check for duplicates */
          fprintf(stderr, "fatal error: Cannot perfect hash: cannot find distinct (A,B)\n");
          exit(EXIT_FAILURE);
        }
        bad_initkey = 0;
        bad_perfect = 0;
      }
      continue;                             /* two keys have same (a,b) pair */
    }

    /* Given distinct (A,B) for all keys, build a perfect hash */
    if (!perfect(*tabb, *tabh, tabq, *blen, *smax, scramble, nkeys, form))
    {
      if ((form->hashtype != INT_HT && ++bad_perfect >= RETRY_PERFECT) || 
          (form->hashtype == INT_HT && ++bad_perfect >= RETRY_HEX))
      {
        if (*blen < *smax)
        {
          *blen *= 2;
          free(*tabb);
          free(tabq);
          *tabb  = (bstuff *)yasm_xmalloc((size_t)(sizeof(bstuff)*(*blen)));
          tabq  = (qstuff *)yasm_xmalloc((size_t)(sizeof(qstuff)*(*blen+1)));
          --trysalt;               /* we know this salt got distinct (A,B) */
        }
        else
        {
          fprintf(stderr, "fatal error: Cannot perfect hash: cannot build tab[]\n");
          exit(EXIT_FAILURE);
        }
        bad_perfect = 0;
      }
      continue;
    }
    
    *salt = trysalt;
    break;
  }

  /* free working memory */
  free((void *)tabq);
}

#if 0
/*
------------------------------------------------------------------------------
Input/output type routines
------------------------------------------------------------------------------
*/

/* get the list of keys */
static void getkeys(keys, nkeys, textroot, keyroot, form)
key      **keys;                                         /* list of all keys */
ub4       *nkeys;                                          /* number of keys */
reroot    *textroot;                          /* get space to store key text */
reroot    *keyroot;                                    /* get space for keys */
hashform  *form;                                          /* user directives */
{
  key  *mykey;
  char *mytext;
  mytext = (char *)renew(textroot);
  *keys = 0;
  *nkeys = 0;
  while (fgets(mytext, MAXKEYLEN, stdin))
  {
    mykey = (key *)renew(keyroot);
    if (form->mode == AB_HM)
    {
      sscanf(mytext, "%lx %lx ", &mykey->a_k, &mykey->b_k);
    }
    else if (form->mode == ABDEC_HM)
    {
      sscanf(mytext, "%ld %ld ", &mykey->a_k, &mykey->b_k);
    }
    else if (form->mode == HEX_HM)
    {
      sscanf(mytext, "%lx ", &mykey->hash_k);
    }
    else if (form->mode == DECIMAL_HM)
    {
      sscanf(mytext, "%ld ", &mykey->hash_k);
    }
    else
    {
      mykey->name_k = (ub1 *)mytext;
      mytext = (char *)renew(textroot);
      mykey->len_k  = (ub4)(strlen((char *)mykey->name_k)-1);
    }
    mykey->next_k = *keys;
    *keys = mykey;
    ++*nkeys;
  }
  redel(textroot, mytext);
}

/* make the .c file */
static void make_c(tab, smax, blen, scramble, final, form)
bstuff   *tab;                                         /* table indexed by b */
ub4       smax;                                       /* range of scramble[] */
ub4       blen;                                /* b in 0..blen-1, power of 2 */
ub4      *scramble;                                    /* used in final hash */
gencode  *final;                                  /* code for the final hash */
hashform *form;                                           /* user directives */
{
  ub4   i;
  FILE *f;
  f = fopen("phash.c", "w");
  fprintf(f, "/* table for the mapping for the perfect hash */\n");
  fprintf(f, "#include \"lookupa.h\"\n");
  fprintf(f, "\n");
  if (blen >= USE_SCRAMBLE)
  {
    fprintf(f, "/* A way to make the 1-byte values in tab bigger */\n");
    if (smax > UB2MAXVAL+1)
    {
      fprintf(f, "unsigned long scramble[] = {\n");
      for (i=0; i<=UB1MAXVAL; i+=4)
        fprintf(f, "0x%.8lx, 0x%.8lx, 0x%.8lx, 0x%.8lx,\n",
                scramble[i+0], scramble[i+1], scramble[i+2], scramble[i+3]);
    }
    else
    {
      fprintf(f, "unsigned short scramble[] = {\n");
      for (i=0; i<=UB1MAXVAL; i+=8)
        fprintf(f, 
"0x%.4lx, 0x%.4lx, 0x%.4lx, 0x%.4lx, 0x%.4lx, 0x%.4lx, 0x%.4lx, 0x%.4lx,\n",
                scramble[i+0], scramble[i+1], scramble[i+2], scramble[i+3],
                scramble[i+4], scramble[i+5], scramble[i+6], scramble[i+7]);
    }
    fprintf(f, "};\n");
    fprintf(f, "\n");
  }
  if (blen > 0)
  {
    fprintf(f, "/* small adjustments to _a_ to make values distinct */\n");

    if (smax <= UB1MAXVAL+1 || blen >= USE_SCRAMBLE)
      fprintf(f, "unsigned char tab[] = {\n");
    else
      fprintf(f, "unsigned short tab[] = {\n");

    if (blen < 16)
    {
      for (i=0; i<blen; ++i) fprintf(f, "%3d,", scramble[tab[i].val_b]);
    }
    else if (blen <= 1024)
    {
      for (i=0; i<blen; i+=16)
        fprintf(f, "%ld,%ld,%ld,%ld,%ld,%ld,%ld,%ld,%ld,%ld,%ld,%ld,%ld,%ld,%ld,%ld,\n",
                scramble[tab[i+0].val_b], scramble[tab[i+1].val_b], 
                scramble[tab[i+2].val_b], scramble[tab[i+3].val_b], 
                scramble[tab[i+4].val_b], scramble[tab[i+5].val_b], 
                scramble[tab[i+6].val_b], scramble[tab[i+7].val_b], 
                scramble[tab[i+8].val_b], scramble[tab[i+9].val_b], 
                scramble[tab[i+10].val_b], scramble[tab[i+11].val_b], 
                scramble[tab[i+12].val_b], scramble[tab[i+13].val_b], 
                scramble[tab[i+14].val_b], scramble[tab[i+15].val_b]); 
    }
    else if (blen < USE_SCRAMBLE)
    {
      for (i=0; i<blen; i+=8)
        fprintf(f, "%ld,%ld,%ld,%ld,%ld,%ld,%ld,%ld,\n",
                scramble[tab[i+0].val_b], scramble[tab[i+1].val_b], 
                scramble[tab[i+2].val_b], scramble[tab[i+3].val_b], 
                scramble[tab[i+4].val_b], scramble[tab[i+5].val_b], 
                scramble[tab[i+6].val_b], scramble[tab[i+7].val_b]); 
    }
    else 
    {
      for (i=0; i<blen; i+=16)
        fprintf(f, "%ld,%ld,%ld,%ld,%ld,%ld,%ld,%ld,%ld,%ld,%ld,%ld,%ld,%ld,%ld,%ld,\n",
                tab[i+0].val_b, tab[i+1].val_b, 
                tab[i+2].val_b, tab[i+3].val_b, 
                tab[i+4].val_b, tab[i+5].val_b, 
                tab[i+6].val_b, tab[i+7].val_b, 
                tab[i+8].val_b, tab[i+9].val_b, 
                tab[i+10].val_b, tab[i+11].val_b, 
                tab[i+12].val_b, tab[i+13].val_b, 
                tab[i+14].val_b, tab[i+15].val_b); 
    }
    fprintf(f, "};\n");
    fprintf(f, "\n");
  }
  fprintf(f, "/* The hash function */\n");
  switch(form->mode)
  {
  case NORMAL_HM:
    fprintf(f, "ub4 phash(key, len)\n");
    fprintf(f, "char *key;\n");
    fprintf(f, "int   len;\n");
    break;
  case INLINE_HM:
  case HEX_HM:
  case DECIMAL_HM:
    fprintf(f, "ub4 phash(val)\n");
    fprintf(f, "ub4 val;\n");
    break;
  case AB_HM:
  case ABDEC_HM:
    fprintf(f, "ub4 phash(a,b)\n");
    fprintf(f, "ub4 a;\n");
    fprintf(f, "ub4 b;\n");
    break;
  }
  fprintf(f, "{\n");
  for (i=0; i<final->used; ++i)
    fprintf(f, final->line[i]);
  fprintf(f, "  return rsl;\n");
  fprintf(f, "}\n");
  fprintf(f, "\n");
  fclose(f);
}

/*
------------------------------------------------------------------------------
Read in the keys, find the hash, and write the .c and .h files
------------------------------------------------------------------------------
*/
static void driver(form)
hashform *form;                                           /* user directives */
{
  ub4       nkeys;                                         /* number of keys */
  key      *keys;                                    /* head of list of keys */
  bstuff   *tab;                                       /* table indexed by b */
  ub4       smax;            /* scramble[] values in 0..smax-1, a power of 2 */
  ub4       alen;                            /* a in 0..alen-1, a power of 2 */
  ub4       blen;                            /* b in 0..blen-1, a power of 2 */
  ub4       salt;                       /* a parameter to the hash function */
  reroot   *textroot;                      /* MAXKEYLEN-character text lines */
  reroot   *keyroot;                                       /* source of keys */
  gencode   final;                                    /* code for final hash */
  ub4       i;
  ub4       scramble[SCRAMBLE_LEN];           /* used in final hash function */
  char      buf[10][80];                        /* buffer for generated code */
  char     *buf2[10];                             /* also for generated code */

  /* set up memory sources */
  textroot = remkroot((size_t)MAXKEYLEN);
  keyroot  = remkroot(sizeof(key));

  /* set up code for final hash */
  final.line = buf2;
  final.used = 0;
  final.len  = 10;
  for (i=0; i<10; ++i) final.line[i] = buf[i];

  /* read in the list of keywords */
  getkeys(&keys, &nkeys, textroot, keyroot, form);

  /* find the hash */
  findhash(&tab, &alen, &blen, &salt, &final, 
           scramble, &smax, keys, nkeys, form);

  /* generate the phash.c file */
  make_c(tab, smax, blen, scramble, &final, form);

  /* clean up memory sources */
  refree(textroot);
  refree(keyroot);
  free((void *)tab);
}


/* Interpret arguments and call the driver */
/* See usage_error for the expected arguments */
int main(argc, argv)
int    argc;
char **argv;
{
  int      mode_given = FALSE;
  int      minimal_given = FALSE;
  int      speed_given = FALSE;
  hashform form;
  char    *c;

  /* default behavior */
  form.mode = NORMAL_HM;
  form.hashtype = STRING_HT;
  form.perfect = MINIMAL_HP;
  form.speed = SLOW_HS;

  /* Generate the [minimal] perfect hash */
  driver(&form);

  return EXIT_SUCCESS;
}
#endif
