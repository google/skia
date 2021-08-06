/*
 * Adds a mutex implementation based on c++ mutex to harfbuzz.
 */
#include <mutex>

using hb_mutex_impl_t = std::mutex;
#define HB_MUTEX_IMPL_INIT      UNUSED
#define hb_mutex_impl_init(M)   HB_STMT_START { new (M) hb_mutex_impl_t;  } HB_STMT_END
#define hb_mutex_impl_lock(M)   (M)->lock ()
#define hb_mutex_impl_unlock(M) (M)->unlock ()
#define hb_mutex_impl_finish(M) HB_STMT_START { (M)->~hb_mutex_impl_t(); } HB_STMT_END
