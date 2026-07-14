# SSD Storage Simulator (C++)

A menu-driven simulator of an SSD's flash translation layer - logical to
physical address mapping, block/page management, wear leveling and
garbage collection - built with core OOP and STL concepts.

## Why this project

Real SSD firmware validation involves testing exactly these behaviors:
read/write correctness, address translation, wear leveling, and garbage
collection under boundary conditions. This project reproduces the
software layer of that logic (no physical hardware involved) so it can
be built, tested, and demoed entirely in software.

## Build & Run

```
make
./ssd_sim
```

Requires a C++17 compiler (g++ recommended).

## Architecture

```
StorageDevice (abstract base)
      |
      +-- SSD   (block/page based, wear leveling, garbage collection, L2P mapping)
      +-- HDD   (simple sequential sector access, no wear leveling)
```

`Controller` owns a `StorageDevice*` and calls `write()/read()/erase()/display()`
through that base pointer - this is where polymorphism actually gets
exercised at runtime, since the same call resolves to completely
different logic depending on whether an SSD or HDD is active.

## Key data structures

| STL structure | Used for |
|---|---|
| `vector<Block>` | All blocks that make up the SSD |
| `vector<Page>` (inside Block) | Pages within a block |
| `unordered_map<int,int>` | Logical address -> physical (block,page) location - the L2P table |
| `queue<int>` | Pending write requests waiting to be flushed |
| `priority_queue` (min-heap on wear count) | Wear leveling - always offers the least worn block first, with lazy deletion for stale entries after a block is erased |
| `sort()` | Displaying blocks ranked by wear in the statistics view |

## Notes on design choices

- A page can only be FREE, VALID, or INVALID. An invalidated page cannot
  be reused for a new write until its whole block is erased - this
  mirrors real NAND flash behavior where in-place overwrite isn't
  possible.
- Garbage collection only kicks in on blocks where more than 50% of
  pages are invalid; valid pages are migrated to another block before
  the original block is erased.
- Wear leveling always tries to allocate from the least-worn block that
  still has a free page, using a min-heap. Since STL's priority_queue
  has no direct way to update/remove an arbitrary entry, stale entries
  (blocks that got erased since being pushed) are just discarded when
  popped - a standard lazy deletion approach.
- HDD exists purely to give the base class pointer somewhere real to
  point at other than SSD, which is what actually makes the
  polymorphism in this project meaningful instead of decorative.

## File format for Save/Load

State is stored in a simple binary layout: block count, pages per
block, then each block's wear count and page states, followed by the
LBA mapping table and read/write/erase counters. See `SSD::saveState`
and `SSD::loadState` in `src/SSD.cpp`.
