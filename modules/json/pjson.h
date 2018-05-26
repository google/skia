// File: pjson.h - written by Rich Geldreich 2012 - License: Unlicense http://unlicense.org/
#ifndef PURPLE_JSON_H
#define PURPLE_JSON_H

#ifdef WIN32
#pragma once
#endif

#include <string>
#include <vector>
#include <limits>
#include <assert.h>
#include <stddef.h>

// ---- Macros

#define PJSON_ASSERT assert
#ifdef _MSC_VER
#define PJSON_FORCEINLINE __forceinline
#else
#define PJSON_FORCEINLINE __attribute__((always_inline))
#endif

#define PJSON_PARSE_STATS 0

#define PJSON_DEFAULT_MIN_CHUNK_SIZE 4096
#define PJSON_MAX_CHUNK_GROW_SIZE 8*1024*1024
#define PJSON_DEFAULT_MAX_BYTES_TO_PRESERVE_ACROSS_RESETS 16*1024*1024

#define PJSON_MIN(a, b) (((a) < (b)) ? (a) : (b))
#define PJSON_MAX(a, b) (((a) < (b)) ? (b) : (a))

namespace pjson
{
   // ---- Types
   typedef unsigned char uint8;
   typedef unsigned int uint;
   typedef signed int int32;
   typedef unsigned int uint32;

   typedef int64_t int64;
   typedef uint64_t uint64;

   class document;
   class value_variant;
   struct value_variant_data;
   struct key_value_t;
   
   typedef std::vector<char> char_vec_t;
   typedef std::string string_t;

   // Memory allocation

   inline void* pjson_malloc(size_t size) { return malloc(size); }
   inline void* pjson_realloc(void* p, size_t size) { return realloc(p, size); }
   inline void pjson_free(void* p) { free(p); }
   
   // Misc. Helpers
   template<typename T> inline void swap(T& l, T& r) { T temp(l); l = r; r = temp; }

   inline bool is_power_of_2(uint32 x) { return x && ((x & (x - 1U)) == 0U); }
   inline uint32 next_pow2(uint32 val) { val--; val |= val >> 16; val |= val >> 8; val |= val >> 4; val |= val >> 2; val |= val >> 1; return val + 1; }

#ifdef _MSC_VER
   inline int pjson_stricmp(const char* p, const char* q) { return _stricmp(p, q); }
#else
   inline int pjson_stricmp(const char* p, const char* q) { return strcasecmp(p, q); }
#endif

   // ---- Global Arrays

   // This template utilizes the One Definition Rule to create global arrays in a header.
   template<typename unused=void>
   struct globals_struct
   {
      static const uint8 s_str_serialize_flags[256];
      static const double s_pow10_table[63];
      static const uint8 s_parse_flags[256];
   };
   typedef globals_struct<> globals;
   
   template<typename unused>
   const uint8 globals_struct<unused>::s_str_serialize_flags[256] = 
   {
   // 0    1    2    3    4    5    6    7      8    9    A    B    C    D    E    F
      1,   1,   1,   1,   1,   1,   1,   1,     1,   1,   1,   1,   1,   1,   1,   1, // 0
      1,   1,   1,   1,   1,   1,   1,   1,     1,   1,   1,   1,   1,   1,   1,   1, // 1
      0,   0,   1,   0,   0,   0,   0,   0,     0,   0,   0,   0,   0,   0,   0,   0, // 2
      0,   0,   0,   0,   0,   0,   0,   0,     0,   0,   0,   0,   0,   0,   0,   0, // 1
      0,   0,   0,   0,   0,   0,   0,   0,     0,   0,   0,   0,   0,   0,   0,   0, // 4
      0,   0,   0,   0,   0,   0,   0,   0,     0,   0,   0,   0,   1,   0,   0,   0, // 5
      0,   0,   0,   0,   0,   0,   0,   0,     0,   0,   0,   0,   0,   0,   0,   0, // 6
      0,   0,   0,   0,   0,   0,   0,   0,     0,   0,   0,   0,   0,   0,   0,   0, // 7
   // 128-255
      0,0,0,0,0,0,0,0,  0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,  0,0,0,0,0,0,0,0, 
      0,0,0,0,0,0,0,0,  0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,  0,0,0,0,0,0,0,0, 
      0,0,0,0,0,0,0,0,  0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,  0,0,0,0,0,0,0,0, 
      0,0,0,0,0,0,0,0,  0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,  0,0,0,0,0,0,0,0
   };

   template<typename unused>
   const double globals_struct<unused>::s_pow10_table[63] = 
   {
      1.e-031,1.e-030,1.e-029,1.e-028,1.e-027,1.e-026,1.e-025,1.e-024,1.e-023,1.e-022,1.e-021,1.e-020,1.e-019,1.e-018,1.e-017,1.e-016,
      1.e-015,1.e-014,1.e-013,1.e-012,1.e-011,1.e-010,1.e-009,1.e-008,1.e-007,1.e-006,1.e-005,1.e-004,1.e-003,1.e-002,1.e-001,1.e+000,
      1.e+001,1.e+002,1.e+003,1.e+004,1.e+005,1.e+006,1.e+007,1.e+008,1.e+009,1.e+010,1.e+011,1.e+012,1.e+013,1.e+014,1.e+015,1.e+016,
      1.e+017,1.e+018,1.e+019,1.e+020,1.e+021,1.e+022,1.e+023,1.e+024,1.e+025,1.e+026,1.e+027,1.e+028,1.e+029,1.e+030,1.e+031 
   };

   // bit 0 (1) - set if: \0 cr lf " \
   // bit 1 (2) - set if: \0 cr lf
   // bit 2 (4) - set if: whitespace
   // bit 3 (8) - set if: 0-9
   // bit 4 (0x10) - set if: 0-9 e E .
   template<typename unused>
   const uint8 globals_struct<unused>::s_parse_flags[256] = 
   {
   // 0    1    2    3    4    5    6    7      8    9    A    B    C    D    E    F
      7,   4,   4,   4,   4,   4,   4,   4,     4,   4,   7,   4,   4,   7,   4,   4, // 0
      4,   4,   4,   4,   4,   4,   4,   4,     4,   4,   4,   4,   4,   4,   4,   4, // 1
      4,   0,   1,   0,   0,   0,   0,   0,     0,   0,   0,   0,   0,   0,   0x10,0, // 2
      0x18,0x18,0x18,0x18,0x18,0x18,0x18,0x18,  0x18,0x18,0,   0,   0,   0,   0,   0, // 3
      0,   0,   0,   0,   0,   0x10,0,   0,     0,   0,   0,   0,   0,   0,   0,   0, // 4
      0,   0,   0,   0,   0,   0,   0,   0,     0,   0,   0,   0,   1,   0,   0,   0, // 5
      0,   0,   0,   0,   0,   0x10,0,   0,     0,   0,   0,   0,   0,   0,   0,   0, // 6
      0,   0,   0,   0,   0,   0,   0,   0,     0,   0,   0,   0,   0,   0,   0,   0, // 7

   // 128-255
      0,0,0,0,0,0,0,0,  0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,  0,0,0,0,0,0,0,0, 
      0,0,0,0,0,0,0,0,  0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,  0,0,0,0,0,0,0,0, 
      0,0,0,0,0,0,0,0,  0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,  0,0,0,0,0,0,0,0, 
      0,0,0,0,0,0,0,0,  0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,  0,0,0,0,0,0,0,0
   };

   // ---- Pool Allocator

   struct pool_allocator
   {
      inline pool_allocator(uint initial_size = 0, uint min_chunk_size = PJSON_DEFAULT_MIN_CHUNK_SIZE, size_t max_bytes_to_preserve_across_resets = PJSON_DEFAULT_MAX_BYTES_TO_PRESERVE_ACROSS_RESETS) : 
         m_pActive_chunks(NULL), 
         m_pFree_chunks(NULL),
         m_total_free_bytes(0),
         m_initial_size(initial_size), 
         m_min_chunk_size(min_chunk_size),
         m_max_to_preserve_across_resets(max_bytes_to_preserve_across_resets),
         m_cur_grow_size(min_chunk_size)
      { 
         if (initial_size)
         {
            m_pActive_chunks = static_cast<chunk*>(pjson_malloc(sizeof(chunk) + initial_size));
            m_pActive_chunks->m_pNext = NULL;
            m_pActive_chunks->m_ofs = 0;
            m_pActive_chunks->m_size = initial_size;
         }
      }

      inline ~pool_allocator()
      {
         clear();
      }

      // Release all active/free chunks
      void clear()
      {
         free_chunk_chain(m_pActive_chunks);
         m_pActive_chunks = NULL;
         
         free_chunk_chain(m_pFree_chunks);
         m_pFree_chunks = NULL;

         m_total_free_bytes = 0;
         
         m_cur_grow_size = m_min_chunk_size;
      }

      inline size_t get_total_free_bytes() const { return m_total_free_bytes; }
      
      inline uint get_min_chunk_size() const { return m_min_chunk_size; }
      inline size_t get_max_bytes_to_preserve_across_resets() const { return m_max_to_preserve_across_resets; }

      inline void set_min_chunk_size(uint s) { m_min_chunk_size = m_cur_grow_size = s; }
      inline void set_max_bytes_to_preserve_across_resets(size_t s) { m_max_to_preserve_across_resets = s; }

      inline uint get_cur_grow_size() const { return m_cur_grow_size; }
      
      inline void* Alloc(size_t size)
      {
         size = (size + 3) & ~3;
         if ((!m_pActive_chunks) || ((m_pActive_chunks->m_size - m_pActive_chunks->m_ofs) < size))
         {
            chunk* pNew_chunk = m_pFree_chunks;
            if ((pNew_chunk) && (pNew_chunk->m_size >= size))
            {
               PJSON_ASSERT(m_total_free_bytes >= pNew_chunk->m_size);
               m_total_free_bytes -= pNew_chunk->m_size;
               m_pFree_chunks = pNew_chunk->m_pNext;
               PJSON_ASSERT(!pNew_chunk->m_ofs);
            }
            else
            {
               size_t alloc_size = PJSON_MAX(size, m_cur_grow_size);
               m_cur_grow_size = PJSON_MIN(m_cur_grow_size * 2, PJSON_MAX_CHUNK_GROW_SIZE);

               pNew_chunk = static_cast<chunk*>(pjson_malloc(sizeof(chunk) + alloc_size));
               pNew_chunk->m_size = alloc_size;
               pNew_chunk->m_ofs = 0;
            }

            pNew_chunk->m_pNext = m_pActive_chunks;
            m_pActive_chunks = pNew_chunk;
         }
         void* pRet = (uint8*)m_pActive_chunks + sizeof(chunk) + m_pActive_chunks->m_ofs;
         m_pActive_chunks->m_ofs += size;
         PJSON_ASSERT(m_pActive_chunks->m_ofs <= m_pActive_chunks->m_size);
         return pRet;
      }

      inline void* Realloc(void* p, size_t new_size, size_t cur_size)
      {
         if (!p)
            return Alloc(new_size);
         
         new_size = (new_size + 3) & ~3;
         cur_size = (cur_size + 3) & ~3;
         if (new_size == cur_size)
            return p;

         uint8* pTop = (uint8*)m_pActive_chunks + sizeof(chunk) + m_pActive_chunks->m_ofs;
         if ((static_cast<uint8*>(p) + cur_size) == pTop)
         {
            if (new_size > cur_size)
            {
               size_t bytes_needed = new_size - cur_size;
               if ((m_pActive_chunks->m_size - m_pActive_chunks->m_ofs) >= bytes_needed)
               {
                  m_pActive_chunks->m_ofs += bytes_needed;
                  PJSON_ASSERT(m_pActive_chunks->m_ofs <= m_pActive_chunks->m_size);
                  return p;
               }
            }
            else 
            {
               PJSON_ASSERT(m_pActive_chunks->m_ofs >= (cur_size - new_size));
               m_pActive_chunks->m_ofs -= (cur_size - new_size);
               return new_size ? p : NULL;
            }
         }

         if (!new_size)
            return NULL;

         void* pNew_block = Alloc(new_size);
         memcpy(pNew_block, p, cur_size);
         return pNew_block;
      }

      // Move all active chunks to the free chunk list, then free any chunks if we're over the preserve limit.
      inline void reset()
      {
         if (!m_pActive_chunks)
            return;
         
         chunk* pCur_active_tail = m_pActive_chunks;
         size_t total_allocated_bytes = 0;
         for ( ; ; )
         {
            total_allocated_bytes += pCur_active_tail->m_size;
            pCur_active_tail->m_ofs = 0;
            if (!pCur_active_tail->m_pNext)
               break;
            pCur_active_tail = pCur_active_tail->m_pNext;
         }
         pCur_active_tail->m_pNext = m_pFree_chunks;
                           
         m_pFree_chunks = m_pActive_chunks;
         m_pActive_chunks = NULL;

         m_total_free_bytes += total_allocated_bytes;
         while (m_total_free_bytes > m_max_to_preserve_across_resets)
         {
            PJSON_ASSERT(m_pFree_chunks);
            chunk* pNext_free = m_pFree_chunks->m_pNext;
            PJSON_ASSERT(m_total_free_bytes >= m_pFree_chunks->m_size);
            m_total_free_bytes -= m_pFree_chunks->m_size;
            pjson_free(m_pFree_chunks);
            m_pFree_chunks = pNext_free;
         }

         m_cur_grow_size = m_min_chunk_size;
      }

      struct stats_t
      {
         size_t m_total_allocated;

         uint m_num_active_chunks;
         size_t m_num_active_bytes_allocated;
         size_t m_num_active_bytes_avail;
         size_t m_max_active_chunk_size;

         uint m_num_free_chunks;
         size_t m_num_free_chunk_bytes_avail;
         size_t m_max_free_chunk_size;
      };

      inline void get_stats(stats_t& s) const
      {
         memset(&s, 0, sizeof(s));

         chunk* pChunk = m_pActive_chunks;
         while (pChunk)
         {
            s.m_num_active_chunks++;
            s.m_total_allocated += pChunk->m_size;
            s.m_num_active_bytes_allocated += pChunk->m_ofs;
            s.m_num_active_bytes_avail += (pChunk->m_size - pChunk->m_ofs);
            s.m_max_active_chunk_size = PJSON_MAX(s.m_max_active_chunk_size, pChunk->m_size);
            pChunk = pChunk->m_pNext;
         }

         pChunk = m_pFree_chunks;
         while (pChunk)
         {
            s.m_num_free_chunks++;
            s.m_total_allocated += pChunk->m_size;
            PJSON_ASSERT(!pChunk->m_ofs);
            s.m_num_free_chunk_bytes_avail += pChunk->m_size;
            s.m_max_free_chunk_size = PJSON_MAX(s.m_max_free_chunk_size, pChunk->m_size);
            pChunk = pChunk->m_pNext;
         }

         PJSON_ASSERT(s.m_num_free_chunk_bytes_avail == m_total_free_bytes);
      }

   private:
      pool_allocator(const pool_allocator&);
      pool_allocator& operator= (const pool_allocator&);

      struct chunk
      {
         chunk* m_pNext;
         size_t m_size;
         size_t m_ofs;
      };

      chunk* m_pActive_chunks;
      chunk* m_pFree_chunks;
      size_t m_total_free_bytes;

      uint m_initial_size;
      uint m_min_chunk_size;
      size_t m_max_to_preserve_across_resets;
      
      uint m_cur_grow_size;

      inline void free_chunk_chain(chunk* pChunk)
      {
         while (pChunk)
         {
            chunk* pNext_chunk = pChunk->m_pNext;
            pjson_free(pChunk);
            pChunk = pNext_chunk;
         }
      }
   };

   // ---- Simple vector (growable array)

   template<typename T>
   struct simple_vector_default_copy_construction_policy
   {
       inline static void copy_construct(void *pDst, const T& init, pool_allocator& alloc) { (void)alloc; new (pDst) T(init); }
       inline static void assign(void *pDst, const T& src, pool_allocator& alloc) { (void)alloc; *static_cast<T*>(pDst) = src; }
   };

   template<typename T>
   struct simple_vector_allocator_copy_construction_policy
   {
      inline static void copy_construct(void *pDst, const T& init, pool_allocator& alloc) { (void)alloc; new (pDst) T(init, alloc); }
      inline static void assign(void *pDst, const T& src, pool_allocator& alloc) { static_cast<T*>(pDst)->assign(src, alloc); }
   };

   template <typename T> inline T* construct(T* p) { return new (static_cast<void*>(p)) T; }
   template <typename T> inline void construct_array(T* p, uint n) { T* q = p + n; for ( ; p != q; ++p) new (static_cast<void*>(p)) T; }
   
   template<typename T>
   struct elemental_vector
   {
      typedef T               value_type;
      typedef T&              reference;
      typedef const T&        const_reference;
      typedef T*              pointer;
      typedef const T*        const_pointer;

      T*       m_p;
      uint32   m_size;
   };

   template<typename T, bool UseConstructor, typename ConstructionPolicy = simple_vector_default_copy_construction_policy<T> >
   struct simple_vector : elemental_vector<T>
   {
      inline simple_vector() { construct(); }
      inline simple_vector(const simple_vector& other, pool_allocator& alloc) { construct(other, alloc); }
            
      // Manual constructor methods
      inline void construct() { this->m_p = NULL; this->m_size = 0; }
      inline void construct(uint size, pool_allocator& alloc) { construct(); enlarge(size, alloc, false); }
      inline void construct(const T* p, uint size, pool_allocator& alloc)
      { 
         this->m_size = size;
         this->m_p = NULL;
         if (size)
         {
            uint num_bytes = sizeof(T) * size;
            this->m_p = static_cast<T*>(alloc.Alloc(num_bytes));
            if (UseConstructor)
            {
               T* pDst = this->m_p;
               T* pDst_end = pDst + size;
               const T* pSrc = p;
               while (pDst != pDst_end)
                  ConstructionPolicy::copy_construct(pDst++, *pSrc++, alloc);
            }
            else
               memcpy(this->m_p, p, num_bytes);
         }
      }
      inline void construct(const simple_vector& other, pool_allocator& alloc) 
      { 
         construct(other.m_p, other.m_size, alloc); 
      }
      
      inline uint size() const { return this->m_size; }
      inline uint size_in_bytes() const { return this->m_size * sizeof(T); }
      
      inline const T& operator[] (uint i) const  { PJSON_ASSERT(i < this->m_size); return this->m_p[i]; }
      inline       T& operator[] (uint i)        { PJSON_ASSERT(i < this->m_size); return this->m_p[i]; }
            
      inline const T* get_ptr() const   { return this->m_p; }
      inline       T* get_ptr()         { return this->m_p; }

      inline const T* get_ptr(const T* pDef) const   { return this->m_p ? this->m_p : pDef; }
      inline       T* get_ptr(T* pDef)               { return this->m_p ? this->m_p : pDef; }

      inline void clear() { this->m_p = NULL; this->m_size = 0; }
                  
      inline void resize(uint new_size, pool_allocator& alloc)
      {
         if (new_size > this->m_size)
         {
            grow(new_size, alloc);
            
            if (UseConstructor)
               construct_array(this->m_p + this->m_size, new_size - this->m_size);
         }
         
         this->m_size = new_size;
      }

      inline void shrink(uint new_size)
      {
         this->m_size = new_size;
      }
         
      inline T* enlarge_no_construct(uint n, pool_allocator& alloc) 
      { 
         PJSON_ASSERT(n);
         uint cur_size = this->m_size, new_size = this->m_size + n;
         grow(new_size, alloc);
         this->m_size = new_size;
         return this->m_p + cur_size; 
      }

      inline T* enlarge(uint n, pool_allocator& alloc)
      { 
         T* p = enlarge_no_construct(n, alloc);
         if (UseConstructor)
            construct_array(p, n);
         return p;
      }

      inline void push_back(const T& obj, pool_allocator& alloc)
      {
         PJSON_ASSERT(!this->m_p || (&obj < this->m_p) || (&obj >= (this->m_p + this->m_size)));
         grow(this->m_size + 1, alloc);
         if (UseConstructor)
            ConstructionPolicy::copy_construct(this->m_p + this->m_size, obj, alloc);
         else
            memcpy(this->m_p + this->m_size, &obj, sizeof(T));
         this->m_size++;
      }

      inline void push_back(const T* p, uint n, pool_allocator& alloc)
      {
         PJSON_ASSERT(!this->m_p || ((p + n) <= this->m_p) || (p >= (this->m_p + this->m_size)));
         T* pDst = enlarge_no_construct(n, alloc);
         if (UseConstructor)
         {
            T* pDst_end = pDst + n;
            const T* pSrc = p;
            while (pDst != pDst_end)
               ConstructionPolicy::copy_construct(pDst, *pSrc++, alloc);
         }
         else
            memcpy(pDst, p, sizeof(T) * n);
      }

      inline void assign(const T* p, uint n, pool_allocator& alloc)
      {
         PJSON_ASSERT(!this->m_p || ((p + n) <= this->m_p) || (p >= (this->m_p + this->m_size)));
         
         const uint num_to_assign = PJSON_MIN(this->m_size, n);
         if (num_to_assign)
         {
            if (UseConstructor)
            {
               for (uint i = 0; i < num_to_assign; ++i)
                  ConstructionPolicy::assign(&this->m_p[i], p[i], alloc);
            }
            else
               memcpy(this->m_p, p, sizeof(T) * num_to_assign);
         }

         if (n > this->m_size)
            push_back(p + num_to_assign, n - num_to_assign, alloc);
         else
            shrink(n);
      }

      inline void assign(const simple_vector& other, pool_allocator& alloc)
      { 
         assign(other.m_p, other.m_size, alloc);
      }
      
      inline void erase(uint start, uint n)
      {
         PJSON_ASSERT((start + n) <= this->m_size);
         if ((!n) || ((start + n) > this->m_size))
            return;

         const uint num_to_move = this->m_size - (start + n);

         T* pDst = this->m_p + start;

         memmove(pDst, this->m_p + start + n, num_to_move * sizeof(T));

         this->m_size -= n;
      }

      inline void swap(simple_vector& other) 
      { 
         pjson::swap(this->m_p, other.m_p); 
         pjson::swap(this->m_size, other.m_size); 
      }
      
      inline void grow(uint new_size, pool_allocator& alloc)
      {
         if (new_size > this->m_size)
            this->m_p = static_cast<T*>(alloc.Realloc(this->m_p, sizeof(T) * new_size, this->m_size * sizeof(T)));
      }
   };
         
   enum json_value_type_t
   {
      cJSONValueTypeNull = 0,
      cJSONValueTypeBool,
      cJSONValueTypeInt,
      cJSONValueTypeDouble,

      // All types that follow require storage. Do not change the relative order of these types.
      cJSONValueTypeString,
      cJSONValueTypeArray,
      cJSONValueTypeObject,
   };
   
   // ---- struct value_variant_data

   typedef simple_vector<char, false> string_vec_t;
   typedef simple_vector<key_value_t, true, simple_vector_allocator_copy_construction_policy<key_value_t> > key_value_vec_t;
   typedef simple_vector<value_variant, true, simple_vector_allocator_copy_construction_policy<value_variant> > value_variant_vec_t;
         
   #pragma pack(push, 4)
   struct value_variant_data
   {
      union json_value_data_t
      {
         elemental_vector<key_value_vec_t::value_type> m_object;
         elemental_vector<value_variant_vec_t::value_type> m_array;
         elemental_vector<string_vec_t::value_type> m_string;
         int64 m_nVal;
         double m_flVal;
      };

      json_value_data_t m_data;
      json_value_type_t m_type;

      inline const string_vec_t& get_string() const  { return (const string_vec_t&)m_data.m_string; }
      inline       string_vec_t& get_string()        { return (string_vec_t&)m_data.m_string; }
      inline const char* get_string_ptr() const { return get_string().get_ptr(""); }

      inline const value_variant_vec_t& get_array() const    { return (const value_variant_vec_t&)m_data.m_array; }
      inline       value_variant_vec_t& get_array()          { return (value_variant_vec_t&)m_data.m_array; }

      inline const key_value_vec_t& get_object() const  { return (const key_value_vec_t&)m_data.m_object; }
      inline       key_value_vec_t& get_object()        { return (key_value_vec_t&)m_data.m_object; }
   };
         
   // ---- struct key_value_t

   struct key_value_t
   {
      inline key_value_t() { }
      inline key_value_t(const key_value_t& other, pool_allocator& alloc);
      
      inline void assign(const key_value_t& src, pool_allocator& alloc);
      
      inline const string_vec_t& get_key() const  { return m_key; }
      inline       string_vec_t& get_key()        { return m_key; }

      inline const value_variant& get_value() const  { return (const value_variant&)m_value_data; }
      inline       value_variant& get_value()        { return (value_variant&)m_value_data; }

      string_vec_t m_key;
      value_variant_data m_value_data;
   };
   #pragma pack(pop)

   // ---- class char_vector_print_helper
   
   class char_vector_print_helper
   {
      char_vector_print_helper(const char_vector_print_helper&);
      char_vector_print_helper& operator= (const char_vector_print_helper&);

   public:
      inline char_vector_print_helper(char_vec_t& buf) : m_buf(buf) { }

      inline void resize(size_t new_size) { m_buf.resize(new_size); }
      inline size_t size() const { return m_buf.size(); }
      inline char* get_ptr() const { return &m_buf[0]; }
      
      inline const char_vec_t& get_buf() const  { return m_buf; }
      inline       char_vec_t& get_buf()        { return m_buf; }

      inline void puts(const char* pStr, size_t l) { m_buf.insert(m_buf.end(), pStr, pStr + l); }
      inline void print_tabs(size_t n) { m_buf.insert(m_buf.end(), n, '\t'); }
      inline void print_char(char c) { m_buf.push_back(c); }

      void print_escaped(const string_vec_t& str)
      {
         const char* pStr = str.m_p;
         uint len = str.m_size; (void)len;
      
         static const char* s_to_hex = "0123456789abcdef";
         print_char('\"');
         while (*pStr)
         {
            uint8 c = *pStr++;
            if ((c >= ' ') && (c != '\"') && (c != '\\')) 
               print_char(c);
            else
            {
               print_char('\\');
               switch (c)
               {
                  case '\b':	print_char('b'); break;
                  case '\r':	print_char('r'); break;
                  case '\t':	print_char('t'); break;
                  case '\f':	print_char('f'); break;
                  case '\n':	print_char('n'); break;
                  case '\\':	print_char('\\'); break;
                  case '\"':	print_char('\"'); break;
                  default: puts("u00", 3); print_char(s_to_hex[c >> 4]); print_char(s_to_hex[c & 0xF]); break;
               }
            }
         }
         print_char('\"');
      }
           
   private:
      char_vec_t& m_buf;
   };

   // ---- class char_buf_print_helper

   class char_buf_print_helper
   {
      char_buf_print_helper(const char_buf_print_helper&);
      char_buf_print_helper& operator= (const char_buf_print_helper&);

   public:
      inline char_buf_print_helper(char* pBuf, size_t buf_size) : m_pDst(pBuf), m_pStart(pBuf), m_pEnd(pBuf + buf_size) { }

      inline void resize(size_t new_size) { PJSON_ASSERT(new_size <= (size_t)(m_pEnd - m_pStart)); m_pDst = m_pStart + new_size; }
      inline size_t size() const { return m_pDst - m_pStart; }
      inline char* get_ptr() const { return m_pStart; }
      
      inline void puts(const char* pStr, size_t l) { memcpy(m_pDst, pStr, l = PJSON_MIN(l, (size_t)(m_pEnd - m_pDst))); m_pDst += l; }
      inline void print_tabs(size_t n) { n = PJSON_MIN(n, (size_t)(m_pEnd - m_pDst)); memset(m_pDst, '\t', n); m_pDst += n; }
      inline void print_char(char c) { if (m_pDst < m_pEnd) *m_pDst++ = c; }

      void print_escaped(const string_vec_t& str)
      {
         static const char* s_to_hex = "0123456789abcdef";

         const char* pStr = str.m_p;
         char* pDst = m_pDst;
         char* pEnd = m_pEnd;
         uint len = str.m_size;
         
         // If len!=0, it includes the terminating null, so this expression is conservative.
         if (static_cast<size_t>(pEnd - pDst) < (len + 2)) { m_pDst = pEnd; return; }
                                    
         *pDst++ = '\"';

         uint8 c = 0; if (pStr) c = pStr[0];
         while (!globals::s_str_serialize_flags[c])
         {
            pDst[0] = c; c = pStr[1]; if (globals::s_str_serialize_flags[c]) { ++pStr; ++pDst; break; }
            pDst[1] = c; c = pStr[2]; if (globals::s_str_serialize_flags[c]) { pStr += 2; pDst += 2; break; }
            pDst[2] = c; c = pStr[3]; if (globals::s_str_serialize_flags[c]) { pStr += 3; pDst += 3; break; }
            pDst[3] = c; c = pStr[4]; pStr += 4; pDst += 4;
         }
         
         while (c)
         {
            if ((pEnd - pDst) < 7)
            { 
               m_pDst = pEnd; 
               return; 
            }
            if (!globals::s_str_serialize_flags[c])
               *pDst++ = c;
            else
            {
               pDst[0] = '\\';
               switch (c)
               {
                  case '\b':	pDst[1] = 'b'; break; 
                  case '\r':	pDst[1] = 'r'; break;
                  case '\t':	pDst[1] = 't'; break;
                  case '\f':	pDst[1] = 'f'; break;
                  case '\n':	pDst[1] = 'n'; break;
                  case '\\':	pDst[1] = '\\'; break;
                  case '\"':	pDst[1] = '\"'; break;
                  default: pDst[1] = 'u'; pDst[2] = '0'; pDst[3] = '0'; pDst[4] = s_to_hex[c >> 4]; pDst[5] = s_to_hex[c & 0xF]; pDst += 3; break;
               }
               pDst += 2;
            }
            c = *pStr++;
         }

         *pDst++ = '\"';
         PJSON_ASSERT(pDst <= pEnd);
         m_pDst = pDst;
      }

   private:
      char* m_pDst, *m_pStart, *m_pEnd;
   };

   // ---- class serialize_helper

   template<typename T>
   class serialize_helper : public T
   {
   public:
      typedef T base;

      template<typename I> inline serialize_helper(I& init) : T(init) { }
      template<typename I, typename J> inline serialize_helper(I& init1, J& init2) : T(init1, init2) { }

      inline void puts(const char* pStr) { T::puts(pStr, strlen(pStr)); }
      inline void puts(const char* pStr, size_t l) { T::puts(pStr, l); }
   };

   // ---- class value_variant
   
   #pragma pack(push, 4)
   class value_variant : public value_variant_data
   {
      friend document;
      friend key_value_t;

      value_variant(const value_variant&);
      value_variant& operator= (const value_variant&);

   public:
      inline value_variant() { m_type = cJSONValueTypeNull; m_data.m_nVal = 0; }
      inline value_variant(bool val) { m_type = cJSONValueTypeBool; m_data.m_nVal = val; }
      inline value_variant(int32 nVal) { m_type = cJSONValueTypeInt; m_data.m_nVal = nVal; }
      inline value_variant(uint32 nVal) { m_type = cJSONValueTypeInt; m_data.m_nVal = nVal; }
      inline value_variant(int64 nVal) { m_type = cJSONValueTypeInt; m_data.m_nVal = nVal; }
      inline value_variant(double flVal) { m_type = cJSONValueTypeDouble; m_data.m_flVal = flVal; }
      
      inline value_variant(const char* pStr, pool_allocator& alloc)
      { 
         m_type = cJSONValueTypeString;
         if (!pStr) pStr = "";
         get_string().construct(pStr, static_cast<uint>(strlen(pStr)) + 1, alloc); 
      }

      inline value_variant(json_value_type_t type)
      {
         construct(type);
      }

      inline value_variant(const value_variant& other, pool_allocator& alloc)
      {
         construct(other, alloc);
      }

      inline value_variant &assign(const value_variant& rhs, pool_allocator& alloc)
      {
         if (this == &rhs)
            return *this;
         if ((m_type >= cJSONValueTypeString) && (m_type == rhs.m_type))
         {
            if (is_string())
               get_string().assign(rhs.get_string(), alloc);
            else if (is_object())
               get_object().assign(rhs.get_object(), alloc);
            else
               get_array().assign(rhs.get_array(), alloc);
         }
         else
         {
            construct(rhs, alloc);
         }
         return *this;
      }

      inline json_value_type_t get_type() const { return m_type; }

      inline bool is_null() const { return m_type == cJSONValueTypeNull; }
      inline bool is_valid() const { return m_type != cJSONValueTypeNull; }
      inline bool is_bool() const { return m_type == cJSONValueTypeBool; }
      inline bool is_int() const { return m_type == cJSONValueTypeInt; }
      inline bool is_double() const { return m_type == cJSONValueTypeDouble; }
      inline bool is_numeric() const { return (m_type == cJSONValueTypeInt) || (m_type == cJSONValueTypeDouble); }
      inline bool is_string() const { return m_type == cJSONValueTypeString; }
      inline bool is_object_or_array() const { return m_type >= cJSONValueTypeArray; }
      inline bool is_object() const { return m_type == cJSONValueTypeObject; }
      inline bool is_array() const { return m_type == cJSONValueTypeArray; }
            
      inline void clear() { set_to_null(); }

      inline void assume_ownership(value_variant& src_val) { set_to_null(); swap(src_val); }
      inline void release_ownership(value_variant& dst_value) { dst_value.set_to_null(); dst_value.swap(*this); }

      inline value_variant& set_to_object() { construct(cJSONValueTypeObject); return *this; }
      inline value_variant& set_to_array() { construct(cJSONValueTypeArray); return *this; }
      inline value_variant& set_to_node(bool is_object) { construct(is_object ? cJSONValueTypeObject : cJSONValueTypeArray); return *this; }

      inline value_variant& set_to_null() { m_data.m_nVal = 0; m_type = cJSONValueTypeNull; return *this; }
      inline value_variant& set(bool val) { m_data.m_nVal = val; m_type = cJSONValueTypeBool; return *this; }
      inline value_variant& set(int32 nVal) { m_data.m_nVal = nVal; m_type = cJSONValueTypeInt; return *this; }
      inline value_variant& set(int64 nVal) { m_data.m_nVal = nVal; m_type = cJSONValueTypeInt; return *this; }
      inline value_variant& set(uint32 nVal) { set(static_cast<int64>(nVal)); return *this; }
      inline value_variant& set(double flVal) { m_data.m_flVal = flVal; m_type = cJSONValueTypeDouble; return *this; }
      
      inline value_variant& set(const char* pStr, pool_allocator& alloc) 
      { 
         if (!pStr) pStr = "";
         uint l = static_cast<uint>(strlen(pStr)) + 1;
         if (!is_string())
         {
            m_type = cJSONValueTypeString;
            get_string().construct(pStr, l, alloc);
         }
         else
            get_string().assign(pStr, l, alloc);
         return *this;
      }

      inline value_variant& set_assume_ownership(char* pStr, uint len)
      { 
         m_type = cJSONValueTypeString;
         string_vec_t& str = get_string();
         str.m_p = pStr;
         str.m_size = len;
         return *this;
      }

      inline value_variant& set(const value_variant* pVals, uint n, pool_allocator& alloc) 
      { 
         if (!is_array())
         {
            m_type = cJSONValueTypeArray;
            get_array().construct(pVals, n, alloc);
         }
         else
            get_array().assign(pVals, n, alloc);
         return *this;
      }

      inline value_variant& set_assume_ownership(value_variant* pVals, uint n) 
      { 
         m_type = cJSONValueTypeArray;
         value_variant_vec_t& arr = get_array();
         arr.m_p = pVals;
         arr.m_size = n;
         return *this;
      }

      inline value_variant& set(const key_value_t* pKey_values, uint n, pool_allocator& alloc) 
      { 
         if (!is_object())
         {
            m_type = cJSONValueTypeObject;
            get_object().construct(pKey_values, n, alloc);
         }
         else
            get_object().assign(pKey_values, n, alloc);
         return *this;
      }

      inline value_variant& set_assume_ownership(key_value_t* pKey_values, uint n) 
      { 
         m_type = cJSONValueTypeObject;
         key_value_vec_t& obj = get_object();
         obj.m_p = pKey_values;
         obj.m_size = n;
         return *this;
      }

      inline value_variant &operator=(bool val) { set(val); return *this; }
      inline value_variant &operator=(int32 nVal) { set(nVal); return *this; }
      inline value_variant &operator=(uint32 nVal) { set(nVal); return *this; }
      inline value_variant &operator=(int64 nVal) { set(nVal); return *this; }
      inline value_variant &operator=(double flVal) { set(flVal); return *this; }

      inline bool get_bool_value(bool& val, bool def = false) const { if (is_bool()) { val = (m_data.m_nVal != 0); return true; } else return convert_to_bool(val, def); }
      inline bool get_numeric_value(int32& val, int32 def = 0) const { if ((is_int()) && (m_data.m_nVal == static_cast<int32>(m_data.m_nVal))) { val = static_cast<int32>(m_data.m_nVal); return true; } else return convert_to_int32(val, def); }
      inline bool get_numeric_value(int64& val, int64 def = 0) const { if (is_int()) { val = m_data.m_nVal; return true; } else return convert_to_int64(val, def); }
      inline bool get_numeric_value(float& val, float def = 0.0f) const { if (is_double()) { val = static_cast<float>(m_data.m_flVal); return true; } else return convert_to_float(val, def); }
      inline bool get_numeric_value(double& val, double def = 0.0f) const { if (is_double()) { val = m_data.m_flVal; return true; } else return convert_to_double(val, def); }
      inline bool get_string_value(string_t& val, const char* pDef = "") const { if (is_string()) { val = get_string_ptr(); return true; } else return convert_to_string(val, pDef); }
      
      inline bool as_bool(bool def = false) const { bool result; get_bool_value(result, def); return result; }
      inline int as_int32(int32 def = 0) const { int32 result; get_numeric_value(result, def); return result; }
      inline int64 as_int64(int64 def = 0) const { int64 result; get_numeric_value(result, def); return result; }
      inline float as_float(float def = 0.0f) const { float result; get_numeric_value(result, def); return result; }
      inline double as_double(double def = 0.0f) const { double result; get_numeric_value(result, def); return result; }

      // Returns value as a string, or the default string if the value cannot be converted. 
      inline string_t as_string(const char* pDef = "") const { string_t result; get_string_value(result, pDef); return result; }

      // Returns pointer to null terminated string or NULL if the value is not a string.
      inline const char* as_string_ptr() const { return is_string() ? get_string_ptr() : NULL; }

      inline void swap(value_variant& other)
      {
         pjson::swap(m_type, other.m_type);
         get_object().swap(other.get_object());
      }

      inline uint size() const { PJSON_ASSERT(is_object_or_array()); return is_object_or_array() ? get_array().size() : 0; }

      inline const char *get_key_name_at_index(uint index) const { PJSON_ASSERT(is_object()); return get_object()[index].get_key().get_ptr(""); }

      inline const value_variant *find_child_array(const char *pName) const
      {
         int index = find_key(pName);
         if ((index >= 0) && (get_object()[index].get_value().is_array()))
            return &get_object()[index].get_value();
         return NULL;
      }

      inline const value_variant *find_child_object(const char *pName) const
      {
         int index = find_key(pName);
         if ((index >= 0) && (get_object()[index].get_value().is_object()))
            return &get_object()[index].get_value();
         return NULL;
      }

      inline const value_variant *find_value_variant(const char *pName) const
      {
         int index = find_key(pName);
         return (index < 0) ? NULL : &get_object()[index].get_value();
      }

      inline int find_key(const char *pName) const
      {
         if (!is_object())
         {
            PJSON_ASSERT(0);
            return -1;
         }
         
         const uint n = get_array().size();
         const key_value_vec_t &obj = get_object();

         for (uint i = 0; i < n; i++)
            if (strcmp(pName, obj[i].get_key().get_ptr("")) == 0)
               return i;

         return -1;
      }

      inline bool has_key(const char *pName) const
      {
         return find_key(pName) >= 0;
      }

      inline bool as_bool(const char *pName, bool def = false) const 
      { 
         int index = find_key(pName);
         return (index < 0) ? def : get_object()[index].get_value().as_bool(def);
      }
      
      inline int as_int32(const char *pName, int32 def = 0) const 
      {  
         int index = find_key(pName);
         return (index < 0) ? def : get_object()[index].get_value().as_int32(def);
      }
      
      inline int64 as_int64(const char *pName, int64 def = 0) const 
      { 
         int index = find_key(pName);
         return (index < 0) ? def : get_object()[index].get_value().as_int64(def);
      }
      
      inline float as_float(const char *pName, float def = 0.0f) const 
      { 
         int index = find_key(pName);
         return (index < 0) ? def : get_object()[index].get_value().as_float(def);
      }

      inline double as_double(const char *pName, double def = 0.0f) const 
      { 
         int index = find_key(pName);
         return (index < 0) ? def : get_object()[index].get_value().as_double(def);
      }
      
      inline const char* as_string_ptr(const char *pName, const char *pDef = "") const 
      { 
         int index = find_key(pName);
         if (index < 0) 
            return pDef;
         const char *p = get_object()[index].get_value().as_string_ptr();
         return p ? p : pDef;
      }
     
      inline       value_variant& get_value_at_index(uint index)        { PJSON_ASSERT(is_object_or_array()); return is_object() ? get_object()[index].get_value() : get_array()[index]; }
      inline const value_variant& get_value_at_index(uint index) const  { PJSON_ASSERT(is_object_or_array()); return is_object() ? get_object()[index].get_value() : get_array()[index]; }

      inline       value_variant& operator[](uint index)        { PJSON_ASSERT(is_object_or_array()); return is_object() ? get_object()[index].get_value() : get_array()[index]; }
      inline const value_variant& operator[](uint index) const  { PJSON_ASSERT(is_object_or_array()); return is_object() ? get_object()[index].get_value() : get_array()[index]; }

      inline json_value_type_t get_value_type_at_index(uint index) const { return get_value_at_index(index).get_type(); }

      inline bool is_child_at_index(uint index) const { return get_value_type_at_index(index) >= cJSONValueTypeArray; }
      
      inline bool has_children() const
      {
         if (is_object())
         {
            const key_value_vec_t& obj = get_object();
            const uint s = obj.size();
            for (uint i = 0; i < s; ++i)
               if (obj[i].get_value().is_object_or_array())
                  return true;
         }
         else if (is_array())
         {
            const value_variant_vec_t& arr = get_array();
            const uint s = arr.size();
            for (uint i = 0; i < s; ++i)
               if (arr[i].is_object_or_array())
                  return true;
         }
         return false;
      }
      
      inline void clear_object_or_array()
      {
         PJSON_ASSERT(is_object_or_array());
         if (is_object())
            get_object().clear();
         else if (is_array())
            get_array().clear();
      }

      inline void resize(uint n, pool_allocator& alloc)
      {
         PJSON_ASSERT(is_object_or_array());
         if (is_object())
            get_object().resize(n, alloc);
         else if (is_array())
            get_array().resize(n, alloc);
      }

      inline void set_key_name_at_index(uint index, const char *pKey, uint key_len, pool_allocator& alloc)
      {
         PJSON_ASSERT(is_object()); 
         string_vec_t& str = get_object()[index].get_key();
         str.assign(pKey, key_len + 1, alloc);
      }

      inline void set_key_name_at_index(uint index, const char *pKey, pool_allocator& alloc)
      {
         set_key_name_at_index(index, pKey, static_cast<uint>(strlen(pKey)) + 1, alloc);
      }
      
      inline value_variant& add_key_value(const char* pKey, uint key_len, const value_variant& val, pool_allocator& alloc)
      {
         PJSON_ASSERT(is_object());
         key_value_vec_t& obj = get_object();
         key_value_t* pKey_value = obj.enlarge_no_construct(1, alloc);
         pKey_value->get_key().construct(pKey, key_len + 1, alloc);
         pKey_value->get_value().construct(val, alloc);
         return *this;
      }

      inline value_variant& add_key_value(const char* pKey, const value_variant& val, pool_allocator& alloc)
      {
         return add_key_value(pKey, static_cast<uint>(strlen(pKey)), val, alloc);
      }

      inline value_variant& add_value(const value_variant& val, pool_allocator& alloc)
      {
         PJSON_ASSERT(is_array());
         get_array().enlarge_no_construct(1, alloc)->construct(val, alloc);
         return *this;
      }
      
      bool serialize(char* pBuf, size_t buf_size, size_t* pSize = NULL, bool formatted = true, bool null_terminate = true) const
      {
         serialize_helper<char_buf_print_helper> helper(pBuf, buf_size);
         serialize_internal(helper, formatted, null_terminate, 0);
         if (pSize)
            *pSize = helper.size();
         return (helper.size() < buf_size);
      }

      bool serialize(char_vec_t& buf, bool formatted = true, bool null_terminate = true) const
      {
         serialize_helper<char_vector_print_helper> helper(buf);
         serialize_internal(helper, formatted, null_terminate, 0);
         return true;
      }
      
   protected:
      // Manual constructor
      inline void construct(json_value_type_t type)
      {
         m_type = type;
         memset(&m_data, 0, sizeof(m_data));
      }

      // Assumes variant has NOT been constructed yet.
      inline void construct(const value_variant& other, pool_allocator& alloc)
      {
         m_type = other.m_type;
         m_data.m_nVal = other.m_data.m_nVal;
         if (m_type >= cJSONValueTypeString)
         {
            if (m_type == cJSONValueTypeObject)
               get_object().construct(other.get_object(), alloc);
            else if (m_type == cJSONValueTypeArray)
               get_array().construct(other.get_array(), alloc);
            else
               get_string().construct(other.get_string(), alloc);
         }
      }

      inline bool convert_to_bool(bool& val, bool def) const
      {
         switch (m_type)
         {
            case cJSONValueTypeBool:
            case cJSONValueTypeInt:
            {
               val = (m_data.m_nVal != 0);
               return true;
            }
            case cJSONValueTypeDouble:
            {
               val = (m_data.m_flVal != 0);
               return true;
            }
            case cJSONValueTypeString:
            {
               if (!pjson_stricmp(get_string_ptr(), "false"))
               {
                  val = false;
                  return true;
               }
               else if (!pjson_stricmp(get_string_ptr(), "true"))
               {
                  val = true;
                  return true;
               }
               val = (atof(get_string_ptr()) != 0.0f);
               return true;
            }
            default:
               break;
         }
         val = def;
         return false;
      }

      inline bool convert_to_int32(int32& val, int32 def) const
      {
         val = def;
         int64 val64;
         if (!convert_to_int64(val64, def))
            return false;
         if ((val64 < std::numeric_limits<int32>::min()) || (val64 > std::numeric_limits<int32>::max()))
            return false;
         val = static_cast<int32>(val64);
         return true;
      }

      inline bool convert_to_int64(int64& val, int64 def) const
      {
         switch (m_type)
         {
            case cJSONValueTypeBool:
            case cJSONValueTypeInt:
            {
               val = m_data.m_nVal;
               return true;
            }
            case cJSONValueTypeDouble:
            {
               val = static_cast<int64>(m_data.m_flVal);
               return true;
            }
            case cJSONValueTypeString:
            {
               if (!pjson_stricmp(get_string_ptr(), "false"))
               {
                  val = 0;
                  return true;
               }
               else if (!pjson_stricmp(get_string_ptr(), "true"))
               {
                  val = 1;
                  return true;
               }
               double flVal = floor(atof(get_string_ptr()));
               if ((flVal >= std::numeric_limits<int64>::min()) && (flVal <= std::numeric_limits<int64>::max()))
               {
                  val = static_cast<int64>(flVal);
                  return true;
               }
               break;
            }
            default:
	       break;
         }
         val = def;
         return false;
      }

      inline bool convert_to_float(float& val, float def) const
      {
         switch (m_type)
         {
            case cJSONValueTypeBool:
            case cJSONValueTypeInt:
            {
               val = static_cast<float>(m_data.m_nVal);
               return true;
            }
            case cJSONValueTypeDouble:
            {
               val = static_cast<float>(m_data.m_flVal);
               return true;
            }
            case cJSONValueTypeString:
            {
               if (!pjson_stricmp(get_string_ptr(), "false"))
               {
                  val = 0;
                  return true;
               }
               else if (!pjson_stricmp(get_string_ptr(), "true"))
               {
                  val = 1;
                  return true;
               }
               val = static_cast<float>(atof(get_string_ptr()));
               return true;
            }
            default:
	       break;          
         }
         val = def;
         return false;
      }

      inline bool convert_to_double(double& val, double def) const
      {
         switch (m_type)
         {
            case cJSONValueTypeBool:
            case cJSONValueTypeInt:
            {
               val = static_cast<double>(m_data.m_nVal);
               return true;
            }
            case cJSONValueTypeDouble:
            {
               val = m_data.m_flVal;
               return true;
            }
            case cJSONValueTypeString:
            {
               if (!pjson_stricmp(get_string_ptr(), "false"))
               {
                  val = 0;
                  return true;
               }
               else if (!pjson_stricmp(get_string_ptr(), "true"))
               {
                  val = 1;
                  return true;
               }
               val = atof(get_string_ptr());
               return true;
            }
            default:
	       break;
         }
         val = def;
         return false;
      }
      
      inline bool convert_to_string(char* pBuf, size_t buf_size) const
      {
         switch (m_type)
         {
            case cJSONValueTypeNull:
            {
               pBuf[0] = 'n'; pBuf[1] = 'u'; pBuf[2] = 'l'; pBuf[3] = 'l'; pBuf[4] = '\0';
               return true;
            }
            case cJSONValueTypeBool:
            {
               if (m_data.m_nVal) {
                  pBuf[0] = 't'; pBuf[1] = 'r'; pBuf[2] = 'u'; pBuf[3] = 'e'; pBuf[4] = '\0';
               } else {
                  pBuf[0] = 'f'; pBuf[1] = 'a'; pBuf[2] = 'l'; pBuf[3] = 's'; pBuf[4] = 'e'; pBuf[5] = '\0';
               }
               return true;
            }
            case cJSONValueTypeInt:
            {
               char* pDst = pBuf;
               int64 n = m_data.m_nVal;
               
               uint64 s = static_cast<uint64>(n >> 63);
               *pDst = '-';
               pDst -= s;
               n = (n ^ s) - s;
               
               char* pLeft = pDst;

               do 
               {
                  *pDst++ = '0' + (n % 10);
                  n /= 10;
               } while (n);
               
               *pDst = '\0';
                              
               do
               {
                  char c = *--pDst;
                  *pDst = *pLeft;
                  *pLeft++ = c;
               } while (pDst > pLeft);
               return true;
            }
            case cJSONValueTypeDouble:
            {
#ifdef _MSC_VER
               return 0 == _gcvt_s(pBuf, buf_size, m_data.m_flVal, 15);
#else // TODO
	       abort();
#endif
            }
            default:
	       break;
         }
         return false;
      }

      inline bool convert_to_string(string_t& val, const char* pDef) const
      {
         char buf[64];
         if (m_type == cJSONValueTypeString)
            val = get_string_ptr();
         else
         {
            if (!convert_to_string(buf, sizeof(buf)))
            {
               val.assign(pDef);
               return false;
            }
            val.assign(buf);
         }
         return true;
      }

      inline uint8 get_end_char() const { return (m_type == cJSONValueTypeArray) ? ']' : '}'; }

      template<typename serializer>
      void serialize_node(serializer& out, bool formatted, uint cur_indent) const
      {
         char buf[64];
         const uint size = get_array().size();
         
         if (!size)
         {
            static const char* g_empty_object_strs[4] = { "[]", "[ ]", "{}", "{ }" };
            out.puts(g_empty_object_strs[is_object() * 2 + formatted], 2 + formatted);
            return;
         }

         if (formatted && is_array() && !has_children())
         {
            size_t start_of_line_ofs = out.size();
            out.puts("[ ", 2);

            const uint cMaxLineLen = 100;
            
            for (uint i = 0; i < size; i++)
            {
               const value_variant& child_val = get_value_at_index(i);
               if (child_val.is_string())
                  out.print_escaped(child_val.get_string());
               else
               {
                  child_val.convert_to_string(buf, sizeof(buf));
                  out.puts(buf);
               }

               if (i != size - 1)
                  out.puts(", ", 2);

               if (((out.size() - start_of_line_ofs) > cMaxLineLen) && (i != size - 1))
               {
                  out.print_char('\n');
                  out.print_tabs(cur_indent + 1);

                  start_of_line_ofs = out.size();
               }
            }

            out.puts(" ]", 2);
            return;
         }

         out.print_char(is_object() ? '{' : '[');
         if (formatted)
            out.print_char('\n');

         cur_indent++;

         for (uint i = 0; i < size; i++)
         {
            const value_variant& child_val = get_value_at_index(i);

            if (formatted)
               out.print_tabs(cur_indent);

            if (is_object())
            {
               out.print_escaped(get_object()[i].get_key());
               if (formatted)
                  out.puts(" : ", 3);
               else
                  out.print_char(':');
            }

            json_value_type_t val_type = child_val.get_type();
            if (val_type >= cJSONValueTypeArray)
               child_val.serialize_node(out, formatted, cur_indent);
            else if (val_type == cJSONValueTypeString)
               out.print_escaped(child_val.get_string());
            else
            {
               child_val.convert_to_string(buf, sizeof(buf));
               out.puts(buf);
            }

            if (i != size - 1)
               out.print_char(',');
            if (formatted)
               out.print_char('\n');
         }

         cur_indent--;

         if (formatted)
            out.print_tabs(cur_indent);
         out.print_char(is_object() ? '}' : ']');
      }

      template<typename serializer>
      void serialize_internal(serializer& out, bool formatted, bool null_terminate, uint cur_indent) const
      {
         if (formatted)
            out.print_tabs(cur_indent);
         if (is_object_or_array())
         {
            serialize_node(out, formatted, cur_indent);
            if (formatted)
               out.print_char('\n');
         }
         else 
         {
            if (is_string())
               out.print_escaped(get_string());
            else
            {
               string_t str;
               if (get_string_value(str))
                  out.puts(str.c_str(), str.length());
            }
         }
         if (null_terminate)
            out.print_char('\0');
      }
      
      template<typename T> value_variant(T*);
      template<typename T> value_variant(const T*);
      template<typename T> value_variant& operator= (T*);
      template<typename T> value_variant& operator= (const T*);
   };
   #pragma pack(pop)

   inline key_value_t::key_value_t(const key_value_t& other, pool_allocator& alloc) : 
      m_key(other.get_key(), alloc)
   { 
      get_value().construct(other.get_value(), alloc);
   }

   inline void key_value_t::assign(const key_value_t& src, pool_allocator& alloc) 
   { 
      get_key().assign(src.get_key(), alloc); 
      get_value().assign(src.get_value(), alloc); 
   }

   // ---- class error_info
   
   class error_info
   {
   public:
      inline error_info() : m_ofs(0), m_pError_message(NULL) { }
      inline void set(size_t ofs, const char* pMsg) { m_ofs = ofs; m_pError_message = pMsg; }

      size_t m_ofs;
      const char* m_pError_message;
   };

   // ---- class growable_stack
      
   class growable_stack
   {
   public:
      inline growable_stack(uint initial_size) : 
         m_pBuf(NULL),
         m_size(initial_size),
         m_ofs(0)
      {
         if (initial_size)
            m_pBuf = static_cast<uint8*>(pjson_malloc(initial_size));
      }

      inline ~growable_stack()
      {
         pjson_free(m_pBuf);
      }

      inline void clear()
      {
         pjson_free(m_pBuf);
         m_pBuf = NULL;
         m_size = 0;
         m_ofs = 0;
      }

      inline uint8* get_top_ptr() { return reinterpret_cast<uint8*>(m_pBuf) + m_ofs; }

      template<typename T> inline T* get_top_obj() { return reinterpret_cast<T*>(m_pBuf + m_ofs - sizeof(T)); }

      inline void reset() { m_ofs = 0; }

      inline size_t get_ofs() { return m_ofs; }

      template<typename T>
      PJSON_FORCEINLINE T* push(uint num)
      {
         const size_t bytes_needed = sizeof(T) * num;
         T* pResult = reinterpret_cast<T*>(m_pBuf + m_ofs);
         m_ofs += bytes_needed;
         
         if (m_ofs > m_size)
         {
            m_ofs -= bytes_needed;
            
            m_size = PJSON_MAX(1, m_size * 2);
            while(m_size <= (m_ofs + bytes_needed))
               m_size *= 2;
            m_pBuf = static_cast<uint8*>(pjson_realloc(m_pBuf, m_size));

            pResult = reinterpret_cast<T*>(m_pBuf + m_ofs);
            m_ofs += bytes_needed;
         }
         
         PJSON_ASSERT(m_ofs <= m_size);
         return pResult;
      }

      template<typename T>
      inline T* pop(uint num)
      {
         size_t bytes_needed = sizeof(T) * num;
         PJSON_ASSERT(bytes_needed <= m_ofs);
         m_ofs -= bytes_needed;
         return reinterpret_cast<T*>(m_pBuf + m_ofs);
      }
      
   private:
      uint8* m_pBuf;
      size_t m_size;
      size_t m_ofs;
   };

   // ---- class document
      
   class document : public value_variant
   {
      document(const document&);
      document& operator= (const document&);

   public:
      inline document(uint initial_pool_size = 0, uint min_pool_chunk_size = PJSON_DEFAULT_MIN_CHUNK_SIZE, uint initial_stack_size = 0) :
         m_allocator(initial_pool_size, min_pool_chunk_size),
         m_initial_stack_size(initial_stack_size),
         m_stack(initial_stack_size)
      {
#if PJSON_PARSE_STATS
         m_parse_stats.clear();
#endif
      }

      inline ~document()
      {
      }

      const pool_allocator& get_allocator() const  { return m_allocator; }
            pool_allocator& get_allocator()        { return m_allocator; }

      void clear()
      {
         set_to_null();
         m_allocator.clear();
         m_stack.clear();
#if PJSON_PARSE_STATS
         m_parse_stats.clear();
#endif
      }
      
      bool deserialize_in_place(char* pStr)
      {
         return deserialize_start((uint8*)pStr);
      }
      
#if PJSON_PARSE_STATS      
      struct parse_stats_t
      {
         size_t m_num_string, m_num_string_chars;
         size_t m_num_numeric, m_num_numeric_chars;
         size_t m_num_whitespace_blocks, m_num_whitespace_chars;
         size_t m_num_control;
         size_t m_num_comment, m_num_comment_chars;
         size_t m_num_value_pop, m_value_pop_bytes;
         size_t m_num_bool_chars;
         size_t m_num_escape_breaks;
         size_t m_num_unicode_escapes;

         void clear() { memset(this, 0, sizeof(*this)); }
      };

      const parse_stats_t& get_parse_stats() const { return m_parse_stats; }
            parse_stats_t& get_parse_stats()       { return m_parse_stats; }
#endif
           
   private:
      pool_allocator m_allocator;
      uint m_initial_stack_size;
      growable_stack m_stack;
      error_info m_error_info;
      const uint8* m_pStart;
      const uint8* m_pStr;
      
      inline bool set_error(const uint8* pStr, const char* pMsg) 
      { 
         m_pStr = pStr; 
         m_error_info.set(m_pStr - m_pStart, pMsg); 
         return false; 
      }

#if PJSON_PARSE_STATS      
      parse_stats_t m_parse_stats;
#endif

#if PJSON_PARSE_STATS
   #define PJSON_INCREMENT_STAT(x) do { ++m_parse_stats.x; } while(0)
   #define PJSON_UPDATE_STAT(x, n) do { m_parse_stats.x += n; } while(0)
#else
   #define PJSON_INCREMENT_STAT(x) do { } while(0)
   #define PJSON_UPDATE_STAT(x, n) do { } while(0)
#endif
      
#define PJSON_SKIP_WHITESPACE \
   while (globals::s_parse_flags[*pStr] & 4) \
   { \
      PJSON_INCREMENT_STAT(m_num_whitespace_blocks); \
      do { \
         if (!(globals::s_parse_flags[pStr[1]] & 4)) { ++pStr; PJSON_INCREMENT_STAT(m_num_whitespace_chars); break; } \
         if (!(globals::s_parse_flags[pStr[2]] & 4)) { pStr += 2; PJSON_UPDATE_STAT(m_num_whitespace_chars, 2); break; } \
         if (!(globals::s_parse_flags[pStr[3]] & 4)) { pStr += 3; PJSON_UPDATE_STAT(m_num_whitespace_chars, 3); break; } \
         pStr += 4; PJSON_UPDATE_STAT(m_num_whitespace_chars, 4); \
      } while (globals::s_parse_flags[*pStr] & 4); \
      if ((*pStr != '/') || (pStr[1] != '/')) break; \
      pStr += 2; PJSON_INCREMENT_STAT(m_num_comment); PJSON_UPDATE_STAT(m_num_comment_chars, 2); \
      while ((*pStr) && (*pStr != '\n') && (*pStr != '\r')) { PJSON_INCREMENT_STAT(m_num_comment_chars); ++pStr; } \
   }

      inline const uint8* skip_whitespace(const uint8* p)
      {
         uint8 c;
         while ((c = *p) != '\0')
         {
            if ((c == ' ') || (c == '\t'))
            {
               do 
               {
                  PJSON_INCREMENT_STAT(m_num_whitespace_chars);
               } while (*++p == c);
               continue;
            }

            if ((c == '/') && (p[1] == '/'))
            {
               p += 2; PJSON_INCREMENT_STAT(m_num_comment); PJSON_UPDATE_STAT(m_num_comment_chars, 2);
               while ((*p) && (*p != '\n') && (*p != '\r'))
               {
                  PJSON_UPDATE_STAT(m_num_comment_chars, 2);
                  ++p;
               }
               continue;
            }
            else if (c > ' ')
               break;

            PJSON_INCREMENT_STAT(m_num_whitespace_chars);
            ++p;
         }
         return p;
      }
            
      bool deserialize_internal()
      {
         static const uint8 g_utf8_first_byte[7] = { 0x00, 0x00, 0xC0, 0xE0, 0xF0, 0xF8, 0xFC };
         
         m_stack.reset();
         memcpy(m_stack.push<value_variant>(1), static_cast<value_variant*>(this), sizeof(value_variant));

         const uint8* pStr = ++m_pStr;
         PJSON_UPDATE_STAT(m_num_control, 1);
                  
         bool cur_is_object = is_object();
         uint8 cur_end_char = get_end_char();
         uint cur_num_elements = 0;
                  
         for ( ; ; )
         {
            PJSON_SKIP_WHITESPACE;

            uint8 c = *pStr;
            
            if (c == ',')
            {
               if (!cur_num_elements)
                  return set_error(pStr, "Unexpected comma");

               ++pStr;
               PJSON_UPDATE_STAT(m_num_control, 1);
               PJSON_SKIP_WHITESPACE;
               c = *pStr;
            }
            else if ((cur_num_elements) && (c != cur_end_char))
               return set_error(pStr, "Expected comma or object/array end character");
            
            while (c == cur_end_char)
            {
               PJSON_UPDATE_STAT(m_num_control, 1);
               ++pStr;
                              
               for ( ; ; )
               {
                  uint n = cur_num_elements, num_bytes = cur_num_elements * (cur_is_object ? sizeof(key_value_t) : sizeof(value_variant));
                  void* pSrc = m_stack.pop<uint8>(num_bytes);
                  PJSON_INCREMENT_STAT(m_num_value_pop);
                  PJSON_UPDATE_STAT(m_value_pop_bytes, num_bytes);

                  // The top of the stack (after popping the current array/object) could contain either a value_variant (if cur_is_object is set), 
                  // or a key_value_t, which ends in a value_variant. So all we need to do is look at the very end, which always has a value_variant.
                  value_variant* pCur_variant = m_stack.get_top_obj<value_variant>();

                  value_variant_vec_t& arr = pCur_variant->get_array();
                  cur_is_object = (arr.m_p != NULL);
                  cur_end_char = cur_is_object ? '}' : ']';
                  cur_num_elements = arr.m_size;
                  
                  arr.m_size = n;
                  arr.m_p = NULL;
                  if (num_bytes)
                     memcpy(arr.m_p = static_cast<value_variant*>(m_allocator.Alloc(num_bytes)), pSrc, num_bytes);
                  
                  if (m_stack.get_ofs() <= sizeof(value_variant))
                  {
                     PJSON_ASSERT(m_stack.get_ofs() == sizeof(value_variant));
                     memcpy(static_cast<value_variant*>(this), m_stack.pop<value_variant>(1), sizeof(value_variant));
                     m_pStr = pStr;
                     return true;
                  }
                                    
                  PJSON_SKIP_WHITESPACE;                  

                  if (*pStr == ',')
                  {
                     PJSON_UPDATE_STAT(m_num_control, 1);
                     ++pStr;
                     
                     PJSON_SKIP_WHITESPACE;

                     c = *pStr;
                     break;
                  }
                                    
                  if (*pStr++ != cur_end_char)
                     return set_error(pStr, "Unexpected character within object or array");
                  PJSON_UPDATE_STAT(m_num_control, 1);
               }
            }

            ++cur_num_elements;

            value_variant* pChild_variant;
            
            if (!cur_is_object)
               pChild_variant = m_stack.push<value_variant>(1);
            else
            {
               if (c != '\"')
                  return set_error(pStr, "Expected quoted key string");
               
               ++pStr; PJSON_INCREMENT_STAT(m_num_string); PJSON_INCREMENT_STAT(m_num_string_chars);

               uint8* pBuf = (uint8*)pStr;

               c = *pStr++; PJSON_INCREMENT_STAT(m_num_string_chars);
               if (!(globals::s_parse_flags[c] & 1))
               {
                  do
                  {
                     c = pStr[0]; if (globals::s_parse_flags[c] & 1) { ++pStr; PJSON_UPDATE_STAT(m_num_string_chars, 1); break; }
                     c = pStr[1]; if (globals::s_parse_flags[c] & 1) { pStr += 2; PJSON_UPDATE_STAT(m_num_string_chars, 2); break; }
                     c = pStr[2]; if (globals::s_parse_flags[c] & 1) { pStr += 3; PJSON_UPDATE_STAT(m_num_string_chars, 3); break; }
                     c = pStr[3];
                     pStr += 4; PJSON_UPDATE_STAT(m_num_string_chars, 4);
                  } while (!(globals::s_parse_flags[c] & 1));
               }

               uint8* pDst = (uint8*)pStr - 1;

               if (c != '\"') PJSON_INCREMENT_STAT(m_num_escape_breaks); 

               while (c != '\"')
               {
                  if (globals::s_parse_flags[c] & 2)
                     return set_error(pStr, "Missing end quote");

                  c = *pStr++; PJSON_INCREMENT_STAT(m_num_string_chars);
                  if (c == 'u')
                  {
                     PJSON_INCREMENT_STAT(m_num_unicode_escapes); 
                     uint u = 0;
                     for (uint i = 0; i < 4; i++)
                     {
                        u <<= 4;
                        int cc = *pStr++; PJSON_INCREMENT_STAT(m_num_string_chars);
                        if ((cc >= 'A') && (cc <= 'F'))
                           u += cc - 'A' + 10;
                        else if ((cc >= 'a') && (cc <= 'f'))
                           u += cc - 'a' + 10;
                        else if ((cc >= '0') && (cc <= '9'))
                           u += cc - '0';
                        else
                           return set_error(pStr, "Invalid Unicode escape");
                     }

                     uint len = 3; if ((u) && (u < 0x80)) len = 1; else if (u < 0x800) len = 2;

                     pDst += len;
                     uint8* q = pDst;
                     switch (len) 
                     {
                        case 3: *--q = static_cast<uint8>((u | 0x80) & 0xBF); u >>= 6; // falls through
                        case 2: *--q = static_cast<uint8>((u | 0x80) & 0xBF); u >>= 6; // falls through
                        case 1: *--q = static_cast<uint8>(u | g_utf8_first_byte[len]);
                     }
                  }
                  else
                  {
                     switch (c)
                     {
                        case 'b': c = '\b'; break;
                        case 'f': c = '\f'; break;
                        case 'n': c = '\n'; break;
                        case 'r': c = '\r'; break;
                        case 't': c = '\t'; break;
                        case '\\': case '\"': case '/': break;
                        case '\0': return set_error(pStr, "Incomplete string escape");
                        default: { *pDst++ = '\\'; break; } // unrecognized escape, so forcefully escape the backslash (not standard)
                     }

                     *pDst++ = c;
                  }

                  c = *pStr++; PJSON_INCREMENT_STAT(m_num_string_chars);
                  while (!(globals::s_parse_flags[c] & 1))
                  {
                     pDst[0] = c; 
                     c = pStr[0]; if (globals::s_parse_flags[c] & 1) { ++pDst; ++pStr; PJSON_INCREMENT_STAT(m_num_string_chars); break; }
                     pDst[1] = c; 
                     c = pStr[1]; if (globals::s_parse_flags[c] & 1) { pDst += 2; pStr += 2; PJSON_UPDATE_STAT(m_num_string_chars, 2); break; }
                     pDst[2] = c; 
                     c = pStr[2]; if (globals::s_parse_flags[c] & 1) { pDst += 3; pStr += 3; PJSON_UPDATE_STAT(m_num_string_chars, 3); break; }
                     pDst[3] = c; 
                     pDst += 4;
                     c = pStr[3];
                     pStr += 4; PJSON_UPDATE_STAT(m_num_string_chars, 4);
                  }
               }
               
               *pDst++ = '\0';

               key_value_t* pKey_value = m_stack.push<key_value_t>(1);
               pChild_variant = &pKey_value->get_value();
               pKey_value->get_key().m_p = (char*)pBuf;
               pKey_value->get_key().m_size = static_cast<uint>(pDst - pBuf - 1);
               
               PJSON_SKIP_WHITESPACE;

               if (*pStr != ':')
                  return set_error(pStr, "Missing colon after key");

               ++pStr; PJSON_INCREMENT_STAT(m_num_control);
               PJSON_SKIP_WHITESPACE;
               
               c = *pStr;
            }
                                                
            switch (c)
            {
               case '{':
               case '[':
               {
                  ++pStr; PJSON_INCREMENT_STAT(m_num_control);

                  pChild_variant->m_type = (c == '{') ? cJSONValueTypeObject : cJSONValueTypeArray;
                  pChild_variant->m_data.m_object.m_size = cur_num_elements;
                  pChild_variant->m_data.m_object.m_p = (key_value_t*)cur_is_object;
                  
                  cur_is_object = (c == '{');
                  cur_num_elements = 0;
                  cur_end_char = c + 2;
                  break;
               }
               case '\"':
               {
                  ++pStr; PJSON_INCREMENT_STAT(m_num_string); PJSON_INCREMENT_STAT(m_num_string_chars);
                  
                  uint8* pBuf = (uint8*)pStr;
                  
                  c = *pStr++; PJSON_INCREMENT_STAT(m_num_string_chars);
                  if (!(globals::s_parse_flags[c] & 1))
                  {
                     do
                     {
                        c = pStr[0]; if (globals::s_parse_flags[c] & 1) { ++pStr; PJSON_INCREMENT_STAT(m_num_string_chars); break; }
                        c = pStr[1]; if (globals::s_parse_flags[c] & 1) { pStr += 2; PJSON_UPDATE_STAT(m_num_string_chars, 2); break; }
                        c = pStr[2]; if (globals::s_parse_flags[c] & 1) { pStr += 3; PJSON_UPDATE_STAT(m_num_string_chars, 3); break; }
                        c = pStr[3];
                        pStr += 4; PJSON_UPDATE_STAT(m_num_string_chars, 4);
                     } while (!(globals::s_parse_flags[c] & 1));
                  }

                  uint8* pDst = (uint8*)pStr - 1;

                  if (c != '\"') PJSON_INCREMENT_STAT(m_num_escape_breaks); 

                  while (c != '\"')
                  {
                     if (globals::s_parse_flags[c] & 2)
                        return set_error(pStr, "Missing end quote");
                     
                     c = *pStr++; PJSON_INCREMENT_STAT(m_num_string_chars);
                     if (c == 'u')
                     {
                        PJSON_INCREMENT_STAT(m_num_unicode_escapes); 
                        uint u = 0;
                        for (uint i = 0; i < 4; i++)
                        {
                           u <<= 4;
                           int cc = *pStr++; PJSON_INCREMENT_STAT(m_num_string_chars);
                           if ((cc >= 'A') && (cc <= 'F'))
                              u += cc - 'A' + 10;
                           else if ((cc >= 'a') && (cc <= 'f'))
                              u += cc - 'a' + 10;
                           else if ((cc >= '0') && (cc <= '9'))
                              u += cc - '0';
                           else
                              return set_error(pStr, "Invalid Unicode escape");
                        }

                        uint len = 3; if ((u) && (u < 0x80)) len = 1; else if (u < 0x800) len = 2;

                        pDst += len;
                        uint8* q = pDst;
                        switch (len) 
                        {
                           case 3: *--q = static_cast<uint8>((u | 0x80) & 0xBF); u >>= 6; // falls through
                           case 2: *--q = static_cast<uint8>((u | 0x80) & 0xBF); u >>= 6; // falls through
                           case 1: *--q = static_cast<uint8>(u | g_utf8_first_byte[len]);
                        }
                     }
                     else
                     {
                        switch (c)
                        {
                           case 'b': c = '\b'; break;
                           case 'f': c = '\f'; break;
                           case 'n': c = '\n'; break;
                           case 'r': c = '\r'; break;
                           case 't': c = '\t'; break;
                           case '\\': case '\"': case '/': break;
                           case '\0': return set_error(pStr, "Incomplete string escape");
                           default: { *pDst++ = '\\'; break; } // unrecognized escape, so forcefully escape the backslash (not standard)
                        }

                        *pDst++ = c;
                     }

                     c = *pStr++; PJSON_INCREMENT_STAT(m_num_string_chars);
                     while (!(globals::s_parse_flags[c] & 1))
                     {
                        pDst[0] = c; 
                        c = pStr[0]; if (globals::s_parse_flags[c] & 1) { ++pDst; ++pStr; PJSON_INCREMENT_STAT(m_num_string_chars); break; }
                        pDst[1] = c; 
                        c = pStr[1]; if (globals::s_parse_flags[c] & 1) { pDst += 2; pStr += 2; PJSON_UPDATE_STAT(m_num_string_chars, 2); break; }
                        pDst[2] = c; 
                        c = pStr[2]; if (globals::s_parse_flags[c] & 1) { pDst += 3; pStr += 3; PJSON_UPDATE_STAT(m_num_string_chars, 3); break; }
                        pDst[3] = c; 
                        pDst += 4;
                        c = pStr[3];
                        pStr += 4; PJSON_UPDATE_STAT(m_num_string_chars, 4); 
                     }
                  }

                  *pDst++ = '\0';

                  pChild_variant->m_type = cJSONValueTypeString;

                  string_vec_t& str = pChild_variant->get_string();
                  str.m_p = (char*)pBuf;
                  str.m_size = static_cast<uint>(pDst - pBuf - 1);

                  break;
               }
               case 'n': 
               {
                  if ((pStr[1] == 'u') && (pStr[2] == 'l') && (pStr[3] == 'l'))
                  {
                     pStr += 4; PJSON_UPDATE_STAT(m_num_bool_chars, 4); 
                     pChild_variant->construct(cJSONValueTypeNull);
                  }
                  else
                     return set_error(pStr, "Unrecognized character");
                  break;
               }
               case 't':
               {
                  if ((pStr[1] == 'r') && (pStr[2] == 'u') && (pStr[3] == 'e'))
                  {
                     pStr += 4; PJSON_UPDATE_STAT(m_num_bool_chars, 4); 
                     pChild_variant->construct(cJSONValueTypeBool);
                     pChild_variant->m_data.m_nVal = 1;
                  }
                  else
                     return set_error(pStr, "Unrecognized character");
                  break;
               }
               case 'f':
               {
                  if ((pStr[1] == 'a') && (pStr[2] == 'l') && (pStr[3] == 's') && (pStr[4] == 'e'))
                  {
                     pStr += 5; PJSON_UPDATE_STAT(m_num_bool_chars, 5); 
                     pChild_variant->construct(cJSONValueTypeBool);
                  }
                  else
                     return set_error(pStr, "Unrecognized character");
                  break;
               }
               case '0': case '1': case '2': case '3': case '4': case '5':
               case '6': case '7': case '8': case '9': case '-': case '.':
               {
                  PJSON_INCREMENT_STAT(m_num_numeric); 
                  if (c == '-') PJSON_INCREMENT_STAT(m_num_numeric_chars); 
                  
                  uint32 n32 = 0;
                  int is_neg = (c == '-');
                  c = *(pStr += is_neg);
                  
                  if (globals::s_parse_flags[c] & 8) 
                  { 
                     n32 = c - '0'; c = *++pStr; PJSON_UPDATE_STAT(m_num_numeric_chars, 1); 
                     if (globals::s_parse_flags[c] & 8) 
                     { 
                        n32 = (n32 * 10U) + (c - '0'); c = *++pStr; PJSON_UPDATE_STAT(m_num_numeric_chars, 1); 
                        if (globals::s_parse_flags[c] & 8) 
                        { 
                           n32 = (n32 * 10U) + (c - '0'); c = *++pStr; PJSON_UPDATE_STAT(m_num_numeric_chars, 1); 
                           if (globals::s_parse_flags[c] & 8) 
                           { 
                              n32 = (n32 * 10U) + (c - '0'); c = *++pStr; PJSON_UPDATE_STAT(m_num_numeric_chars, 1); 
                              if (globals::s_parse_flags[c] & 8) 
                              { 
                                 n32 = (n32 * 10U) + (c - '0'); c = *++pStr; PJSON_UPDATE_STAT(m_num_numeric_chars, 1); 
                                 if (globals::s_parse_flags[c] & 8) 
                                 { 
                                    n32 = (n32 * 10U) + (c - '0'); c = *++pStr; PJSON_UPDATE_STAT(m_num_numeric_chars, 1); 
                                    if (globals::s_parse_flags[c] & 8) 
                                    { 
                                       n32 = (n32 * 10U) + (c - '0'); c = *++pStr; PJSON_UPDATE_STAT(m_num_numeric_chars, 1); 
                                       if (globals::s_parse_flags[c] & 8) 
                                       { 
                                          n32 = (n32 * 10U) + (c - '0'); c = *++pStr; PJSON_UPDATE_STAT(m_num_numeric_chars, 1); 
                                       }
                                    }
                                 }
                              }
                           }
                        }
                     }
                  }

                  if (!(globals::s_parse_flags[c] & 0x10))
                  {
                     pChild_variant->m_type = cJSONValueTypeInt;
                     pChild_variant->m_data.m_nVal = is_neg + (static_cast<int32>(n32) ^ (-is_neg));
                  }
                  else
                  {
                     uint64 n64 = n32;
                     while (globals::s_parse_flags[c] & 8) 
                     {
                        n64 = n64 * 10U + (c - '0'); PJSON_INCREMENT_STAT(m_num_numeric_chars); c = *++pStr;
                        
                        if ((!(globals::s_parse_flags[c] & 8)) || (n64 > 0xCCCCCCCCCCCCCCBULL))
                           break;
                        
                        n64 = n64 * 10U + (c - '0'); PJSON_INCREMENT_STAT(m_num_numeric_chars); c = *++pStr;

                        if (n64 > 0xCCCCCCCCCCCCCCBULL) 
                           break;
                     }

                     if (!(globals::s_parse_flags[c] & 0x10))
                     {
                        pChild_variant->m_type = cJSONValueTypeInt;
                        pChild_variant->m_data.m_nVal = is_neg + (static_cast<int64>(n64) ^ (-is_neg));
                     }
                     else
                     {
                        double f = static_cast<double>(n64);
                        int scale = 0, escalesign = 1, escale = 0;

                        while (globals::s_parse_flags[c] & 8) 
                        {
                           f = f * 10.0f + (c - '0'); PJSON_INCREMENT_STAT(m_num_numeric_chars); c = *++pStr;
                           if (!(globals::s_parse_flags[c] & 8)) 
                              break;
                           
                           f = f * 10.0f + (c - '0'); PJSON_INCREMENT_STAT(m_num_numeric_chars); c = *++pStr;
                        } 

                        if (c == '.')
                        {
                           PJSON_INCREMENT_STAT(m_num_numeric_chars); c = *++pStr;
                           while (globals::s_parse_flags[c] & 8)
                           {
                              scale--; f = f * 10.0f + (c - '0'); PJSON_INCREMENT_STAT(m_num_numeric_chars); c = *++pStr;

                              if (!(globals::s_parse_flags[c] & 8)) 
                                 break;
                              
                              scale--; f = f * 10.0f + (c - '0'); PJSON_INCREMENT_STAT(m_num_numeric_chars); c = *++pStr;
                           }
                        }

                        if ((c == 'e') || (c == 'E'))
                        {
                           PJSON_INCREMENT_STAT(m_num_numeric_chars); 
                           c = *++pStr;
                           if (c == '-')
                           {
                              escalesign = -1;
                              PJSON_INCREMENT_STAT(m_num_numeric_chars); c = *++pStr;
                           }
                           else if (c == '+')
                           {
                              PJSON_INCREMENT_STAT(m_num_numeric_chars); c = *++pStr;
                           }
                           while (globals::s_parse_flags[c] & 8)
                           {
                              if (escale > 0xCCCCCCB)
                                 return set_error(pStr, "Failed parsing numeric value");
                              escale = escale * 10 + (c - '0');
                              PJSON_INCREMENT_STAT(m_num_numeric_chars); c = *++pStr;
                           }
                        }

                        static const float s_neg[2] = { 1.0f, -1.0f };
                        double v = f * s_neg[is_neg];
                        int64 final_scale = scale + escale * escalesign;
                        if (static_cast<uint64>(final_scale + 31) <= 62)
                           v *= globals::s_pow10_table[static_cast<int>(final_scale) + 31];
                        else
                        {
                           if ((final_scale < INT32_MIN) || (final_scale > INT32_MAX))
                              return set_error(pStr, "Failed parsing numeric value");
                           v *= pow(10.0, static_cast<int>(final_scale));
                        }

                        pChild_variant->m_type = cJSONValueTypeDouble;
                        pChild_variant->m_data.m_flVal = v;
                     }
                  }
                  
                  break;
               }
               case '\0':
                  return set_error(pStr, "Premature end of string (expected name or value)");
               default:
                  return set_error(pStr, "Unrecognized character");
            }
         } 
      }

      bool deserialize_start(uint8* pStr)
      {
         set_to_null();

#if PJSON_PARSE_STATS
         m_parse_stats.clear();
#endif

         m_allocator.reset();

         m_pStart = pStr;
         
         m_pStr = skip_whitespace(pStr);
         
         if (!*m_pStr)
            return set_error(m_pStr, "Nothing to deserialize");

         bool success = false;

         uint8 c = *m_pStr;
         if ((c == '{') || (c == '['))
         {
            set_to_node(c == '{'); PJSON_INCREMENT_STAT(m_num_control);
            success = deserialize_internal();
         }
         else
            return set_error(m_pStr, "Root value must be an object or array");

         if (success)
         {
            m_pStr = skip_whitespace(m_pStr);
            success = !*m_pStr;
            if (!success)
               set_error(m_pStr, "Unknown data at end of document");
         }

         return success;
      }
   };

} // namespace pjson

#endif // PJSON_H
