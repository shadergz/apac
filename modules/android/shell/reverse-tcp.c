
static void module_init() __attribute__((constructor));

static void module_init() {}

static void module_deinit() __attribute__((destructor));

static void module_deinit() {}
