/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (C) 2020 Philippe Reynes <philippe.reynes@softathome.com>
 */

#ifndef __BUTTON_H
#define __BUTTON_H


struct udevice;

/**
 * struct button_uc_plat - Platform data the uclass stores about each device
 *
 * @label:	Button label
 */
struct button_uc_plat {
	const char *label;
};

/**
 * enum button_state_t - State used for button
 * - BUTTON_OFF - Button is not pressed
 * - BUTTON_ON - Button is pressed
 * - BUTTON_COUNT - Number of button state
 */
enum button_state_t {
	BUTTON_OFF = 0,
	BUTTON_ON = 1,
	BUTTON_COUNT,
};

struct button_ops {
	/**
	 * get_state() - get the state of a button
	 *
	 * @dev:	button device to change
	 * @return button state button_state_t, or -ve on error
	 */
	enum button_state_t (*get_state)(struct udevice *dev);
};

#define button_get_ops(dev)	((struct button_ops *)(dev)->driver->ops)

/**
 * button_get_by_label() - Find a button device by label
 *
 * @label:	button label to look up
 * @devp:	Returns the associated device, if found
 * @return 0 if found, -ENODEV if not found, other -ve on error
 */
int button_get_by_label(const char *label, struct udevice **devp);

/**
 * button_get_state() - get the state of a button
 *
 * @dev:	button device to change
 * @return button state button_state_t, or -ve on error
 */
enum button_state_t button_get_state(struct udevice *dev);

#endif
