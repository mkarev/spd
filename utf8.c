// https://github.com/skeeto/scratch/tree/master/libwinsane
#include <fcntl.h>
#include <io.h>
#include <windows.h>

// https://stackoverflow.com/a/2390626
#if defined(_MSC_VER)
    #pragma section(".CRT$XCU",read)
    #define INITIALIZER2_(f,p) \
        static void f(void); \
        __declspec(allocate(".CRT$XCU")) void (*f##_)(void) = f; \
        __pragma(comment(linker,"/include:" p #f "_")) \
        static void f(void)
    #ifdef _WIN64
        #define INITIALIZER(f) INITIALIZER2_(f,"")
    #else
        #define INITIALIZER(f) INITIALIZER2_(f,"_")
    #endif
#else
    #define INITIALIZER(f) \
        static void f(void) __attribute__((constructor)); \
        static void f(void)
#endif

INITIALIZER(utf8_init)
{
    _setmode(0, _O_BINARY);
    _setmode(1, _O_BINARY);
    SetConsoleCP(CP_UTF8);  // maybe will work someday
    SetConsoleOutputCP(CP_UTF8);
}
