#include "cli/cli.h"
#include <stdlib.h>

int main(int argc, char ** argv)
{
    if (!vmcli_main(argc, argv)) return EXIT_FAILURE;
    return EXIT_SUCCESS;
}
