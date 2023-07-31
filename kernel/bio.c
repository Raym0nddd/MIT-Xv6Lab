// Buffer cache.
//
// The buffer cache is a linked list of buf structures holding
// cached copies of disk block contents.  Caching disk blocks
// in memory reduces the number of disk reads and also provides
// a synchronization point for disk blocks used by multiple processes.
//
// Interface:
// * To get a buffer for a particular disk block, call bread.
// * After changing buffer data, call bwrite to write it to disk.
// * When done with the buffer, call brelse.
// * Do not use the buffer after calling brelse.
// * Only one process at a time can use a buffer,
//     so do not keep them longer than necessary.


#include "types.h"
#include "param.h"
#include "spinlock.h"
#include "sleeplock.h"
#include "riscv.h"
#include "defs.h"
#include "fs.h"
#include "buf.h"

#define hashNum 13

struct {
  struct spinlock lock[hashNum];
  struct buf head[hashNum];
  struct buf buf[NBUF];

  // Linked list of all buffers, through prev/next.
  // Sorted by how recently the buffer was used.
  // head.next is most recent, head.prev is least.
} bcache;

struct spinlock bcachelock;

uint
hashIndex(uint dev, uint blockno)
{
  return blockno%hashNum;
}

void
binit(void)
{
  struct buf *b;
  char name[10];

  initlock(&bcachelock, "search_lock");

  for(int i = 0 ; i < hashNum; i++){
    snprintf(name, 1, "bcache-%d", i);
    initlock(&bcache.lock[i], name);
  }
    
  // initialize block's lock
  bcache.head[0].next = &bcache.buf[0];
  for(b = bcache.buf; b < bcache.buf + NBUF - 1; b++) {
    b->next = b + 1;
    initsleeplock(&b->lock, "b_lock");
  }
  initsleeplock(&b->lock,"b_lock");
  
}

// Look through buffer cache for block on device dev.
// If not found, allocate a buffer.
// In either case, return locked buffer.
static struct buf*
bget(uint dev, uint blockno)
{
  // acquire(&bcachelock);
  struct buf *b, *pre, *off = 0, *tmp = 0, *bfoff = 0;
  uint index = hashIndex(dev, blockno);
  uint64 minticks = 999999999999;

  acquire(&bcache.lock[index]);

  // Is the block already cached?
  for(b = bcache.head[index].next, pre = &bcache.head[index]; b; b = b->next, pre = pre->next){
    if(b->dev == dev && b->blockno == blockno){
      b->refcnt++;
      release(&bcache.lock[index]);
      // release(&bcachelock);
      acquiresleep(&b->lock);
      return b;
    }
    if(b->refcnt == 0 && b->ticks < minticks){
      off = b;
      minticks = b->ticks;
    }
  }

  // Not cached.
  // Recycle the least recently used (LRU) unused buffer.
  if(off){
    off->dev = dev;
    off->blockno = blockno;
    off->valid = 0;
    off->refcnt = 1;
    release(&bcache.lock[index]);
    acquiresleep(&off->lock);
    // printf("replace!\n");
    // release(&bcachelock);
    return off;
  }

  //find free block in other bucket
  acquire(&bcachelock);
  for(int i = 0; i < hashNum; i++){
    if(i == index)
      continue;
    
    acquire(&bcache.lock[i]);
    for(b = bcache.head[i].next, tmp = &bcache.head[i]; b; b=b->next, tmp = tmp->next){
      if(b->refcnt == 0 && b->ticks < minticks){
        off = b;
        minticks = b->ticks;
        bfoff = tmp;
      }
    }

    if(off){
      bfoff->next = off->next;
      off->next = 0;
      pre->next = off;

      off->dev = dev;
      off->blockno = blockno;
      off->valid = 0;
      off->refcnt = 1;
      acquiresleep(&off->lock);

      release(&bcache.lock[index]);
      release(&bcache.lock[i]);
      release(&bcachelock);
      return off;
    }
    
    release(&bcache.lock[i]);
  }

  panic("bget: no buffers");
}

// Return a locked buf with the contents of the indicated block.
struct buf*
bread(uint dev, uint blockno)
{
  struct buf *b;

  b = bget(dev, blockno);
  if(!b->valid) {
    virtio_disk_rw(b, 0);
    b->valid = 1;
  }
  return b;
}

// Write b's contents to disk.  Must be locked.
void
bwrite(struct buf *b)
{
  if(!holdingsleep(&b->lock))
    panic("bwrite");
  virtio_disk_rw(b, 1);
}

// Release a locked buffer.
// Move to the head of the most-recently-used list.
void
brelse(struct buf *b)
{
  if(!holdingsleep(&b->lock))
    panic("brelse");

  releasesleep(&b->lock);

  uint index = hashIndex(b->dev, b->blockno);
  acquire(&bcache.lock[index]);     //avoid 
  b->refcnt--;
  // acquire(&tickslock);
  if(b->refcnt == 0)
    b->ticks = ticks;
  // release(&tickslock);
  release(&bcache.lock[index]);

}

void
bpin(struct buf *b) {
  uint index = hashIndex(b->dev, b->blockno);
  acquire(&bcache.lock[index]);
  b->refcnt++;
  release(&bcache.lock[index]);
}

void
bunpin(struct buf *b) {
  uint index = hashIndex(b->dev, b->blockno);
  acquire(&bcache.lock[index]);
  b->refcnt--;
  release(&bcache.lock[index]);
}


