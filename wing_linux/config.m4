PHP_ARG_ENABLE(wing,
    [Whether to enable the "wing" extension],
    [  enable-wing        Enable "wing" extension support])

if test $PHP_WING != "no"; then
    PHP_SUBST(WING_SHARED_LIBADD)
    PHP_NEW_EXTENSION(wing, wing.c wing_hardware_info.c wing_base.c, $ext_shared)
fi
