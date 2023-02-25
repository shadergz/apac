target_sources(apac PRIVATE 
    main_thread.c 
    
    user/cli.c
    
    layer.c
    rt.c
    session.c
    trace.c
    memctrlext.c
    vec_core.c
    cyclic_hw.c 
    doubly_int.c 
    backend_space.c 
    ocl_hardware.c 
    sys_object.c

    storage/dirio.c
    storage/fio.c
    storage/tree.c
    storage/io_native.c
    
    embed/user.c
    
    pool/spin_lock.c
    pool/sched_max.c
    pool/sched_ctrl.c
    echo/int.c

)

