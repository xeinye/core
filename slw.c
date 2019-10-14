/* See LICENSE file for copyright and license details. */

#include <err.h>
#include <stdio.h>
#include <stdlib.h>
#include <xcb/xcb.h>
#include <xcb/xcb_aux.h>
#include <xcb/xcb_cursor.h>

#include "util.h"

static xcb_connection_t *conn;
static xcb_screen_t *scr;

static xcb_window_t
select_window(void)
{
	uint32_t val[] = { XCB_EVENT_MASK_BUTTON_PRESS | XCB_EVENT_MASK_BUTTON_RELEASE  };
	xcb_window_t w = 0;
	xcb_grab_pointer_cookie_t c;
	xcb_grab_pointer_reply_t *r;
	xcb_generic_event_t *e;

	c = xcb_grab_pointer(conn, 0, scr->root,
		XCB_EVENT_MASK_BUTTON_PRESS | XCB_EVENT_MASK_BUTTON_RELEASE,
		XCB_GRAB_MODE_ASYNC, XCB_GRAB_MODE_ASYNC, XCB_NONE, XCB_NONE,
		XCB_CURRENT_TIME);

	r = xcb_grab_pointer_reply(conn, c, NULL);
	if (!r || r->status != XCB_GRAB_STATUS_SUCCESS)
		errx(1, "couldn't grab pointer");

	xcb_change_window_attributes(conn, scr->root, XCB_CW_EVENT_MASK, val);
	xcb_flush(conn);

	for (;;) {
		e = xcb_wait_for_event(conn);
		switch ((e->response_type & ~0x80)) {
		case XCB_BUTTON_PRESS:
			w = ((xcb_button_press_event_t*)e)->child;
			break;
		case XCB_BUTTON_RELEASE:
			return w;
			break;
		}
	}
}

int
main(int argc, char **argv)
{
	init_xcb(&conn);
	get_screen(conn, &scr);

	printf("0x%08x\n", select_window());

	kill_xcb(&conn);
	return 0;
}
