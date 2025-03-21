/*
 *  Copyright (C) 2002-2021  The DOSBox Team
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License along
 *  with this program; if not, write to the Free Software Foundation, Inc.,
 *  51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#ifndef DOSBOX_MEM_H
#define DOSBOX_MEM_H

#include "dosbox.h"

#include "byteorder.h"

#define MEM_PAGESIZE        (4096U)

/* HostPt and ConstHostPt is for holding linear addresses within this emulator i.e. a normal pointer.
 *
 * PhysPt is for 32-bit physical memory addresses within the emulation environment.
 * PhysPt64 is for 64-bit physical memory addresses for code and device/memory emulation that supports addresses above 4GB.
 * LinearPt is a 32-bit linear memory address from the point of view of the CPU execution context, meaning the linear address
 *   of the code prior to translation through the page tables to physical addresses.
 * RealPt is a 32-bit value that holds segment in the upper 16 bits, offset in the lower 16 bits.
 *
 * Please do not mix these types up in the code, even if they happen to have the same underlying data types */

typedef uint8_t const *       ConstHostPt;        /* host (virtual) memory address aka ptr */
typedef uint8_t *             HostPt;             /* host (virtual) memory address aka ptr */

typedef uint32_t              PhysPt;             /* guest physical memory pointer */
typedef uint32_t              LinearPt;           /* guest linear memory address */
typedef uint32_t              RealPt;             /* guest real-mode memory address (16:16 -> seg:offset) */
typedef uint16_t              SegmentVal;         /* guest segment value */
typedef uint32_t              PageNum;            /* page frame number */

typedef uint64_t              PhysPt64;           /* guest physical memory pointer */

typedef int32_t               MemHandle;

extern HostPt                 MemBase;
extern size_t                 MemSize;

HostPt                      GetMemBase(void);
bool                        MEM_A20_Enabled(void);
void                        MEM_A20_Enable(bool enabled);

/* Memory management / EMS mapping */
Bitu                        MEM_FreeTotal(void);           //Free 4 kb pages
Bitu                        MEM_FreeLargest(void);         //Largest free 4 kb pages block
Bitu                        MEM_TotalPages(void);          //Total amount of 4 kb pages
Bitu                        MEM_TotalPagesAt4GB(void);          //Total amount of 4 kb pages starting at 4GB
Bitu                        MEM_ConventionalPages(void);
Bitu                        MEM_AllocatedPages(MemHandle handle); // amount of allocated pages of handle
MemHandle                   MEM_AllocatePages(Bitu pages,bool sequence);
MemHandle                   MEM_AllocatePages_A20_friendly(Bitu pages,bool sequence);
MemHandle                   MEM_GetNextFreePage(void);
void                        MEM_ReleasePages(MemHandle handle);
bool                        MEM_ReAllocatePages(MemHandle & handle,Bitu pages,bool sequence);

MemHandle                   MEM_NextHandle(MemHandle handle);
MemHandle                   MEM_NextHandleAt(MemHandle handle,Bitu where);

uint32_t                    MEM_HardwareAllocate(const char *name,uint32_t sz);

static constexpr bool build_memlimit_32bit(void) {
	return sizeof(void*) < 8;
}

/* 
    The following six functions are used everywhere in the end so these should be changed for
    Working on big or little endian machines 
*/

static INLINE uint8_t host_readb(ConstHostPt const off) {
    return *off;
}

static INLINE void host_writeb(HostPt const off,const uint8_t val) {
    *off = val;
}

// use __builtin_bswap* for gcc >= 4.3
#if defined(WORDS_BIGENDIAN) && defined(__GNUC__) && \
    (__GNUC__ > 4 || (__GNUC__ == 4 && __GNUC_MINOR__ >= 3))

static INLINE uint16_t host_readw(ConstHostPt off) {
    return __builtin_bswap16(*(uint16_t *)off);
}
static INLINE uint32_t host_readd(ConstHostPt off) {
    return __builtin_bswap32(*(uint32_t *)off);
}
static INLINE void host_writew(HostPt const off, const uint16_t val) {
    *(uint16_t *)off = __builtin_bswap16(val);
}
static INLINE void host_writed(HostPt const off, const uint32_t val) {
    *(uint32_t *)off = __builtin_bswap32(val);
}
static INLINE void host_writeq(HostPt const off, const uint64_t val) {
    *(uint64_t *)off = __builtin_bswap64(val);
}
#elif !defined(C_UNALIGNED_MEMORY)
/* !defined(C_UNALIGNED_MEMORY) meaning: we're probably being compiled for a processor that doesn't like unaligned WORD access,
        on such processors typecasting memory as uint16_t and higher can cause a fault if the
        address is not aligned to that datatype when we read/write through it. */

static INLINE uint16_t host_readw(ConstHostPt const off) {
    return (uint16_t)host_readb(off) + ((uint16_t)host_readb(off+(uintptr_t)1U) << (uint16_t)8U);
}
static INLINE uint32_t host_readd(ConstHostPt const off) {
    return (uint32_t)host_readw(off) + ((uint32_t)host_readw(off+(uintptr_t)2U) << (uint32_t)16U);
}
static INLINE uint64_t host_readq(ConstHostPt const off) {
    return (uint64_t)host_readd(off) + ((uint64_t)host_readd(off+(uintptr_t)4U) << (uint64_t)32U);
}


static INLINE void host_writew(HostPt const off,const uint16_t val) {
    host_writeb(off,   (uint8_t)(val));
    host_writeb(off+1U,(uint8_t)(val >> (uint16_t)8U));
}
static INLINE void host_writed(HostPt const off,const uint32_t val) {
    host_writew(off,   (uint16_t)(val));
    host_writew(off+2U,(uint16_t)(val >> (uint32_t)16U));
}
static INLINE void host_writeq(HostPt const off,const uint64_t val) {
    host_writed(off,   (uint32_t)(val));
    host_writed(off+4U,(uint32_t)(val >> (uint64_t)32U));
}

#else

static INLINE uint16_t host_readw(ConstHostPt const off) {
    return le16toh((*(const uint16_t *)off)); // BSD endian.h
}
static INLINE uint32_t host_readd(ConstHostPt const off) {
    return le32toh((*(const uint32_t *)off)); // BSD endian.h
}
static INLINE uint64_t host_readq(ConstHostPt const off) {
    return le64toh((*(const uint64_t *)off)); // BSD endian.h
}

static INLINE void host_writew(HostPt const off,const uint16_t val) {
    *(uint16_t *)(off) = htole16(val);
}
static INLINE void host_writed(HostPt const off,const uint32_t val) {
    *(uint32_t *)(off) = htole32(val);
}
static INLINE void host_writeq(HostPt const off,const uint64_t val) {
    *(uint64_t *)(off) = htole64(val);
}

#endif


static INLINE void var_write(uint8_t * const var, const uint8_t val) {
    host_writeb(var, val);
}

static INLINE void var_write(uint16_t * const var, const uint16_t val) {
    host_writew((HostPt)var, val);
}

static INLINE void var_write(uint32_t * const var, const uint32_t val) {
    host_writed((HostPt)var, val);
}

static INLINE void var_write(uint64_t * const var, const uint64_t val) {
    host_writeq((HostPt)var, val);
}

static INLINE uint16_t var_read(uint16_t * var) {
    return host_readw((ConstHostPt)var);
}

static INLINE uint32_t var_read(uint32_t * var) {
    return host_readd((ConstHostPt)var);
}

/* The Following six functions are slower but they recognize the paged memory system */

uint8_t  mem_readb(const LinearPt address);
uint16_t mem_readw(const LinearPt address);
uint32_t mem_readd(const LinearPt address);

void mem_writeb(const LinearPt address,const uint8_t val);
void mem_writew(const LinearPt address,const uint16_t val);
void mem_writed(const LinearPt address,const uint32_t val);

void phys_writes(PhysPt addr, const char* string, Bitu length);

/* WARNING: These will cause a segfault or out of bounds access IF
 *          addr is beyond the end of memory */
/* 2024/12/22: Looking across the DOSBox-X codebase, these functions
 *             aren't used TOO often, and where they are used, some
 *             code has memory range checks anyway. So it doesn't hurt
 *             performance very much if at all to just put the range
 *             check here instead, in order not to segfault if somehow
 *             the address given is beyond end of system memory. --J.C.
 *
 *             There is one more important detail here. These functions
 *             take only a 32-bit physical address. Which means if more
 *             than 32 address bits are enabled on the CPU and the OS
 *             has PSE/PAE page tables enabled, these functions will not
 *             be able to reach above 4GB. Given how memory will be
 *             segmented between the 'below 4GB' and 'above 4GB' regions,
 *             if emulating 4GB or more, that is perfectly fine. S3 XGA
 *             and ISA DMA emulation will never reach above 4GB anyway.
 *
 *             The way the range check is done is ideal for performance,
 *             yet may fail to work correctly if MemSize is very close
 *             to zero, low enough that subtraction would cause it to
 *             wrap back around to the largest possible value. The code,
 *             when MemBase is a valid pointer, will never set MemSize
 *             that small. */
/* NTS: Technically these phys_ functions are misnamed. They don't read/write
 *      all physically addressable memory and MMIO attached to the CPU. These
 *      functions can only address system RAM. So perhaps they should be named
 *      physmem_read/physmem_write instead and a physmmio_read/physmmio_write
 *      function should be added that addresses the "physical" memory addresses
 *      accessible to the CPU. That hackery in the debugger to dump by physical
 *      memory addresse could be a useful guide on how to do that. --J.C. */

static INLINE void phys_writeb(const PhysPt addr,const uint8_t val) {
    if (addr < MemSize)
        host_writeb(MemBase+addr,val);
}
static INLINE void phys_writew(const PhysPt addr,const uint16_t val) {
    if (addr < (MemSize-1u))
        host_writew(MemBase+addr,val);
}
static INLINE void phys_writed(const PhysPt addr,const uint32_t val) {
    if (addr < (MemSize-3u))
        host_writed(MemBase+addr,val);
}

static INLINE uint8_t phys_readb(const PhysPt addr) {
    if (addr < MemSize)
        return host_readb(MemBase+addr);
    else
        return 0xFF;
}
static INLINE uint16_t phys_readw(const PhysPt addr) {
    if (addr < (MemSize-1u))
        return host_readw(MemBase+addr);
    else
        return 0xFFFFu;
}
static INLINE uint32_t phys_readd(const PhysPt addr) {
    if (addr < (MemSize-3u))
        return host_readd(MemBase+addr);
    else
        return 0xFFFFFFFFu;
}

/* These don't check for alignment, better be sure it's correct */

void MEM_BlockWrite(LinearPt pt, const void *data, size_t size);
void MEM_BlockRead(LinearPt pt,void * data,Bitu size);
void MEM_BlockWrite32(LinearPt pt,void * data,Bitu size);
void MEM_BlockRead32(LinearPt pt,void * data,Bitu size);
void MEM_BlockCopy(LinearPt dest,LinearPt src,Bitu size);
void MEM_StrCopy(LinearPt pt,char * data,Bitu size);

void mem_memcpy(LinearPt dest,LinearPt src,Bitu size);
Bitu mem_strlen(LinearPt pt);
void mem_strcpy(LinearPt dest,LinearPt src);

/* The following functions are all shortcuts to the above functions using physical addressing */

static inline constexpr LinearPt PhysMake(const uint16_t seg,const uint16_t off) {
    return ((LinearPt)seg << 4U) + (LinearPt)off;
}

static inline constexpr uint16_t RealSeg(const RealPt pt) {
    return (uint16_t)(pt >> 16U);
}

static inline constexpr uint16_t RealOff(const RealPt pt) {
    return (uint16_t)(pt & 0xffffu);
}

static inline constexpr LinearPt Real2Phys(const RealPt pt) {
    return (((LinearPt)RealSeg(pt) << 4U) + (LinearPt)RealOff(pt));
}

static inline constexpr RealPt RealMake(const uint16_t seg,const uint16_t off) {
    return (((RealPt)seg << 16U) + (RealPt)off);
}

/* convert physical address to 4:16 real pointer (example: 0xABCDE -> 0xA000:0xBCDE) */
static inline constexpr RealPt PhysToReal416(const LinearPt phys) {
    return RealMake((uint16_t)((phys >> 4U) & 0xF000U),(uint16_t)(phys & 0xFFFFU));
}

static inline constexpr LinearPt RealVecAddress(const uint8_t vec) {
    return ((unsigned int)vec << 2U);
}


static INLINE uint8_t real_readb(const uint16_t seg,const uint16_t off) {
    return mem_readb(PhysMake(seg,off));
}
static INLINE uint16_t real_readw(const uint16_t seg,const uint16_t off) {
    return mem_readw(PhysMake(seg,off));
}
static INLINE uint32_t real_readd(const uint16_t seg,const uint16_t off) {
    return mem_readd(PhysMake(seg,off));
}

static INLINE void real_writeb(const uint16_t seg,const uint16_t off,const uint8_t val) {
    mem_writeb(PhysMake(seg,off),val);
}
static INLINE void real_writew(const uint16_t seg,const uint16_t off,const uint16_t val) {
    mem_writew(PhysMake(seg,off),val);
}
static INLINE void real_writed(const uint16_t seg,const uint16_t off,const uint32_t val) {
    mem_writed(PhysMake(seg,off),val);
}


static INLINE RealPt RealGetVec(const uint8_t vec) {
    return mem_readd(RealVecAddress(vec));
}

static INLINE void RealSetVec(const uint8_t vec,const RealPt pt) {
    mem_writed(RealVecAddress(vec),pt);
}

static INLINE void RealSetVec(const uint8_t vec,const RealPt pt,RealPt &old) {
    const LinearPt addr = RealVecAddress(vec);
    old = mem_readd(addr);
    mem_writed(addr,pt);
}

uint8_t physdev_readb(const PhysPt64 addr);
uint16_t physdev_readw(const PhysPt64 addr);
uint32_t physdev_readd(const PhysPt64 addr);
void physdev_writeb(const PhysPt64 addr,const uint8_t val);
void physdev_writew(const PhysPt64 addr,const uint16_t val);
void physdev_writed(const PhysPt64 addr,const uint32_t val);

uint32_t MEM_get_address_bits();
uint32_t MEM_get_address_bits4GB();

void MEM_ResetPageHandler_Unmapped(Bitu phys_page, Bitu pages);

#endif
