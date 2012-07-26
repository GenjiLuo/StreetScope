#!/usr/bin/python
# usage: cache_route.py "starting location" "ending location" <outdir>
# route info will be saved to routes/outdir, which can be passed as an argument to street_view_sphere.

import SVMap


if __name__ == '__main__':
    svmap = SVMap.SVMap()
    svmap.set_map_dir("/home/erik/Code/streetview/python/map_data/")
    # boston area (somerville through south boston)
    svmap.cache_map(42.32657009662252, -71.12480163574219, 42.39202286040117, -71.00532531738281)

