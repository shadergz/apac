#include <string.h>

#include <user/line_format.h>

static const char cli_bool_true[] = "true";
static const char cli_switcher_enb[] = "enable";

bool cli_fmt_bool(const char* boovalue) {
	if (!boovalue) return false;
	
	return strncasecmp(boovalue, cli_bool_true, sizeof cli_bool_true) == 0;
}

bool cli_fmt_switcher(const char* switvalue) {
	if (switvalue == NULL) return false;

	return strncasecmp(switvalue, cli_switcher_enb, sizeof cli_switcher_enb) == 0;
}

  
