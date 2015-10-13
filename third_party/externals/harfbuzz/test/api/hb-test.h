/*
 * Copyright © 2011  Google, Inc.
 *
 *  This is part of HarfBuzz, a text shaping library.
 *
 * Permission is hereby granted, without written agreement and without
 * license or royalty fees, to use, copy, modify, and distribute this
 * software and its documentation for any purpose, provided that the
 * above copyright notice and the following two paragraphs appear in
 * all copies of this software.
 *
 * IN NO EVENT SHALL THE COPYRIGHT HOLDER BE LIABLE TO ANY PARTY FOR
 * DIRECT, INDIRECT, SPECIAL, INCIDENTAL, OR CONSEQUENTIAL DAMAGES
 * ARISING OUT OF THE USE OF THIS SOFTWARE AND ITS DOCUMENTATION, EVEN
 * IF THE COPYRIGHT HOLDER HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH
 * DAMAGE.
 *
 * THE COPYRIGHT HOLDER SPECIFICALLY DISCLAIMS ANY WARRANTIES, INCLUDING,
 * BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND
 * FITNESS FOR A PARTICULAR PURPOSE.  THE SOFTWARE PROVIDED HEREUNDER IS
 * ON AN "AS IS" BASIS, AND THE COPYRIGHT HOLDER HAS NO OBLIGATION TO
 * PROVIDE MAINTENANCE, SUPPORT, UPDATES, ENHANCEMENTS, OR MODIFICATIONS.
 *
 * Google Author(s): Behdad Esfahbod
 */

#ifndef HB_TEST_H
#define HB_TEST_H

#include <config.h>

#include <hb-glib.h>

#include <stdlib.h>
#include <string.h>
#include <stdio.h>

HB_BEGIN_DECLS

/* Just in case */
#undef G_DISABLE_ASSERT


/* Misc */

/* This is too ugly to be public API, but quite handy. */
#define HB_TAG_CHAR4(s)   (HB_TAG(((const char *) s)[0], \
				  ((const char *) s)[1], \
				  ((const char *) s)[2], \
				  ((const char *) s)[3]))


static inline const char *
srcdir (void)
{
  static const char *s;

  if (!s) {
    s = getenv ("srcdir");

#ifdef SRCDIR
    if (!s || !s[0])
      s = SRCDIR;
#endif

    if (!s || !s[0])
      s = ".";
  }

  return s;
}


/* Helpers */

static inline void
hb_test_init (int *argc, char ***argv)
{
  g_test_init (argc, argv, NULL);
}

static inline int
hb_test_run (void)
{
  return g_test_run ();
}


/* Bugzilla helpers */

static inline void
hb_test_bug (const char *uri_base, unsigned int number)
{
  char *s = g_strdup_printf ("%u", number);

  g_test_bug_base (uri_base);
  g_test_bug (s);

  g_free (s);
}

static inline void
hb_test_bug_freedesktop (unsigned int number)
{
  hb_test_bug ("http://bugs.freedesktop.org/", number);
}

static inline void
hb_test_bug_gnome (unsigned int number)
{
  hb_test_bug ("http://bugzilla.gnome.org/", number);
}

static inline void
hb_test_bug_mozilla (unsigned int number)
{
  hb_test_bug ("http://bugzilla.mozilla.org/", number);
}

static inline void
hb_test_bug_redhat (unsigned int number)
{
  hb_test_bug ("http://bugzilla.redhat.com/", number);
}


/* Wrap glib test functions to simplify.  Should have been in glib already. */

/* Drops the "test_" prefix and converts '_' to '/'.
 * Essentially builds test path from function name. */
static inline char *
hb_test_normalize_path (const char *path)
{
  char *s, *p;

  g_assert (0 == strncmp (path, "test_", 5));
  path += 4;

  s = g_strdup (path);
  for (p = s; *p; p++)
    if (*p == '_')
      *p = '/';

  return s;
}


#if GLIB_CHECK_VERSION(2,25,12)
typedef GTestFunc        hb_test_func_t;
typedef GTestDataFunc    hb_test_data_func_t;
typedef GTestFixtureFunc hb_test_fixture_func_t;
#else
typedef void (*hb_test_func_t)         (void);
typedef void (*hb_test_data_func_t)    (gconstpointer user_data);
typedef void (*hb_test_fixture_func_t) (void);
#endif

#if !GLIB_CHECK_VERSION(2,30,0)
#define g_test_fail() g_error("Test failed")
#endif

static inline void
hb_test_add_func (const char *test_path,
		  hb_test_func_t   test_func)
{
  char *normal_path = hb_test_normalize_path (test_path);
  g_test_add_func (normal_path, test_func);
  g_free (normal_path);
}
#define hb_test_add(Func) hb_test_add_func (#Func, Func)

static inline void
hb_test_add_func_flavor (const char *test_path,
			 const char *flavor,
			 hb_test_func_t   test_func)
{
  char *path = g_strdup_printf ("%s/%s", test_path, flavor);
  hb_test_add_func (path, test_func);
  g_free (path);
}
#define hb_test_add_flavor(Flavor, Func) hb_test_add_func (#Func, Flavor, Func)

static inline void
hb_test_add_data_func (const char          *test_path,
		       gconstpointer        test_data,
		       hb_test_data_func_t  test_func)
{
  char *normal_path = hb_test_normalize_path (test_path);
  g_test_add_data_func (normal_path, test_data, test_func);
  g_free (normal_path);
}
#define hb_test_add_data(UserData, Func) hb_test_add_data_func (#Func, UserData, Func)

static inline void
hb_test_add_data_func_flavor (const char          *test_path,
			      const char          *flavor,
			      gconstpointer        test_data,
			      hb_test_data_func_t  test_func)
{
  char *path = g_strdup_printf ("%s/%s", test_path, flavor);
  hb_test_add_data_func (path, test_data, test_func);
  g_free (path);
}
#define hb_test_add_data_flavor(UserData, Flavor, Func) hb_test_add_data_func_flavor (#Func, Flavor, UserData, Func)


static inline void
hb_test_add_vtable (const char             *test_path,
		    gsize                   data_size,
		    gconstpointer           test_data,
		    hb_test_fixture_func_t  data_setup,
		    hb_test_fixture_func_t  data_test,
		    hb_test_fixture_func_t  data_teardown)
{
  char *normal_path = hb_test_normalize_path (test_path);
  g_test_add_vtable (normal_path, data_size, test_data, data_setup, data_test, data_teardown);
  g_free (normal_path);
}
#define hb_test_add_fixture(FixturePrefix, UserData, Func) \
G_STMT_START { \
  typedef G_PASTE (FixturePrefix, _t) Fixture; \
  void (*add_vtable) (const char*, gsize, gconstpointer, \
		      void (*) (Fixture*, gconstpointer), \
		      void (*) (Fixture*, gconstpointer), \
		      void (*) (Fixture*, gconstpointer)) \
	= (void (*) (const gchar *, gsize, gconstpointer, \
		     void (*) (Fixture*, gconstpointer), \
		     void (*) (Fixture*, gconstpointer), \
		     void (*) (Fixture*, gconstpointer))) hb_test_add_vtable; \
  add_vtable (#Func, sizeof (G_PASTE (FixturePrefix, _t)), UserData, \
	      G_PASTE (FixturePrefix, _init), Func, G_PASTE (FixturePrefix, _finish)); \
} G_STMT_END

static inline void
hb_test_add_vtable_flavor (const char             *test_path,
			   const char             *flavor,
			   gsize                   data_size,
			   gconstpointer           test_data,
			   hb_test_fixture_func_t  data_setup,
			   hb_test_fixture_func_t  data_test,
			   hb_test_fixture_func_t  data_teardown)
{
  char *path = g_strdup_printf ("%s/%s", test_path, flavor);
  hb_test_add_vtable (path, data_size, test_data, data_setup, data_test, data_teardown);
  g_free (path);
}
#define hb_test_add_fixture_flavor(FixturePrefix, UserData, Flavor, Func) \
G_STMT_START { \
  typedef G_PASTE (FixturePrefix, _t) Fixture; \
  void (*add_vtable) (const char*, const char *, gsize, gconstpointer, \
		      void (*) (Fixture*, gconstpointer), \
		      void (*) (Fixture*, gconstpointer), \
		      void (*) (Fixture*, gconstpointer)) \
	= (void (*) (const gchar *, const char *, gsize, gconstpointer, \
		     void (*) (Fixture*, gconstpointer), \
		     void (*) (Fixture*, gconstpointer), \
		     void (*) (Fixture*, gconstpointer))) hb_test_add_vtable_flavor; \
  add_vtable (#Func, Flavor, sizeof (G_PASTE (FixturePrefix, _t)), UserData, \
	      G_PASTE (FixturePrefix, _init), Func, G_PASTE (FixturePrefix, _finish)); \
} G_STMT_END


HB_END_DECLS

#endif /* HB_TEST_H */
