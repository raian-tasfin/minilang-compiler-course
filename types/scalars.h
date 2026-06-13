#ifndef SCALARS_H
#define SCALARS_H 1

enum scalar_type {
    SCAL_INTEGER,
    SCAL_BOOLEAN,
};

char *
scalar_type_to_str(enum scalar_type type);


#endif
// SCALARS_H
