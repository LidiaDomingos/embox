/**
 * @file
 *
 * @data 26 march 2016
 * @author: Anton Bondarev
 */

#include <assert.h>
#include <stdint.h>

#include <kernel/irq.h>
#include <kernel/printk.h>
#include <drivers/irqctrl.h>
#include <hal/reg.h>
#include <embox/unit.h>
#include <framework/mod/options.h>

#define GIC_CPU_BASE           OPTION_GET(NUMBER, cpu_base_addr)
#define GIC_DISTRIBUTOR_BASE   OPTION_GET(NUMBER, distributor_base_addr)

EMBOX_UNIT_INIT(gic_init);

/* GIC CPU registers */
#define GICC_CTLR          (GIC_CPU_BASE + 0x00)
#define GICC_PMR           (GIC_CPU_BASE + 0x04)
#define GICC_BPR           (GIC_CPU_BASE + 0x08)
#define GICC_IAR           (GIC_CPU_BASE + 0x0C)
#define GICC_EOIR          (GIC_CPU_BASE + 0x10)
#define GICC_RPR           (GIC_CPU_BASE + 0x14)
#define GICC_HPPIR         (GIC_CPU_BASE + 0x18)
#define GICC_ABPR          (GIC_CPU_BASE + 0x1C)

#define GICC_AIAR          (GIC_CPU_BASE + 0x20)
#define GICC_AEOIR         (GIC_CPU_BASE + 0x24)
#define GICC_AHPPIR        (GIC_CPU_BASE + 0x28)

#define GICC_APR(n)       (GIC_CPU_BASE + 0x0D0 + 4 * (n))
#define GICC_NSAPR(n)     (GIC_CPU_BASE + 0x0D0 + 4 * (n))

#define GICC_IIDR          (GIC_CPU_BASE + 0xFC)
#define GICC_DIR           (GIC_CPU_BASE + 0x1000)

/* GIC Distributor registers */
#define GICD_CTLR           (GIC_DISTRIBUTOR_BASE + 0x00)
#define GICD_TYPER          (GIC_DISTRIBUTOR_BASE + 0x04)
#define GICD_IIDR           (GIC_DISTRIBUTOR_BASE + 0x08)

#define BITS_PER_REGISTER     32

#define GICD_IGROUPR(n)      (GIC_DISTRIBUTOR_BASE + 0x080 + 4 * (n))
#define GICD_ISENABLER(n)    (GIC_DISTRIBUTOR_BASE + 0x100 + 4 * (n))
#define GICD_ICENABLER(n)    (GIC_DISTRIBUTOR_BASE + 0x180 + 4 * (n))
#define GICD_ISPENDR(n)      (GIC_DISTRIBUTOR_BASE + 0x200 + 4 * (n))
#define GICD_ICPENDR(n)      (GIC_DISTRIBUTOR_BASE + 0x280 + 4 * (n))

#define GICD_ISACTIVER(n)    (GIC_DISTRIBUTOR_BASE + 0x300 + 4 * (n))
#define GICD_ICACTIVER(n)    (GIC_DISTRIBUTOR_BASE + 0x380 + 4 * (n))
#define GICD_IPRIORITYR(n)   (GIC_DISTRIBUTOR_BASE + 0x400 + 4 * (n))

#define GICD_ITARGETSR(n)    (GIC_DISTRIBUTOR_BASE + 0x800 + 4 * (n))

#define GICD_ITARGETSR(n)    (GIC_DISTRIBUTOR_BASE + 0x800 + 4 * (n))

#define GICD_ICFGR(n)        (GIC_DISTRIBUTOR_BASE + 0xC00 + 4 * (n))
#define GICD_NSACR(n)        (GIC_DISTRIBUTOR_BASE + 0xE00 + 4 * (n))

#define GICD_SGIR            (GIC_DISTRIBUTOR_BASE + 0xF00) /* Software Generated Interrupt Register */
#define GICD_CPENDSGIR(n)    (GIC_DISTRIBUTOR_BASE + 0xF10 + 4 * (n))
#define GICD_SPENDSGIR(n)    (GIC_DISTRIBUTOR_BASE + 0xF20 + 4 * (n))

static int gic_init(void) {
	uint32_t tmp = REG_LOAD(GICD_CTLR);
	REG_STORE(GICD_CTLR, tmp | 0x1);

	tmp = REG_LOAD(GICC_CTLR);
	REG_STORE(GICC_CTLR, tmp | 0x1);

	/* Set priority filter level */
	REG_STORE(GICC_PMR, 0xFF);

	return 0;
}

void irqctrl_enable(unsigned int irq) {
	int n = irq / BITS_PER_REGISTER;
	int m = irq % BITS_PER_REGISTER;

	/* Writing zeroes to this register has no
	 * effect, so we just write single "1" */
	REG_STORE(GICD_ISENABLER(n), 1 << m);
}

void irqctrl_disable(unsigned int irq) {
	int n = irq / BITS_PER_REGISTER;
	int m = irq % BITS_PER_REGISTER;

	/* Writing zeroes to this register has no
	 * effect, so we just write single "1" */
	REG_STORE(GICD_ICENABLER(n), 1 << m);
}

void irqctrl_force(unsigned int irq) {
}

int irqctrl_pending(unsigned int irq) {
	return 0;
}

/* Sends an EOI (end of interrupt) signal to the PICs. */
void irqctrl_eoi(unsigned int irq) {
	REG_STORE(GICC_EOIR, irq);
}

void interrupt_handle(void) {
	//unsigned int stat;
	unsigned int irq;

	irq = REG_LOAD(GICC_IAR);

	irq_dispatch(irq);

	irqctrl_eoi(irq);
#if 0
	stat = REG_LOAD(ICU_IRQSTS);

	assert(!critical_inside(CRITICAL_IRQ_LOCK));
	for (irq = 0; irq < IRQCTRL_IRQS_TOTAL; irq++) {
		if (stat & (uint32_t)(1 << irq))
			break;
	}

	irqctrl_disable(irq);

	critical_enter(CRITICAL_IRQ_HANDLER);
	{
		ipl_enable();

		irq_dispatch(irq);

		ipl_disable();

	}
	irqctrl_enable(irq);
	critical_leave(CRITICAL_IRQ_HANDLER);
	critical_dispatch_pending();
#endif
}

void swi_handle(void) {
	//printk("swi!\n");
}
