#include "scalars.h"

char *
scalar_type_to_str(enum scalar_type type)
{
    switch (type) {
    case SCAL_INTEGER: return "INTEGER";
    case SCAL_BOOLEAN: return "BOOLEAN";
    }
}
