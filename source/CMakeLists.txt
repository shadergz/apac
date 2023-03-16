
add_subdirectory(user)
add_subdirectory(storage)
add_subdirectory(sched)

target_sources(apac PRIVATE 
	main_thread.c

	strplus.c
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
	conf.c
	fast_cache.c

	echo/int.c

	thread/ctx_now.c
	thread/sleep.c
	pkg/heart.c

)

