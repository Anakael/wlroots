#define _POSIX_C_SOURCE 200809L

#include <assert.h>
#include <string.h>
#include <stdlib.h>
#include <wlr/types/wlr_workspace_v1.h>
#include <wlr/types/wlr_seat.h>
#include <wlr/util/log.h>
#include "util/signal.h"

#include "wlr-workspace-unstable-v1-protocol.h"

#define WORKSPACE_V1_VERSION 1

static void workspace_handle_destroy(struct wl_client *client,
		struct wl_resource *resource) {
	// TODO
}

static void workspace_handle_activate(struct wl_client *client,
		struct wl_resource *resource) {
	// TODO
}

static void workspace_handle_deactivate(struct wl_client *client,
		struct wl_resource *resource) {
	// TODO
}

static void workspace_handle_resource_destroy(struct wl_resource *resource) {
	wl_list_remove(wl_resource_get_link(resource));
}

static void workspace_handle_send_state_to_resource(
		struct wlr_workspace_handle_v1 *workspace, struct wl_resource *resource) {
	// TODO send state
}

static const struct zwlr_workspace_handle_v1_interface workspace_handle_impl = {
	.destroy    = workspace_handle_destroy,
	.activate   = workspace_handle_activate,
	.deactivate = workspace_handle_deactivate,
};

static struct wl_resource *create_workspace_resource_for_group_resource(
		struct wlr_workspace_handle_v1 *workspace,
		struct wl_resource *group_resource) {

	struct wl_client *client = wl_resource_get_client(group_resource);
	struct wl_resource *resource = wl_resource_create(client,
			&zwlr_workspace_handle_v1_interface,
			wl_resource_get_version(group_resource), 0);
	if (!resource) {
		wl_client_post_no_memory(client);
		return NULL;
	}

	wl_resource_set_implementation(resource, &workspace_handle_impl, workspace,
			workspace_handle_resource_destroy);

	wl_list_insert(&workspace->resources, wl_resource_get_link(resource));
	zwlr_workspace_group_handle_v1_send_workspace(group_resource, resource);

	return resource;
}

static void workspace_group_resource_destroy(struct wl_resource *resource) {
	// TODO: check
	wl_list_remove(wl_resource_get_link(resource));
}

/**
 * Create the workspace group resource and child workspace resources as well.
 */
static struct wl_resource *create_workspace_group_resource_for_resource(
		struct wlr_workspace_group_handle_v1 *group,
		struct wl_resource *manager_resource) {
	struct wl_client *client = wl_resource_get_client(manager_resource);
	struct wl_resource *resource = wl_resource_create(client,
			&zwlr_workspace_group_handle_v1_interface,
			wl_resource_get_version(manager_resource), 0);
	if (!resource) {
		wl_client_post_no_memory(client);
		return NULL;
	}

	// TODO: is NULL good enough for impl, as there are no requests?
	wl_resource_set_implementation(resource, NULL, group,
			workspace_group_resource_destroy);

	wl_list_insert(&group->resources, wl_resource_get_link(resource));
	zwlr_workspace_manager_v1_send_workspace_group(manager_resource, resource);

	struct wlr_workspace_handle_v1 *tmp, *workspace;
	wl_list_for_each_safe(workspace, tmp, &group->workspaces, link) {
		// TODO create resource
		// TODO send details
	}

	return resource;
}

static void workspace_manager_commit(struct wl_client *client,
		struct wl_resource *resource) {
	// TODO
}

static void workspace_manager_stop(struct wl_client *client,
		     struct wl_resource *resource) {
	// TODO
}

static void group_send_details_to_resource(
		struct wlr_workspace_group_handle_v1 *group,
		struct wl_resource *resource) {
	// TODO send outputs
}

static const struct zwlr_workspace_manager_v1_interface
		workspace_manager_impl = {
	.commit = workspace_manager_commit,
	.stop   = workspace_manager_stop,
};

static void workspace_manager_resource_destroy( struct wl_resource *resource) {
	// TODO
}

static void workspace_manager_bind(struct wl_client *client, void *data,
		uint32_t version, uint32_t id) {
	struct wlr_workspace_manager_v1 *manager = data;
	struct wl_resource *resource = wl_resource_create(client,
			&zwlr_workspace_manager_v1_interface, version, id);
	if (!resource) {
		wl_client_post_no_memory(client);
		return;
	}
	wl_resource_set_implementation(resource, &workspace_manager_impl,
			manager, workspace_manager_resource_destroy);

	wl_list_insert(&manager->resources, wl_resource_get_link(resource));

	struct wlr_workspace_group_handle_v1 *group, *tmp;
	wl_list_for_each_safe(group, tmp, &manager->workspace_group_handles, link) {
		struct wl_resource *group_resource =
			create_workspace_group_resource_for_resource(group, resource);
		group_send_details_to_resource(group, group_resource);
	}
}

static void handle_display_destroy(struct wl_listener *listener, void *data) {
	struct wlr_workspace_manager_v1 *manager =
		wl_container_of(listener, manager, display_destroy);
	wlr_signal_emit_safe(&manager->events.destroy, manager);
	wl_list_remove(&manager->display_destroy.link);
	wl_global_destroy(manager->global);
	free(manager);
}

struct wlr_workspace_manager_v1 *wlr_workspace_manager_v1_create(
		struct wl_display *display) {

	struct wlr_workspace_manager_v1 *manager = calloc(1,
			sizeof(struct wlr_workspace_manager_v1));
	if (!manager) {
		return NULL;
	}

	manager->event_loop = wl_display_get_event_loop(display);
	manager->global = wl_global_create(display,
			&zwlr_workspace_manager_v1_interface,
			WORKSPACE_V1_VERSION, manager,
			workspace_manager_bind);
	if (!manager->global) {
		free(manager);
		return NULL;
	}

	wl_signal_init(&manager->events.destroy);
	wl_list_init(&manager->resources);
	wl_list_init(&manager->workspace_group_handles);

	manager->display_destroy.notify = handle_display_destroy;
	wl_display_add_destroy_listener(display, &manager->display_destroy);

	return manager;
}
