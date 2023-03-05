
add_subdirectory(user)
add_subdirectory(storage)

target_sources(apac PRIVATE 
	main_thread.c 

	strhandler.c
	layer.c
	rt.c
	inner.c
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

	embed/user.c

	pool/spin_lock.c
	pool/sched_max.c
	pool/sched_ctrl.c
	echo/int.c

	thread/ctx_now.c
	thread/sleep.c
)

