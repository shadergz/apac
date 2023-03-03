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
	dyn_loader.c
	tip.c
	locker.c

	storage/dirio.c
	storage/fio.c
	
	storage/extio/flock.c
	storage/extio/stream_mime.c

	storage/tree.c
	storage/io_native.c

	embed/user.c

	pool/spin_lock.c
	pool/sched_max.c
	pool/sched_ctrl.c
	echo/int.c
)

