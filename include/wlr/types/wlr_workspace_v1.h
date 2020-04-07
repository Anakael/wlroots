/*
 * This an unstable interface of wlroots. No guarantees are made regarding the
 * future consistency of this API.
 */
#ifndef WLR_USE_UNSTABLE
#error "Add -DWLR_USE_UNSTABLE to enable unstable wlroots features"
#endif

#ifndef WLR_TYPES_WLR_WORKSPACE_V1_H
#define WLR_TYPES_WLR_WORKSPACE_V1_H

#include <wayland-server-core.h>
#include <wlr/types/wlr_output.h>

struct wlr_workspace_manager_v1 {
	struct wl_event_loop *event_loop;
	struct wl_global *global;
	struct wl_list resources; // wl_resource_get_link
	struct wl_list workspace_group_handles; // wlr_workspace_group_handle_v1::link

	struct wl_listener display_destroy;

	struct {
		struct wl_signal commit; // wlr_workspace_manager_v1
		struct wl_signal destroy;
	} events;

	void *data;
};

struct wlr_workspace_group_handle_v1 {
	struct wl_list link; // wlr_workspace_manager_v1::workspace_group_handles
	struct wl_list resources; // wl_resource_get_link

	struct wl_list workspaces; // wlr_workspace_handle_v1::link
	struct wl_list outputs; // wlr_workspace_group_handle_v1_output::link

	struct wl_event_source *idle_source;

	void *data;
};

struct wlr_workspace_group_handle_v1_output {
	struct wl_list link; // wlr_workspace_group_handle_v1::outputs
	struct wl_listener output_destroy;
	struct wlr_output *output;

	struct wlr_workspace_group_handle_v1 *workspace_group_handle;
};

enum wlr_workspace_handle_v1_state_field
{
	WLR_WORKSPACE_HANDLE_V1_STATE_ACTIVATED = 1 << 0,
};

struct wlr_workspace_handle_v1_state {
	enum wlr_workspace_handle_v1_state_field committed;
	bool activated;
};

struct wlr_workspace_handle_v1 {
	struct wl_list link; // wlr_workspace_group_handle_v1::workspaces
	struct wl_list resources;

	// request from the client
	struct wlr_workspace_handle_v1_state pending, current;

	// set by the compositor
	struct wlr_workspace_handle_v1_state server_state;

	char *name;
	struct wl_array coordinates;

	void *data;
};

struct wlr_workspace_manager_v1 *wlr_workspace_manager_v1_create(
	struct wl_display *display);

struct wlr_workspace_group_handle_v1 *wlr_workspace_group_handle_v1_create(
	struct wlr_workspace_manager_v1 *manager);

struct wlr_workspace_group_handle_v1 *wlr_workspace_handle_v1_create(
	struct wlr_workspace_group_handle_v1 *workspace_group_handle);

void wlr_workspace_group_handle_v1_output_enter(
	struct wlr_workspace_manager_v1 *manager, struct wlr_output *output);

void wlr_workspace_group_handle_v1_output_leave(
	struct wlr_workspace_manager_v1 *manager, struct wlr_output *output);

void wlr_workspace_handle_v1_set_name(struct wlr_workspace_handle_v1 *workspace,
	const char* name);

void wlr_workspace_handle_v1_set_coordinates(
	struct wlr_workspace_handle_v1 *workspace, struct wl_array coordinates);

void wlr_workspace_handle_v1_set_state(
	struct wlr_workspace_handle_v1 *workspace,
	const struct wlr_workspace_handle_v1_state state);

#endif
