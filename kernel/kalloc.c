// Physical memory allocator, for user processes,
// kernel stacks, page-table pages,
// and pipe buffers. Allocates whole 4096-byte pages.

#include "types.h"
#include "param.h"
#include "memlayout.h"
#include "spinlock.h"
#include "riscv.h"
#include "defs.h"

void freerange(void *pa_start, void *pa_end);

extern char end[]; // first address after kernel.
                   // defined by kernel.ld.

struct run {
  struct run *next;
};

struct {
  struct spinlock lock;
  struct run *freelist;    // the head of the list
  int    refcnt[MAXPAGES]; // the physical pages reference count
} kmem;                    // skip the struct name

void
kinit() // 初始化整个内存(包括user, kernel)
{
  initlock(&kmem.lock, "kmem");
  memset(kmem.refcnt, 0, MAXPAGES);
  freerange(end, (void *)PHYSTOP);
}

void
freerange(void *pa_start, void *pa_end)
{
  char *p;
  p = (char *)PGROUNDUP((uint64)pa_start);
  for (; p + PGSIZE <= (char *)pa_end; p += PGSIZE)
    kfree(p);
}

// Free the page of physical memory pointed at by pa,
// which normally should have been returned by a
// call to kalloc().  (The exception is when
// initializing the allocator; see kinit above.)
// Can only free the page when ref is 0.
void
kfree(void *pa)
{
  struct run *r;

  if (((uint64)pa % PGSIZE) != 0 || (char *)pa < end || (uint64)pa >= PHYSTOP)
    panic("kfree");

  r = (struct run *)pa;

  acquire(&kmem.lock);

  if(kmem.refcnt[REF((uint64)r)] != 0)
    panic("kfree : invalid ref number");

  // Fill with junk to catch dangling refs.
  memset(pa, 1, PGSIZE);
  r->next = kmem.freelist;
  kmem.freelist = r;
  release(&kmem.lock);
}

// Allocate one 4096-byte page of physical memory. (from freelist)
// Returns a pointer that the kernel can use.
// Returns 0 if the memory cannot be allocated.
void *
kalloc(void)
{
  struct run *r;

  acquire(&kmem.lock);
  r = kmem.freelist;
  if (r) {
    kmem.freelist = r->next;     // 去除链表头
  }
  release(&kmem.lock);

  if (r)
    memset((char *)r, 5, PGSIZE); // fill with junk
  return (void *)r;
}


// calculate the freememory
uint64
freemem(void)
{
  uint64 bytes = 0;
  struct run *r;

  acquire(&kmem.lock);
  for(r = kmem.freelist; r != 0; r = r->next)
    bytes += PGSIZE;
  release(&kmem.lock);

  return bytes;
}

// increase the refs of the data physical page
void
refincr(uint64 pa)
{
  if ((pa % PGSIZE) != 0 || (char *)pa < end || pa >= PHYSTOP)
    panic("refincr");

  acquire(&kmem.lock);
  kmem.refcnt[REF(pa)]++;
  release(&kmem.lock);
}

// decrease the refs of the data physical page
void
refdecr(uint64 pa)
{
  int freepage = 0;

  if ((pa % PGSIZE) != 0 || (char *)pa < end || pa >= PHYSTOP)
    panic("refdecr");

  acquire(&kmem.lock);
  if (kmem.refcnt[REF(pa)] <= 0)
    panic("refdecr: invalid ref");
  if (--kmem.refcnt[REF(pa)] == 0)
    freepage = 1;
  release(&kmem.lock);

  if (freepage)
    kfree((void *)pa);
}

uint
getref(uint64 pa)
{
  uint n;

  if ((pa % PGSIZE) != 0 || (char *)pa < end || pa >= PHYSTOP)
    panic("getref");

  acquire(&kmem.lock);
  n = kmem.refcnt[REF(pa)];
  release(&kmem.lock);
  return n;
}
