/**
*  @defgroup DriverResourceManager 
*  @ingroup DDK
*
*  定义了硬件资源（IO端口，内存）管理接口
*  @{
*/

#ifndef _DDK_RESOURCE_H_
#define _DDK_RESOURCE_H_

#include <ddk/types.h>

typedef phys_addr_t resource_size_t;
#define DR_RESOURCE_TYPE_IO		0
#define DR_RESOURCE_TYPE_MEM	1

/*
	IO端口的上限地址，x86、mips都是0xffff.
	TODO:应该从HAL中获取
*/
#ifndef IO_SPACE_LIMIT
#define IO_SPACE_LIMIT 0xffff
#endif

struct resource
{
	resource_size_t start;
	resource_size_t end;
	const char *name;
	unsigned long flags;
	struct resource *parent, *sibling, *child;
};

#define MAX_RESOURCE ((resource_size_t)~0)

/*
 * IO resources have these defined flags.
 */
#define IORESOURCE_BITS		0x000000ff	/* Bus-specific bits */

#define IORESOURCE_TYPE_BITS	0x00001f00	/* Resource type */
#define IORESOURCE_IO		0x00000100
#define IORESOURCE_MEM		0x00000200
#define IORESOURCE_IRQ		0x00000400
#define IORESOURCE_DMA		0x00000800
#define IORESOURCE_BUS		0x00001000

#define IORESOURCE_PREFETCH	0x00002000	/* No side effects */
#define IORESOURCE_READONLY	0x00004000
#define IORESOURCE_CACHEABLE	0x00008000
#define IORESOURCE_RANGELENGTH	0x00010000
#define IORESOURCE_SHADOWABLE	0x00020000

#define IORESOURCE_SIZEALIGN	0x00040000	/* size indicates alignment */
#define IORESOURCE_STARTALIGN	0x00080000	/* start field is alignment */

#define IORESOURCE_MEM_64	0x00100000
#define IORESOURCE_WINDOW	0x00200000	/* forwarded by bridge */
#define IORESOURCE_MUXED	0x00400000	/* Resource is software muxed */

#define IORESOURCE_EXCLUSIVE	0x08000000	/* Userland may not map this resource */
#define IORESOURCE_DISABLED	0x10000000
#define IORESOURCE_UNSET	0x20000000
#define IORESOURCE_AUTO		0x40000000
#define IORESOURCE_BUSY		0x80000000	/* Driver has marked this resource busy */

/* PnP IRQ specific bits (IORESOURCE_BITS) */
#define IORESOURCE_IRQ_HIGHEDGE		(1<<0)
#define IORESOURCE_IRQ_LOWEDGE		(1<<1)
#define IORESOURCE_IRQ_HIGHLEVEL	(1<<2)
#define IORESOURCE_IRQ_LOWLEVEL		(1<<3)
#define IORESOURCE_IRQ_SHAREABLE	(1<<4)
#define IORESOURCE_IRQ_OPTIONAL 	(1<<5)

/* PnP DMA specific bits (IORESOURCE_BITS) */
#define IORESOURCE_DMA_TYPE_MASK	(3<<0)
#define IORESOURCE_DMA_8BIT		(0<<0)
#define IORESOURCE_DMA_8AND16BIT	(1<<0)
#define IORESOURCE_DMA_16BIT		(2<<0)

#define IORESOURCE_DMA_MASTER		(1<<2)
#define IORESOURCE_DMA_BYTE		(1<<3)
#define IORESOURCE_DMA_WORD		(1<<4)

#define IORESOURCE_DMA_SPEED_MASK	(3<<6)
#define IORESOURCE_DMA_COMPATIBLE	(0<<6)
#define IORESOURCE_DMA_TYPEA		(1<<6)
#define IORESOURCE_DMA_TYPEB		(2<<6)
#define IORESOURCE_DMA_TYPEF		(3<<6)

/* PnP memory I/O specific bits (IORESOURCE_BITS) */
#define IORESOURCE_MEM_WRITEABLE	(1<<0)	/* dup: IORESOURCE_READONLY */
#define IORESOURCE_MEM_CACHEABLE	(1<<1)	/* dup: IORESOURCE_CACHEABLE */
#define IORESOURCE_MEM_RANGELENGTH	(1<<2)	/* dup: IORESOURCE_RANGELENGTH */
#define IORESOURCE_MEM_TYPE_MASK	(3<<3)
#define IORESOURCE_MEM_8BIT		(0<<3)
#define IORESOURCE_MEM_16BIT		(1<<3)
#define IORESOURCE_MEM_8AND16BIT	(2<<3)
#define IORESOURCE_MEM_32BIT		(3<<3)
#define IORESOURCE_MEM_SHADOWABLE	(1<<5)	/* dup: IORESOURCE_SHADOWABLE */
#define IORESOURCE_MEM_EXPANSIONROM	(1<<6)

/* PnP I/O specific bits (IORESOURCE_BITS) */
#define IORESOURCE_IO_16BIT_ADDR	(1<<0)
#define IORESOURCE_IO_FIXED		(1<<1)

/* PCI ROM control bits (IORESOURCE_BITS) */
#define IORESOURCE_ROM_ENABLE		(1<<0)	/* ROM is enabled, same as PCI_ROM_ADDRESS_ENABLE */
#define IORESOURCE_ROM_SHADOW		(1<<1)	/* ROM is copy at C000:0 */
#define IORESOURCE_ROM_COPY		(1<<2)	/* ROM is alloc'd copy, resource field overlaid */
#define IORESOURCE_ROM_BIOS_COPY	(1<<3)	/* ROM is BIOS copy, resource field overlaid */

/* PCI control bits.  Shares IORESOURCE_BITS with above PCI ROM.  */
#define IORESOURCE_PCI_FIXED		(1<<4)	/* Do not move resource */


/**
	@brief 向系统申请注册硬件资源
*/
struct resource * dr_register(int type, phys_addr_t start, resource_size_t size, const char * name, int flags);

/**
	@brief 向系统申请释放硬件资源
*/
void dr_release(int type, resource_size_t start, resource_size_t n);

/**
	@brief 获取几个常用的RESOURCE对象常量
*/
struct resource *dr_get_constance_resource(int type);

/**
	@brief	向资源树中插入一段具体的资源描述

	@param[in] parent 要插入资源的父节点
	@param[in] new 要在父节点上插入的具体资源描述信息

	@return
		成功返回NULL, 失败返回冲突的资源指针
*/
struct resource * dr_insert(struct resource *parent, struct resource *new);

/**
	@brief 调整一个资源在系统中的记录描述

	@return
		0:正确，负数则为错误码
*/
int dr_adjust_resource(struct resource *res, resource_size_t start, resource_size_t size);

/**
	@brief 获取资源的类型
*/
static inline unsigned long dr_type(const struct resource *res)
{
	return res->flags & IORESOURCE_TYPE_BITS;
}
/************************************************************************/
/* 传统驱动直接用的接口，我们要对其转换                                       */
/************************************************************************/
#define request_region(start, n, name)	dr_register(DR_RESOURCE_TYPE_IO, start, n, name, 0)
#define release_region(start, n)		dr_release(DR_RESOURCE_TYPE_IO, start, n)
#define insert_resource(parent, new)	(dr_insert(parent, new) == NULL?0:-EBUSY/*兼容返回值*/)
#define insert_resource_conflict		dr_insert
#define get_ioport_resource()			dr_get_constance_resource(DR_RESOURCE_TYPE_IO)
#define get_iomem_resource()			dr_get_constance_resource(DR_RESOURCE_TYPE_MEM)
#define resource_type					dr_type
#define adjust_resource					dr_adjust_resource
#endif

/** @} */