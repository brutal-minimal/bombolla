bombolla_basewindow = shared_library('lba-basewindow', 'lba-basewindow.c',
              dependencies: bombolla_dep)

bombolla_basedrawable = shared_library('lba-basedrawable', 'lba-basedrawable.c',
              dependencies: bombolla_dep)

bombolla_base3d = shared_library('lba-base3d', 'lba-base3d.c',
                                 dependencies: bombolla_dep,
                                 link_with: bombolla_basedrawable)

bombolla_base_cogl3d = shared_library('lba-base-cogl3d', 'lba-base-cogl3d.c',
                                 dependencies: [bombolla_dep, cogl_dep],
                                 link_with: [bombolla_basedrawable, bombolla_base3d])
