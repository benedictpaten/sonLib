//
// nazghul - an old-school RPG engine
// Copyright (C) 2002, 2003 Gordon McNutt
//
// This program is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 of the License, or (at your option)
// any later version.
//
// This program is distributed in the hope that it will be useful, but WITHOUT
// ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
// FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
// more details.
//
// You should have received a copy of the GNU General Public License along with
// this program; if not, write to the Free Foundation, Inc., 59 Temple Place,
// Suite 330, Boston, MA 02111-1307 USA
//
// Gordon McNutt
// gmcnutt@users.sourceforge.net
//
  
#ifndef HEAPC_H_
#define HEAPC_H_
  
#include "fastCMaths.h"

//#include "macros.h"
  
//BEGIN_DECL

//#define heap_empty(h) (!(h)->num_entries)

struct heap {
        uint32_t max_entries;
        uint32_t num_entries;
        int64_t *entries; 
};

extern int32_t heap_empty(struct heap *heap);
extern struct heap *heap_create(uint32_t max_entries);
extern void heap_destroy(struct heap *heap);
extern void heapify(struct heap *heap, int64_t i);
extern int32_t heap_expand(struct heap *heap);
extern int32_t heap_insert(struct heap *heap, int64_t entry);
extern int64_t heap_extract(struct heap *heap);
extern int64_t heap_peek(struct heap *heap); 
extern void heap_clean(struct heap *heap);

#define HEADROOM 100

//END_DECL
 
#endif
