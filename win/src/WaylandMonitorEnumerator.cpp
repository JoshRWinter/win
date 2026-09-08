#include <win/Win.hpp>

#ifdef WINPLAT_LINUX

#include <cstring>
#include <map>

#include <EGL/egl.h>
#include <wayland-client.h>

#include <win/WaylandMonitorEnumerator.hpp>

struct UserData
{
    std::map<wl_output *, win::Monitor> *map = NULL;
    wl_output_listener *output_listener = NULL;
};

win::WaylandMonitorEnumerator::WaylandMonitorEnumerator()
{
    init();
}

void win::WaylandMonitorEnumerator::refresh()
{
    init();
}

void win::WaylandMonitorEnumerator::init()
{
    wl_registry_listener registry_listener = { .global =
                                                   [](void *data, wl_registry *registry, uint32_t name, const char *interface, uint32_t version)
                                               {
                                                   auto &userdata = *(UserData *)data;

                                                   if (!strcmp(interface, wl_output_interface.name))
                                                   {
                                                       auto output = (wl_output *)wl_registry_bind(registry, name, &wl_output_interface, 4);
                                                       userdata.map->emplace(output, Monitor("", false, 0, 0, 0, 0, 1.0f, 60.0f));
                                                       wl_output_add_listener(output, userdata.output_listener, userdata.map);
                                                   }
                                               },
                                               .global_remove = [](void *, wl_registry *, uint32_t) {} };

    wl_output_listener output_listener = { .geometry =
                                               [](void *data,
                                                  wl_output *output,
                                                  int32_t x,
                                                  int32_t y,
                                                  int32_t physical_width,
                                                  int32_t physical_height,
                                                  int32_t subpixel,
                                                  const char *make,
                                                  const char *model,
                                                  int32_t transform)
                                           {
                                               auto &mon = ((std::map<wl_output *, Monitor> *)data)->at(output);

                                               mon.x = x;
                                               mon.y = y;
                                           },
                                           .mode =
                                               [](void *data, wl_output *output, uint32_t flags, int32_t width, int32_t height, int32_t refresh)
                                           {
                                               auto &mon = ((std::map<wl_output *, Monitor> *)data)->at(output);

                                               mon.width = width;
                                               mon.height = height;
                                               mon.rate = refresh / 1000.0f;
                                           },
                                           .done = [](void *, wl_output *) {},
                                           .scale =
                                               [](void *data, wl_output *output, int32_t factor)
                                           {
                                               auto &mon = ((std::map<wl_output *, Monitor> *)data)->at(output);
                                               mon.scale = factor;
                                           },
                                           .name =
                                               [](void *data, wl_output *output, const char *name)
                                           {
                                               auto &mon = ((std::map<wl_output *, Monitor> *)data)->at(output);
                                               mon.id = name;
                                           },
                                           .description = [](void *, wl_output *, const char *) {} };

    wl_display *display = wl_display_connect(NULL);
    if (display == NULL)
        win::bug("WaylandMonitorEnumerator: couldn't connect to display");

    std::map<wl_output *, Monitor> map;

    UserData userdata { .map = &map, .output_listener = &output_listener };

    wl_registry *registry = wl_display_get_registry(display);
    wl_registry_add_listener(registry, &registry_listener, &userdata);

    wl_display_roundtrip(display);

    eglGetDisplay(display); // gotta call this for some reason otherwise it don't work

    while (wl_display_dispatch_pending(display) != 0)
        ;

    for (const auto &mon : map)
    {
        if (mon.second.id.empty())
            continue;

        monitors.emplace_back(mon.second);
        wl_output_destroy(mon.first);
    }

    map.clear();

    wl_registry_destroy(registry);
    wl_display_disconnect(display);
}

#endif
