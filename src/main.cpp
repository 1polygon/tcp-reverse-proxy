#include "reverse_proxy.h"

int main()
{
    reverse_proxy proxy(8080, "127.0.0.1", 5000);
    proxy.run();
    return EXIT_SUCCESS;
}
