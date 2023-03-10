#include <string.h>

#include <strhandler.h>

const char *
strhandler_search (const char *here, const char *this[])
{
  for (; *this && here; this ++)
    {
      const char *it;
      if ((it = strstr (here, *this)))
        return it;
    }
  return NULL;
}

const char *
strhandler_skip (const char *str, const char *skip)
{
  if (!str || !skip)
    return NULL;

  const char *next = strstr (str, skip);
  if (!next)
    return NULL;

  const char *ok = next + strlen (skip);

  return ok;
}
