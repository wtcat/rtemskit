// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (c) 2011 The Chromium OS Authors.
 */

#include <rtems/malloc.h>
#include <rtems/sysinit.h>
#include <bsp/fdt.h>

#include <common.h>
#include <dm.h>
#include <log.h>
#include <dm/of_extra.h>
#include <errno.h>
#include <fdtdec.h>
#include <fdt_support.h>
#include <of_live.h>
#include <linux/libfdt.h>
#include <asm/sections.h>
#include <linux/ctype.h>
#include <linux/lzo.h>
#include <linux/ioport.h>

const void *_dm_fdt_blob;
/*
 * Here are the type we know about. One day we might allow drivers to
 * register. For now we just put them here. The COMPAT macro allows us to
 * turn this into a sparse list later, and keeps the ID with the name.
 *
 * NOTE: This list is basically a TODO list for things that need to be
 * converted to driver model. So don't add new things here unless there is a
 * good reason why driver-model conversion is infeasible. Examples include
 * things which are used before driver model is available.
 */
#define COMPAT(id, name) name
static const char * const compat_names[COMPAT_COUNT] = {
	COMPAT(UNKNOWN, "<none>"),
	COMPAT(NVIDIA_TEGRA20_EMC, "nvidia,tegra20-emc"),
	COMPAT(NVIDIA_TEGRA20_EMC_TABLE, "nvidia,tegra20-emc-table"),
	COMPAT(NVIDIA_TEGRA20_NAND, "nvidia,tegra20-nand"),
	COMPAT(NVIDIA_TEGRA124_XUSB_PADCTL, "nvidia,tegra124-xusb-padctl"),
	COMPAT(NVIDIA_TEGRA210_XUSB_PADCTL, "nvidia,tegra210-xusb-padctl"),
	COMPAT(SMSC_LAN9215, "smsc,lan9215"),
	COMPAT(SAMSUNG_EXYNOS5_SROMC, "samsung,exynos-sromc"),
	COMPAT(SAMSUNG_EXYNOS_USB_PHY, "samsung,exynos-usb-phy"),
	COMPAT(SAMSUNG_EXYNOS5_USB3_PHY, "samsung,exynos5250-usb3-phy"),
	COMPAT(SAMSUNG_EXYNOS_TMU, "samsung,exynos-tmu"),
	COMPAT(SAMSUNG_EXYNOS_MIPI_DSI, "samsung,exynos-mipi-dsi"),
	COMPAT(SAMSUNG_EXYNOS_DWMMC, "samsung,exynos-dwmmc"),
	COMPAT(GENERIC_SPI_FLASH, "jedec,spi-nor"),
	COMPAT(SAMSUNG_EXYNOS_SYSMMU, "samsung,sysmmu-v3.3"),
	COMPAT(INTEL_MICROCODE, "intel,microcode"),
	COMPAT(INTEL_QRK_MRC, "intel,quark-mrc"),
	COMPAT(ALTERA_SOCFPGA_DWMAC, "altr,socfpga-stmmac"),
	COMPAT(ALTERA_SOCFPGA_DWMMC, "altr,socfpga-dw-mshc"),
	COMPAT(ALTERA_SOCFPGA_DWC2USB, "snps,dwc2"),
	COMPAT(INTEL_BAYTRAIL_FSP, "intel,baytrail-fsp"),
	COMPAT(INTEL_BAYTRAIL_FSP_MDP, "intel,baytrail-fsp-mdp"),
	COMPAT(INTEL_IVYBRIDGE_FSP, "intel,ivybridge-fsp"),
	COMPAT(COMPAT_SUNXI_NAND, "allwinner,sun4i-a10-nand"),
	COMPAT(ALTERA_SOCFPGA_CLK, "altr,clk-mgr"),
	COMPAT(ALTERA_SOCFPGA_PINCTRL_SINGLE, "pinctrl-single"),
	COMPAT(ALTERA_SOCFPGA_H2F_BRG, "altr,socfpga-hps2fpga-bridge"),
	COMPAT(ALTERA_SOCFPGA_LWH2F_BRG, "altr,socfpga-lwhps2fpga-bridge"),
	COMPAT(ALTERA_SOCFPGA_F2H_BRG, "altr,socfpga-fpga2hps-bridge"),
	COMPAT(ALTERA_SOCFPGA_F2SDR0, "altr,socfpga-fpga2sdram0-bridge"),
	COMPAT(ALTERA_SOCFPGA_F2SDR1, "altr,socfpga-fpga2sdram1-bridge"),
	COMPAT(ALTERA_SOCFPGA_F2SDR2, "altr,socfpga-fpga2sdram2-bridge"),
	COMPAT(ALTERA_SOCFPGA_FPGA0, "altr,socfpga-a10-fpga-mgr"),
	COMPAT(ALTERA_SOCFPGA_NOC, "altr,socfpga-a10-noc"),
	COMPAT(ALTERA_SOCFPGA_CLK_INIT, "altr,socfpga-a10-clk-init")
};

const char *fdtdec_get_compatible(enum fdt_compat_id id)
{
	/* We allow reading of the 'unknown' ID for testing purposes */
	assert(id >= 0 && id < COMPAT_COUNT);
	return compat_names[id];
}

fdt_addr_t fdtdec_get_addr_size_fixed(const void *blob, int node,
				      const char *prop_name, int index, int na,
				      int ns, fdt_size_t *sizep,
				      bool translate)
{
	const fdt32_t *prop, *prop_end;
	const fdt32_t *prop_addr, *prop_size, *prop_after_size;
	int len;
	fdt_addr_t addr;

	debug("%s: %s: ", __func__, prop_name);

	prop = fdt_getprop(blob, node, prop_name, &len);
	if (!prop) {
		debug("(not found)\n");
		return FDT_ADDR_T_NONE;
	}
	prop_end = prop + (len / sizeof(*prop));

	prop_addr = prop + (index * (na + ns));
	prop_size = prop_addr + na;
	prop_after_size = prop_size + ns;
	if (prop_after_size > prop_end) {
		debug("(not enough data: expected >= %d cells, got %d cells)\n",
		      (u32)(prop_after_size - prop), ((u32)(prop_end - prop)));
		return FDT_ADDR_T_NONE;
	}

#if CONFIG_IS_ENABLED(OF_TRANSLATE)
	if (translate)
		addr = fdt_translate_address(blob, node, prop_addr);
	else
#endif
		addr = fdtdec_get_number(prop_addr, na);

	if (sizep) {
		*sizep = fdtdec_get_number(prop_size, ns);
		debug("addr=%08llx, size=%llx\n", (unsigned long long)addr,
		      (unsigned long long)*sizep);
	} else {
		debug("addr=%08llx\n", (unsigned long long)addr);
	}

	return addr;
}

fdt_addr_t fdtdec_get_addr_size_auto_parent(const void *blob, int parent,
					    int node, const char *prop_name,
					    int index, fdt_size_t *sizep,
					    bool translate)
{
	int na, ns;

	debug("%s: ", __func__);

	na = fdt_address_cells(blob, parent);
	if (na < 1) {
		debug("(bad #address-cells)\n");
		return FDT_ADDR_T_NONE;
	}

	ns = fdt_size_cells(blob, parent);
	if (ns < 0) {
		debug("(bad #size-cells)\n");
		return FDT_ADDR_T_NONE;
	}

	debug("na=%d, ns=%d, ", na, ns);

	return fdtdec_get_addr_size_fixed(blob, node, prop_name, index, na,
					  ns, sizep, translate);
}

fdt_addr_t fdtdec_get_addr_size_auto_noparent(const void *blob, int node,
					      const char *prop_name, int index,
					      fdt_size_t *sizep,
					      bool translate)
{
	int parent;

	debug("%s: ", __func__);

	parent = fdt_parent_offset(blob, node);
	if (parent < 0) {
		debug("(no parent found)\n");
		return FDT_ADDR_T_NONE;
	}

	return fdtdec_get_addr_size_auto_parent(blob, parent, node, prop_name,
						index, sizep, translate);
}

fdt_addr_t fdtdec_get_addr_size(const void *blob, int node,
				const char *prop_name, fdt_size_t *sizep)
{
	int ns = sizep ? (sizeof(fdt_size_t) / sizeof(fdt32_t)) : 0;

	return fdtdec_get_addr_size_fixed(blob, node, prop_name, 0,
					  sizeof(fdt_addr_t) / sizeof(fdt32_t),
					  ns, sizep, false);
}

fdt_addr_t fdtdec_get_addr(const void *blob, int node, const char *prop_name)
{
	return fdtdec_get_addr_size(blob, node, prop_name, NULL);
}

#if CONFIG_IS_ENABLED(PCI) && defined(CONFIG_DM_PCI)
int fdtdec_get_pci_vendev(const void *blob, int node, u16 *vendor, u16 *device)
{
	const char *list, *end;
	int len;

	list = fdt_getprop(blob, node, "compatible", &len);
	if (!list)
		return -ENOENT;

	end = list + len;
	while (list < end) {
		len = strlen(list);
		if (len >= strlen("pciVVVV,DDDD")) {
			char *s = strstr(list, "pci");

			/*
			 * check if the string is something like pciVVVV,DDDD.RR
			 * or just pciVVVV,DDDD
			 */
			if (s && s[7] == ',' &&
			    (s[12] == '.' || s[12] == 0)) {
				s += 3;
				*vendor = simple_strtol(s, NULL, 16);

				s += 5;
				*device = simple_strtol(s, NULL, 16);

				return 0;
			}
		}
		list += (len + 1);
	}

	return -ENOENT;
}

int fdtdec_get_pci_bar32(const struct udevice *dev, struct fdt_pci_addr *addr,
			 u32 *bar)
{
	int barnum;

	/* extract the bar number from fdt_pci_addr */
	barnum = addr->phys_hi & 0xff;
	if (barnum < PCI_BASE_ADDRESS_0 || barnum > PCI_CARDBUS_CIS)
		return -EINVAL;

	barnum = (barnum - PCI_BASE_ADDRESS_0) / 4;
	*bar = dm_pci_read_bar32(dev, barnum);

	return 0;
}

int fdtdec_get_pci_bus_range(const void *blob, int node,
			     struct fdt_resource *res)
{
	const u32 *values;
	int len;

	values = fdt_getprop(blob, node, "bus-range", &len);
	if (!values || len < sizeof(*values) * 2)
		return -EINVAL;

	res->start = fdt32_to_cpu(*values++);
	res->end = fdt32_to_cpu(*values);

	return 0;
}
#endif

uint64_t fdtdec_get_uint64(const void *blob, int node, const char *prop_name,
			   uint64_t default_val)
{
	const unaligned_fdt64_t *cell64;
	int length;

	cell64 = fdt_getprop(blob, node, prop_name, &length);
	if (!cell64 || length < sizeof(*cell64))
		return default_val;

	return fdt64_to_cpu(*cell64);
}

int fdtdec_get_is_enabled(const void *blob, int node)
{
	const char *cell;

	/*
	 * It should say "okay", so only allow that. Some fdts use "ok" but
	 * this is a bug. Please fix your device tree source file. See here
	 * for discussion:
	 *
	 * http://www.mail-archive.com/u-boot@lists.denx.de/msg71598.html
	 */
	cell = fdt_getprop(blob, node, "status", NULL);
	if (cell)
		return strcmp(cell, "okay") == 0;
	return 1;
}

enum fdt_compat_id fdtdec_lookup(const void *blob, int node)
{
	enum fdt_compat_id id;

	/* Search our drivers */
	for (id = COMPAT_UNKNOWN; id < COMPAT_COUNT; id++)
		if (fdt_node_check_compatible(blob, node,
					      compat_names[id]) == 0)
			return id;
	return COMPAT_UNKNOWN;
}

int fdtdec_next_compatible(const void *blob, int node, enum fdt_compat_id id)
{
	return fdt_node_offset_by_compatible(blob, node, compat_names[id]);
}

int fdtdec_next_compatible_subnode(const void *blob, int node,
				   enum fdt_compat_id id, int *depthp)
{
	do {
		node = fdt_next_node(blob, node, depthp);
	} while (*depthp > 1);

	/* If this is a direct subnode, and compatible, return it */
	if (*depthp == 1 && 0 == fdt_node_check_compatible(
						blob, node, compat_names[id]))
		return node;

	return -FDT_ERR_NOTFOUND;
}

int fdtdec_next_alias(const void *blob, const char *name, enum fdt_compat_id id,
		      int *upto)
{
#define MAX_STR_LEN 20
	char str[MAX_STR_LEN + 20];
	int node, err;

	/* snprintf() is not available */
	assert(strlen(name) < MAX_STR_LEN);
	sprintf(str, "%.*s%d", MAX_STR_LEN, name, *upto);
	node = fdt_path_offset(blob, str);
	if (node < 0)
		return node;
	err = fdt_node_check_compatible(blob, node, compat_names[id]);
	if (err < 0)
		return err;
	if (err)
		return -FDT_ERR_NOTFOUND;
	(*upto)++;
	return node;
}

int fdtdec_find_aliases_for_id(const void *blob, const char *name,
			       enum fdt_compat_id id, int *node_list,
			       int maxcount)
{
	memset(node_list, '\0', sizeof(*node_list) * maxcount);

	return fdtdec_add_aliases_for_id(blob, name, id, node_list, maxcount);
}

/* TODO: Can we tighten this code up a little? */
int fdtdec_add_aliases_for_id(const void *blob, const char *name,
			      enum fdt_compat_id id, int *node_list,
			      int maxcount)
{
	int name_len = strlen(name);
	int nodes[maxcount];
	int num_found = 0;
	int offset, node;
	int alias_node;
	int count;
	int i, j;

	/* find the alias node if present */
	alias_node = fdt_path_offset(blob, "/aliases");

	/*
	 * start with nothing, and we can assume that the root node can't
	 * match
	 */
	memset(nodes, '\0', sizeof(nodes));

	/* First find all the compatible nodes */
	for (node = count = 0; node >= 0 && count < maxcount;) {
		node = fdtdec_next_compatible(blob, node, id);
		if (node >= 0)
			nodes[count++] = node;
	}
	if (node >= 0)
		debug("%s: warning: maxcount exceeded with alias '%s'\n",
		      __func__, name);

	/* Now find all the aliases */
	for (offset = fdt_first_property_offset(blob, alias_node);
			offset > 0;
			offset = fdt_next_property_offset(blob, offset)) {
		const struct fdt_property *prop;
		const char *path;
		int number;
		int found;

		node = 0;
		prop = fdt_get_property_by_offset(blob, offset, NULL);
		path = fdt_string(blob, fdt32_to_cpu(prop->nameoff));
		if (prop->len && 0 == strncmp(path, name, name_len))
			node = fdt_path_offset(blob, prop->data);
		if (node <= 0)
			continue;

		/* Get the alias number */
		number = simple_strtoul(path + name_len, NULL, 10);
		if (number < 0 || number >= maxcount) {
			debug("%s: warning: alias '%s' is out of range\n",
			      __func__, path);
			continue;
		}

		/* Make sure the node we found is actually in our list! */
		found = -1;
		for (j = 0; j < count; j++)
			if (nodes[j] == node) {
				found = j;
				break;
			}

		if (found == -1) {
			debug("%s: warning: alias '%s' points to a node "
				"'%s' that is missing or is not compatible "
				" with '%s'\n", __func__, path,
				fdt_get_name(blob, node, NULL),
			       compat_names[id]);
			continue;
		}

		/*
		 * Add this node to our list in the right place, and mark
		 * it as done.
		 */
		if (fdtdec_get_is_enabled(blob, node)) {
			if (node_list[number]) {
				debug("%s: warning: alias '%s' requires that "
				      "a node be placed in the list in a "
				      "position which is already filled by "
				      "node '%s'\n", __func__, path,
				      fdt_get_name(blob, node, NULL));
				continue;
			}
			node_list[number] = node;
			if (number >= num_found)
				num_found = number + 1;
		}
		nodes[found] = 0;
	}

	/* Add any nodes not mentioned by an alias */
	for (i = j = 0; i < maxcount; i++) {
		if (!node_list[i]) {
			for (; j < maxcount; j++)
				if (nodes[j] &&
				    fdtdec_get_is_enabled(blob, nodes[j]))
					break;

			/* Have we run out of nodes to add? */
			if (j == maxcount)
				break;

			assert(!node_list[i]);
			node_list[i] = nodes[j++];
			if (i >= num_found)
				num_found = i + 1;
		}
	}

	return num_found;
}

int fdtdec_get_alias_seq(const void *blob, const char *base, int offset,
			 int *seqp)
{
	int base_len = strlen(base);
	const char *find_name;
	int find_namelen;
	int prop_offset;
	int aliases;

	find_name = fdt_get_name(blob, offset, &find_namelen);
	debug("Looking for '%s' at %d, name %s\n", base, offset, find_name);

	aliases = fdt_path_offset(blob, "/aliases");
	for (prop_offset = fdt_first_property_offset(blob, aliases);
	     prop_offset > 0;
	     prop_offset = fdt_next_property_offset(blob, prop_offset)) {
		const char *prop;
		const char *name;
		const char *slash;
		int len, val;

		prop = fdt_getprop_by_offset(blob, prop_offset, &name, &len);
		debug("   - %s, %s\n", name, prop);
		if (len < find_namelen || *prop != '/' || prop[len - 1] ||
		    strncmp(name, base, base_len))
			continue;

		slash = strrchr(prop, '/');
		if (strcmp(slash + 1, find_name))
			continue;

		/*
		 * Adding an extra check to distinguish DT nodes with
		 * same name
		 */
		if (IS_ENABLED(CONFIG_PHANDLE_CHECK_SEQ)) {
			if (fdt_get_phandle(blob, offset) !=
			    fdt_get_phandle(blob, fdt_path_offset(blob, prop)))
				continue;
		}

		val = trailing_strtol(name);
		if (val != -1) {
			*seqp = val;
			debug("Found seq %d\n", *seqp);
			return 0;
		}
	}

	debug("Not found\n");
	return -ENOENT;
}

int fdtdec_get_alias_highest_id(const void *blob, const char *base)
{
	int base_len = strlen(base);
	int prop_offset;
	int aliases;
	int max = -1;

	debug("Looking for highest alias id for '%s'\n", base);

	aliases = fdt_path_offset(blob, "/aliases");
	for (prop_offset = fdt_first_property_offset(blob, aliases);
	     prop_offset > 0;
	     prop_offset = fdt_next_property_offset(blob, prop_offset)) {
		const char *prop;
		const char *name;
		int len, val;

		prop = fdt_getprop_by_offset(blob, prop_offset, &name, &len);
		debug("   - %s, %s\n", name, prop);
		if (*prop != '/' || prop[len - 1] ||
		    strncmp(name, base, base_len))
			continue;

		val = trailing_strtol(name);
		if (val > max) {
			debug("Found seq %d\n", val);
			max = val;
		}
	}

	return max;
}

const char *fdtdec_get_chosen_prop(const void *blob, const char *name)
{
	int chosen_node;

	if (!blob)
		return NULL;
	chosen_node = fdt_path_offset(blob, "/chosen");
	return fdt_getprop(blob, chosen_node, name, NULL);
}

int fdtdec_get_chosen_node(const void *blob, const char *name)
{
	const char *prop;

	prop = fdtdec_get_chosen_prop(blob, name);
	if (!prop)
		return -FDT_ERR_NOTFOUND;
	return fdt_path_offset(blob, prop);
}

int fdtdec_check_fdt(void)
{
	/*
	 * We must have an FDT, but we cannot panic() yet since the console
	 * is not ready. So for now, just assert(). Boards which need an early
	 * FDT (prior to console ready) will need to make their own
	 * arrangements and do their own checks.
	 */
	assert(!fdtdec_prepare_fdt());
	return 0;
}

/*
 * This function is a little odd in that it accesses global data. At some
 * point if the architecture board.c files merge this will make more sense.
 * Even now, it is common code.
 */
int fdtdec_prepare_fdt(void)
{
	if (!_dm_fdt_blob || ((uintptr_t)_dm_fdt_blob & 3) ||
	    fdt_check_header(_dm_fdt_blob)) {
		printk("No valid device tree binary found at %p\n",
		       _dm_fdt_blob);
# ifdef DEBUG
		if (_dm_fdt_blob) {
			printf("fdt_blob=%p\n", _dm_fdt_blob);
			print_buffer((ulong)_dm_fdt_blob, _dm_fdt_blob, 4,
				     32, 0);
		}
# endif
		return -1;
	}
	return 0;
}

int fdtdec_lookup_phandle(const void *blob, int node, const char *prop_name)
{
	const u32 *phandle;
	int lookup;

	debug("%s: %s\n", __func__, prop_name);
	phandle = fdt_getprop(blob, node, prop_name, NULL);
	if (!phandle)
		return -FDT_ERR_NOTFOUND;

	lookup = fdt_node_offset_by_phandle(blob, fdt32_to_cpu(*phandle));
	return lookup;
}

/**
 * Look up a property in a node and check that it has a minimum length.
 *
 * @param blob		FDT blob
 * @param node		node to examine
 * @param prop_name	name of property to find
 * @param min_len	minimum property length in bytes
 * @param err		0 if ok, or -FDT_ERR_NOTFOUND if the property is not
			found, or -FDT_ERR_BADLAYOUT if not enough data
 * @return pointer to cell, which is only valid if err == 0
 */
static const void *get_prop_check_min_len(const void *blob, int node,
					  const char *prop_name, int min_len,
					  int *err)
{
	const void *cell;
	int len;

	debug("%s: %s\n", __func__, prop_name);
	cell = fdt_getprop(blob, node, prop_name, &len);
	if (!cell)
		*err = -FDT_ERR_NOTFOUND;
	else if (len < min_len)
		*err = -FDT_ERR_BADLAYOUT;
	else
		*err = 0;
	return cell;
}

int fdtdec_get_int_array(const void *blob, int node, const char *prop_name,
			 u32 *array, int count)
{
	const u32 *cell;
	int err = 0;

	debug("%s: %s\n", __func__, prop_name);
	cell = get_prop_check_min_len(blob, node, prop_name,
				      sizeof(u32) * count, &err);
	if (!err) {
		int i;

		for (i = 0; i < count; i++)
			array[i] = fdt32_to_cpu(cell[i]);
	}
	return err;
}

int fdtdec_get_int_array_count(const void *blob, int node,
			       const char *prop_name, u32 *array, int count)
{
	const u32 *cell;
	int len, elems;
	int i;

	debug("%s: %s\n", __func__, prop_name);
	cell = fdt_getprop(blob, node, prop_name, &len);
	if (!cell)
		return -FDT_ERR_NOTFOUND;
	elems = len / sizeof(u32);
	if (count > elems)
		count = elems;
	for (i = 0; i < count; i++)
		array[i] = fdt32_to_cpu(cell[i]);

	return count;
}

const u32 *fdtdec_locate_array(const void *blob, int node,
			       const char *prop_name, int count)
{
	const u32 *cell;
	int err;

	cell = get_prop_check_min_len(blob, node, prop_name,
				      sizeof(u32) * count, &err);
	return err ? NULL : cell;
}

int fdtdec_get_bool(const void *blob, int node, const char *prop_name)
{
	const s32 *cell;
	int len;

	debug("%s: %s\n", __func__, prop_name);
	cell = fdt_getprop(blob, node, prop_name, &len);
	return cell != NULL;
}

int fdtdec_parse_phandle_with_args(const void *blob, int src_node,
				   const char *list_name,
				   const char *cells_name,
				   int cell_count, int index,
				   struct fdtdec_phandle_args *out_args)
{
	const __be32 *list, *list_end;
	int rc = 0, size, cur_index = 0;
	uint32_t count = 0;
	int node = -1;
	int phandle;

	/* Retrieve the phandle list property */
	list = fdt_getprop(blob, src_node, list_name, &size);
	if (!list)
		return -ENOENT;
	list_end = list + size / sizeof(*list);

	/* Loop over the phandles until all the requested entry is found */
	while (list < list_end) {
		rc = -EINVAL;
		count = 0;

		/*
		 * If phandle is 0, then it is an empty entry with no
		 * arguments.  Skip forward to the next entry.
		 */
		phandle = be32_to_cpup(list++);
		if (phandle) {
			/*
			 * Find the provider node and parse the #*-cells
			 * property to determine the argument length.
			 *
			 * This is not needed if the cell count is hard-coded
			 * (i.e. cells_name not set, but cell_count is set),
			 * except when we're going to return the found node
			 * below.
			 */
			if (cells_name || cur_index == index) {
				node = fdt_node_offset_by_phandle(blob,
								  phandle);
				if (node < 0) {
					debug("%s: could not find phandle\n",
					      fdt_get_name(blob, src_node,
							   NULL));
					goto err;
				}
			}

			if (cells_name) {
				count = fdtdec_get_int(blob, node, cells_name,
						       -1);
				if (count == -1) {
					debug("%s: could not get %s for %s\n",
					      fdt_get_name(blob, src_node,
							   NULL),
					      cells_name,
					      fdt_get_name(blob, node,
							   NULL));
					goto err;
				}
			} else {
				count = cell_count;
			}

			/*
			 * Make sure that the arguments actually fit in the
			 * remaining property data length
			 */
			if (list + count > list_end) {
				debug("%s: arguments longer than property\n",
				      fdt_get_name(blob, src_node, NULL));
				goto err;
			}
		}

		/*
		 * All of the error cases above bail out of the loop, so at
		 * this point, the parsing is successful. If the requested
		 * index matches, then fill the out_args structure and return,
		 * or return -ENOENT for an empty entry.
		 */
		rc = -ENOENT;
		if (cur_index == index) {
			if (!phandle)
				goto err;

			if (out_args) {
				int i;

				if (count > MAX_PHANDLE_ARGS) {
					debug("%s: too many arguments %d\n",
					      fdt_get_name(blob, src_node,
							   NULL), count);
					count = MAX_PHANDLE_ARGS;
				}
				out_args->node = node;
				out_args->args_count = count;
				for (i = 0; i < count; i++) {
					out_args->args[i] =
							be32_to_cpup(list++);
				}
			}

			/* Found it! return success */
			return 0;
		}

		node = -1;
		list += count;
		cur_index++;
	}

	/*
	 * Result will be one of:
	 * -ENOENT : index is for empty phandle
	 * -EINVAL : parsing error on data
	 * [1..n]  : Number of phandle (count mode; when index = -1)
	 */
	rc = index < 0 ? cur_index : -ENOENT;
 err:
	return rc;
}

int fdtdec_get_byte_array(const void *blob, int node, const char *prop_name,
			  u8 *array, int count)
{
	const u8 *cell;
	int err;

	cell = get_prop_check_min_len(blob, node, prop_name, count, &err);
	if (!err)
		memcpy(array, cell, count);
	return err;
}

const u8 *fdtdec_locate_byte_array(const void *blob, int node,
				   const char *prop_name, int count)
{
	const u8 *cell;
	int err;

	cell = get_prop_check_min_len(blob, node, prop_name, count, &err);
	if (err)
		return NULL;
	return cell;
}

int fdtdec_get_config_int(const void *blob, const char *prop_name,
			  int default_val)
{
	int config_node;

	debug("%s: %s\n", __func__, prop_name);
	config_node = fdt_path_offset(blob, "/config");
	if (config_node < 0)
		return default_val;
	return fdtdec_get_int(blob, config_node, prop_name, default_val);
}

int fdtdec_get_config_bool(const void *blob, const char *prop_name)
{
	int config_node;
	const void *prop;

	debug("%s: %s\n", __func__, prop_name);
	config_node = fdt_path_offset(blob, "/config");
	if (config_node < 0)
		return 0;
	prop = fdt_get_property(blob, config_node, prop_name, NULL);

	return prop != NULL;
}

char *fdtdec_get_config_string(const void *blob, const char *prop_name)
{
	const char *nodep;
	int nodeoffset;
	int len;

	debug("%s: %s\n", __func__, prop_name);
	nodeoffset = fdt_path_offset(blob, "/config");
	if (nodeoffset < 0)
		return NULL;

	nodep = fdt_getprop(blob, nodeoffset, prop_name, &len);
	if (!nodep)
		return NULL;

	return (char *)nodep;
}

u64 fdtdec_get_number(const fdt32_t *ptr, unsigned int cells)
{
	u64 number = 0;

	while (cells--)
		number = (number << 32) | fdt32_to_cpu(*ptr++);

	return number;
}

int fdt_get_resource(const void *fdt, int node, const char *property,
		     unsigned int index, struct fdt_resource *res)
{
	const fdt32_t *ptr, *end;
	int na, ns, len, parent;
	unsigned int i = 0;

	parent = fdt_parent_offset(fdt, node);
	if (parent < 0)
		return parent;

	na = fdt_address_cells(fdt, parent);
	ns = fdt_size_cells(fdt, parent);

	ptr = fdt_getprop(fdt, node, property, &len);
	if (!ptr)
		return len;

	end = ptr + len / sizeof(*ptr);

	while (ptr + na + ns <= end) {
		if (i == index) {
			if (CONFIG_IS_ENABLED(OF_TRANSLATE))
				res->start = fdt_translate_address(fdt, node, ptr);
			else
				res->start = fdtdec_get_number(ptr, na);

			res->end = res->start;
			res->end += fdtdec_get_number(&ptr[na], ns) - 1;
			return 0;
		}

		ptr += na + ns;
		i++;
	}

	return -FDT_ERR_NOTFOUND;
}

int fdt_get_named_resource(const void *fdt, int node, const char *property,
			   const char *prop_names, const char *name,
			   struct fdt_resource *res)
{
	int index;

	index = fdt_stringlist_search(fdt, node, prop_names, name);
	if (index < 0)
		return index;

	return fdt_get_resource(fdt, node, property, index, res);
}

static int decode_timing_property(const void *blob, int node, const char *name,
				  struct timing_entry *result)
{
	int length, ret = 0;
	const u32 *prop;

	prop = fdt_getprop(blob, node, name, &length);
	if (!prop) {
		debug("%s: could not find property %s\n",
		      fdt_get_name(blob, node, NULL), name);
		return length;
	}

	if (length == sizeof(u32)) {
		result->typ = fdtdec_get_int(blob, node, name, 0);
		result->min = result->typ;
		result->max = result->typ;
	} else {
		ret = fdtdec_get_int_array(blob, node, name, &result->min, 3);
	}

	return ret;
}

int fdtdec_decode_display_timing(const void *blob, int parent, int index,
				 struct display_timing *dt)
{
	int i, node, timings_node;
	u32 val = 0;
	int ret = 0;

	timings_node = fdt_subnode_offset(blob, parent, "display-timings");
	if (timings_node < 0)
		return timings_node;

	for (i = 0, node = fdt_first_subnode(blob, timings_node);
	     node > 0 && i != index;
	     node = fdt_next_subnode(blob, node))
		i++;

	if (node < 0)
		return node;

	memset(dt, 0, sizeof(*dt));

	ret |= decode_timing_property(blob, node, "hback-porch",
				      &dt->hback_porch);
	ret |= decode_timing_property(blob, node, "hfront-porch",
				      &dt->hfront_porch);
	ret |= decode_timing_property(blob, node, "hactive", &dt->hactive);
	ret |= decode_timing_property(blob, node, "hsync-len", &dt->hsync_len);
	ret |= decode_timing_property(blob, node, "vback-porch",
				      &dt->vback_porch);
	ret |= decode_timing_property(blob, node, "vfront-porch",
				      &dt->vfront_porch);
	ret |= decode_timing_property(blob, node, "vactive", &dt->vactive);
	ret |= decode_timing_property(blob, node, "vsync-len", &dt->vsync_len);
	ret |= decode_timing_property(blob, node, "clock-frequency",
				      &dt->pixelclock);

	dt->flags = 0;
	val = fdtdec_get_int(blob, node, "vsync-active", -1);
	if (val != -1) {
		dt->flags |= val ? DISPLAY_FLAGS_VSYNC_HIGH :
				DISPLAY_FLAGS_VSYNC_LOW;
	}
	val = fdtdec_get_int(blob, node, "hsync-active", -1);
	if (val != -1) {
		dt->flags |= val ? DISPLAY_FLAGS_HSYNC_HIGH :
				DISPLAY_FLAGS_HSYNC_LOW;
	}
	val = fdtdec_get_int(blob, node, "de-active", -1);
	if (val != -1) {
		dt->flags |= val ? DISPLAY_FLAGS_DE_HIGH :
				DISPLAY_FLAGS_DE_LOW;
	}
	val = fdtdec_get_int(blob, node, "pixelclk-active", -1);
	if (val != -1) {
		dt->flags |= val ? DISPLAY_FLAGS_PIXDATA_POSEDGE :
				DISPLAY_FLAGS_PIXDATA_NEGEDGE;
	}

	if (fdtdec_get_bool(blob, node, "interlaced"))
		dt->flags |= DISPLAY_FLAGS_INTERLACED;
	if (fdtdec_get_bool(blob, node, "doublescan"))
		dt->flags |= DISPLAY_FLAGS_DOUBLESCAN;
	if (fdtdec_get_bool(blob, node, "doubleclk"))
		dt->flags |= DISPLAY_FLAGS_DOUBLECLK;

	return ret;
}

static int fdtdec_init_reserved_memory(void *blob)
{
	int na, ns, node, err;
	fdt32_t value;

	/* inherit #address-cells and #size-cells from the root node */
	na = fdt_address_cells(blob, 0);
	ns = fdt_size_cells(blob, 0);

	node = fdt_add_subnode(blob, 0, "reserved-memory");
	if (node < 0)
		return node;

	err = fdt_setprop(blob, node, "ranges", NULL, 0);
	if (err < 0)
		return err;

	value = cpu_to_fdt32(ns);

	err = fdt_setprop(blob, node, "#size-cells", &value, sizeof(value));
	if (err < 0)
		return err;

	value = cpu_to_fdt32(na);

	err = fdt_setprop(blob, node, "#address-cells", &value, sizeof(value));
	if (err < 0)
		return err;

	return node;
}

int fdtdec_add_reserved_memory(void *blob, const char *basename,
			       const struct fdt_memory *carveout,
			       uint32_t *phandlep, bool no_map)
{
	fdt32_t cells[4] = {}, *ptr = cells;
	uint32_t upper, lower, phandle;
	int parent, node, na, ns, err;
	fdt_size_t size;
	char name[64];

	/* create an empty /reserved-memory node if one doesn't exist */
	parent = fdt_path_offset(blob, "/reserved-memory");
	if (parent < 0) {
		parent = fdtdec_init_reserved_memory(blob);
		if (parent < 0)
			return parent;
	}

	/* only 1 or 2 #address-cells and #size-cells are supported */
	na = fdt_address_cells(blob, parent);
	if (na < 1 || na > 2)
		return -FDT_ERR_BADNCELLS;

	ns = fdt_size_cells(blob, parent);
	if (ns < 1 || ns > 2)
		return -FDT_ERR_BADNCELLS;

	/* find a matching node and return the phandle to that */
	fdt_for_each_subnode(node, blob, parent) {
		const char *name = fdt_get_name(blob, node, NULL);
		fdt_addr_t addr;
		fdt_size_t size;

		addr = fdtdec_get_addr_size_fixed(blob, node, "reg", 0, na, ns,
						  &size, false);
		if (addr == FDT_ADDR_T_NONE) {
			debug("failed to read address/size for %s\n", name);
			continue;
		}

		if (addr == carveout->start && (addr + size - 1) ==
						carveout->end) {
			if (phandlep)
				*phandlep = fdt_get_phandle(blob, node);
			return 0;
		}
	}

	/*
	 * Unpack the start address and generate the name of the new node
	 * base on the basename and the unit-address.
	 */
	upper = upper_32_bits(carveout->start);
	lower = lower_32_bits(carveout->start);

	if (na > 1 && upper > 0)
		snprintf(name, sizeof(name), "%s@%x,%x", basename, upper,
			 lower);
	else {
		if (upper > 0) {
			debug("address %08x:%08x exceeds addressable space\n",
			      upper, lower);
			return -FDT_ERR_BADVALUE;
		}

		snprintf(name, sizeof(name), "%s@%x", basename, lower);
	}

	node = fdt_add_subnode(blob, parent, name);
	if (node < 0)
		return node;

	if (phandlep) {
		err = fdt_generate_phandle(blob, &phandle);
		if (err < 0)
			return err;

		err = fdtdec_set_phandle(blob, node, phandle);
		if (err < 0)
			return err;
	}

	/* store one or two address cells */
	if (na > 1)
		*ptr++ = cpu_to_fdt32(upper);

	*ptr++ = cpu_to_fdt32(lower);

	/* store one or two size cells */
	size = carveout->end - carveout->start + 1;
	upper = upper_32_bits(size);
	lower = lower_32_bits(size);

	if (ns > 1)
		*ptr++ = cpu_to_fdt32(upper);

	*ptr++ = cpu_to_fdt32(lower);

	err = fdt_setprop(blob, node, "reg", cells, (na + ns) * sizeof(*cells));
	if (err < 0)
		return err;

	if (no_map) {
		err = fdt_setprop(blob, node, "no-map", NULL, 0);
		if (err < 0)
			return err;
	}

	/* return the phandle for the new node for the caller to use */
	if (phandlep)
		*phandlep = phandle;

	return 0;
}

int fdtdec_get_carveout(const void *blob, const char *node, const char *name,
			unsigned int index, struct fdt_memory *carveout)
{
	const fdt32_t *prop;
	uint32_t phandle;
	int offset, len;
	fdt_size_t size;

	offset = fdt_path_offset(blob, node);
	if (offset < 0)
		return offset;

	prop = fdt_getprop(blob, offset, name, &len);
	if (!prop) {
		debug("failed to get %s for %s\n", name, node);
		return -FDT_ERR_NOTFOUND;
	}

	if ((len % sizeof(phandle)) != 0) {
		debug("invalid phandle property\n");
		return -FDT_ERR_BADPHANDLE;
	}

	if (len < (sizeof(phandle) * (index + 1))) {
		debug("invalid phandle index\n");
		return -FDT_ERR_BADPHANDLE;
	}

	phandle = fdt32_to_cpu(prop[index]);

	offset = fdt_node_offset_by_phandle(blob, phandle);
	if (offset < 0) {
		debug("failed to find node for phandle %u\n", phandle);
		return offset;
	}

	carveout->start = fdtdec_get_addr_size_auto_noparent(blob, offset,
							     "reg", 0, &size,
							     true);
	if (carveout->start == FDT_ADDR_T_NONE) {
		debug("failed to read address/size from \"reg\" property\n");
		return -FDT_ERR_NOTFOUND;
	}

	carveout->end = carveout->start + size - 1;

	return 0;
}

int fdtdec_set_carveout(void *blob, const char *node, const char *prop_name,
			unsigned int index, const char *name,
			const struct fdt_memory *carveout)
{
	uint32_t phandle;
	int err, offset, len;
	fdt32_t value;
	void *prop;

	err = fdtdec_add_reserved_memory(blob, name, carveout, &phandle, false);
	if (err < 0) {
		debug("failed to add reserved memory: %d\n", err);
		return err;
	}

	offset = fdt_path_offset(blob, node);
	if (offset < 0) {
		debug("failed to find offset for node %s: %d\n", node, offset);
		return offset;
	}

	value = cpu_to_fdt32(phandle);

	if (!fdt_getprop(blob, offset, prop_name, &len)) {
		if (len == -FDT_ERR_NOTFOUND)
			len = 0;
		else
			return len;
	}

	if ((index + 1) * sizeof(value) > len) {
		err = fdt_setprop_placeholder(blob, offset, prop_name,
					      (index + 1) * sizeof(value),
					      &prop);
		if (err < 0) {
			debug("failed to resize reserved memory property: %s\n",
			      fdt_strerror(err));
			return err;
		}
	}

	err = fdt_setprop_inplace_namelen_partial(blob, offset, prop_name,
						  strlen(prop_name),
						  index * sizeof(value),
						  &value, sizeof(value));
	if (err < 0) {
		debug("failed to update %s property for node %s: %s\n",
		      prop_name, node, fdt_strerror(err));
		return err;
	}

	return 0;
}

__weak int fdtdec_board_setup(const void *fdt_blob)
{
	return 0;
}

static void fdtdec_setup(void)
{
	_dm_fdt_blob = bsp_fdt_get();
	int ret = fdtdec_prepare_fdt();
	if (!ret) {
		ret = fdtdec_board_setup(_dm_fdt_blob);
		if (ret)
			rtems_panic("FDT board setup failed\n");
		ret = of_live_build(dm_fdt_blob(),
				(struct device_node **)&_of_root);
		if (ret)
			rtems_panic("Build of-tree failed: %d\n", ret);
	}
}

RTEMS_SYSINIT_ITEM(fdtdec_setup,
    RTEMS_SYSINIT_BSP_START,
    RTEMS_SYSINIT_ORDER_FIRST
);
